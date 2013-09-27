/*******************************************************************
                    MPEG Video Decoding Module
 *******************************************************************/
#include <stdio.h>

#include "gop_list.h"
#include "filename.h"
#include "registry.h"

#include "idct_llm_int.h"
#include "idct_llm_mmx.h"
#include "idct_reference.h"
#include "idct_reference_sse.h"
#include "idct_ap922_int.h"
#include "idct_ap922_mmx.h"
#include "idct_ap922_sse.h"
#include "idct_ap922_sse2.h"

#define MPEG_VIDEO_C
#include "mpeg_video.h"

/* grobal */
MPEG_VIDEO *open_mpeg_video(char *path);
int close_mpeg_video(MPEG_VIDEO *p);
OUT_BUFFER_ELEMENT *read_frame(MPEG_VIDEO *in, __int64 frame);
int get_picture_coding_type(MPEG_VIDEO *in, __int64 frame);

/* local */
static OUT_BUFFER_ELEMENT *proc_sequence_header(MPEG_VIDEO *in, __int64 frame);
static OUT_BUFFER_ELEMENT *proc_gop_header(MPEG_VIDEO *in, __int64 frame);
static OUT_BUFFER_ELEMENT *proc_picture_data(MPEG_VIDEO *in, __int64 frame);

static OUT_BUFFER_ELEMENT *store_forward_reference_frame(MPEG_VIDEO *in, __int64 frame);
static OUT_BUFFER_ELEMENT *store_backward_reference_frame(MPEG_VIDEO *in, __int64 frame);
static OUT_BUFFER_ELEMENT *store_current_decoding_frame(MPEG_VIDEO *in, __int64 frame);
static OUT_BUFFER_ELEMENT *rotate_reference_frame(MPEG_VIDEO *in, __int64 frame);

static int is_seek_required(MPEG_VIDEO *p, __int64 frame);

static void sequence_header_to_decode_picture_parameter(SEQUENCE_HEADER *in, DECODE_PICTURE_PARAMETER *out);
static void picture_header_to_decode_picture_parameter(PICTURE_HEADER *in, DECODE_PICTURE_PARAMETER *out);

static void decode_2nd_field(MPEG_VIDEO *p);

static OUT_BUFFER_ELEMENT *add_frame_out_buffer_with_resize(MPEG_VIDEO *p, FRAME *data, OUTPUT_PARAMETER *prm);

static int is_registered_suffix(char *filepath);

static void clear_output_parameters(MPEG_VIDEO *p);

static void setup_m2v_config(M2V_CONFIG *p);

static void setup_chroma_upsampling_function(MPEG_VIDEO *p, int chroma_format, int simd);
static void setup_convert_function(MPEG_VIDEO *p, int chroma_format, int simd);
static void setup_qw_function(DECODE_PICTURE_PARAMETER *p, int simd);
static void setup_idct_function(DECODE_PICTURE_PARAMETER *p, M2V_CONFIG *prm);
static void setup_mc_function(DECODE_PICTURE_PARAMETER *p, M2V_CONFIG *prm);
static void setup_add_block_function(DECODE_PICTURE_PARAMETER *p, M2V_CONFIG *prm);
static void setup_field_order(MPEG_VIDEO *p, M2V_CONFIG *prm);
static void resize_parameter_to_bgr_conversion_parameter(RESIZE_PARAMETER *in, BGR_CONVERSION_PARAMETER *out);
static YUY2_CONVERT select_yuy2_convert_function(int simd);

