//---------------------------------------------------------------------------
#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE 管理のコンポーネント
    TLabel *Label1;
    TButton *Button1;
    TButton *Button2;
    TGroupBox *GroupBox1;
    TGroupBox *GroupBox2;
    TRadioButton *no_remap;
    TRadioButton *remap;
    TRadioButton *idct_double;
    TRadioButton *idct_llm;
    TGroupBox *GroupBox3;
    TCheckBox *mmx;
    TCheckBox *sse;
    TGroupBox *GroupBox4;
    TRadioButton *keep_frame;
    TRadioButton *top_first;
    TRadioButton *bottom_first;
    TGroupBox *GroupBox5;
    TRadioButton *ignore_aspect_ratio;
    TRadioButton *use_aspect_ratio;
    TCheckBox *sse2;
    TGroupBox *GroupBox6;
    TComboBox *color_matrix;
    TButton *Button3;
    TRadioButton *idct_ap922;
    TGroupBox *GroupBox7;
    TCheckBox *never_use_timecode;
    TCheckBox *never_save_gl_file;
    TGroupBox *GroupBox8;
    TRadioButton *open_multi_file;
    TRadioButton *open_single_file;
    TGroupBox *GroupBox9;
    TComboBox *yuy2_matrix;
    void __fastcall Button2Click(TObject *Sender);
    void __fastcall Button1Click(TObject *Sender);
    void __fastcall FormCreate(TObject *Sender);
    
    void __fastcall Button3Click(TObject *Sender);
private:	// ユーザー宣言
    int is_mmx_enable();
    int is_sse_enable();
    int is_sse2_enable();
    void set_language();
public:		// ユーザー宣言
    __fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
