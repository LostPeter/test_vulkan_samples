# author: LostPeter
# time:   2022-11-05

name_shader=${1}

folderGLSL="../../Bin/Assets/Shader/GLSL"
folderVulkan="../../Bin/Assets/Shader/Vulkan"
mkdir -p $folderVulkan

glslangValidator -V $folderGLSL/$name_shader -o $folderVulkan/$name_shader.spv