/*******************************************************************
 Registry Module
 *******************************************************************/
#include <windows.h>
#include <winreg.h>

#include "config.h"

#define REGISTRY_C
#include "registry.h"

int get_color_conversion_type();
int get_idct_type();
int get_simd_mode();
int get_field_mode();
int get_resize_mode();
int get_file_check_limit();
int get_color_matrix();
int get_gl_mode();
int get_file_mode();
int get_yuy2_mode();

static const char REGISTRY_POSITION[] = "Software\\marumo\\mpeg2vid_vfp";

int get_color_conversion_type()
{
	HKEY key;
	DWORD type;
	DWORD size;
	DWORD value;

	if(RegOpenKeyEx(HKEY_CURRENT_USER, REGISTRY_POSITION, 0, KEY_READ, &key) != ERROR_SUCCESS){
		return 1;
	}

	type = REG_DWORD;
	size = sizeof(DWORD);
	
	if(RegQueryValueEx(key, "re_map", NULL, &type, (LPBYTE)&value, &size) != ERROR_SUCCESS){
		RegCloseKey(key);
		return 1;
	}

	RegCloseKey(key);

	if(value){
		return 1;
	}

	return 0;
}

int get_idct_type()
{
	HKEY key;
	DWORD type;
	DWORD size;
	DWORD value;

	if(RegOpenKeyEx(HKEY_CURRENT_USER, REGISTRY_POSITION, 0, KEY_READ, &key) != ERROR_SUCCESS){
		return 1;
	}

	type = REG_DWORD;
	size = sizeof(DWORD);
	
	if(RegQueryValueEx(key, "idct_func", NULL, &type, (LPBYTE)&value, &size) != ERROR_SUCCESS){
		RegCloseKey(key);
		return 1;
	}

	RegCloseKey(key);

	return value;
}

int get_simd_mode()
{
	HKEY key;
	DWORD type;
	DWORD size;
	DWORD value;

	if(RegOpenKeyEx(HKEY_CURRENT_USER, REGISTRY_POSITION, 0, KEY_READ, &key) != ERROR_SUCCESS){
		return 0;
	}

	type = REG_DWORD;
	size = sizeof(DWORD);
	
	if(RegQueryValueEx(key, "simd", NULL, &type, (LPBYTE)&value, &size) != ERROR_SUCCESS){
		RegCloseKey(key);
		return 0;
	}

	RegCloseKey(key);

	return value;
}

int get_field_mode()
{
	HKEY key;
	DWORD type;
	DWORD size;
	DWORD value;

	if(RegOpenKeyEx(HKEY_CURRENT_USER, REGISTRY_POSITION, 0, KEY_READ, &key) != ERROR_SUCCESS){
		return 0;
	}

	type = REG_DWORD;
	size = sizeof(DWORD);
	
	if(RegQueryValueEx(key, "field_order", NULL, &type, (LPBYTE)&value, &size) != ERROR_SUCCESS){
		RegCloseKey(key);
		return 0;
	}

	RegCloseKey(key);

	return value;
}

int get_resize_mode()
{
	HKEY key;
	DWORD type;
	DWORD size;
	DWORD value;

	if(RegOpenKeyEx(HKEY_CURRENT_USER, REGISTRY_POSITION, 0, KEY_READ, &key) != ERROR_SUCCESS){
		return 0;
	}

	type = REG_DWORD;
	size = sizeof(DWORD);
	
	if(RegQueryValueEx(key, "aspect_ratio", NULL, &type, (LPBYTE)&value, &size) != ERROR_SUCCESS){
		RegCloseKey(key);
		return 0;
	}

	RegCloseKey(key);

	return value;
}

int get_filecheck_limit()
{
	HKEY key;
	DWORD type;
	DWORD size;
	DWORD value;

	if(RegOpenKeyEx(HKEY_CURRENT_USER, REGISTRY_POSITION, 0, KEY_READ, &key) != ERROR_SUCCESS){
		return 1024*1024*8;
	}

	type = REG_DWORD;
	size = sizeof(DWORD);
	
	if(RegQueryValueEx(key, "limit", NULL, &type, (LPBYTE)&value, &size) != ERROR_SUCCESS){
		RegCloseKey(key);
		return 1024*1024*8;
	}

	RegCloseKey(key);

	return value;
}

int get_color_matrix()
{
	HKEY key;
	DWORD type;
	DWORD size;
	DWORD value;

	if(RegOpenKeyEx(HKEY_CURRENT_USER, REGISTRY_POSITION, 0, KEY_READ, &key) != ERROR_SUCCESS){
		return 0;
	}

	type = REG_DWORD;
	size = sizeof(DWORD);
	
	if(RegQueryValueEx(key, "color_matrix", NULL, &type, (LPBYTE)&value, &size) != ERROR_SUCCESS){
		RegCloseKey(key);
		return 0;
	}

	RegCloseKey(key);

	return value;
}

int get_gl_mode()
{
	HKEY key;
	DWORD type;
	DWORD size;
	DWORD value;

	if(RegOpenKeyEx(HKEY_CURRENT_USER, REGISTRY_POSITION, 0, KEY_READ, &key) != ERROR_SUCCESS){
		return 0;
	}

	type = REG_DWORD;
	size = sizeof(DWORD);
	
	if(RegQueryValueEx(key, "gl", NULL, &type, (LPBYTE)&value, &size) != ERROR_SUCCESS){
		RegCloseKey(key);
		return 0;
	}

	RegCloseKey(key);

	return value;
}

int get_file_mode()
{
	HKEY key;
	DWORD type;
	DWORD size;
	DWORD value;

	if(RegOpenKeyEx(HKEY_CURRENT_USER, REGISTRY_POSITION, 0, KEY_READ, &key) != ERROR_SUCCESS){
		return M2V_CONFIG_SINGLE_FILE;
	}

	type = REG_DWORD;
	size = sizeof(DWORD);
	
	if(RegQueryValueEx(key, "file", NULL, &type, (LPBYTE)&value, &size) != ERROR_SUCCESS){
		RegCloseKey(key);
		return M2V_CONFIG_SINGLE_FILE;
	}

	RegCloseKey(key);

	if(value != M2V_CONFIG_MULTI_FILE){
		return M2V_CONFIG_SINGLE_FILE;
	}

	return value;
}

int get_yuy2_mode()
{
	HKEY key;
	DWORD type;
	DWORD size;
	DWORD value;

	if(RegOpenKeyEx(HKEY_CURRENT_USER, REGISTRY_POSITION, 0, KEY_READ, &key) != ERROR_SUCCESS){
		return M2V_CONFIG_YUY2_CONVERT_NONE;
	}

	type = REG_DWORD;
	size = sizeof(DWORD);
	
	if(RegQueryValueEx(key, "yuy2_matrix", NULL, &type, (LPBYTE)&value, &size) != ERROR_SUCCESS){
		RegCloseKey(key);
		return M2V_CONFIG_YUY2_CONVERT_NONE;
	}

	RegCloseKey(key);

	return value;
}

