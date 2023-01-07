#****************************************************************************
# test_vulkan_samples - Copyright (C) 2022 by LostPeter
# 
# Author: LostPeter
# Time:   2022-11-05
#
# This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
#****************************************************************************/

name_shader=${1}

folderGLSL="../../Bin/Assets/Shader/GLSL"
folderVulkan="../../Bin/Assets/Shader/Vulkan"
mkdir -p $folderVulkan

glslangValidator -V $folderGLSL/$name_shader -o $folderVulkan/$name_shader.spv