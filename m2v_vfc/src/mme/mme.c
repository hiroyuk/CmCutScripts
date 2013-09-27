/*******************************************************************
                      MARUMO MPEG-2 EDITOR
 *******************************************************************/
#include <limits.h>
#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include "resource.h"
#include "m2v_vfapi.h"
#include "mme_project.h"
#include "callback.h"
#include "query_frame.h"
#include "message.h"

#define MAIN_WINDOW_WIDTH  328
#define MAIN_WINDOW_HEIGHT 286

/*******************************************************************
   type definition
 *******************************************************************/
typedef struct {
	HINSTANCE        instance;
	HWND             main_hwnd;
	HWND             callback_dlg;
	HWND             slide_bar;
	HACCEL           accel_tbl;
	M2V_VFAPI        vfapi;
	MME_PROJECT      project;
	HANDLE           clipboard;
	int              lang;
	char             openpath[1024];
	char             savepath[1024];
} MME_APP;
#define MME_LANG_JA 0
#define MME_LANG_EN 1

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

/*******************************************************************
   menu command procedures
 *******************************************************************/
static void on_file_open();
static void on_file_close();
static void on_file_output();
static void on_file_benchmark();
static void on_file_goplist();
static void on_file_end();
static void on_edit_in();
static void on_edit_out();
static void on_edit_undo();
static void on_edit_copy();
static void on_move_frame();
static void on_move_timecode();
static void on_move_forward();
static void on_move_back();
static void on_move_next_i();
static void on_move_prev_i();
static void on_move_in();
static void on_move_out();
static void on_move_start();
static void on_move_end();
static void on_help_about();

/*******************************************************************
   other windows message procedure
 *******************************************************************/
static void on_destory();
static void on_paint();
static void on_cbdialog_cancel();
static void on_h_scroll(int code, HWND hwnd);
static void on_dropfiles(HANDLE hdrop);

/*------------------------------------------------------------------
 file local functions
 ------------------------------------------------------------------*/
static void enable_menu_from_list(const UINT *list, int count);
static void disable_menu_from_list(const UINT *list, int count);
static void showdib(HDC hdc, const BITMAPINFOHEADER *header, const VOID *data);
static void resize(int client_width, int client_height);
static void update_slider();
static HRESULT _stdcall callback(char *path, DWORD percent);
static void msg_pump();
static void show_error(char *msg, DWORD code);
static void repaint();
static void open_file(char *path);
static void parse_cmdline(char *arg);
static void get_lastpath();
static void put_openpath(char *path);
static void put_savepath(char *path);
static void cut_filename(char *path);
static int has_suffix(char *path);
static DWORD get_goplist_mode();
static void put_goplist_mode(DWORD gl);
static void create_goplist(M2V_VFAPI *vfapi, char *path);

/*******************************************************************
   global val
 *******************************************************************/
WNDCLASS main_window = {
	CS_HREDRAW | CS_VREDRAW,
	WndProc,
	0,
	0,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	"MME_MAIN_WINDOW",
};

MME_APP mme_app;

/*******************************************************************
 constants
 *******************************************************************/
const char APP_NAME[] = "MME ver. 0.1.21";
const UINT SWITCHING_MENU_ID[] = {
	ID_FILE_CLOSE,
	ID_FILE_OUTPUT,
	ID_FILE_BENCHMARK,
	ID_EDIT_IN,
	ID_EDIT_OUT,
	ID_EDIT_COPY,
	ID_MOVE_FRAME,
	ID_MOVE_TIMECODE,
	ID_MOVE_FORWARD,
	ID_MOVE_BACK,
	ID_MOVE_NEXT_I,
	ID_MOVE_PREVIOUS_I,
	ID_MOVE_IN,
	ID_MOVE_OUT,
	ID_MOVE_START,
	ID_MOVE_END,
};

