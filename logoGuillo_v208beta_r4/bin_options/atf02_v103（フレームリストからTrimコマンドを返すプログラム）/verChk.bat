@echo off
setlocal

echo;

echo atf02.class���u���Ă���f�B���N�g��
set ATF_PATH="./"
echo %ATF_PATH%

echo �o�[�W������\��

for /f "usebackq tokens=*" %%i in (`java -cp %ATF_PATH% atf02 "-ver"`) do @set AVS_TRIM=%%i
echo %AVS_TRIM%

echo;
endlocal
pause

