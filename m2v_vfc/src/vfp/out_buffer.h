/*******************************************************************
                    Output Buffer Interface
 *******************************************************************/
#ifndef OUT_BUFFER_H
#define OUT_BUFFER_H

#include "frame.h"

typedef struct {
	__int64             index;
	int                 picture_coding_type;
	int                 repeat_first_field;
	int                 top_field_first;
	int                 progressive_frame;
	int                 closed_gop;
} OUTPUT_PARAMETER;

typedef struct {
	OUTPUT_PARAMETER    prm;
	
	FRAME              *data;

	void               *prev;
	void               *next;
} OUT_BUFFER_ELEMENT;

typedef struct {
	OUT_BUFFER_ELEMENT *head;
	OUT_BUFFER_ELEMENT *tail;
	int                 i_frame_count;
	int                 picture_count;
} OUT_BUFFER;

#ifdef __cplusplus
extern "C" {
#endif

#ifndef OUT_BUFFER_C

extern OUT_BUFFER_ELEMENT *add_frame_out_buffer(OUT_BUFFER *p, FRAME *data, OUTPUT_PARAMETER *prm);
extern OUT_BUFFER_ELEMENT *search_out_buffer(OUT_BUFFER *p, __int64 index);
extern void clear_out_buffer(OUT_BUFFER *p);

#endif /* OUT_BUFFER_C */

#ifdef __cplusplus
}
#endif

#endif /* OUT_BUFFER_H */
