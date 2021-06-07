@rem Usage _tkf_place_order.bat +/- [!] TICKER  SIZE    [PRICE]


@set TSET=msvc2017
@set ARCH=x64
@set REL_TYPE=Debug

@call "%~dp0\bat\setup_deploy_root.bat"
@call "%~dp0\bat\setup_deploy_path.bat"
@call "%~dp0\bat\setup_output_root.bat"
@call "%~dp0\bat\setup_toolset_platform_config.bat" %TSET% %ARCH% %REL_TYPE%

@rem call "%~dp0\cp_exe.bat"


%DEPLOY_PATH%\_tkf_place_order %*