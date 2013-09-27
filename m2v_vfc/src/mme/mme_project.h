/*******************************************************************
 MME project interface
 *******************************************************************/
#ifndef MME_PROJECT_H
#define MME_PROJECT_H

#include <windows.h>
#include "vfapi.h"
#include "vfapi_edit_ext.h"

typedef struct {
	VF_PluginFunc *func;
	VF_PluginFuncEditExt *func_edit_ext;
	int  lang;

	VF_FileHandle file;

	VF_StreamInfo_Video video_info;
	VF_ReadData_Video video_buffer;

	__int64 frame_index;

	__int64 in_point;
	__int64 out_point;

	VOID *dib_data;
	BITMAPINFOHEADER *dib_header;

	char path[512];
	char error_msg[512];
	char status_str[512];
	
} MME_PROJECT;

#ifndef MME_PROJECT_C

#ifdef __cplusplus
extern "C" {
#endif

extern int open_mme_project(const char *path, MME_PROJECT *project, VF_PluginFunc *func, VF_PluginFuncEditExt *func_edit_ext, int lang);
extern void close_mme_project(MME_PROJECT *project);

extern int output_mme_project(MME_PROJECT *project, char *out_path, HRESULT (_stdcall *callback)(char *path, DWORD percent));

extern const char *get_error_msg_mme_project(MME_PROJECT *project);
extern const char *get_status_str_mme_project(MME_PROJECT *project);

extern int is_open_mme_project(MME_PROJECT *project);

extern __int64 set_frame_index_mme_project(MME_PROJECT *project, __int64 frame_index);
extern __int64 get_frame_index_mme_project(MME_PROJECT *project);
extern __int64 go_out_point_mme_project(MME_PROJECT *project);
extern __int64 go_in_point_mme_project(MME_PROJECT *project);
extern __int64 go_start_frame_mme_project(MME_PROJECT *project);
extern __int64 go_end_frame_mme_project(MME_PROJECT *project);
extern __int64 go_prev_i_frame_mme_project(MME_PROJECT *project);
extern __int64 go_next_i_frame_mme_project(MME_PROJECT *project);

extern int set_in_point_mme_project(MME_PROJECT *project);
extern int set_out_point_mme_project(MME_PROJECT *project);

extern const VOID *get_frame_mme_project(MME_PROJECT *project);
extern const BITMAPINFOHEADER *get_dib_header_mme_project(MME_PROJECT *project);

extern int get_frame_width_mme_project(MME_PROJECT *project);
extern int get_frame_height_mme_project(MME_PROJECT *project);

extern __int64 get_frame_count_mme_project(MME_PROJECT *project);
extern __int64 get_in_point_mme_project(MME_PROJECT *project);
extern __int64 get_out_point_mme_project(MME_PROJECT *project);

#ifdef __cplusplus
}
#endif

#endif /* MME_PROJECT_C */

#endif /* MME_PROJECT_H */