@rem http://microsin.net/adminstuff/windows/for-in-bat-files.html

@for %%i in ("%~dp0\vcxproj\*.vcxproj") do (
   @if not exist "%%i.user" copy "%~dp0\vcxproj\_user.template" "%%i.user"
)
