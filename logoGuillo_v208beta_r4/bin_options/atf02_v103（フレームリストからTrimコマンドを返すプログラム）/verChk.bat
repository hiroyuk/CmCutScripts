@echo off
setlocal

echo;

echo atf02.classが置いてあるディレクトリ
set ATF_PATH="./"
echo %ATF_PATH%

echo バージョンを表示

for /f "usebackq tokens=*" %%i in (`java -cp %ATF_PATH% atf02 "-ver"`) do @set AVS_TRIM=%%i
echo %AVS_TRIM%

echo;
endlocal
pause

