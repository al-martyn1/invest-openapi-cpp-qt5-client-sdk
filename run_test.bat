if "%1" == "" goto test_name_not_taken

set ARCH=x64
if "%2" NEQ "" @set ARCH="%2"

set RELEASE_TYPE=Debug
if "%3" NEQ "" @set RELEASE_TYPE="%3"

@set RUN_LOGS=_run_logs
@if not exist %RUN_LOGS% mkdir %RUN_LOGS%

_deploy\%ARCH%\%RELEASE_TYPE%\%1.exe  > %RUN_LOGS%\%1.log  2>&1
@goto done

:test_name_not_taken
@call bat\run_tests.bat x64 Debug

:done
@echo Test done