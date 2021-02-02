@if not exist %OUTDIR%\*.exe goto END
@call bat\setup_deploy_path.bat
@copy /Y %OUTDIR%\*.exe %DEPLOY_PATH%\"
:END

