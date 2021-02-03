@echo You need to make deploy (run mk_distr.bat) before calling this script
@rem must be called from upper directory
@rem Usage: call bat\run_tests.bat ARCH RELEASE_TYPE

@set ARCH=%1
@set RELEASE_TYPE=%2
@set RUN_LOGS=_run_logs

@rem if exist "%RUN_LOGS%" rmdir /Q /S "%RUN_LOGS%"
@if not exist "%RUN_LOGS%" mkdir "%RUN_LOGS%"

@call cp_exe.bat
@rem https://stackoverflow.com/questions/15567809/batch-extract-path-and-filename-from-a-variable
@rem https://rsdn.org/article/winshell/batanyca.xml

@for %%i in ("%~dp0\_deploy\%ARCH%\%RELEASE_TYPE%\*.exe") do (
   "%%~dpni.exe" > "%RUN_LOGS%\%%~ni.log"  2>&1
)


