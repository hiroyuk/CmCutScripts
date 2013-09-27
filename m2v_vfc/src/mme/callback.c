/*******************************************************************
                  callback dialog procedure
 *******************************************************************/
#include <windows.h>

#include "resource.h"

HWND open_callback_dialog(HINSTANCE instance, HWND hwnd, int lang);

static BOOL CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
static void center_dialog(HWND hwnd);

HWND open_callback_dialog(HINSTANCE instance, HWND hwnd, int lang)
{
	HWND r;
	
	r = CreateDialog(instance, MAKEINTRESOURCE(IDD_CALLBACK_DIALOG_JA+lang), hwnd, dialog_proc);
	ShowWindow(r, SW_SHOW);
	return r;
}

static BOOL CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	
	switch(msg){
	case WM_INITDIALOG:
		center_dialog(hwnd);
		SetFocus(hwnd);
		return FALSE;
	case WM_COMMAND:
		switch(LOWORD(wparam)){
		case IDCANCEL:
			SendMessage(GetParent(hwnd), WM_CBDIALOG_CANCEL, 0, 0);
			DestroyWindow(hwnd);
			break;
		}
		break;
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
