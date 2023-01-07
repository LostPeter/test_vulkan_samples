@REM #****************************************************************************
@REM # test_vulkan_samples - Copyright (C) 2022 by LostPeter
@REM # 
@REM # Author: LostPeter
@REM # Time:   2022-11-05
@REM #
@REM # This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
@REM #****************************************************************************/

@echo off

set name_shader=%1

set folderGLSL="..\..\Bin\Assets\Shader\GLSL"
set folderVulkan="..\..\Bin\Assets\Shader\Vulkan"
if not exist %folderVulkan% (
    mkdir %folderVulkan%
)

glslangValidator -V %folderGLSL%\%name_shader% -o %folderVulkan%\%name_shader%.spv