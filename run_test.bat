@if "%1" == "" goto NAME_NOT_TAKEN

@set INPUT_MASK=*
@if "%1" NEQ "" set INPUT_MASK=%1

@set INPUT_EXT=%~x1

@set MASK=%INPUT_MASK%
@if "%INPUT_EXT%"=="" set MASK=%INPUT_MASK%.exe

@echo MASK - %MASK%

@set TSET=msvc2017
@if "%2" NEQ "" set TSET=%2

@set ARCH=x64
@if "%2" NEQ "" set ARCH=%3

@set REL_TYPE=Debug
@if "%3" NEQ "" set REL_TYPE=%4

@call "%~dp0\bat\setup_deploy_root.bat"
@call "%~dp0\bat\setup_deploy_path.bat"
@call "%~dp0\bat\setup_output_root.bat"
@call "%~dp0\bat\setup_toolset_platform_config.bat" %TSET% %ARCH% %REL_TYPE%

@call "%~dp0\cp_exe.bat"

@echo TOOLSET      - %TOOLSET%
@echo ARCH         - %PLATFORM%
@echo Release type - %CONFIGURATION%
@rem echo REL_TYPE     - %REL_TYPE%
@rem echo DEPLOY_PATH  - %DEPLOY_PATH%

@set RUN_LOGS=%~dp0\_run_logs
@if not exist "%RUN_LOGS%" mkdir "%RUN_LOGS%"

@for %%i in ("%DEPLOY_PATH%\%MASK%") do "%%i" > "%RUN_LOGS%\%%~ni.log" 2>&1
@rem (
@rem     @echo Test  name - %%i
@rem     @echo Short name - %%~si
@rem     @echo Name only  - %%~ni
@rem     @echo ---
@rem )

exit /B


:NAME_NOT_TAKEN
@echo Test name not taken