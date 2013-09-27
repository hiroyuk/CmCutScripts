/*******************************************************************
 MME project interface
 *******************************************************************/
#include <stdio.h>

#define MME_PROJECT_C
#include "mme_project.h"
#include "timecode.h"
#include "message.h"

/*------------------------------------------------------------------
  global functions
 ------------------------------------------------------------------*/
int open_mme_project(const char *path, MME_PROJECT *project, VF_PluginFunc *func, VF_PluginFuncEditExt *func_edit_ext, int lang);
void close_mme_project(MME_PROJECT *project);

int output_mme_project(MME_PROJECT *project, char *out_path, HRESULT (_stdcall *callback)(char *path, DWORD percent));

const char *get_error_msg_mme_project(MME_PROJECT *project);
const char *get_status_str_mme_project(MME_PROJECT *project);

int is_open_mme_project(MME_PROJECT *project);

__int64 set_frame_index_mme_project(MME_PROJECT *project, __int64 frame_index);
__int64 get_frame_index_mme_project(MME_PROJECT *project);
__int64 go_out_point_mme_project(MME_PROJECT *project);
__int64 go_in_point_mme_project(MME_PROJECT *project);
__int64 go_start_frame_mme_project(MME_PROJECT *project);
__int64 go_end_frame_mme_project(MME_PROJECT *project);
__int64 go_next_i_frame_mme_project(MME_PROJECT *project);
__int64 go_prev_i_frame_mme_project(MME_PROJECT *project);

int set_in_point_mme_project(MME_PROJECT *project);
int set_out_point_mme_project(MME_PROJECT *project);

const VOID *get_frame_mme_project(MME_PROJECT *project);
const BITMAPINFOHEADER *get_dib_header_mme_project(MME_PROJECT *project);

int get_frame_width_mme_project(MME_PROJECT *project);
int get_frame_height_mme_project(MME_PROJECT *project);

__int64 get_frame_count_mme_project(MME_PROJECT *project);
__int64 get_in_point_mme_project(MME_PROJECT *project);
__int64 get_out_point_mme_project(MME_PROJECT *project);

/*------------------------------------------------------------------
 file local functions
 ------------------------------------------------------------------*/
static void set_status_str(MME_PROJECT *project);
static const char *read_filename(const char *path);
static void cut_suffix(char *path);

/*------------------------------------------------------------------
 file local constants
 ------------------------------------------------------------------*/
static const char PATH_DELIMITER = '\\';
static const char FILENAME_DELIMITER = '.';

/*------------------------------------------------------------------
  global functions - implementation
 ------------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
int open_mme_project(const char *path, MME_PROJECT *project, VF_PluginFunc *func, VF_PluginFuncEditExt *func_edit_ext, int lang)
{
	int size;
	VF_FileInfo file_info;

	project->func = func;
	project->func_edit_ext = func_edit_ext;
	project->lang = lang;

	strcpy(project->error_msg, msg_table[project->lang][MSG_NO_ERROR]);

	strncpy(project->path, path, sizeof(project->path)-1);

	if(project->func->OpenFile(project->path, &(project->file)) != VF_OK){
		sprintf(project->error_msg, msg_table[project->lang][MSG_FAILED_TO_OPEN_FILE], path);
		return 0;
	}

	file_info.dwSize = sizeof(VF_FileInfo);
	if(project->func->GetFileInfo(project->file, &file_info) != VF_OK){
		sprintf(project->error_msg, msg_table[project->lang][MSG_FAILED_TO_GET_FILE_INFORMATION], path);
		project->func->CloseFile(project->file);
		return 0;
	}

	if( (file_info.dwHasStreams & VF_STREAM_VIDEO) == 0 ){
		sprintf(project->error_msg, msg_table[project->lang][MSG_FILE_HAS_NO_VIDEO_STREAM], path);
		project->func->CloseFile(project->file);
		return 0;
	}

	project->video_info.dwSize = sizeof(VF_StreamInfo_Video);
	if(project->func->GetStreamInfo(project->file, VF_STREAM_VIDEO, &(project->video_info)) != VF_OK){
		sprintf(project->error_msg, msg_table[project->lang][MSG_FAILED_TO_GET_VIDEO_INFORMATION], path);
		project->func->CloseFile(project->file);
		return 0;
	}

	project->frame_index = 0;
	project->in_point = 0;
	project->out_point = project->video_info.dwLengthH;
	project->out_point <<= 32;
	project->out_point += project->video_info.dwLengthL;

	project->video_buffer.dwSize = sizeof(VF_ReadData_Video);
	project->video_buffer.lPitch = (project->video_info.dwWidth * 3 + 3) / 4 * 4;
	size = project->video_buffer.lPitch * project->video_info.dwHeight;
	
	project->dib_header = (BITMAPINFOHEADER *)malloc(sizeof(BITMAPINFOHEADER));
	if(project->dib_header == NULL){
		strcpy(project->error_msg, msg_table[project->lang][MSG_NO_ENOUGH_MEMORY]);
		project->func->CloseFile(project->file);
		return 0;
	}
	
	project->dib_header->biSize = sizeof(BITMAPINFOHEADER);
	project->dib_header->biWidth = project->video_info.dwWidth;
	project->dib_header->biHeight = project->video_info.dwHeight;
	project->dib_header->biPlanes = 1;
	project->dib_header->biBitCount = 24;
	project->dib_header->biCompression = BI_RGB;
	project->dib_header->biSizeImage = size;
	project->dib_header->biXPelsPerMeter = 2400;
	project->dib_header->biYPelsPerMeter = 2400;
	project->dib_header->biClrUsed = 0;
	project->dib_header->biClrImportant = 0;
	
	project->dib_data = malloc(size);
	if(project->dib_data == NULL){
		strcpy(project->error_msg, msg_table[project->lang][MSG_NO_ENOUGH_MEMORY]);
		free(project->dib_header);
		project->dib_header = NULL;
		project->func->CloseFile(project->file);
		return 0;
	}

	project->video_buffer.lpData = (char *)project->dib_data + (project->video_buffer.lPitch * (project->video_info.dwHeight - 1));
	project->video_buffer.lPitch = 0 - project->video_buffer.lPitch;

	set_status_str(project);
	
	return 1;
}

/*-----------------------------------------------------------------*/
void close_mme_project(MME_PROJECT *project)
{
	
	if(project->dib_data != NULL){
		free(project->dib_data);
		project->dib_data = NULL;
	}
	
	if(project->dib_header != NULL){
		free(project->dib_header);
		project->dib_header = NULL;
	}

	project->func->CloseFile(project->file);
}

