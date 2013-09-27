@echo off
setlocal

echo フレームリストからAvisynth用のTrimコマンドを生成するサンプルBATです。
echo JAVAのランタイム(JRE)がインストールされ、"java"コマンドにパスが通っている状態で動作します。
echo カット後のチャプターを出力するには -chap "出力Path" を指定します。
echo ソースのfpsを指定する場合は -fps 25.0 等と指定します。無指定の場合、29.97(30000/1001)fpsが使用されます。
echo -offset -1.0 と指定するとチャプターの位置を1秒前に指定できます。

echo;

echo atf02.classが置いてあるディレクトリ
set ATF_PATH="./"
echo %ATF_PATH%

echo 読み込むフレームリストのパス
set FLIST_PATH="./inputlist.txt"
echo %FLIST_PATH%

echo *.chapters.txtの出力先
set CHAP_PATH="./inputlist.chapters.txt"
echo %CHAP_PATH%

echo;
echo 実行結果を表示

rem AVS_TRIMにフレームリストから生成したTrimコマンドが入ります。
for /f "usebackq tokens=*" %%i in (`java -cp %ATF_PATH% atf02 "%FLIST_PATH%" -chap "%CHAP_PATH%" -offset -1.0`) do @set AVS_TRIM=%%i
echo %AVS_TRIM%

echo;
endlocal
pause

