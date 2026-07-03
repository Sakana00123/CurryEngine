@REM @echo off
@REM for /R %%i in (*.cpp) do nkf32.exe -S -w --overwrite "%%i"
@REM for /R %%i in (*.h) do nkf32.exe -S -w --overwrite "%%i"
@REM echo 変換が完了しました。
@REM pause

nkf32.exe -S -w --overwrite *.cpp *.h