@ECHO OFF

cd > PWD
set /p PWD= < PWD

set DESTDIR=%PWD%\..\..\ComputreeInstall

echo "Generating Translations"
lrelease *.pro
mkdir %DESTDIR%\languages 
copy languages\*.qm %DESTDIR%\languages

cd %PWD%