/******************************************************************************/
MPEG_VIDEO *open_mpeg_video(char *path)
{
	int code;
	__int64 limit;
	
	GOP g;
	READ_GOP_PARAMETER *gp;
	GOP_LIST *gl;

	MPEG_VIDEO *out;

	char gl_path[FILENAME_MAX]; /* GOP LIST file path */

	/* initialize */
	out = (MPEG_VIDEO *)calloc(1, sizeof(MPEG_VIDEO)+sizeof(DECODE_PICTURE_PARAMETER)+16);
	if(out == NULL){
		return NULL;
	}

	code = (int)out;
	code += sizeof(MPEG_VIDEO)+15;
	code &= 0xfffffff0;
	out->dec_prm = (DECODE_PICTURE_PARAMETER *)code;
	
	code = 0;

	/* start file check */

	if(!is_registered_suffix(path)){
		free(out);
		return NULL;
	}

	if(!open_video_stream(path, &(out->bitstream))){
		/* can't open */
		free(out);
		return NULL;
	}

	limit = get_filecheck_limit();
	while(vs_next_start_code(&(out->bitstream))){
		code = vs_get_bits(&(out->bitstream), 32);
		if(code == 0x1B3){
			break;
		}else if(limit < video_stream_tell(&(out->bitstream))){
			/* limit over */
			break;
		}
	}

	if(code != 0x1B3){
		/* failed to find sequence_start_code */
		close_video_stream(&(out->bitstream));
		free(out);
		return 0;
	}

	if(!read_sequence_header(&(out->bitstream), &(out->seq))){
		/* has invalid sequence header */
		close_video_stream(&(out->bitstream));
		free(out);
		return 0;
	}

	sequence_header_to_read_picture_header_option(&(out->seq), &(out->pic_opt));
	sequence_header_to_decode_picture_parameter(&(out->seq), out->dec_prm);

	while(vs_next_start_code(&(out->bitstream))){
		code = vs_get_bits(&(out->bitstream), 32);
		if(code == 0x100){
			break;
		}else if(limit < video_stream_tell(&(out->bitstream))){
			/* limit over */
			break;
		}
	}
	if(code != 0x100){
		/* failed to find picture_start_code */
		close_video_stream(&(out->bitstream));
		free(out);
		return 0;
	}
	
	if(!read_picture_header(&(out->bitstream), &(out->pic), &(out->pic_opt))){
		/* has invalid picture header */
		close_video_stream(&(out->bitstream));
		free(out);
		return 0;
	}

	/* finish file check */
	
	setup_m2v_config(&(out->config));

	out->orig_field_order = out->pic.pc.top_field_first;
	
	video_stream_seek(&(out->bitstream), 0, SEEK_SET);
	
	gp = new_read_gop_parameter(&(out->bitstream), &(out->seq), &(out->pic_opt), out->orig_field_order);
	if(gp == NULL){
		/* malloc failed */
		close_video_stream(&(out->bitstream));
		free(out);
		return 0;
	}

	out->rate = gp->rate;
	out->scale = gp->scale;

	if( (out->config.gl & M2V_CONFIG_GL_ALWAYS_SCAN) == 0){
		if(read_gop(gp, &g)){
			/* gop timecode is sequential */
			out->fg.arg1 = (void *)gp;
			out->fg.func = find_gop_with_timecode;
			out->fg.release = delete_read_gop_parameter;
			
			out->total = count_frame(gp);
		}
	}

	if(out->total <= 0){
		/* gop timecode is not sequential */
		/* or GL_ALWAYS_SCAN selected     */
		delete_read_gop_parameter(gp);
		
		if(out->config.gl & M2V_CONFIG_GL_NEVER_SAVE){
			video_stream_seek(&(out->bitstream), 0, SEEK_SET);
			gl = new_gop_list(&(out->bitstream), &(out->pic_opt), out->orig_field_order);
			if(gl == NULL){
				/* stream has something probrem */
				close_video_stream(&(out->bitstream));
				return 0;
			}
		}else{
			strcpy(gl_path, path);
			cut_suffix(gl_path);
			strcat(gl_path, ".gl");

			gl = load_gop_list(gl_path);
			if( (gl == NULL) || (gl->stream_length != out->bitstream.file_length) ){
				video_stream_seek(&(out->bitstream), 0, SEEK_SET);
				gl = new_gop_list(&(out->bitstream), &(out->pic_opt), out->orig_field_order);
				if(gl == NULL){
					/* stream has something probrem */
					close_video_stream(&(out->bitstream));
					return 0;
				}
				store_gop_list(gl, gl_path);
			}
		}
		out->fg.arg1 = (void *) gl;
		out->fg.func = find_gop_with_gop_list;
		out->fg.release = delete_gop_list;

		out->total = gl->num_of_frame;
	}

	sequence_header_to_bgr_conversion_parameter(&(out->seq), &(out->bgr_prm), &(out->config));
	if(sequence_header_to_yuy2_conversion_parameter(&(out->seq), &(out->ycc_prm), &(out->config))){
		out->yuy2_cc = select_yuy2_convert_function(out->config.simd);
	}else{
		out->yuy2_cc = yuy2_convert_none;
	}

	if( (out->seq.has_sequence_extension == 0) || (out->seq.se.progressive) ){
		out->config.field_mode = M2V_CONFIG_FRAME_KEEP_ORIGINAL;
	}
	setup_field_order(out, &(out->config));

	setup_idct_function(out->dec_prm, &(out->config));
	setup_mc_function(out->dec_prm, &(out->config));
	setup_add_block_function(out->dec_prm, &(out->config));

	out->rsz_prm = create_resize_parameter(&(out->seq), &(out->config));
	resize_parameter_to_bgr_conversion_parameter(out->rsz_prm, &(out->bgr_prm));

	if(out->seq.has_sequence_extension){
		setup_chroma_upsampling_function(out, out->seq.se.chroma_format, out->config.simd);
		setup_convert_function(out, out->seq.se.chroma_format, out->config.simd);
	}else{
		setup_chroma_upsampling_function(out, 1, out->config.simd);
		setup_convert_function(out, 1, out->config.simd);
	}
	setup_qw_function(out->dec_prm, out->config.simd);

	out->width = out->bgr_prm.prm.width;
	out->height = out->bgr_prm.prm.height;

	video_stream_seek(&(out->bitstream), 0, SEEK_SET);

	out->dec_buf.forward = NULL;
	out->dec_buf.backward = NULL;
	out->dec_buf.current = NULL;

	out->current.frame_count = 0;
	out->current.start_frame = -1;

	clear_output_parameters(out);

	InitializeCriticalSection(&(out->lock));

	return out;
}
	
