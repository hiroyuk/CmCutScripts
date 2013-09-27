/*******************************************************************
                  set frame dialog procedure
 *******************************************************************/
#ifndef SET_FRAME_H
#define SET_FRAME_H

#include <windows.h>

typedef struct {
	__int64 current;
	__int64 max;
	int lang;
} FRAME_BY_NUMBER;

typedef struct {
	__int64 current;
	__int64 max;
	int rate;
	int scale;
	int lang;
} FRAME_BY_TIMECODE;

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SET_FRAME_C
extern __int64 query_frame_by_number(HINSTANCE instance, HWND hwnd, FRAME_BY_NUMBER *opt);
extern __int64 query_frame_by_timecode(HINSTANCE instance, HWND hwnd, FRAME_BY_TIMECODE *opt);
#endif

#ifdef __cplusplus
}
#endif

#endif /* SET_FRAME_H */
