@rem http://microsin.net/adminstuff/windows/for-in-bat-files.html

@for %%i in (vcxproj\*.vcxproj) do (
   @if not exist %%i.user copy vcxproj\_user.template %%i.user
)

@rem call bat\gen-vcxproj-user-files-impl.bat test001 _user.template
@rem call bat\gen-vcxproj-user-files-impl.bat test002 _user.template
@rem call bat\gen-vcxproj-user-files-impl.bat test003 _user.template
@rem call bat\gen-vcxproj-user-files-impl.bat test004 _user.template
@rem call bat\gen-vcxproj-user-files-impl.bat test005 _user.template
@rem call bat\gen-vcxproj-user-files-impl.bat test006 _user.template
@rem call bat\gen-vcxproj-user-files-impl.bat test007 _user.template
@rem call bat\gen-vcxproj-user-files-impl.bat test008 _user.template