/*-----------------------------------------------------------------*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR lpszCmdLine, int nCmdShow)
{
	MSG  msg;
	RECT rect;
	int  width;
	int  height;

	memset(&mme_app, 0, sizeof(MME_APP));

	if(GetUserDefaultLangID() == 0x0411){
		mme_app.lang = MME_LANG_JA;
		main_window.lpszMenuName = "MME_MAIN_MENU_JA";
	}else{
		mme_app.lang = MME_LANG_EN;
		main_window.lpszMenuName = "MME_MAIN_MENU_EN";
	}
	
	if(!open_m2v_vfapi(&(mme_app.vfapi))){
		MessageBox(0, "failed to find \"m2v.vfp\" VFAPI Plugin", "ERROR", MB_OK|MB_ICONERROR);
		exit(EXIT_FAILURE);
	}
	
	if (!hPreInst) {
		main_window.hInstance =hInstance;
		main_window.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
		main_window.hCursor =LoadCursor(NULL, IDC_ARROW);
		main_window.hbrBackground =GetStockObject(WHITE_BRUSH);
		if (!RegisterClass(&main_window)){
			return FALSE;
		}
	}

	mme_app.instance = hInstance;
	
	mme_app.accel_tbl = LoadAccelerators(hInstance, "MME_MAIN_ACCELERATOR");
	
	InitCommonControls();
	
	mme_app.main_hwnd = CreateWindow(main_window.lpszClassName, APP_NAME, WS_OVERLAPPED|WS_MINIMIZEBOX|WS_SYSMENU|WS_CAPTION, CW_USEDEFAULT, CW_USEDEFAULT, MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);
	GetClientRect(mme_app.main_hwnd, &rect);
	width = rect.right-rect.left;
	height = rect.bottom-rect.top;
	
	mme_app.slide_bar = CreateWindowEx(0, TRACKBAR_CLASS, "MME_SLIDE_BAR", WS_CHILD|WS_VISIBLE|TBS_NOTICKS|TBS_ENABLESELRANGE, 0, height-24, width, 24, mme_app.main_hwnd, (HMENU)IDC_SLIDE_BAR, mme_app.instance, NULL);
	SendMessage(mme_app.slide_bar, TBM_SETRANGE, (WPARAM) TRUE, (LPARAM) MAKELONG(0, 100));
	SendMessage(mme_app.slide_bar, TBM_SETSEL, (WPARAM) TRUE, (LPARAM) MAKELONG(0, 100));
	EnableWindow(mme_app.slide_bar, FALSE);
	
	ShowWindow(mme_app.main_hwnd, nCmdShow);
	UpdateWindow(mme_app.main_hwnd);
	
	DragAcceptFiles(mme_app.main_hwnd, TRUE);

	parse_cmdline(lpszCmdLine);

	get_lastpath();
	
	while (GetMessage(&msg, NULL, 0, 0)) {
		if(!TranslateAccelerator(mme_app.main_hwnd, mme_app.accel_tbl, &msg)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}

/*-----------------------------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_FILE_OPEN:
			on_file_open();
			break;
		case ID_FILE_CLOSE:
			on_file_close();
			break;
		case ID_FILE_OUTPUT:
			on_file_output();
			break;
		case ID_FILE_BENCHMARK:
			on_file_benchmark();
			break;
		case ID_FILE_GOPLIST:
			on_file_goplist();
			break;
		case ID_FILE_END:
			on_file_end();
			break;
		case ID_EDIT_IN:
			on_edit_in();
			break;
		case ID_EDIT_OUT:
			on_edit_out();
			break;
		case ID_EDIT_UNDO:
			on_edit_undo();
			break;
		case ID_EDIT_COPY:
			on_edit_copy();
			break;
		case ID_MOVE_FRAME:
			on_move_frame();
			break;
		case ID_MOVE_TIMECODE:
			on_move_timecode();
			break;
		case ID_MOVE_FORWARD:
			on_move_forward();
			break;
		case ID_MOVE_BACK:
			on_move_back();
			break;
		case ID_MOVE_NEXT_I:
			on_move_next_i();
			break;
		case ID_MOVE_PREVIOUS_I:
			on_move_prev_i();
			break;
		case ID_MOVE_IN:
			on_move_in();
			break;
		case ID_MOVE_OUT:
			on_move_out();
			break;
		case ID_MOVE_START:
			on_move_start();
			break;
		case ID_MOVE_END:
			on_move_end();
			break;
		case ID_HELP_ABOUT:
			on_help_about();
			break;
		}
		break;
	case WM_DESTROY:
		on_destory();
		break;
	case WM_PAINT:
		on_paint();
		return (1L);
	case WM_CBDIALOG_CANCEL:
		on_cbdialog_cancel();
		break;
	case WM_HSCROLL:
		on_h_scroll(LOWORD(wParam), (HWND)lParam);
		break;
	case WM_DROPFILES:
		on_dropfiles((HANDLE)wParam);
		break;
	default:
		return(DefWindowProc(hWnd, msg, wParam, lParam));
	}

	return (0L);
}
 
/*-----------------------------------------------------------------*/
static void on_file_open()
{
	OPENFILENAME of;
	char path[1024];
	
	/* initialize */
	memset(&of, 0, sizeof(OPENFILENAME));
	memset(path, 0, sizeof(path));
	of.lStructSize = sizeof(OPENFILENAME);
	of.hwndOwner = mme_app.main_hwnd;
	of.lpstrFile = path;
	of.nMaxFile = sizeof(path);
	of.lpstrFilter = mme_app.vfapi.info.cFileType;
	of.nFilterIndex = 0;
	of.lpstrFileTitle = NULL;
	of.nMaxFileTitle = 0;
	of.lpstrInitialDir = mme_app.openpath;
	of.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if(GetOpenFileName(&of)){
		put_openpath(of.lpstrFile);
		open_file(of.lpstrFile);
	}

	return;
}

