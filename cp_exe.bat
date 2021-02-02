@call bat\setup-qt.bat
@call bat\setup_output_root.bat
@call bat\setup_deploy_root.bat create

@call bat\setup_toolset_platform_config.bat msvc2017 x86 Debug
@call copy_exe_single_config.bat

@call bat\setup_toolset_platform_config.bat msvc2017 x86 Release
@call copy_exe_single_config.bat

@call bat\setup_toolset_platform_config.bat msvc2017 x64 Debug
@call copy_exe_single_config.bat

@call bat\setup_toolset_platform_config.bat msvc2017 x64 Release
@call copy_exe_single_config.bat

