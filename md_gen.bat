@if not exist "%~dp0\_run_logs" mkdir "%~dp0\_run_logs"

@call "%~dp0\cp_exe.bat"

@set GEN=%~dp0\_deploy\x64\Debug\md_gen.exe
@set YAML=%~dp0\..\pptrade\_invest-openapi\invest-openapi\src\docs\swagger-ui\swagger.yaml
@set TARGET=-
@set TARGET_PATH=%~dp0\src\invest_openapi
@set LOGBASENM=%~dp0\_run_logs\md_gen
@set SCHEMA_PATH=%~dp0\src\invest_openapi

@rem "%GEN%" "%YAML%" "%SCHEMA%" %TARGET%  SQLITE root-node             2    > %LOGBASENM%-01-root-node.log           2>&1
@rem "%GEN%" "%YAML%" "%SCHEMA%" %TARGET%  SQLITE openapi               5    > %LOGBASENM%-02-openapi.log             2>&1
@rem "%GEN%" "%YAML%" "%SCHEMA%" %TARGET%  SQLITE info                  5    > %LOGBASENM%-03-info.log                2>&1
@rem "%GEN%" "%YAML%" "%SCHEMA%" %TARGET%  SQLITE tags                  5    > %LOGBASENM%-04-tags.log                2>&1
@rem "%GEN%" "%YAML%" "%SCHEMA%" %TARGET%  SQLITE paths                 5    > %LOGBASENM%-05-paths.log               2>&1
@rem "%GEN%" "%YAML%" "%SCHEMA%" %TARGET%  SQLITE servers               5    > %LOGBASENM%-06-servers.log             2>&1
@rem "%GEN%" "%YAML%" "%SCHEMA%" %TARGET%  SQLITE externalDocs          5    > %LOGBASENM%-07-externalDocs.log        2>&1
@rem "%GEN%" "%YAML%" "%SCHEMA%" %TARGET%  SQLITE components            5    > %LOGBASENM%-08-components.log          2>&1
"%GEN%" "%YAML%" "%SCHEMA%" %TARGET% SQLITE      components/schemas    15    > %LOGBASENM%-09-components-schemas.log  2>&1
@rem "%GEN% %YAML% %SCHEMA% %LOGBASENM%.h SQLITE                                   > %LOGBASENM%-10-generation.log          2>&1
"%GEN%" "%YAML%" "%SCHEMA_PATH%\models_schema_SQLITE.ini" "%TARGET_PATH%\model_to_strings_generated_SQLITE.h" SQLITE    > "%LOGBASENM%-10-generation.log"          2>&1



@rem $(SolutionDir)\..\pptrade\_invest-openapi\invest-openapi\src\docs\swagger-ui\swagger.yaml