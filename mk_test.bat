@rem BAT tips and tricks - https://www.dostips.com/DtTipsStringManipulation.php#Snippets.Replace
@rem https://stackoverflow.com/questions/3068929/how-to-read-file-contents-into-a-variable-in-a-batch-file
@rem https://rsdn.org/article/winshell/batanyca.xml


@if "%1"=="" goto arg_not_taken

@set TEST=%1
@if exist "%~dp0\vcxproj\%TEST%.vcxproj" goto already_exist

@rem umba-mm-tpl -i "%~dp0\vcxproj\_filters.template" -o "%~dp0\vcxproj\%TEST%.vcxproj.filters" TEST_NAME=%TEST%
@rem umba-mm-tpl -i "%~dp0\vcxproj\_user.template"    -o "%~dp0\vcxproj\%TEST%.vcxproj.user"    TEST_NAME=%TEST%

@copy /Y "%~dp0\vcxproj\_filters.template" "%~dp0\vcxproj\%TEST%.vcxproj.filters"
@copy /Y "%~dp0\vcxproj\_user.template"    "%~dp0\vcxproj\%TEST%.vcxproj.user"
@copy /Y "%~dp0\vcxproj\_vcxproj.template" "%~dp0\vcxproj\%TEST%.vcxproj"
@copy /Y "%~dp0\src\main\_cpp.template"    "%~dp0\src\main\%TEST%.cpp"

git add "%~dp0\vcxproj\%TEST%.vcxproj.filters"
git add "%~dp0\vcxproj\%TEST%.vcxproj"
@rem git add -f "%~dp0\vcxproj\%TEST%.vcxproj.user"
git add "%~dp0\src\main\%TEST%.cpp"

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