int close_mpeg_video(MPEG_VIDEO *p)
{
	if(p == NULL){
		return 0;
	}

	clear_out_buffer(&(p->out_buf));

	if(p->dec_buf.forward){
		delete_frame(p->dec_buf.forward);
		p->dec_buf.forward = NULL;
	}
	if(p->dec_buf.backward){
		delete_frame(p->dec_buf.backward);
		p->dec_buf.backward = NULL;
	}
	
	if(p->fg.release){
		p->fg.release(p->fg.arg1);
	}

	release_resize_parameter(p->rsz_prm);
	
	close_video_stream(&(p->bitstream));

	DeleteCriticalSection(&(p->lock));

	free(p);

	return 1;
}

OUT_BUFFER_ELEMENT *read_frame(MPEG_VIDEO *in, __int64 frame)
{
	int code;
	
	OUT_BUFFER_ELEMENT *r;
	
	EnterCriticalSection(&(in->lock));
	
	r = search_out_buffer(&(in->out_buf), frame);
	if(r){
		goto READ_FRAME_END;
	}

	if(is_seek_required(in, frame)){
		in->current = in->fg.func(in->fg.arg1, frame);
		video_stream_seek(&(in->bitstream), in->current.offset, SEEK_SET);
		if(in->dec_buf.forward){
			delete_frame(in->dec_buf.forward);
			in->dec_buf.forward = NULL;
		}
		if(in->dec_buf.backward){
			delete_frame(in->dec_buf.backward);
			in->dec_buf.backward = NULL;
		}
		
		if(in->current.frame_count == 0){
			goto READ_FRAME_END;
		}
		if(in->current.start_frame > frame){
			goto READ_FRAME_END;
		}

		clear_output_parameters(in);
		
		clear_out_buffer(&(in->out_buf));

		if(in->current.sh){
			if( (in->seq.orig_h_size != in->current.sh->orig_h_size) || (in->seq.orig_v_size != in->current.sh->orig_v_size) ){
				release_resize_parameter(in->rsz_prm);
				in->rsz_prm = create_force_resize_parameter(in->current.sh, in->width, in->height);
			}
			memcpy(&(in->seq), in->current.sh, sizeof(SEQUENCE_HEADER));
			sequence_header_to_bgr_conversion_parameter(&(in->seq), &(in->bgr_prm), &(in->config));
			if(sequence_header_to_yuy2_conversion_parameter(&(in->seq), &(in->ycc_prm), &(in->config))){
				in->yuy2_cc = select_yuy2_convert_function(in->config.simd);
			}else{
				in->yuy2_cc = yuy2_convert_none;
			}
			sequence_header_to_read_picture_header_option(&(in->seq), &(in->pic_opt));
			sequence_header_to_decode_picture_parameter(&(in->seq), in->dec_prm);
			resize_parameter_to_bgr_conversion_parameter(in->rsz_prm, &(in->bgr_prm));
		}
	}

	while(vs_next_start_code(&(in->bitstream))){
		code = vs_get_bits(&(in->bitstream), 32);

		if(code == 0x1B3){
			r = proc_sequence_header(in, frame);
		}else if(code == 0x1B8){
			r = proc_gop_header(in, frame);
		}else if(code == 0x100){
			r = proc_picture_data(in, frame);
		}
		
		if(r || (in->bwd_prm.index > frame)){
			goto READ_FRAME_END;
		}
	}
				
	if(in->dec_buf.forward){
		r = store_forward_reference_frame(in, frame);
	}
	
	if(in->dec_buf.backward){
		if(r){
			store_backward_reference_frame(in, frame);
		}else{
			r = store_backward_reference_frame(in, frame);
		}
	}

	in->fwd_prm.index = -1;
	in->bwd_prm.index = -1;

READ_FRAME_END:
	LeaveCriticalSection(&(in->lock));

	return r;
}

