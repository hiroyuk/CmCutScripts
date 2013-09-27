#define TIMECODE_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "timecode.h"

void timecode2frame(TIMECODE *timecode, __int64 *frame, int rate, int drop);
void frame2timecode(__int64 *frame, TIMECODE *timecode, int rate, int drop);
void timecode2buffer(TIMECODE *timecode, char *buffer);
void buffer2timecode(const char *buffer, TIMECODE *timecode);

void frame2timecode(__int64 *frame, TIMECODE *timecode, int rate, int drop)
{
	__int64 w;

	w = *frame;
	
	if( (rate == 30) && (drop) ){
		timecode->hh = (int)(w / 17982 / 6);
		timecode->mm = (int)((w / 17982 % 6 * 10) + (w % 17982 - 2) / 1798);
		timecode->ss = (int)(((w % 17982 - 2) % 1798 + 2) / 30);
		timecode->ff = (int)(((w % 17982 - 2) % 1798 + 2) % 30);
	}else{
		timecode->ff = (int)(w % rate);
		w /= rate;
		timecode->ss = (int)(w % 60);
		w /= 60;
		timecode->mm = (int)(w % 60);
		w /= 60;
		timecode->hh = (int)(w);
	}
}

void timecode2frame(TIMECODE *timecode, __int64 *frame, int rate, int drop)
{
	__int64 w,m;

	if( (rate == 30) && (drop) ){
		m = timecode->hh * 60 + timecode->mm;
		w = 17982 * (m/10) + 1798 * (m%10);
		w += timecode->ss * 30 + timecode->ff;
	}else{
		w = timecode->hh * 60 * 60 * rate;
		w += timecode->mm * 60 * rate;
		w += timecode->ss * rate;
		w += timecode->ff;
	}

	*frame = w;
}

void timecode2buffer(TIMECODE *timecode, char *buffer)
{
	sprintf(buffer, "%02d:%02d:%02d:%02d", timecode->hh, timecode->mm, timecode->ss, timecode->ff);
}

void buffer2timecode(const char *buffer, TIMECODE *timecode)
{
	int n;
	const char *p,*head;
	const char *part[4];

	n = 0;
	p = buffer;
	head = p;
	while( p[0] && (n<3) ){
		if(p[0] == ':'){
			part[n] = head;
			n += 1;
			head = p+1;
		}
		p+=1;
	}
	part[n] = head;
	n += 1;
	
	switch(n){
	case 1:
		timecode->ff = atoi(part[0]);
		break;
	case 2:
		timecode->ss = atoi(part[0]);
		timecode->ff = atoi(part[1]);
		break;
	case 3:
		timecode->mm = atoi(part[0]);
		timecode->ss = atoi(part[1]);
		timecode->ff = atoi(part[2]);
		break;
	case 4:
		timecode->hh = atoi(part[0]);
		timecode->mm = atoi(part[1]);
		timecode->ss = atoi(part[2]);
		timecode->ff = atoi(part[3]);
		break;
	default:
		memset(timecode, 0, sizeof(TIMECODE));
	}
}

