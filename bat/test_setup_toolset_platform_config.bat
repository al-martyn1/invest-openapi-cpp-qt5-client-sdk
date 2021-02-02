@call setup-qt.bat

@echo TKF_IOA_QTROOT - %TKF_IOA_QTROOT%
@echo TKF_IOA_QTVER  - %TKF_IOA_QTVER%
@echo QTROOT - %QTROOT%
@echo QTVER  - %QTVER%

@echo ---
@echo Testing without OUTPUT_ROOT set
@call setup_toolset_platform_config.bat msvc2014 x86 Release


@set OUTPUT_ROOT=_out

@echo ---
@echo Testing without params
@call setup_toolset_platform_config.bat

@echo ---
@echo Testing with params - msvc2014 x86 Release
@call setup_toolset_platform_config.bat msvc2014 x86 Release

@echo ---
@echo Testing with params - msvc2017 x86 Release
@call setup_toolset_platform_config.bat msvc2017 x86 Release
@echo QT_PLATFORM_SUBPATH  - %QT_PLATFORM_SUBPATH%
@echo QT_PLATFORM_ROOT     - %QT_PLATFORM_ROOT%
@echo WINDEPLOYQT_CONFIGURATION_OPTION - %WINDEPLOYQT_CONFIGURATION_OPTION%
@echo WINDEPLOYQT          - %WINDEPLOYQT%
@echo OUTPUT_ROOT          - %OUTPUT_ROOT%
@echo TOOLSET              - %TOOLSET%
@echo PLATFORM             - %PLATFORM%
@echo CONFIGURATION        - %CONFIGURATION%
@echo PLATFORM_OUTPUT_ROOT - %PLATFORM_OUTPUT_ROOT%
@echo OUTDIR               - %OUTDIR%

@echo ---
@echo Testing with params - msvc2017 x64 Debug
@call setup_toolset_platform_config.bat msvc2017 x64 Debug
@echo QT_PLATFORM_SUBPATH  - %QT_PLATFORM_SUBPATH%
@echo QT_PLATFORM_ROOT     - %QT_PLATFORM_ROOT%
@echo WINDEPLOYQT_CONFIGURATION_OPTION - %WINDEPLOYQT_CONFIGURATION_OPTION%
@echo WINDEPLOYQT          - %WINDEPLOYQT%
@echo OUTPUT_ROOT          - %OUTPUT_ROOT%
@echo TOOLSET              - %TOOLSET%
@echo PLATFORM             - %PLATFORM%
@echo CONFIGURATION        - %CONFIGURATION%
@echo PLATFORM_OUTPUT_ROOT - %PLATFORM_OUTPUT_ROOT%
@echo OUTDIR               - %OUTDIR%

