@echo off
setlocal

echo �t���[�����X�g����Avisynth�p��Trim�R�}���h�𐶐�����T���v��BAT�ł��B
echo JAVA�̃����^�C��(JRE)���C���X�g�[������A"java"�R�}���h�Ƀp�X���ʂ��Ă����Ԃœ��삵�܂��B
echo �J�b�g��̃`���v�^�[���o�͂���ɂ� -chap "�o��Path" ���w�肵�܂��B
echo �\�[�X��fps���w�肷��ꍇ�� -fps 25.0 ���Ǝw�肵�܂��B���w��̏ꍇ�A29.97(30000/1001)fps���g�p����܂��B
echo -offset -1.0 �Ǝw�肷��ƃ`���v�^�[�̈ʒu��1�b�O�Ɏw��ł��܂��B

echo;

echo atf02.class���u���Ă���f�B���N�g��
set ATF_PATH="./"
echo %ATF_PATH%

echo �ǂݍ��ރt���[�����X�g�̃p�X
set FLIST_PATH="./inputlist.txt"
echo %FLIST_PATH%

echo *.chapters.txt�̏o�͐�
set CHAP_PATH="./inputlist.chapters.txt"
echo %CHAP_PATH%

echo;
echo ���s���ʂ�\��

rem AVS_TRIM�Ƀt���[�����X�g���琶������Trim�R�}���h������܂��B
for /f "usebackq tokens=*" %%i in (`java -cp %ATF_PATH% atf02 "%FLIST_PATH%" -chap "%CHAP_PATH%" -offset -1.0`) do @set AVS_TRIM=%%i
echo %AVS_TRIM%

echo;
endlocal
pause

