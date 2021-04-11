call run_test.bat test024 msvc2017 x86 Release
call run_test.bat test025 msvc2017 x86 Release

copy ^
_run_logs\test024.log+^
_run_logs\test025.log ^
_run_logs\decimal_perf_test_results.txt

