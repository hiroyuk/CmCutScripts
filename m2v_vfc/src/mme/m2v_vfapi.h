/*******************************************************************
  m2v.vfp interface
 *******************************************************************/
#ifndef M2V_VFAPI_H
#define M2V_VFAPI_H

#include "vfapi.h"
#include "vfapi_edit_ext.h"

typedef struct {
	HINSTANCE            dll;
	VF_PluginInfo        info;
	VF_PluginFunc        func;
	VF_PluginFuncEditExt func_edit_ext;
} M2V_VFAPI;

#ifndef M2V_VFAPI_C

#ifdef __cplusplus
extern "C" {
#endif

extern int open_m2v_vfapi(M2V_VFAPI *m2v);
extern void close_m2v_vfapi(M2V_VFAPI *m2v);

#ifdef __cplusplus
}
#endif

#endif /* M2V_VFAPI_C */

#endif /* M2V_VFAPI_H */
	