int get_picture_coding_type(MPEG_VIDEO *in, __int64 frame)
{
	int r;
	OUT_BUFFER_ELEMENT *e;
	
	e = read_frame(in, frame);
	if(e == NULL){
		return 0;
	}

	r = e->prm.picture_coding_type;
	if( (r == 3) && e->prm.closed_gop ){
		r = 4;
	}
	
	return r;
}

static OUT_BUFFER_ELEMENT *proc_sequence_header(MPEG_VIDEO *in, __int64 frame)
{
	int width, height;

	OUT_BUFFER_ELEMENT *fwd;
	OUT_BUFFER_ELEMENT *bwd;
	
	width = in->seq.orig_h_size;
	height = in->seq.orig_v_size;

	fwd = bwd = NULL;
	
	read_sequence_header(&(in->bitstream), &(in->seq));
	if( (width != in->seq.orig_h_size) || (height != in->seq.orig_v_size) ){
		if(in->dec_buf.forward){
			fwd =  store_forward_reference_frame(in, frame);
		}
		if(in->dec_buf.backward){
			bwd = store_backward_reference_frame(in, frame);
		}
		release_resize_parameter(in->rsz_prm);
		in->rsz_prm = create_force_resize_parameter(&(in->seq), in->width, in->height);
	}
	sequence_header_to_bgr_conversion_parameter(&(in->seq), &(in->bgr_prm), &(in->config));
	if(sequence_header_to_yuy2_conversion_parameter(&(in->seq), &(in->ycc_prm), &(in->config))){
		in->yuy2_cc = select_yuy2_convert_function(in->config.simd);
	}else{
		in->yuy2_cc = yuy2_convert_none;
	}
	sequence_header_to_read_picture_header_option(&(in->seq), &(in->pic_opt));
	sequence_header_to_decode_picture_parameter(&(in->seq), in->dec_prm);
	resize_parameter_to_bgr_conversion_parameter(in->rsz_prm, &(in->bgr_prm));

	if(fwd){
		return fwd;
	}
	return bwd;
}

static OUT_BUFFER_ELEMENT *proc_gop_header(MPEG_VIDEO *in, __int64 frame)
{
	OUT_BUFFER_ELEMENT *fwd;
	OUT_BUFFER_ELEMENT *bwd;
	
	fwd = bwd = NULL;
	
	vs_erase_bits(&(in->bitstream), 25); /* erase timecode */
	in->closed_gop = vs_get_bits(&(in->bitstream), 1);
	
	if(vs_get_bits(&(in->bitstream), 1)){ /* broken link */

		if(in->dec_buf.forward){
			fwd = store_forward_reference_frame(in, frame);
		}
	
		if(in->dec_buf.backward){
			bwd = store_backward_reference_frame(in, frame);
		}
	}

	if(fwd){
		return fwd;
	}
	return bwd;
}

