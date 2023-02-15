copy .\bin\Debug-windows-x86_64\ARI-S\*.exe . /Y
for /f %%a in ('dir /b *.exe') do start "" "C:\Program Files\RenderDoc\qrenderdoc.exe" "%%a"