/*-----------------------------------------------------------------*/
static void on_file_close()
{
	if(is_open_mme_project(&(mme_app.project))){
		close_mme_project(&(mme_app.project));
		disable_menu_from_list(SWITCHING_MENU_ID, sizeof(SWITCHING_MENU_ID)/sizeof(UINT));
		InvalidateRgn(mme_app.main_hwnd, NULL, TRUE); /* repaint */
		SetWindowText(mme_app.main_hwnd, APP_NAME);
		SendMessage(mme_app.slide_bar, TBM_SETRANGE, (WPARAM) TRUE, (LPARAM) MAKELONG(0, 100));
		SendMessage(mme_app.slide_bar, TBM_SETSEL, (WPARAM) TRUE, (LPARAM) MAKELONG(0, 100));
		EnableWindow(mme_app.slide_bar, FALSE);
	}
	
	return;
}

/*-----------------------------------------------------------------*/
static void on_file_output()
{
	OPENFILENAME of;
	char path[260];
	
	/* initialize */
	memset(&of, 0, sizeof(OPENFILENAME));
	memset(path, 0, sizeof(path));
	of.lStructSize = sizeof(OPENFILENAME);
	of.hwndOwner = mme_app.main_hwnd;
	of.lpstrFile = path;
	of.nMaxFile = sizeof(path);
	of.lpstrFilter = mme_app.vfapi.info.cFileType;
	of.nFilterIndex = 0;
	of.lpstrFileTitle = NULL;
	of.nMaxFileTitle = 0;
	of.lpstrInitialDir = mme_app.savepath;
	of.Flags = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

	if(GetSaveFileName(&of)){
		
		if(!has_suffix(of.lpstrFile)){
			strcat(of.lpstrFile, ".mpg");
		}
		
		put_savepath(of.lpstrFile);
		
		mme_app.callback_dlg = open_callback_dialog(mme_app.instance, mme_app.main_hwnd, mme_app.lang);
		SetActiveWindow(mme_app.callback_dlg);
		ShowWindow(mme_app.main_hwnd, FALSE);
		if(! output_mme_project(&(mme_app.project), path, callback)){
			MessageBox(mme_app.main_hwnd, get_error_msg_mme_project(&(mme_app.project)), "INFO", MB_OK|MB_ICONINFORMATION);
		}
		DestroyWindow(mme_app.callback_dlg);
		mme_app.callback_dlg = (HWND)INVALID_HANDLE_VALUE;
		ShowWindow(mme_app.main_hwnd, TRUE);
	}
	
	return;
}

/*-----------------------------------------------------------------*/
static void on_file_benchmark()
{
	__int64 i;
	int msec;
	int start;
	__int64 fps;
	char buffer[1024];

	EnableWindow(mme_app.main_hwnd, FALSE);
	
	start = GetTickCount();

	for(i=mme_app.project.in_point;i<mme_app.project.out_point+1;i++){
		set_frame_index_mme_project(&(mme_app.project), i);
		update_slider();
		SetWindowText(mme_app.main_hwnd, get_status_str_mme_project(&(mme_app.project)));
		InvalidateRgn(mme_app.main_hwnd, NULL, FALSE); /* repaint */
		msg_pump();
	}

	i = mme_app.project.out_point - mme_app.project.in_point;
	msec = GetTickCount() - start;
	fps = i * 1000 * 1000 / msec; 
	sprintf(buffer, "frame %I64d, time %d msec, fps %d.%03d", i, msec, (int)fps/1000, (int)fps%1000);
	MessageBox(mme_app.main_hwnd, buffer, "INFO", MB_ICONINFORMATION|MB_OK);
	
	EnableWindow(mme_app.main_hwnd, TRUE);
	
	return;
}

/*-----------------------------------------------------------------*/
static void on_file_goplist()
{
	OPENFILENAME of;
	char path[32768];
	char work[1024];
	char *head;
	int  n;

	DWORD gl_mode;
	
	/* initialize */
	memset(&of, 0, sizeof(OPENFILENAME));
	memset(path, 0, sizeof(path));
	of.lStructSize = sizeof(OPENFILENAME);
	of.hwndOwner = mme_app.main_hwnd;
	of.lpstrFile = path;
	of.nMaxFile = sizeof(path);
	of.lpstrFilter = mme_app.vfapi.info.cFileType;
	of.lpstrInitialDir = mme_app.openpath;
	of.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if(GetOpenFileName(&of) == 0){
		return;
	}

	put_openpath(of.lpstrFile);
	
	gl_mode = get_goplist_mode();
	put_goplist_mode(2); // NEVER_USE_TIMECODE + SAVE_GL_FILE
	
	head = path + of.nFileOffset;
	while( head[0] != '\0' ){
		memcpy(work, path, of.nFileOffset);
		n = strlen(head) + 1;
		if(work[of.nFileOffset-1] == '\0'){
			work[of.nFileOffset-1] = '\\';
		}
		memcpy(work+of.nFileOffset, head, n);
		head += n;
		create_goplist(&(mme_app.vfapi), work);
	}

	put_goplist_mode(gl_mode);
	
	return;
}

/*-----------------------------------------------------------------*/
static void on_file_end()
{
	SendMessage(mme_app.main_hwnd, WM_CLOSE, 0, 0L);
	return;
}

