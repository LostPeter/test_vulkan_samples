@REM #****************************************************************************
@REM # test_vulkan_samples - Copyright (C) 2022 by LostPeter
@REM # 
@REM # Author: LostPeter
@REM # Time:   2022-11-05
@REM #
@REM # This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
@REM #****************************************************************************/

@echo off

set folderGLSL="..\..\Bin\Assets\Shader\GLSL"
set folderVulkan="..\..\Bin\Assets\Shader\Vulkan"
if exist %folderVulkan% (
    rmdir /S/Q %folderVulkan%
)
mkdir %folderVulkan%

echo "************** Shader Source .vert/.frag **************"
for /F %%i in ('Dir %folderGLSL%\*.* /B') do glslangValidator -V %folderGLSL%\%%i -o %folderVulkan%\%%i.spv
echo "************** Shader Compile .spv ********************"
for %%i in (%folderVulkan%\*.*) do echo %%i