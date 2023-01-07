@REM #****************************************************************************
@REM # test_vulkan_samples - Copyright (C) 2022 by LostPeter
@REM # 
@REM # Author: LostPeter
@REM # Time:   2022-11-05
@REM #
@REM # This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
@REM #****************************************************************************/

set debug=%1

call ./Build_glfw_lib_MacOS.bat %debug%
call ./Build_glm_lib_Windows.bat
call ./Build_stb_lib_Windows.bat
call ./Build_tinyobjloader_lib_MacOS.bat