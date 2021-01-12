@set GEN=_deploy\x64\Debug\md_gen.exe
@set YAML=..\pptrade\_invest-openapi\invest-openapi\src\docs\swagger-ui\swagger.yaml
@set TARGET=-
@set TARGET_FILE=.\src\invest_openapi\model_to_strings_generated.h
@set LOGBASENM=_run_logs\md_gen
@set SCHEMA=src\invest_openapi\models_schema.ini

@rem %GEN% %YAML% %SCHEMA% %TARGET%  root-node             2    > %LOGBASENM%-01-root-node.log           2>&1
@rem %GEN% %YAML% %SCHEMA% %TARGET%  openapi               5    > %LOGBASENM%-02-openapi.log             2>&1
@rem %GEN% %YAML% %SCHEMA% %TARGET%  info                  5    > %LOGBASENM%-03-info.log                2>&1
@rem %GEN% %YAML% %SCHEMA% %TARGET%  tags                  5    > %LOGBASENM%-04-tags.log                2>&1
@rem %GEN% %YAML% %SCHEMA% %TARGET%  paths                 5    > %LOGBASENM%-05-paths.log               2>&1
@rem %GEN% %YAML% %SCHEMA% %TARGET%  servers               5    > %LOGBASENM%-06-servers.log             2>&1
@rem %GEN% %YAML% %SCHEMA% %TARGET%  externalDocs          5    > %LOGBASENM%-07-externalDocs.log        2>&1
@rem %GEN% %YAML% %SCHEMA% %TARGET%  components            5    > %LOGBASENM%-08-components.log          2>&1
%GEN% %YAML% %SCHEMA% %TARGET%      components/schemas    15    > %LOGBASENM%-09-components-schemas.log  2>&1
@rem %GEN% %YAML% %SCHEMA% %LOGBASENM%.h                             > %LOGBASENM%-10-generation.log          2>&1
%GEN% %YAML% %SCHEMA% %TARGET_FILE%                             > %LOGBASENM%-10-generation.log          2>&1



@rem $(SolutionDir)\..\pptrade\_invest-openapi\invest-openapi\src\docs\swagger-ui\swagger.yaml