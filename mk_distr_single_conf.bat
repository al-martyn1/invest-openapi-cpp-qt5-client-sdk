@rem %1 - toolset name, eg msvc2017
@rem %2 - platform name, eg x86 or x64
@rem %3 - configuration name, eg Debug or Release

@call "%~dp0\bat\setup_toolset_platform_config.bat" %1 %2 %3 
@rem exit /B %ERRORLEVEL%
@call "%~dp0\bat\setup_deploy_path.bat"
@call "%~dp0\copy_exe_single_config.bat"

echo call VC bat confuguration file
call "%~dp0\bat\setup-toolset-vars-%TOOLSET%-%PLATFORM%.bat"

@set WINDEPLOYQT_MODULES=--compiler-runtime --network --websockets --sql

@echo Executing %WINDEPLOYQT% %WINDEPLOYQT_MODULES% "%DEPLOY_PATH%"
@rem if exist "%DEPLOY_PATH%\*.exe" 
@rem --verbose 2
%WINDEPLOYQT% %WINDEPLOYQT_MODULES% "%DEPLOY_PATH%"
@echo Deploing done with result %ERRORLEVEL%
@exit /B