/*-----------------------------------------------------------------*/
static void on_edit_in()
{
	if(!set_in_point_mme_project(&(mme_app.project))){
		MessageBox(mme_app.main_hwnd, get_error_msg_mme_project(&(mme_app.project)), "INFO", MB_OK|MB_ICONINFORMATION);
	}
	if(get_in_point_mme_project(&(mme_app.project)) > LONG_MAX){
		SendMessage(mme_app.slide_bar, TBM_SETSELSTART, TRUE, LONG_MAX);
	}else{
		SendMessage(mme_app.slide_bar, TBM_SETSELSTART, TRUE, (LONG)get_in_point_mme_project(&(mme_app.project)));
	}	
	SetWindowText(mme_app.main_hwnd, get_status_str_mme_project(&(mme_app.project)));
	return;
}

/*-----------------------------------------------------------------*/
static void on_edit_out()
{
	if(!set_out_point_mme_project(&(mme_app.project))){
		MessageBox(mme_app.main_hwnd, get_error_msg_mme_project(&(mme_app.project)), "INFO", MB_OK|MB_ICONINFORMATION);
	}
	if(get_out_point_mme_project(&(mme_app.project)) > LONG_MAX){
		SendMessage(mme_app.slide_bar, TBM_SETSELEND, TRUE, LONG_MAX);
	}else{
		SendMessage(mme_app.slide_bar, TBM_SETSELEND, TRUE, (LONG)get_out_point_mme_project(&(mme_app.project)));
	}	
	SetWindowText(mme_app.main_hwnd, get_status_str_mme_project(&(mme_app.project)));
	return;
}

/*-----------------------------------------------------------------*/
static void on_edit_undo()
{
	MessageBox(mme_app.main_hwnd, "編集 -> やりなおしメニューが選択されました", "デバッグ", MB_OK|MB_ICONINFORMATION);
	return;
}

/*-----------------------------------------------------------------*/
static void on_edit_copy()
{
	char *p;
	const BITMAPINFOHEADER *bih;
	
	if(!OpenClipboard(mme_app.main_hwnd)){
		show_error(msg_table[mme_app.lang][MSG_FAILED_TO_OPEN_CLIPBOARD], GetLastError());
		return;
	}

	if(!EmptyClipboard()){
		CloseClipboard();
		show_error(msg_table[mme_app.lang][MSG_FAILED_TO_GET_CLIPBOARD_OWNERSHIP], GetLastError());
		return;
	}

	bih = get_dib_header_mme_project(&(mme_app.project));
	if(mme_app.clipboard != NULL){
		GlobalFree(mme_app.clipboard);
	}
	mme_app.clipboard = GlobalAlloc(GMEM_MOVEABLE, bih->biSize + bih->biSizeImage);
	if(mme_app.clipboard == NULL){
		CloseClipboard();
		show_error(msg_table[mme_app.lang][MSG_NO_ENOUGH_MEMORY], GetLastError());
		return;
	}		
	p = (char *)GlobalLock(mme_app.clipboard);

	memcpy(p, bih, bih->biSize);
	memcpy(p+bih->biSize, get_frame_mme_project(&(mme_app.project)), bih->biSizeImage);

	GlobalUnlock(mme_app.clipboard);

	SetClipboardData(CF_DIB, mme_app.clipboard);

	CloseClipboard();
	
	return;
}

/*-----------------------------------------------------------------*/
static void on_move_frame()
{
	__int64 index;

	FRAME_BY_NUMBER prm;

	prm.current = get_frame_index_mme_project(&(mme_app.project));
	prm.max = get_frame_count_mme_project(&(mme_app.project))-1;
	prm.lang = mme_app.lang;
	index = query_frame_by_number(mme_app.instance, mme_app.main_hwnd, &prm);

	if(index != set_frame_index_mme_project(&(mme_app.project), index)){
		MessageBox(mme_app.main_hwnd, get_error_msg_mme_project(&(mme_app.project)), "INFO", MB_OK|MB_ICONINFORMATION);
	}
	
	update_slider();
	SetWindowText(mme_app.main_hwnd, get_status_str_mme_project(&(mme_app.project)));
	InvalidateRgn(mme_app.main_hwnd, NULL, FALSE); /* repaint */
	
	return;
}

/*-----------------------------------------------------------------*/
static void on_move_timecode()
{
	__int64 index;

	FRAME_BY_TIMECODE prm;

	prm.current = get_frame_index_mme_project(&(mme_app.project));
	prm.max = get_frame_count_mme_project(&(mme_app.project))-1;
	prm.lang = mme_app.lang;
	prm.rate = mme_app.project.video_info.dwRate;
	prm.scale = mme_app.project.video_info.dwScale;
	
	index = query_frame_by_timecode(mme_app.instance, mme_app.main_hwnd, &prm);

	if(index != set_frame_index_mme_project(&(mme_app.project), index)){
		MessageBox(mme_app.main_hwnd, get_error_msg_mme_project(&(mme_app.project)), "INFO", MB_OK|MB_ICONINFORMATION);
	}
	
	update_slider();
	SetWindowText(mme_app.main_hwnd, get_status_str_mme_project(&(mme_app.project)));
	InvalidateRgn(mme_app.main_hwnd, NULL, FALSE); /* repaint */
	
	return;
}