static OUT_BUFFER_ELEMENT *proc_picture_data(MPEG_VIDEO *in, __int64 frame)
{
	OUT_BUFFER_ELEMENT *r;

	r = NULL;
	
	read_picture_header(&(in->bitstream), &(in->pic), &(in->pic_opt));
			
	picture_header_to_decode_picture_parameter(&(in->pic), in->dec_prm);

	in->dec_buf.current = new_frame(in->seq.h_size, in->seq.v_size);
	in->cur_prm.index = in->bwd_prm.index;

	picture_header_to_output_parameter(&(in->pic), &(in->cur_prm));
	
	if(in->pic.picture_coding_type != 3){
		r = rotate_reference_frame(in, frame);
	}
	in->cur_prm.closed_gop = in->closed_gop;

	if( in->pic.picture_coding_type == 3 ){
		if( in->dec_buf.backward == NULL) {
			delete_frame(in->dec_buf.current);
			in->dec_prm->mc_parameter.first_field = 0;
			return r;
		}else if( in->dec_buf.forward == NULL ){
			if(in->closed_gop){
				/* closed_gop error concealment */
				in->dec_buf.forward = in->dec_buf.backward;
			}else{
				delete_frame(in->dec_buf.current);
				in->dec_prm->mc_parameter.first_field = 0;
				return r;
			}
		}
	}else if((in->pic.picture_coding_type == 2) && (in->dec_buf.forward == NULL)){
		delete_frame(in->dec_buf.current);
		in->dec_prm->mc_parameter.first_field = 0;
		return r;
	}

	decode_picture(&(in->bitstream), &(in->dec_buf), in->dec_prm);

	if(in->pic.has_picture_coding_extension && in->pic.pc.picture_structure != 3){
		decode_2nd_field(in);
	}

	if(in->dec_buf.forward == in->dec_buf.backward){
		/* teardown closed_gop error concealment */
		in->dec_buf.forward = NULL;
	}

	if(in->pic.picture_coding_type == 3){
		r = store_current_decoding_frame(in, frame);
	}

	return r;
}

static OUT_BUFFER_ELEMENT *store_forward_reference_frame(MPEG_VIDEO *in, __int64 frame)
{
	delete_frame(in->dec_buf.forward);
	in->dec_buf.forward = NULL;

	return NULL;
}

static OUT_BUFFER_ELEMENT *store_backward_reference_frame(MPEG_VIDEO *in, __int64 frame)
{
	OUT_BUFFER_ELEMENT *w;
	OUT_BUFFER_ELEMENT *r;

	FRAME *rff_buf;

	r = NULL;
	
	if(in->bwd_prm.repeat_first_field && (in->bwd_prm.top_field_first == in->disp_field_order)){
		rff_buf = copy_frame(in->dec_buf.backward);
		w = add_frame_out_buffer_with_resize(in, rff_buf, &(in->bwd_prm));
		delete_frame(rff_buf);
		if(in->bwd_prm.index == frame){
			r = w;
		}
		in->bwd_prm.index += 1;
		in->bwd_prm.top_field_first = !(in->disp_field_order);
		in->bwd_prm.repeat_first_field = 0;
		in->bwd_prm.picture_coding_type = 3;
	}
	
	if((in->bwd_prm.picture_coding_type == 1) && (in->bwd_prm.index != in->current.start_frame) ){
		in->current.frame_count = in->bwd_prm.index - in->current.start_frame;
		in->current.start_frame = in->bwd_prm.index;
	}

	w = add_frame_out_buffer_with_resize(in, in->dec_buf.backward, &(in->bwd_prm));
	if(in->bwd_prm.index == frame){
		r = w;
	}
	delete_frame(in->dec_buf.backward);
	in->dec_buf.backward = NULL;

	return r;
}

