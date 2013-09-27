#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>

#include "resource.h"

#include "gl_dialog.h"

#define LANG_JA  0
#define LANG_EN  1

typedef struct {
	HWND dlg;
	int  lang;
	int  percent;
	int  cancel;
} GL_DIALOG_PRIVATE_DATA;

static int  update(void *gl_dialog, int percent);
static int  is_cancel(void *gl_dialog);
static void delete(void *gl_dialog);

static BOOL CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
static void center_dialog(HWND hwnd);

GL_DIALOG *create_gl_dialog()
{
	HMODULE h;
	LANGID  id;

	GL_DIALOG *r;
	GL_DIALOG_PRIVATE_DATA *data;

	const char **name;

	static const char *dll_name[] = {
		"m2v.vfp",
		"m2v.aui",
		NULL,
	};

	r = (GL_DIALOG *)malloc(sizeof(GL_DIALOG));
	if(r == NULL){
		return NULL;
	}

	r->private_data = malloc(sizeof(GL_DIALOG_PRIVATE_DATA));
	if(r->private_data == NULL){
		free(r);
		return NULL;
	}
	data = (GL_DIALOG_PRIVATE_DATA *)r->private_data;

	h = NULL;
	name = dll_name;
	while( (h == NULL) && (*name != NULL) ){
		h = GetModuleHandle(*name);
		name += 1;
	}

	id = GetUserDefaultLangID();
	if(id == 0x0411){
		data->lang = LANG_JA;
		data->dlg = CreateDialogParam(h, MAKEINTRESOURCE(IDD_GL_DIALOG_JA), NULL, dialog_proc, (LPARAM)data);
	}else{
		data->lang = LANG_EN;
		data->dlg = CreateDialogParam(h, MAKEINTRESOURCE(IDD_GL_DIALOG_EN), NULL, dialog_proc, (LPARAM)data);
	}
	data->percent = 0;
	data->cancel = 0;

	r->update = update;
	r->is_cancel = is_cancel;
	r->delete = delete;
	
	return r;
}

static int update(void *gl_dialog, int percent)
{
	MSG msg;
	HWND item;
	GL_DIALOG *dlg;
	GL_DIALOG_PRIVATE_DATA *data;

	static const char *pattern[] = {
		"GOP リストを作成中です…… %3d %%",
		"Creating GOP List .. %3d %%",
	};

	char buffer[256];

	dlg = (GL_DIALOG *)gl_dialog;
	data = (GL_DIALOG_PRIVATE_DATA *)dlg->private_data;

	// pump windows message
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if(!IsWindowVisible(data->dlg)){
		ShowWindow(data->dlg, SW_SHOW);
	}

	if(data->cancel){
		return 0;
	}
	
	if(data->percent != percent){
		sprintf(buffer, pattern[data->lang], percent);
		item = GetDlgItem(data->dlg, IDC_PERCENT_TEXT);
		SetWindowText(item, buffer);
		item = GetDlgItem(data->dlg, IDC_PROGRESS);
		PostMessage(item, PBM_SETPOS, percent, 0);
		data->percent = percent;
	}
	
	return 1;
}

static int  is_cancel(void *gl_dialog)
{
	GL_DIALOG *dlg;
	GL_DIALOG_PRIVATE_DATA *data;

	dlg = (GL_DIALOG *)gl_dialog;
	data = (GL_DIALOG_PRIVATE_DATA *)dlg->private_data;

	return data->cancel;
}

static void delete(void *gl_dialog)
{
	GL_DIALOG *dlg;
	GL_DIALOG_PRIVATE_DATA *data;

	dlg = (GL_DIALOG *)gl_dialog;
	data = (GL_DIALOG_PRIVATE_DATA *)dlg->private_data;

	DestroyWindow(data->dlg);
	free(data);
	free(dlg);
}

static BOOL CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	GL_DIALOG_PRIVATE_DATA *data;
	
	switch(msg){
	case WM_INITDIALOG:
		SetWindowLong(hwnd, GWL_USERDATA, lparam); 
		center_dialog(hwnd);
		return FALSE;
	case WM_COMMAND:
		switch(LOWORD(wparam)){
		case IDCANCEL:
			data = (GL_DIALOG_PRIVATE_DATA *)GetWindowLong(hwnd, GWL_USERDATA);
			data->cancel = 1;
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
