@REM author: LostPeter
@REM time:   2022-11-05

set name="stb-0.02"

set include_folder="..\Include\Windows\"%name%
if exist %include_folder% (
    rmdir /S/Q %include_folder%
)
mkdir %include_folder%

xcopy /S /E /Y /F "..\Sources\%name%\*.h" %include_folder%"\"