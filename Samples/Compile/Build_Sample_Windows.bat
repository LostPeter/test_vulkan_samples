@REM author: LostPeter
@REM time:   2022-11-05

@echo off
set name_sample=%1
set debug=%2

echo %name_sample%
echo %debug%

if "%debug%" == "debug" (
    set name_exe=%name_sample%"_d.exe"
) else (
    set name_exe=%name_sample%".exe"
)

@rem build folder
set build_folder="..\Build\Windows\"%name_sample%
if exist %build_folder% (
    rmdir /S/Q %build_folder%
)
mkdir %build_folder%

@rem Bin folder/file
set bin_folder="..\..\Bin\Windows"
if not exist %bin_folder% (
    mkdir %bin_folder%
)
set bin_file=%bin_folder%"\"%name_exe%
if exist %bin_file% (
    del /S/Q %bin_file%
)


cd ..
cd Build
cd Windows
cd %name_sample%

if "%debug%" == "debug" (
    cmake -DDEBUG=1 "../../../"%name_sample%"/"
    msbuild "%name_sample%".sln /p:configuration=debug
    copy /Y ".\Debug\"%name_sample%".exe" "..\..\..\..\Bin\Windows\"%name_exe%
) else (
    cmake "../../../"%name_sample%"/"
    msbuild "%name_sample%".sln /p:configuration=release
    copy /Y ".\Release\"%name_sample%".exe" "..\..\..\..\Bin\Windows\"%name_exe%
)


cd ..
cd ..
cd ..
cd Compile