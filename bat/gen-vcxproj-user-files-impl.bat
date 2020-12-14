@if %1=="" goto assign_default
@set SRC=%1
@goto src_assigned

:assign_default
@set SRC=example.vcxproj.user.template

:src_assigned
@set EXT=vcxproj.user

@rem copy /Y vcxproj\%SRC% vcxproj\example001.%EXT%
@copy /Y vcxproj\%SRC% vcxproj\test001.%EXT%
