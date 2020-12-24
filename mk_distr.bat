call bat\setup-qt.bat
if exist _deploy rmdir /Q /S _deploy
mkdir _deploy

mkdir _deploy\x64
mkdir _deploy\x64\Debug
mkdir _deploy\x64\Release
mkdir _deploy\x86
mkdir _deploy\x86\Debug
mkdir _deploy\x86\Release

call cp_exe.bat

%QTDIR_X64%\bin\windeployqt.exe --help-all >windeployqt.txt

call bat\call-vcvars64.bat
%QTDIR_X64%\bin\windeployqt.exe --debug   --compiler-runtime --network _deploy\x64\Debug    >deploy.log
@rem %QTDIR_X64%\bin\windeployqt.exe --release --compiler-runtime --network _deploy\x64\Release  >>deploy.log

@rem call bat\call-vcvars32.bat
@rem %QTDIR_X86%\bin\windeployqt.exe --debug   --compiler-runtime --network _deploy\x86\Debug    >>deploy.log
@rem %QTDIR_X86%\bin\windeployqt.exe --release --compiler-runtime --network _deploy\x86\Release  >>deploy.log


rem Qt5Networkd.dll