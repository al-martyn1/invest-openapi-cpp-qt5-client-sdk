@set DEPLOY_ROOT=%~dp0\..\_deploy

@if "%1" NEQ "create" goto END

@if exist "%DEPLOY_ROOT%" rmdir /Q /S "%DEPLOY_ROOT%"
@mkdir "%DEPLOY_ROOT%"

:END