static OUT_BUFFER_ELEMENT *store_current_decoding_frame(MPEG_VIDEO *in, __int64 frame)
{
	OUT_BUFFER_ELEMENT *w;
	OUT_BUFFER_ELEMENT *r;

	FRAME *rff_buf;

	r = NULL;
	
	if(in->cur_prm.repeat_first_field && (in->cur_prm.top_field_first == in->disp_field_order)){
		rff_buf = copy_frame(in->dec_buf.current);
		
		w = add_frame_out_buffer_with_resize(in, rff_buf, &(in->cur_prm));
		delete_frame(rff_buf);
		
		if(in->cur_prm.index == frame){
			r = w;
		}

		in->cur_prm.index += 1;
		in->cur_prm.top_field_first = !(in->disp_field_order);
		in->cur_prm.repeat_first_field = 0;
		in->cur_prm.picture_coding_type = 3;

		in->bwd_prm.index += 1;
	}

	w = add_frame_out_buffer_with_resize(in, in->dec_buf.current, &(in->cur_prm));
	delete_frame(in->dec_buf.current);
	
	if(in->cur_prm.index == frame){
		r = w;
	}
	
	in->bwd_prm.index += 1;

	return r;
}

static OUT_BUFFER_ELEMENT *rotate_reference_frame(MPEG_VIDEO *in, __int64 frame)
{
	OUT_BUFFER_ELEMENT *w;
	OUT_BUFFER_ELEMENT *r;
	
	FRAME *rff_buf;
	OUTPUT_PARAMETER rff_prm;

	r = NULL;
	
	if(in->dec_buf.forward){
		delete_frame(in->dec_buf.forward);
	}
	
	in->dec_buf.forward = in->dec_buf.backward;
	in->dec_buf.backward = in->dec_buf.current;
	in->fwd_prm = in->bwd_prm;
	in->bwd_prm = in->cur_prm;
	in->bwd_prm.index += 1;

	if((in->fwd_prm.picture_coding_type == 1) && (in->fwd_prm.index != in->current.start_frame) ){
		in->current.frame_count = in->fwd_prm.index - in->current.start_frame;
		in->current.start_frame = in->fwd_prm.index;
	}

	if(in->fwd_prm.repeat_first_field && (in->fwd_prm.top_field_first == in->disp_field_order) ){
		rff_buf = copy_frame(in->dec_buf.forward);

		rff_prm = in->fwd_prm;
		rff_prm.index += 1;
		rff_prm.picture_coding_type = 3;
		rff_prm.top_field_first = !(in->disp_field_order);
		rff_prm.repeat_first_field = 0;
		
		w = add_frame_out_buffer_with_resize(in, rff_buf, &rff_prm);
		delete_frame(rff_buf);
		if(rff_prm.index == frame){
			r = w;
		}

		in->bwd_prm.index += 1;
	}

	if(in->dec_buf.forward){
		w = add_frame_out_buffer_with_resize(in, in->dec_buf.forward, &(in->fwd_prm));
		if(in->fwd_prm.index == frame){
			r = w;
		}
	}

	if(in->closed_gop == 1){
		in->closed_gop = 2;
	}else{
		in->closed_gop = 0;
	}

	return r;
}

static int is_seek_required(MPEG_VIDEO *p, __int64 frame)
{
	__int64 n,m;
	
	if(p->dec_buf.backward){
		if(p->bwd_prm.index == frame){
			return 0;
		}
	}
	
	n = p->current.start_frame + p->current.frame_count;
	m = p->bwd_prm.index + 10;
	if(n<m){
		n = m;
	}
	
	if( (p->bwd_prm.index > 0) && (frame > p->bwd_prm.index) && (frame < n) ){
		return 0;
	}

	return 1;
}

static void sequence_header_to_decode_picture_parameter(SEQUENCE_HEADER *in, DECODE_PICTURE_PARAMETER *out)
{
	sequence_header_to_read_slice_header_option(in, &(out->slice_option));
	sequence_header_to_read_macroblock_option(in, &(out->macroblock_option));
	sequence_header_to_read_block_option(in, &(out->block_option));
	sequence_header_to_mc_parameter(in, &(out->mc_parameter));
}

static void picture_header_to_decode_picture_parameter(PICTURE_HEADER *in, DECODE_PICTURE_PARAMETER *out)
{
	picture_header_to_read_macroblock_option(in, &(out->macroblock_option));
	picture_header_to_read_block_option(in, &(out->block_option));
	picture_header_to_mc_parameter(in, &(out->mc_parameter));
}

