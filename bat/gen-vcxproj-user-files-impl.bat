@set DST=%1

@if "%2"=="" goto assign_default
@set SRC=%2
@goto src_assigned

:assign_default
@set SRC=_user.template

:src_assigned
@set EXT=vcxproj.user

@if not exist vcxproj\%DST%.%EXT% copy /Y vcxproj\%SRC% vcxproj\%DST%.%EXT%
