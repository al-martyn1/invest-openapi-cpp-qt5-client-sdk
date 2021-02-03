@call "%~dp0\bat\setup-qt.bat"
@call "%~dp0\bat\setup_output_root.bat"
@call "%~dp0\bat\setup_deploy_root.bat" create

@call "%~dp0\bat\setup_toolset_platform_config.bat" msvc2017 x86 Debug
@call "%~dp0\copy_exe_single_config.bat"

@call "%~dp0\bat\setup_toolset_platform_config.bat" msvc2017 x86 Release
@call "%~dp0\copy_exe_single_config.bat"

@call "%~dp0\bat\setup_toolset_platform_config.bat" msvc2017 x64 Debug
@call "%~dp0\copy_exe_single_config.bat"

@call "%~dp0\bat\setup_toolset_platform_config.bat" msvc2017 x64 Release
@call "%~dp0\copy_exe_single_config.bat"