static void decode_2nd_field(MPEG_VIDEO *p)
{
	int code;

	__int64 offset;
	
	while(vs_next_start_code(&(p->bitstream))){
		code = vs_read_bits(&(p->bitstream), 32);
		if(code == 0x100){
			offset = video_stream_tell(&(p->bitstream));
			vs_erase_bits(&(p->bitstream), 32);
			
			read_picture_header(&(p->bitstream), &(p->pic), &(p->pic_opt));
			
			if(p->pic.has_picture_coding_extension && (p->pic.pc.picture_structure !=3) ){
				picture_header_to_decode_picture_parameter(&(p->pic), p->dec_prm);
				
				decode_picture(&(p->bitstream), &(p->dec_buf), p->dec_prm);

			}else{
				video_stream_seek(&(p->bitstream), offset, SEEK_SET);
			}

			return;
		}else{
			return;
		}
	}
}

static OUT_BUFFER_ELEMENT *add_frame_out_buffer_with_resize(MPEG_VIDEO *p, FRAME *data, OUTPUT_PARAMETER *frame_prm)
{
	OUT_BUFFER_ELEMENT *r;
	FRAME *rsz;
	
	p->upsmp_c[frame_prm->progressive_frame](data);
	rsz = resize(data, p->rsz_prm);
	r = add_frame_out_buffer(&(p->out_buf), rsz, frame_prm);
	if(rsz != data){
		delete_frame(rsz);
	}
	return r;
}

static int is_registered_suffix(char *filepath)
{
	int i;
	
	static char *registered_suffix[] = {
		".mpeg",
		".mpg",
		".m2p",
		".mp2",
		".vob",
		".vro",
		".m2v",
		".m1v",
		".mpv",
		".ves",
		".m2t",
		".ssg",
		".ts",
		".bs",
		".m2ts",
		"", /* sentinel */
	};

	i = 0;
	while(registered_suffix[i][0]){
		if(check_suffix(filepath, registered_suffix[i])){
			return 1;
		}
		i += 1;
	}

	return 0;
}

static void setup_chroma_upsampling_function(MPEG_VIDEO *p, int chroma_format, int simd)
{
	if(chroma_format == 1){
		if(simd & M2V_CONFIG_USE_SSE2){
			p->upsmp_c[0] = upsample_chroma_420i_sse2;
			p->upsmp_c[1] = upsample_chroma_420p_sse2;
		}else if(simd & M2V_CONFIG_USE_MMX){
			p->upsmp_c[0] = upsample_chroma_420i_mmx;
			p->upsmp_c[1] = upsample_chroma_420p_mmx;
		}else{
			p->upsmp_c[0] = upsample_chroma_420i;
			p->upsmp_c[1] = upsample_chroma_420p;
		}
	}else{
		p->upsmp_c[0] = upsample_chroma_none;
		p->upsmp_c[1] = upsample_chroma_none;
	}
}

static void setup_convert_function(MPEG_VIDEO *p, int chroma_format, int simd)
{
	if(chroma_format == 3){
		p->to_bgr = yuv444_to_bgr;
		p->to_yuy2 = yuv444_to_yuy2;
	}else{
		if(simd & M2V_CONFIG_USE_SSE2){
			p->to_bgr = yuv422_to_bgr_sse2;
			p->to_yuy2 = yuv422_to_yuy2_sse2;
		}else if(simd & M2V_CONFIG_USE_MMX){
			p->to_bgr = yuv422_to_bgr_mmx;
			p->to_yuy2 = yuv422_to_yuy2_mmx;
		}else{
			p->to_bgr = yuv422_to_bgr;
			p->to_yuy2 = yuv422_to_yuy2;
		}
	}
}

static void setup_qw_function(DECODE_PICTURE_PARAMETER *p, int simd)
{
	if(simd & M2V_CONFIG_USE_SSE2){
		p->block_option.setup_qw = setup_qw_sse2;
	}else if(simd & M2V_CONFIG_USE_MMX){
		p->block_option.setup_qw = setup_qw_mmx;
	}else{
		p->block_option.setup_qw = setup_qw_nosimd;
	}
}

