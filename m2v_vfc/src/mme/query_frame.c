/*******************************************************************
                  set frame dialog procedure
 *******************************************************************/
#define SET_FRAME_C

#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "query_frame.h"
#include "timecode.h"
#include "message.h"

#ifndef UDM_SETPOS32
#define UDM_SETPOS32 (WM_USER+113)
#endif

__int64 query_frame_by_number(HINSTANCE instance, HWND hwnd, FRAME_BY_NUMBER *opt);
__int64 query_frame_by_timecode(HINSTANCE instance, HWND hwnd, FRAME_BY_TIMECODE *opt);

static BOOL CALLBACK number_dialog_proc(HWND hwnd, UINT msg, WPARAM wprarm, LPARAM lparam);
static BOOL CALLBACK timecode_dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

static void center_dialog(HWND hwnd);

__int64 query_frame_by_number(HINSTANCE instance, HWND hwnd, FRAME_BY_NUMBER *opt)
{
	DialogBoxParam(instance, MAKEINTRESOURCE(IDD_FRAME_BY_NUMBER_DIALOG_JA+opt->lang), hwnd, number_dialog_proc, (LONG)opt);
	return opt->current;
}

__int64 query_frame_by_timecode(HINSTANCE instance, HWND hwnd, FRAME_BY_TIMECODE *opt)
{
	DialogBoxParam(instance, MAKEINTRESOURCE(IDD_FRAME_BY_TIMECODE_DIALOG_JA+opt->lang), hwnd, timecode_dialog_proc, (LONG)opt);
	return opt->current;
}

static BOOL CALLBACK number_dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	FRAME_BY_NUMBER *opt;
	char buffer[256];
	
	switch(msg){
	case WM_INITDIALOG:
		center_dialog(hwnd);
		opt = (FRAME_BY_NUMBER *)lparam;
		SendMessage(GetDlgItem(hwnd, IDC_FRAME_SPIN), UDM_SETRANGE32, 0, (opt->max & 0x7fffffff));
		sprintf(buffer, msg_table[opt->lang][MSG_MAX_FRAME_TEMPLATE], opt->max);
		SetWindowText(GetDlgItem(hwnd, IDC_MAX_FRAME), buffer);
		sprintf(buffer, "%I64d", opt->current);
		SetWindowText(GetDlgItem(hwnd, IDC_FRAME_INDEX), buffer);
		SetFocus(GetDlgItem(hwnd, IDC_FRAME_INDEX));
		SetWindowLong(hwnd, GWL_USERDATA, (LONG)opt);
		return FALSE;
	case WM_COMMAND:
		switch(LOWORD(wparam)){
		case IDOK:
			opt = (FRAME_BY_NUMBER *)GetWindowLong(hwnd, GWL_USERDATA);
			GetWindowText(GetDlgItem(hwnd, IDC_FRAME_INDEX), buffer, sizeof(buffer)-1);
			opt->current = atoi(buffer);
			EndDialog(hwnd, IDOK);
			break;
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			break;
		}
		break;
	}
	
	return FALSE;
}

static BOOL CALLBACK timecode_dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	FRAME_BY_TIMECODE *opt;
	char buffer[256];
	char tcode[64];
	
	TIMECODE tc;
	__int64 w;
	
	int rate;
	int drop;
	
	switch(msg){
	case WM_INITDIALOG:
		center_dialog(hwnd);
		
		SetWindowLong(hwnd, GWL_USERDATA, lparam);
		
		opt = (FRAME_BY_TIMECODE *)lparam;
		
		rate = (opt->rate+opt->scale-1)/opt->scale;
		drop = opt->rate%opt->scale;
		
		SendMessage(GetDlgItem(hwnd, IDC_TIMECODE_SPIN), UDM_SETRANGE32, 0, (opt->max & 0x7fffffff));
		SendMessage(GetDlgItem(hwnd, IDC_TIMECODE_SPIN), UDM_SETPOS32, 0, (opt->current & 0x7fffffff));
		
		frame2timecode(&(opt->max), &tc, rate, drop);
		timecode2buffer(&tc, tcode);
		sprintf(buffer, msg_table[opt->lang][MSG_MAX_TIMECODE_TEMPLATE], tcode);
		SetWindowText(GetDlgItem(hwnd, IDC_MAX_TIMECODE), buffer);

		frame2timecode(&(opt->current), &tc, rate, drop);
		timecode2buffer(&tc, buffer);
		SetWindowText(GetDlgItem(hwnd, IDC_TIMECODE_VIEW), buffer);
		
		return FALSE;
	case WM_COMMAND:
		opt = (FRAME_BY_TIMECODE *)GetWindowLong(hwnd, GWL_USERDATA);

		rate = (opt->rate+opt->scale-1)/opt->scale;
		drop = opt->rate%opt->scale;
		
		switch(LOWORD(wparam)){
		case IDOK:
			GetWindowText(GetDlgItem(hwnd, IDC_TIMECODE_VIEW), buffer, sizeof(buffer)-1);
			buffer2timecode(buffer, &tc);
			timecode2frame(&tc, &(opt->current), rate, drop);
			EndDialog(hwnd, IDOK);
			break;
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			break;
		case IDC_TIMECODE_VIEW:
			if(HIWORD(wparam) == EN_KILLFOCUS){
				GetWindowText(GetDlgItem(hwnd, IDC_TIMECODE_VIEW), buffer, sizeof(buffer)-1);
				buffer2timecode(buffer, &tc);
				timecode2frame(&tc, &w, rate, drop);
				SendMessage(GetDlgItem(hwnd, IDC_TIMECODE_SPIN), UDM_SETPOS32, 0, (w & 0x7fffffff));
			}
			break;
		}
		break;
	case WM_NOTIFY:
		opt = (FRAME_BY_TIMECODE *)GetWindowLong(hwnd, GWL_USERDATA);

		rate = (opt->rate+opt->scale-1)/opt->scale;
		drop = opt->rate%opt->scale;
		
		if( ((NMHDR*)lparam)->code == UDN_DELTAPOS ){
			w = ((NMUPDOWN *)lparam)->iPos + ((NMUPDOWN *)lparam)->iDelta;
			if(w>opt->max){
				w = opt->max;
			}else if(w<0){
				w = 0;
			}
			frame2timecode(&w, &tc, rate, drop);
			timecode2buffer(&tc, buffer);
			SetWindowText(GetDlgItem(hwnd, IDC_TIMECODE_VIEW), buffer);
		}
	}

	return FALSE;
}

static void center_dialog(HWND hwnd)
{
	HWND hwnd_owner; 
	RECT rc, rc_dlg, rc_owner; 
 
        if ((hwnd_owner = GetParent(hwnd)) == NULL) 
        {
            hwnd_owner = GetDesktopWindow(); 
        }

        GetWindowRect(hwnd_owner, &rc_owner); 
        GetWindowRect(hwnd, &rc_dlg); 
        CopyRect(&rc, &rc_owner); 
 
	OffsetRect(&rc_dlg, -rc_dlg.left, -rc_dlg.top); 
	OffsetRect(&rc, -rc.left, -rc.top); 
	OffsetRect(&rc, -rc_dlg.right, -rc_dlg.bottom); 
 
        SetWindowPos(hwnd, HWND_TOP, rc_owner.left + (rc.right / 2), rc_owner.top + (rc.bottom / 2), 0, 0, SWP_NOSIZE); 
}
