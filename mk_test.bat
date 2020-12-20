@rem BAT tips and tricks - https://www.dostips.com/DtTipsStringManipulation.php#Snippets.Replace
@rem https://stackoverflow.com/questions/3068929/how-to-read-file-contents-into-a-variable-in-a-batch-file
@rem https://rsdn.org/article/winshell/batanyca.xml


@if "%1"=="" goto arg_not_taken

@set TEST=%1
@if exist vcxproj\%TEST%.vcxproj goto already_exist

@rem umba-mm-tpl -i vcxproj\_filters.template -o vcxproj\%TEST%.vcxproj.filters TEST_NAME=%TEST%
@rem umba-mm-tpl -i vcxproj\_user.template    -o vcxproj\%TEST%.vcxproj.user    TEST_NAME=%TEST%

@copy /Y vcxproj\_filters.template vcxproj\%TEST%.vcxproj.filters
@copy /Y vcxproj\_user.template    vcxproj\%TEST%.vcxproj.user
@copy /Y vcxproj\_vcxproj.template vcxproj\%TEST%.vcxproj
@copy /Y src\main\cpp.template     src\main\%TEST%.cpp

git add vcxproj\%TEST%.vcxproj.filters
git add vcxproj\%TEST%.vcxproj
git add src\main\%TEST%.cpp

@echo Replace %%(TEST_NAME)%% to your test name in files



@echo Done
@goto end

:arg_not_taken
@echo Argument not defined
@goto end

:already_exist
@echo Project already exists
@goto end

:end



