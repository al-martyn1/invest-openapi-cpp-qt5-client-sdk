@set DST=%1

@if "%2"=="" goto assign_default
@set SRC=%2
@goto src_assigned

:assign_default
@set SRC=_user.template

:src_assigned
@set EXT=vcxproj.user

@if not exist "%~dp0\vcxproj\%DST%.%EXT%" copy /Y "%~dp0\vcxproj\%SRC%" "%~dp0\vcxproj\%DST%.%EXT%"
