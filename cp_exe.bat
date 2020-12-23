@if exist _out\x64\Debug\*.exe    copy _out\x64\Debug\*.exe    _deploy\x64\Debug\
@if exist _out\x64\Release\*.exe  copy _out\x64\Release\*.exe  _deploy\x64\Release\
@if exist _out\x86\Debug\*.exe    copy _out\x86\Debug\*.exe    _deploy\x86\Debug\
@if exist _out\x86\Release\*.exe  copy _out\x86\Release\*.exe  _deploy\x86\Release\