/*-----------------------------------------------------------------*/
int output_mme_project(MME_PROJECT *project, char *out_path, HRESULT (_stdcall *callback)(char *path, DWORD percent))
{
	VF_CutOption opt;

	opt.dwSize = sizeof(VF_CutOption);
	opt.lpOutputFileName = out_path;
	opt.dwInPointL = (DWORD) (project->in_point & 0xffffffff);
	opt.dwInPointH = (DWORD) (project->in_point >> 32);
	opt.dwOutPointL = (DWORD) (project->out_point & 0xffffffff);
	opt.dwOutPointH = (DWORD) (project->out_point >> 32);
	opt.Callback = callback;

	if(project->func_edit_ext->Cut(project->file, &opt) == VF_ERROR){
		sprintf(project->error_msg, msg_table[project->lang][MSG_FAILED_TO_STORE_FILE], out_path);
		return 0;
	}

	return 1;
}

/*-----------------------------------------------------------------*/
const VOID *get_frame_mme_project(MME_PROJECT *project)
{
	project->video_buffer.dwFrameNumberL = (DWORD) (project->frame_index & 0xffffffff);
	project->video_buffer.dwFrameNumberH = (DWORD) (project->frame_index >> 32);

	if(project->func->ReadData(project->file, VF_STREAM_VIDEO, &(project->video_buffer)) != VF_OK){
		sprintf(project->error_msg, msg_table[project->lang][MSG_FAILED_TO_LOAD_FRAME], project->frame_index);
	}

	return project->dib_data;
}

/*-----------------------------------------------------------------*/
int is_open_mme_project(MME_PROJECT *project)
{
	if(project == NULL){
		return 0;
	}
	
	if(project->file == 0){
		return 0;
	}

	if(project->dib_header == NULL){
		return 0;
	}

	if(project->video_buffer.lpData == NULL){
		return 0;
	}

	return 1;
}

/*-----------------------------------------------------------------*/
const char *get_error_msg_mme_project(MME_PROJECT *project)
{
	return project->error_msg;
}

/*-----------------------------------------------------------------*/
const char *get_status_str_mme_project(MME_PROJECT *project)
{
	return project->status_str;
}

/*-----------------------------------------------------------------*/
__int64 set_frame_index_mme_project(MME_PROJECT *project, __int64 frame_index)
{
	__int64 max;
	
	max = project->video_info.dwLengthH;
	max <<= 32;
	max += project->video_info.dwLengthL;

	if(frame_index < 0){
		strcpy(project->error_msg, msg_table[project->lang][MSG_FIRST_FRAME]);
		frame_index = 0;
	}

	if(frame_index >= max){
		strcpy(project->error_msg, msg_table[project->lang][MSG_LAST_FRAME]);
		frame_index = max -1;
	}
		 
	project->frame_index = frame_index;

	set_status_str(project);

	return frame_index;
}