/*-----------------------------------------------------------------*/
static void on_move_forward()
{
	__int64 index;

	index = get_frame_index_mme_project(&(mme_app.project));
	if(index == set_frame_index_mme_project(&(mme_app.project), index+1)){
		MessageBox(mme_app.main_hwnd, get_error_msg_mme_project(&(mme_app.project)), "INFO", MB_OK|MB_ICONINFORMATION);
	}
	
	update_slider();
	SetWindowText(mme_app.main_hwnd, get_status_str_mme_project(&(mme_app.project)));

	repaint();
	
	return;
}

/*-----------------------------------------------------------------*/
static void on_move_back()
{
	__int64 index;

	index = get_frame_index_mme_project(&(mme_app.project));
	if(index == set_frame_index_mme_project(&(mme_app.project), index-1)){
		MessageBox(mme_app.main_hwnd, get_error_msg_mme_project(&(mme_app.project)), "INFO", MB_OK|MB_ICONINFORMATION);
	}
	
	update_slider();
	SetWindowText(mme_app.main_hwnd, get_status_str_mme_project(&(mme_app.project)));
	repaint();
	
	return;
}

/*-----------------------------------------------------------------*/
static void on_move_next_i()
{
	__int64 index;

	index = get_frame_index_mme_project(&(mme_app.project));
	if(index == go_next_i_frame_mme_project(&(mme_app.project))){
		MessageBox(mme_app.main_hwnd, get_error_msg_mme_project(&(mme_app.project)), "INFO", MB_OK|MB_ICONINFORMATION);
	}
	
	update_slider();
	SetWindowText(mme_app.main_hwnd, get_status_str_mme_project(&(mme_app.project)));
	repaint();
	
	return;
}

/*-----------------------------------------------------------------*/
static void on_move_prev_i()
{
	__int64 index;

	index = get_frame_index_mme_project(&(mme_app.project));
	if(index == go_prev_i_frame_mme_project(&(mme_app.project))){
		MessageBox(mme_app.main_hwnd, get_error_msg_mme_project(&(mme_app.project)), "INFO", MB_OK|MB_ICONINFORMATION);
	}
	
	update_slider();
	SetWindowText(mme_app.main_hwnd, get_status_str_mme_project(&(mme_app.project)));
	repaint();
	
	return;
}

/*-----------------------------------------------------------------*/
static void on_move_in()
{
	go_in_point_mme_project(&(mme_app.project));
	update_slider();
	SetWindowText(mme_app.main_hwnd, get_status_str_mme_project(&(mme_app.project)));
	repaint();
	
	return;
}

/*-----------------------------------------------------------------*/
static void on_move_out()
{
	go_out_point_mme_project(&(mme_app.project));
	update_slider();
	SetWindowText(mme_app.main_hwnd, get_status_str_mme_project(&(mme_app.project)));
	repaint();
	
	return;
}

/*-----------------------------------------------------------------*/
static void on_move_start()
{
	go_start_frame_mme_project(&(mme_app.project));
	update_slider();
	SetWindowText(mme_app.main_hwnd, get_status_str_mme_project(&(mme_app.project)));
	repaint();
	
	return;
}

/*-----------------------------------------------------------------*/
static void on_move_end()
{
	go_end_frame_mme_project(&(mme_app.project));
	update_slider();
	SetWindowText(mme_app.main_hwnd, get_status_str_mme_project(&(mme_app.project)));
	repaint();
	
	return;
}

/*-----------------------------------------------------------------*/
static void on_help_about()
{
	MessageBox(mme_app.main_hwnd, "ヘルプ -> MME についてメニューが選択されました", "デバッグ", MB_OK|MB_ICONINFORMATION);
	return;
}

/*-----------------------------------------------------------------*/
static void on_destory()
{
	if(is_open_mme_project(&(mme_app.project))){
		close_mme_project(&(mme_app.project));
	}
	close_m2v_vfapi(&(mme_app.vfapi));
	GlobalFree(mme_app.clipboard);
	
	PostQuitMessage(0);
}

/*-----------------------------------------------------------------*/
static void on_paint()
{
	HDC hdc;
	PAINTSTRUCT ps;

	hdc = BeginPaint(mme_app.main_hwnd, &ps);

	if(is_open_mme_project(&(mme_app.project))){
		showdib(hdc, get_dib_header_mme_project(&(mme_app.project)), get_frame_mme_project(&(mme_app.project)));
	}

	EndPaint(mme_app.main_hwnd, &ps);
}

/*-----------------------------------------------------------------*/
static void on_cbdialog_cancel()
{
	mme_app.callback_dlg = (HWND)INVALID_HANDLE_VALUE;
}

