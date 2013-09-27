/*******************************************************************
                    Output Buffer Module
 *******************************************************************/
#include <stdlib.h>
#define OUT_BUFFER_C
#include "out_buffer.h"

/* global */
OUT_BUFFER_ELEMENT *add_frame_out_buffer(OUT_BUFFER *p, FRAME *data, OUTPUT_PARAMETER *prm);
OUT_BUFFER_ELEMENT *search_out_buffer(OUT_BUFFER *p, __int64 index);
void clear_out_buffer(OUT_BUFFER *p);

/* local */
static void delete_frame_out_buffer(OUT_BUFFER *p);

/* implementation */

OUT_BUFFER_ELEMENT *add_frame_out_buffer(OUT_BUFFER *p, FRAME *data, OUTPUT_PARAMETER *prm)
{
	OUT_BUFFER_ELEMENT *w, *next, *prev;

	static const int min_buffer_size = 5;
	static const int max_buffer_size = 20;

	w = (OUT_BUFFER_ELEMENT *)calloc(1, sizeof(OUT_BUFFER_ELEMENT));
	if(w == NULL){
		return NULL;
	}

	w->prm = *prm;
	w->data = data;
	data->ref_count += 1;

	if(prm->picture_coding_type == 1){ /* I */
		p->i_frame_count += 1;
	}
	p->picture_count += 1;

	prev = p->tail;
	next = NULL;

	while(prev){
		if(prev->prm.index < prm->index){ /* find instert point */
			prev->next = w;
			w->prev = prev;
			w->next = next;
			if(next == NULL){
				p->tail = w;
			}else{
				next->prev = w;
			}
			break;
		}else{
			next = prev;
			prev = prev->prev;
		}
	}

	if(prev == NULL){
		/* can not find instert point */
		if(p->tail == NULL){ /* OUT_BUFFER is empty */
			p->head = w;
			p->tail = w;
		}else{ /* insert data to head */
			w->next = p->head;
			p->head->prev = w;
			p->head = w;
		}
	}
	
	if((p->picture_count > min_buffer_size) && (p->i_frame_count > 1)){
		delete_frame_out_buffer(p);
	}

	if(p->picture_count > max_buffer_size){
		delete_frame_out_buffer(p);
	}

	return w;
}


OUT_BUFFER_ELEMENT *search_out_buffer(OUT_BUFFER *p, __int64 index)
{
	OUT_BUFFER_ELEMENT *r;

	r = p->tail;

	if(p->tail && (p->tail->prm.index < index)){
		return NULL;
	}

	if(p->head && (p->head->prm.index > index)){
		return NULL;
	}

	while(r){
		if(r->prm.index == index){
			return r;
		}else{
			r = r->prev;
		}
	}

	return NULL;
}

void clear_out_buffer(OUT_BUFFER *p)
{
	while(p->head){
		delete_frame_out_buffer(p);
	}
}

static void delete_frame_out_buffer(OUT_BUFFER *p)
{
	OUT_BUFFER_ELEMENT *w;

	w = p->head->next;

	p->picture_count -= 1;
	if(p->head->prm.picture_coding_type == 1){
		p->i_frame_count -= 1;
	}
	
	if(p->head->data){
		delete_frame(p->head->data);
	}
	
	free(p->head);

	if(w){
		p->head = w;
		p->head->prev = NULL;
	}else{
		p->head = NULL;
		p->tail = NULL;
	}
}