/*-----------------------------------------------------------------*/
__int64 go_out_point_mme_project(MME_PROJECT *project)
{
	return set_frame_index_mme_project(project, project->out_point-1);
}

/*-----------------------------------------------------------------*/
__int64 go_in_point_mme_project(MME_PROJECT *project)
{
	return set_frame_index_mme_project(project, project->in_point);
}

/*-----------------------------------------------------------------*/
__int64 go_start_frame_mme_project(MME_PROJECT *project)
{
	return set_frame_index_mme_project(project, 0);
}

/*-----------------------------------------------------------------*/
__int64 go_end_frame_mme_project(MME_PROJECT *project)
{
	__int64 max;

	max = project->video_info.dwLengthH;
	max <<= 32;
	max += project->video_info.dwLengthL;

	return set_frame_index_mme_project(project, max-1);
}

/*-----------------------------------------------------------------*/
__int64 go_next_i_frame_mme_project(MME_PROJECT *project)
{
	__int64 i;
	__int64 max;
	
	VF_FrameType frame_type;

	max = project->video_info.dwLengthH;
	max <<= 32;
	max += project->video_info.dwLengthL;

	frame_type.dwSize = sizeof(VF_FrameType);
	
	for(i=project->frame_index+1;i<max;i++){
		frame_type.dwFrameNumberL = (DWORD)(i & 0xffffffff);
		frame_type.dwFrameNumberH = (DWORD)(i >> 32);
		frame_type.dwFrameType = VF_FRAME_TYPE_UNEDITABLE;
		
		project->func_edit_ext->GetFrameType(project->file, &frame_type);
		if(frame_type.dwFrameType == (VF_FRAME_TYPE_STARTABLE|VF_FRAME_TYPE_ENDABLE)){
			return set_frame_index_mme_project(project, i);
		}
	}

	sprintf(project->error_msg, msg_table[project->lang][MSG_FAILED_TO_FIND_I_FRAME]);
	return project->frame_index;
}

/*-----------------------------------------------------------------*/
__int64 go_prev_i_frame_mme_project(MME_PROJECT *project)
{
	__int64 i;
	__int64 max;
	
	VF_FrameType frame_type;

	max = project->video_info.dwLengthH;
	max <<= 32;
	max += project->video_info.dwLengthL;

	frame_type.dwSize = sizeof(VF_FrameType);
	
	for(i=project->frame_index-1;i>=0;i--){
		frame_type.dwFrameNumberL = (DWORD)(i & 0xffffffff);
		frame_type.dwFrameNumberH = (DWORD)(i >> 32);
		frame_type.dwFrameType = VF_FRAME_TYPE_UNEDITABLE;
		
		project->func_edit_ext->GetFrameType(project->file, &frame_type);
		if(frame_type.dwFrameType == (VF_FRAME_TYPE_STARTABLE|VF_FRAME_TYPE_ENDABLE)){
			return set_frame_index_mme_project(project, i);
		}
	}

	sprintf(project->error_msg, msg_table[project->lang][MSG_FAILED_TO_FIND_I_FRAME]);
	return project->frame_index;
}

/*-----------------------------------------------------------------*/
int set_in_point_mme_project(MME_PROJECT *project)
{
	VF_FrameType frame_type;

	frame_type.dwSize = sizeof(VF_FrameType);
	frame_type.dwFrameNumberL = (DWORD)(project->frame_index & 0xffffffff);
	frame_type.dwFrameNumberH = (DWORD)(project->frame_index >> 32);
	frame_type.dwFrameType = VF_FRAME_TYPE_UNEDITABLE;

	project->func_edit_ext->GetFrameType(project->file, &frame_type);
	if(frame_type.dwFrameType & VF_FRAME_TYPE_STARTABLE){
		project->in_point = project->frame_index;
		set_status_str(project);
		return 1;
	}

	sprintf(project->error_msg, msg_table[project->lang][MSG_FRAME_TYPE_NOT_STARTABLE], project->frame_index);
	return 0;
}