/*-----------------------------------------------------------------*/
static void on_h_scroll(int code, HWND hwnd)
{
	int index;

	if(code == TB_ENDTRACK){
		index = SendMessage(mme_app.slide_bar, TBM_GETPOS, 0, 0);
		if(index != set_frame_index_mme_project(&(mme_app.project), index)){
			MessageBox(mme_app.main_hwnd, get_error_msg_mme_project(&(mme_app.project)), "INFO", MB_OK|MB_ICONINFORMATION);
		}
	
		update_slider();
		SetWindowText(mme_app.main_hwnd, get_status_str_mme_project(&(mme_app.project)));
		repaint();
	}
	return;
		
}
/*-----------------------------------------------------------------*/
static void on_dropfiles(HANDLE hdrop)
{
	int n;
	char *path;

	n = DragQueryFile(hdrop, 0, NULL, 0) + 4;
	path = malloc(n);
	n = DragQueryFile(hdrop, 0, path, n);
	DragFinish(hdrop);
	open_file(path);
	free(path);
}
/*-----------------------------------------------------------------*/
static void enable_menu_from_list(const UINT *list, int count)
{
	int i;

	for(i=0;i<count;i++){
		EnableMenuItem(GetMenu(mme_app.main_hwnd), list[i], MF_ENABLED);
	}
}

/*-----------------------------------------------------------------*/
static void disable_menu_from_list(const UINT *list, int count)
{
	int i;

	for(i=0;i<count;i++){
		EnableMenuItem(GetMenu(mme_app.main_hwnd), list[i], MF_GRAYED);
	}
}

/*-----------------------------------------------------------------*/
static void showdib(HDC hdc, const BITMAPINFOHEADER *header, const VOID *data)
{
	StretchDIBits(hdc, 0, 0, header->biWidth, header->biHeight, 0, 0, header->biWidth, header->biHeight, data, (BITMAPINFO *)header, DIB_RGB_COLORS, SRCCOPY);
}

/*-----------------------------------------------------------------*/
static void resize(int client_width, int client_height)
{
	RECT rect;
	RECT client_rect;
	int h_diff;
	int v_diff;

	GetWindowRect(mme_app.main_hwnd, &rect);
	GetClientRect(mme_app.main_hwnd, &client_rect);

	h_diff = client_width - (client_rect.right - client_rect.left);
	v_diff = client_height - (client_rect.bottom - client_rect.top);

	rect.right += h_diff;
	rect.bottom += v_diff + 24;

	MoveWindow(mme_app.main_hwnd, rect.left, rect.top, (rect.right - rect.left), (rect.bottom - rect.top), TRUE);

	GetClientRect(mme_app.main_hwnd, &client_rect);

	h_diff = client_width - (client_rect.right - client_rect.left);
	v_diff = client_height - (client_rect.bottom - client_rect.top);

	rect.right += h_diff;
	rect.bottom += v_diff + 24;

	MoveWindow(mme_app.main_hwnd, rect.left, rect.top, (rect.right - rect.left), (rect.bottom - rect.top), TRUE);
	
	GetClientRect(mme_app.main_hwnd, &rect);
	
	MoveWindow(mme_app.slide_bar, rect.left, rect.bottom-24, (rect.right-rect.left), 24, TRUE);
}

/*-----------------------------------------------------------------*/
static void update_slider()
{
	__int64 index;

	index = get_frame_index_mme_project(&(mme_app.project));
	if(index > LONG_MAX){
		SendMessage(mme_app.slide_bar, TBM_SETPOS, TRUE, LONG_MAX);
	}else{
		SendMessage(mme_app.slide_bar, TBM_SETPOS, TRUE, (LONG)index);
	}
}

/*-----------------------------------------------------------------*/
static HRESULT _stdcall callback(char *path, DWORD percent)
{
	MSG msg;
	char buffer[64];
	HWND hwnd;
	DWORD old;
	
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if(mme_app.callback_dlg != INVALID_HANDLE_VALUE){
		old = SendDlgItemMessage(mme_app.callback_dlg, IDC_PROGRESS, PBM_GETPOS, 0, 0);
		SendDlgItemMessage(mme_app.callback_dlg, IDC_PROGRESS, PBM_SETPOS, percent, 0);
		if(old != percent){
			sprintf(buffer, msg_table[mme_app.lang][MSG_STORE_CALLBACK_PERCENT_TEMPLATE], percent);
			hwnd = GetDlgItem(mme_app.callback_dlg, IDC_PERCENT_TEXT);
			SetWindowText(hwnd, buffer);
		}
		return VF_OK;
	}else{
		return VF_ERROR;
	}
}

