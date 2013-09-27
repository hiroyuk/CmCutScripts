#ifndef TIMECODE_H
#define TIMECODE_H

typedef struct {
	int hh;
	int mm;
	int ss;
	int ff;
} TIMECODE;

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TIMECODE_C
extern void timecode2frame(TIMECODE *timecode, __int64 *frame, int rate, int drop);
extern void frame2timecode(__int64 *frame, TIMECODE *timecode, int rate, int drop);
extern void timecode2buffer(TIMECODE *timecode, char *buffer);
extern void buffer2timecode(const char *buffer, TIMECODE *timecode);
#endif

#ifdef __cplusplus
}
#endif

#endif /* TIMECODE_H */