/*-----------------------------------------------------------------*/
int set_out_point_mme_project(MME_PROJECT *project)
{
	VF_FrameType frame_type;

	frame_type.dwSize = sizeof(VF_FrameType);
	frame_type.dwFrameNumberL = (DWORD)(project->frame_index & 0xffffffff);
	frame_type.dwFrameNumberH = (DWORD)(project->frame_index >> 32);
	frame_type.dwFrameType = VF_FRAME_TYPE_UNEDITABLE;

	project->func_edit_ext->GetFrameType(project->file, &frame_type);
	if(frame_type.dwFrameType & VF_FRAME_TYPE_ENDABLE){
		project->out_point = project->frame_index+1;
		set_status_str(project);
		return 1;
	}

	sprintf(project->error_msg, msg_table[project->lang][MSG_FRAME_TYPE_NOT_FINISHABLE], project->frame_index);
	return 0;
}

/*-----------------------------------------------------------------*/
__int64 get_frame_index_mme_project(MME_PROJECT *project)
{
	return project->frame_index;
}

/*-----------------------------------------------------------------*/
const BITMAPINFOHEADER *get_dib_header_mme_project(MME_PROJECT *project)
{
	return project->dib_header;
}

/*-----------------------------------------------------------------*/
int get_frame_width_mme_project(MME_PROJECT *project)
{
	const BITMAPINFOHEADER *bi;

	bi = get_dib_header_mme_project(project);

	return bi->biWidth;
}

/*-----------------------------------------------------------------*/
int get_frame_height_mme_project(MME_PROJECT *project)
{
	const BITMAPINFOHEADER *bi;

	bi = get_dib_header_mme_project(project);

	return bi->biHeight;
}

/*-----------------------------------------------------------------*/
__int64 get_frame_count_mme_project(MME_PROJECT *project)
{
	__int64 r;

	r = project->video_info.dwLengthH;
	r <<= 32;
	r += project->video_info.dwLengthL;

	return r;
}

/*-----------------------------------------------------------------*/
__int64 get_in_point_mme_project(MME_PROJECT *project)
{
	return project->in_point;
}

/*-----------------------------------------------------------------*/
__int64 get_out_point_mme_project(MME_PROJECT *project)
{
	return project->out_point;
}

/*------------------------------------------------------------------
 file local functions - implementation
 ------------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static void set_status_str(MME_PROJECT *project)
{
	char *p;

	VF_FrameType frame_type;
	TIMECODE tc;
	int fps;
	int drop;
	
	sprintf(project->status_str, " %s", read_filename(project->path));
	cut_suffix(project->status_str);
	p = strchr(project->status_str, '\0');
	sprintf(p, " : %I64d", project->frame_index);

	fps = (project->video_info.dwRate+project->video_info.dwScale-1) / project->video_info.dwScale;
	drop = project->video_info.dwRate % project->video_info.dwScale;
	frame2timecode(&(project->frame_index), &tc, fps, drop);

	p = strchr(project->status_str, '\0');
	sprintf(p, " [%02d:%02d:%02d:%02d]", tc.hh, tc.mm, tc.ss, tc.ff);

	frame_type.dwSize = sizeof(VF_FrameType);
	frame_type.dwFrameNumberL = (DWORD)(project->frame_index & 0xffffffff);
	frame_type.dwFrameNumberH = (DWORD)(project->frame_index >> 32);
	frame_type.dwFrameType = VF_FRAME_TYPE_UNEDITABLE;

	project->func_edit_ext->GetFrameType(project->file, &frame_type);
	if(frame_type.dwFrameType == (VF_FRAME_TYPE_STARTABLE|VF_FRAME_TYPE_ENDABLE)){
		strcat(project->status_str, " : I");
	}else if(frame_type.dwFrameType & VF_FRAME_TYPE_ENDABLE){
		strcat(project->status_str, " : P");
	}else if(frame_type.dwFrameType & VF_FRAME_TYPE_STARTABLE){
		strcat(project->status_str, " : Bc");
	}else{
		strcat(project->status_str, " : B");
	}
		       
	if(project->frame_index == project->in_point){
		strcat(project->status_str, " [in]");
	}
	
	if(project->frame_index == (project->out_point-1)){
		strcat(project->status_str, " [out]");
	}
}

/*-----------------------------------------------------------------*/
static const char *read_filename(const char *path)
{
	const char *ret;

	ret = path;
	
	while(*path){
		if(*path == PATH_DELIMITER){
			ret = path + 1;
		}
		path = CharNext(path);
	}

	return ret;
}

/*-----------------------------------------------------------------*/
static void cut_suffix(char *path)
{
	char *p;

	p = NULL;

	while(*path){
		if(*path == FILENAME_DELIMITER){
			p = path;
		}
		path = CharNext(path);
	}

	if(p){
		*p = '\0';
	}
}