static void setup_idct_function(DECODE_PICTURE_PARAMETER *p, M2V_CONFIG *prm)
{
	if(prm->idct_type == M2V_CONFIG_IDCT_REFERENCE){
		if(prm->simd & M2V_CONFIG_USE_SSE){
			p->idct_func = idct_reference_sse;
		}else{
			p->idct_func = idct_reference;
		}
	}else if (prm->idct_type == M2V_CONFIG_IDCT_LLM_INT){
		if(prm->simd & M2V_CONFIG_USE_MMX){
			p->idct_func = idct_llm_mmx;
		}else{
			p->idct_func = idct_llm_int;
		}
	}else{
		if(prm->simd & M2V_CONFIG_USE_SSE2){
			p->idct_func = idct_ap922_sse2;
		}else if(prm->simd & M2V_CONFIG_USE_SSE){
			p->idct_func = idct_ap922_sse;
		}else if(prm->simd & M2V_CONFIG_USE_MMX){
			p->idct_func = idct_ap922_mmx;
		}else{
			p->idct_func = idct_ap922_int;
		}
	}
}

static void setup_mc_function(DECODE_PICTURE_PARAMETER *p, M2V_CONFIG *prm)
{
	if(prm->simd & M2V_CONFIG_USE_SSE2){
		p->mc_parameter.prediction_func = prediction_sse2;
	}else if(prm->simd & M2V_CONFIG_USE_SSE){
		p->mc_parameter.prediction_func = prediction_sse;
	}else if(prm->simd & M2V_CONFIG_USE_MMX){
		p->mc_parameter.prediction_func = prediction_mmx;
	}else{
		p->mc_parameter.prediction_func = prediction;
	}
}

static void setup_add_block_function(DECODE_PICTURE_PARAMETER *p, M2V_CONFIG *prm)
{
	if(prm->simd & M2V_CONFIG_USE_MMX){
		p->add_block_func = add_block_data_to_frame_mmx;
	}else{
		p->add_block_func = add_block_data_to_frame;
	}
}

static void clear_output_parameters(MPEG_VIDEO *p)
{
	memset(&(p->fwd_prm), 0, sizeof(OUTPUT_PARAMETER));
	memset(&(p->cur_prm), 0, sizeof(OUTPUT_PARAMETER));
	memset(&(p->bwd_prm), 0, sizeof(OUTPUT_PARAMETER));
		
	p->fwd_prm.index = p->current.start_frame - 1;
	p->bwd_prm.index = p->current.start_frame - 1;
}

static void setup_m2v_config(M2V_CONFIG *p)
{
	p->simd = get_simd_mode();
	p->bt601 = get_color_conversion_type();
	p->yuy2 = get_yuy2_mode();
	p->aspect_ratio = get_resize_mode();
	p->field_mode = get_field_mode();
	p->idct_type = get_idct_type();
	p->color_matrix = get_color_matrix();
	p->gl = get_gl_mode();
}

static void setup_field_order(MPEG_VIDEO *p, M2V_CONFIG *prm)
{
	switch(prm->field_mode){
	case M2V_CONFIG_FRAME_KEEP_ORIGINAL:
		p->disp_field_order = p->orig_field_order;
		break;
	case M2V_CONFIG_FRAME_TOP_FIRST:
		p->disp_field_order = TOP_FIELD_FIRST;
		break;
	case M2V_CONFIG_FRAME_BOTTOM_FIRST:
		p->disp_field_order = BOTTOM_FIELD_FIRST;
		break;
	default:
		p->disp_field_order = p->orig_field_order;
	}

	if(p->disp_field_order != p->orig_field_order){
		p->total -= 1;
	}
}

static void resize_parameter_to_bgr_conversion_parameter(RESIZE_PARAMETER *in, BGR_CONVERSION_PARAMETER *out)
{
	if(in == NULL){
		return;
	}

	out->prm.width = in->l.width;
	out->prm.height = in->l.height;

	out->prm.in_step = in->l.out_step;

	out->prm.c_offset = in->c.out_offset;
}

static YUY2_CONVERT select_yuy2_convert_function(int simd)
{
	YUY2_CONVERT r;

	r = yuy2_convert;
	
	if(simd & M2V_CONFIG_USE_SSE2){
		r = yuy2_convert_sse2;
	}else if(simd & M2V_CONFIG_USE_SSE){
		r = yuy2_convert_mmx;
	}else if(simd & M2V_CONFIG_USE_MMX){
		r = yuy2_convert_mmx;
	}
	
	return r;
}

