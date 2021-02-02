@call bat\setup-qt.bat
@call bat\setup_output_root.bat
@call bat\setup_deploy_root.bat create

@echo Deploing msvc2017 x86 Debug
@call mk_distr_single_conf.bat   msvc2017 x86 Debug

@echo Deploing msvc2017 x86 Release
@call mk_distr_single_conf.bat   msvc2017 x86 Release

@echo Deploing msvc2017 x64 Debug
@call mk_distr_single_conf.bat   msvc2017 x64 Debug

@echo Deploing msvc2017 x64 Release
@call mk_distr_single_conf.bat   msvc2017 x64 Release

