echo ------------------------------------------------------------------------------------------------------------------------------------------------------------------------ >_run_logs\sep.txt

call run_test.bat test024 msvc2017 x64 Release
call run_test.bat test025 msvc2017 x64 Release

copy ^
_run_logs\test024.log+^
_run_logs\sep.txt+^
_run_logs\test025.log+^
_run_logs\sep.txt ^
_run_logs\decimal_perf_test_results.txt