/*-----------------------------------------------------------------*/
static void msg_pump()
{
	MSG msg;
	
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if(!TranslateAccelerator(mme_app.main_hwnd, mme_app.accel_tbl, &msg)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

/*-----------------------------------------------------------------*/
static void show_error(char *msg, DWORD code)
{
	int n;
	char *buf;
	LPVOID sys_msg_buf;

	static const char format[] = "%s\n\n\tCODE:\t%u\nREASON:\t%s";
	static const char emg[] = "No enough memory space!!!";

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, NULL, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&sys_msg_buf, 0, NULL);

	n = strlen(msg) + strlen(sys_msg_buf) + sizeof(format) + 8;
	buf = (char *)malloc(n);
	if(buf == NULL){
		MessageBox(mme_app.main_hwnd, emg, "ERROR", MB_OK|MB_ICONERROR);
		LocalFree(sys_msg_buf);
		return;
	}

	sprintf(buf, format, msg, code, sys_msg_buf);

	MessageBox(mme_app.main_hwnd, buf, "ERROR", MB_OK|MB_ICONERROR);

	free(buf);
	
	LocalFree(sys_msg_buf);
}

static void repaint()
{
	InvalidateRgn(mme_app.main_hwnd, NULL, FALSE);
	UpdateWindow(mme_app.main_hwnd);
}

static void open_file(char *path)
{
	if(is_open_mme_project(&(mme_app.project))){
		close_mme_project(&(mme_app.project));
	}
	
	if(!open_mme_project(path, &(mme_app.project), &(mme_app.vfapi.func), &(mme_app.vfapi.func_edit_ext), mme_app.lang)){
		disable_menu_from_list(SWITCHING_MENU_ID, sizeof(SWITCHING_MENU_ID)/sizeof(UINT));
		SetWindowText(mme_app.main_hwnd, APP_NAME);
		InvalidateRgn(mme_app.main_hwnd, NULL, TRUE); /* repaint */
		SendMessage(mme_app.slide_bar, TBM_SETRANGE, (WPARAM) TRUE, (LPARAM) MAKELONG(0, 100));
		SendMessage(mme_app.slide_bar, TBM_SETSEL, (WPARAM) TRUE, (LPARAM) MAKELONG(0, 100));
		EnableWindow(mme_app.slide_bar, FALSE);
		MessageBox(mme_app.main_hwnd, get_error_msg_mme_project(&(mme_app.project)), "ERROR", MB_OK|MB_ICONERROR);
	}else{
		enable_menu_from_list(SWITCHING_MENU_ID, sizeof(SWITCHING_MENU_ID)/sizeof(UINT));
		SetWindowText(mme_app.main_hwnd, get_status_str_mme_project(&(mme_app.project)));
		resize(get_frame_width_mme_project(&(mme_app.project)), get_frame_height_mme_project(&(mme_app.project)));
		if(get_frame_count_mme_project(&(mme_app.project)) > LONG_MAX){
			SendMessage(mme_app.slide_bar, TBM_SETRANGEMAX, TRUE, LONG_MAX);
			SendMessage(mme_app.slide_bar, TBM_SETSELEND, TRUE, LONG_MAX);
		}else{
			SendMessage(mme_app.slide_bar, TBM_SETRANGEMAX, TRUE, (LONG)get_frame_count_mme_project(&(mme_app.project))-1);
			SendMessage(mme_app.slide_bar, TBM_SETSELEND, TRUE, (LONG)get_frame_count_mme_project(&(mme_app.project))-1);
		}
		SendMessage(mme_app.slide_bar, TBM_SETPOS, TRUE, 0);
		SendMessage(mme_app.slide_bar, TBM_SETSELSTART, TRUE, 0);
		EnableWindow(mme_app.slide_bar, TRUE);
		InvalidateRgn(mme_app.main_hwnd, NULL, FALSE); /* repaint */
	}
}

static void parse_cmdline(char *arg)
{
	int i,n;
	
	char  *work,*p,*pre;

	int    argc;
	char **argv;
	
	int quotation;

	int gop_list;
	int quit;

	n = strlen(arg);

	if(n == 0){
		return;
	}

	work = malloc( (n+1) + (sizeof(char *)*((n+1)/2)) );
	if(work == NULL){
		return;
	}
	argv = (char **)(work+n+1);
	argc = 0;

	memcpy(work, arg, n+1);

	quotation = 0;
	p = work;
	pre = p;
	while(*p){
		if( (p[0] == ' ') && (quotation == 0) ){
			p[0] = 0;
			if( (p-pre) > 0 ){
				argv[argc] = pre;
				argc += 1;
			}
			p += 1;
			pre = p;
			continue;
		}else if(p[0] == '"'){
			p[0] = 0;
			quotation = !quotation;
			if( (quotation == 0) && ((p-pre) > 0) ){
				argv[argc] = pre;
				argc += 1;
			}
			p += 1;
			pre = p;
			continue;
		}
		p = CharNext(p);
	}

	if( (p-pre) > 1 ){
		argv[argc] = pre;
		argc += 1;
	}

	gop_list = 0;
	quit = 0;

	for(i=0;i<argc;i++){
		if(argv[i][0] != '-'){
			break;
		}
		
		switch(argv[i][1]){
		case 'g':
		case 'G':
			gop_list = 1;
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
	}

	for(;i<argc;i++){
		if(gop_list != 0){
			create_goplist(&(mme_app.vfapi), argv[i]);
		}else{
			open_file(argv[i]);
		}
	}

	if(quit != 0){
		exit(0);
	}

	free(work);
}

/*----------------------------------------------------------------*/
static const char REGISTRY_POSITION[] = "Software\\marumo\\mme.exe";
static const char M2V_REG_POSITION[] = "Software\\marumo\\mpeg2vid_vfp";

/*-----------------------------------------------------------------*/
static void get_lastpath()
{
	HKEY key;
	DWORD type;
	DWORD size;

	memset(mme_app.openpath, 0, sizeof(mme_app.openpath));
	memset(mme_app.savepath, 0, sizeof(mme_app.savepath));
	
	if(RegOpenKeyEx(HKEY_CURRENT_USER, REGISTRY_POSITION, 0, KEY_READ, &key) != ERROR_SUCCESS){
		return;
	}
	
	type = REG_SZ;
	size = sizeof(mme_app.openpath);

	if(RegQueryValueEx(key, "openpath", NULL, &type, (LPBYTE)mme_app.openpath, &size) != ERROR_SUCCESS){
		RegCloseKey(key);
		return;
	}

	type = REG_SZ;
	size = sizeof(mme_app.savepath);

	if(RegQueryValueEx(key, "savepath", NULL, &type, (LPBYTE)mme_app.savepath, &size) != ERROR_SUCCESS){
		RegCloseKey(key);
		return;
	}

	RegCloseKey(key);

	return;
}

/*-----------------------------------------------------------------*/
static void put_openpath(char *path)
{
	HKEY key;
	DWORD type;
	DWORD size;
	DWORD trash;
	
	strncpy(mme_app.openpath, path, sizeof(mme_app.openpath)-1);
	cut_filename(mme_app.openpath);

	if(RegCreateKeyEx(HKEY_CURRENT_USER, REGISTRY_POSITION, 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, &trash) != ERROR_SUCCESS){
		return;
	}
	
	type = REG_SZ;
	size = sizeof(mme_app.openpath);

	if(RegSetValueEx(key, "openpath", 0, type, (LPBYTE)mme_app.openpath, size) != ERROR_SUCCESS){
		RegCloseKey(key);
		return;
	}
	
	RegCloseKey(key);
	return;
}

/*-----------------------------------------------------------------*/
static void put_savepath(char *path)
{
	HKEY key;
	DWORD type;
	DWORD trash;
	DWORD size;

	strncpy(mme_app.savepath, path, sizeof(mme_app.savepath)-1);
	cut_filename(mme_app.savepath);
	
	if(RegCreateKeyEx(HKEY_CURRENT_USER, REGISTRY_POSITION, 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, &trash) != ERROR_SUCCESS){
		return;
	}
	
	type = REG_SZ;
	size = sizeof(mme_app.openpath);

	if(RegSetValueEx(key, "savepath", 0, type, (LPBYTE)mme_app.savepath, size) != ERROR_SUCCESS){
		RegCloseKey(key);
		return;
	}
	
	RegCloseKey(key);
	return;
}

/*-----------------------------------------------------------------*/
static void cut_filename(char *path)
{
	HANDLE h;
	WIN32_FIND_DATA find;
	char *p,*last;

	h = FindFirstFile(path, &find);
	if(h != INVALID_HANDLE_VALUE){
		FindClose(h);
		if(find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
			return;
		}
	}
	
	p = path;
	last = NULL;
	while(*p){
		if(*p == '\\'){
			last = p;
		}
		p = CharNext(p);
	}

	if(last){
		*last = 0;
	}
}

/*-----------------------------------------------------------------*/
static int has_suffix(char *path)
{
	char *p;
	char *suffix;
	
	p = path;
	suffix = NULL;

	while(*p){
		if(*p == '.'){
			suffix = p;
		}else if(*p == '\\'){
			suffix = NULL;
		}
		p = CharNext(p);
	}

	if(suffix){
		return 1;
	}

	return 0;
}

/*-----------------------------------------------------------------*/
static DWORD get_goplist_mode()
{
	HKEY key;
	DWORD type;
	DWORD size;
	DWORD value;
	DWORD trash;
	
	if(RegCreateKeyEx(HKEY_CURRENT_USER, M2V_REG_POSITION, 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, &trash) != ERROR_SUCCESS){
		return 0;
	}
	
	type = REG_DWORD;
	size = sizeof(DWORD);

	if(RegQueryValueEx(key, "gl", NULL, &type, (LPBYTE)&value, &size) != ERROR_SUCCESS){
		value = 0;
	}
	
	RegCloseKey(key);
	
	return value;
}

/*-----------------------------------------------------------------*/
static void put_goplist_mode(DWORD gl_mode)
{
	HKEY key;
	DWORD type;
	DWORD size;
	DWORD trash;
	
	if(RegCreateKeyEx(HKEY_CURRENT_USER, M2V_REG_POSITION, 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, &trash) != ERROR_SUCCESS){
		return;
	}
	
	type = REG_DWORD;
	size = sizeof(DWORD);

	RegSetValueEx(key, "gl", 0, REG_DWORD, (LPBYTE)&gl_mode, size);
	RegCloseKey(key);
	
	return;
}

/*-----------------------------------------------------------------*/
static void create_goplist(M2V_VFAPI *vfapi, char *path)
{
	VF_FileHandle file;

	if(vfapi->func.OpenFile(path, &file) != VF_OK){
		return;
	}

	vfapi->func.CloseFile(file);
}


