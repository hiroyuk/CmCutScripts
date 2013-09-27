//---------------------------------------------------------------------------
#include <vcl.h>
#include <winreg.h>
#pragma hdrstop

#include "Unit1.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button2Click(TObject *Sender)
{
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button1Click(TObject *Sender)
{
	AnsiString w;
	HKEY key;
	DWORD trash;
	DWORD value;
	DWORD size;

	size = sizeof(DWORD);
	if(RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\marumo\\mpeg2vid_vfp", 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, &trash) != ERROR_SUCCESS){
		Close();
	}

	if(ignore_aspect_ratio->Checked){
		value = 0;
	}else{
		value = 1;
	}
	RegSetValueEx(key, "aspect_ratio", 0, REG_DWORD, (LPBYTE)&value, size);

	if(no_remap->Checked){
		value = 0;
	}else{
		value = 1;
	}
	RegSetValueEx(key, "re_map", 0, REG_DWORD, (LPBYTE)&value, size);

	if(idct_double->Checked){
		value = 0;
	}else if(idct_llm->Checked){
		value = 1;
	}else if(idct_ap922->Checked){
		value = 2;
	}else{
		value = 1;
	}
	RegSetValueEx(key, "idct_func", 0, REG_DWORD, (LPBYTE)&value, size);

	if(keep_frame->Checked){
		value = 0;
	}else if(top_first->Checked){
		value = 1;
	}else{
		value = 2;
	}
	RegSetValueEx(key, "field_order", 0, REG_DWORD, (LPBYTE)&value, size);

	value = 0;
	if(mmx->Checked){
		value |= 1;
	}
	if(sse->Checked){
		value |= 2;
	}
	if(sse2->Checked){
		value |= 4;
	}
	RegSetValueEx(key, "simd", 0, REG_DWORD, (LPBYTE)&value, size);

	switch(color_matrix->ItemIndex){
	case 0:
		value = 0;
		break;
	case 1:
		value = 1;
		break;
	case 2:
		value = 4;
		break;
	case 3:
		value = 5;
		break;
	case 4:
		value = 6;
		break;
	case 5:
		value = 7;
		break;
	default:
		value = 0;
	}
	RegSetValueEx(key, "color_matrix", 0, REG_DWORD, (LPBYTE)&value, size);

	value = yuy2_matrix->ItemIndex;
	RegSetValueEx(key, "yuy2_matrix", 0, REG_DWORD, (LPBYTE)&value, size);

	value = 0;
	if(this->never_save_gl_file->Checked){
		value |= 1;
	}
	if(this->never_use_timecode->Checked){
		value |= 2;
	}
	RegSetValueEx(key, "gl", 0, REG_DWORD, (LPBYTE)&value, size);

	value = 0;
	if(this->open_multi_file->Checked){
		value |= 1;
	}
	RegSetValueEx(key, "file", 0, REG_DWORD, (LPBYTE)&value, size);
    
	RegCloseKey(key);

	if(RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\VFPlugin", 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, &trash) != ERROR_SUCCESS){
		Close();
	}
	w = ExtractFileDir(ParamStr(0));
	w = w + "\\m2v.vfp";
	
	RegSetValueEx(key, "MPEG2VIDEO", 0, REG_SZ, (LPBYTE)w.c_str(), w.Length());
	RegCloseKey(key);
	
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormCreate(TObject *Sender)
{
	HKEY key;
	DWORD type;
	DWORD value;
	DWORD size;

	size = sizeof(DWORD);
	type = REG_DWORD;
	
	if(is_mmx_enable()){
		mmx->Enabled = true;
	}else{
		mmx->Checked = false;
		mmx->Enabled = false;
	}

	if(is_sse_enable()){
		sse->Enabled = true;
	}else{
		sse->Enabled = false;
		sse->Checked = false;
	}

	if(is_sse2_enable()){
		sse2->Enabled = true;
	}else{
		sse2->Enabled = false;
		sse2->Checked = false;
	}

	this->set_language();

	this->color_matrix->ItemIndex = 0;
	this->yuy2_matrix->ItemIndex = 0;
	
	if(RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\marumo\\mpeg2vid_vfp", 0, KEY_READ, &key) != ERROR_SUCCESS){
		remap->Checked = true;
		idct_ap922->Checked = true;
		use_aspect_ratio->Checked = true;
		top_first->Checked = true;
		return;
	}

	if(RegQueryValueEx(key, "idct_func", NULL, &type, (LPBYTE)&value, &size) != ERROR_SUCCESS){
		idct_ap922->Checked = true;
	}else{
		switch(value){
		case 0:
			idct_double->Checked = true;
			break;
		case 1:
			idct_llm->Checked = true;
			break;
		case 2:
			idct_ap922->Checked = true;
			break;
		default:
			idct_llm->Checked = true;
		}
	}

	if(RegQueryValueEx(key, "simd", NULL, &type, (LPBYTE)&value, &size) != ERROR_SUCCESS){
		mmx->Checked = false;
		sse->Checked = false;
		sse2->Checked = false;
	}else{
		if( (value & 1) && mmx->Enabled){
			mmx->Checked = true;
		}else{
			mmx->Checked = false;
		}
		if( (value & 2) && sse->Enabled){
			sse->Checked = true;
		}else{
			sse->Checked = false;
		}
		if( (value & 4) && sse2->Enabled){
			sse2->Checked = true;
		}else{
			sse2->Checked = false;
		}
	}

	if(RegQueryValueEx(key, "re_map", NULL, &type, (LPBYTE)&value, &size) != ERROR_SUCCESS){
		remap->Checked = true;
	}else{
		if(value){
			remap->Checked = true;
		}else{
			no_remap->Checked = true;
		}
	}

	if(RegQueryValueEx(key, "aspect_ratio", NULL, &type, (LPBYTE)&value, &size) != ERROR_SUCCESS){
		use_aspect_ratio->Checked = true;
	}else{
		if(value){
			use_aspect_ratio->Checked = true;
		}else{
			ignore_aspect_ratio->Checked = true;
		}
	}

	if(RegQueryValueEx(key, "field_order", NULL, &type, (LPBYTE)&value, &size) != ERROR_SUCCESS){
		top_first->Checked = true;
	}else{
		switch(value){
		case 0:
			keep_frame->Checked = true;
			break;
		case 1:
			top_first->Checked = true;
			break;
		case 2:
			bottom_first->Checked = true;
			break;
		default:
			keep_frame->Checked = true;
		}
	}

	if(RegQueryValueEx(key, "color_matrix", NULL, &type, (LPBYTE)&value, &size) != ERROR_SUCCESS){
		color_matrix->ItemIndex = 0;
	}else{
		switch(value){
		case 0:
			color_matrix->ItemIndex = 0;
			break;
		case 1:
			color_matrix->ItemIndex = 1;
			break;
		case 4:
			color_matrix->ItemIndex = 2;
			break;
		case 5:
			color_matrix->ItemIndex = 3;
			break;
		case 6:
			color_matrix->ItemIndex = 4;
			break;
		case 7:
			color_matrix->ItemIndex = 5;
			break;
		default:
			color_matrix->ItemIndex = 0;
			break;
		}
	}

	if(RegQueryValueEx(key, "yuy2_matrix", NULL, &type, (LPBYTE)&value, &size) != ERROR_SUCCESS){
		yuy2_matrix->ItemIndex = 0;
	}else{
		if(value < 0){
			value = 0;
		}else if(value > 4){
			value = 0;
		}
		yuy2_matrix->ItemIndex = value;
	}
	
	if(RegQueryValueEx(key, "gl", NULL, &type, (LPBYTE)&value, &size) != ERROR_SUCCESS){
		this->never_save_gl_file->Checked = false;
		this->never_use_timecode->Checked = false;
	}else{
		if(value & 1){
			this->never_save_gl_file->Checked = true;
		}else{
			this->never_save_gl_file->Checked = false;
		}
		if(value & 2){
			this->never_use_timecode->Checked = true;
		}else{
			this->never_use_timecode->Checked = false;
		}
	}

    if(RegQueryValueEx(key, "file", NULL, &type, (LPBYTE)&value, &size) != ERROR_SUCCESS){
		this->open_single_file->Checked = true;
	}else{
        if(value & 1){
            this->open_multi_file->Checked = true;
        }else{
            this->open_single_file->Checked = true;
        }
    }
    
	RegCloseKey(key);
}
//---------------------------------------------------------------------------
int TForm1::is_mmx_enable()
{
    DWORD mmx_check;

    _asm{
        mov eax,1;
        cpuid;
        mov mmx_check,edx;
    }

    if(mmx_check & 0x00800000){
        return 1;
    }else{
        return 0;
    }
}
//---------------------------------------------------------------------------
int TForm1::is_sse_enable()
{
	DWORD sse_check;

	__asm{
		mov eax, 1
		cpuid
		mov sse_check, edx
	}

	if(sse_check & 0x02000000){
		return 1;
	}else{
		return 0;
	}
}
//---------------------------------------------------------------------------
int TForm1::is_sse2_enable()
{
	DWORD sse2_check;

	__asm{
		mov eax, 1
		cpuid
		mov sse2_check, edx
	}

	if(sse2_check & 0x04000000){
		return 1;
	}else{
		return 0;
	}
}
//---------------------------------------------------------------------------
void TForm1::set_language()
{
	LANGID id;

	id = GetUserDefaultLangID();

	if(id == 0x0411){
		this->Label1->Font->Charset = DEFAULT_CHARSET;
		this->Label1->Font->Name = "ＭＳ ゴシック";
		
		this->Font->Charset = DEFAULT_CHARSET;
		this->Font->Name = "ＭＳ ゴシック";

		this->Caption = "MPEG-2 VIDEO VFAPI Plug-In 設定";
		this->Label1->Caption = "MPEG-2 VIDEO VFAPI Plug-In 設定";
		this->GroupBox5->Caption = "アスペクト比";
		this->GroupBox1->Caption = "YUV → RGB 変換";
		this->GroupBox2->Caption = "IDCT 関数";
		this->GroupBox3->Caption = "CPU 拡張";
		this->GroupBox4->Caption = "フィールド順";
		this->GroupBox6->Caption = "色空間行列（省略時）";
		this->GroupBox7->Caption = "GOP リスト";
		this->GroupBox8->Caption = "連番ファイル";
		this->GroupBox9->Caption = "YUY2 色空間行列 (m2v.aui 用)";
		this->ignore_aspect_ratio->Caption = "無視";
		this->use_aspect_ratio->Caption = "反映";
		this->no_remap->Caption = "ストレート変換";
		this->remap->Caption = "ITU-R BT.601 から伸張";
		this->idct_double->Caption = "浮動小数点";
		this->idct_llm->Caption = "整数 (32bit LLM)";
		this->idct_ap922->Caption = "整数 (32bit AP-922)";
		this->keep_frame->Caption = "ソースフレームを維持";
		this->top_first->Caption = "トップ→ボトム順で出力";
		this->bottom_first->Caption = "ボトム→トップ順で出力";
		this->color_matrix->Items->Strings[0] = "自動認識（解像度から）";
		this->yuy2_matrix->Items->Strings[0] = "元の YUV データを維持";
		this->never_use_timecode->Caption = "GOP タイムコードを使わない";
		this->never_save_gl_file->Caption = "GL ファイルを保存しない";
		this->open_multi_file->Caption = "結合して開く";
		this->open_single_file->Caption = "指定ファイルのみ開く";
		this->Button1->Caption = "決定";
		this->Button2->Caption = "取り消し";
		this->Button3->Caption = "削除";
	}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button3Click(TObject *Sender)
{
	LONG n;
	HKEY key;

	n = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\VFPlugin", 0, KEY_SET_VALUE, &key);
	if(n == ERROR_SUCCESS){
		n = RegDeleteValue(key, "MPEG2VIDEO");
		Close();
	}
	if(n != ERROR_SUCCESS){
		LPVOID buf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, n, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&buf, 0, NULL);
		MessageBox(NULL, (LPCTSTR)buf, "ERROR", MB_OK|MB_ICONERROR);
		LocalFree(buf);
	}		
	Close();
}
//---------------------------------------------------------------------------

