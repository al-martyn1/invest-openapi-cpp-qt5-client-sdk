@set GEN=_deploy\x64\Debug\md_gen.exe
@set YAML=..\pptrade\_invest-openapi\invest-openapi\src\docs\swagger-ui\swagger.yaml
@set TARGET=-
@set TARGET_FILE=.\src\invest_openapi\models_to_string.h
@rem .\src\generated_helpers
@set LOGBASENM=_run_logs\md_gen

@rem %GEN% %YAML% %TARGET%  root-node             2    > %LOGBASENM%-01-root-node.log           2>&1
@rem %GEN% %YAML% %TARGET%  openapi               5    > %LOGBASENM%-02-openapi.log             2>&1
@rem %GEN% %YAML% %TARGET%  info                  5    > %LOGBASENM%-03-info.log                2>&1
@rem %GEN% %YAML% %TARGET%  tags                  5    > %LOGBASENM%-04-tags.log                2>&1
@rem %GEN% %YAML% %TARGET%  paths                 5    > %LOGBASENM%-05-paths.log               2>&1
@rem %GEN% %YAML% %TARGET%  servers               5    > %LOGBASENM%-06-servers.log             2>&1
@rem %GEN% %YAML% %TARGET%  externalDocs          5    > %LOGBASENM%-07-externalDocs.log        2>&1
@rem %GEN% %YAML% %TARGET%  components            5    > %LOGBASENM%-08-components.log          2>&1
%GEN% %YAML% %TARGET%      components/schemas    15    > %LOGBASENM%-09-components-schemas.log  2>&1
%GEN% %YAML% %LOGBASENM%.h                             > %LOGBASENM%-10-generation.log          2>&1


@rem $(SolutionDir)\..\pptrade\_invest-openapi\invest-openapi\src\docs\swagger-ui\swagger.yaml