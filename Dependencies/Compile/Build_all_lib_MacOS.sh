#****************************************************************************
# test_vulkan_samples - Copyright (C) 2022 by LostPeter
# 
# Author: LostPeter
# Time:   2022-11-05
#
# This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
#****************************************************************************/

debug=${1}

./Build_glfw_lib_MacOS.sh $debug
./Build_glm_lib_MacOS.sh
./Build_stb_lib_MacOS.sh
./Build_tinyobjloader_lib_MacOS.sh