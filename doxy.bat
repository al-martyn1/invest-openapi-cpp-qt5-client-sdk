@if exist doc.generated rmdir /S /Q doc.generated
@mkdir doc.generated
@doxygen doxygen.cfg  > doxygen.log 2>doxygen_undocumented.log
