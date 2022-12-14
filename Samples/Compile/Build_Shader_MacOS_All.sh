#****************************************************************************
# test_vulkan_samples - Copyright (C) 2022 by LostPeter
# 
# Author: LostPeter
# Time:   2022-11-05
#
# This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
#****************************************************************************/

dirSrc="../../Bin/Assets/Shader/GLSL"
dirDst="../../Bin/Assets/Shader/Vulkan"

rm -rf $dirDst
mkdir -p $dirDst

function read_dir() {
    for file in `ls $1`
    do 
        if [ -d $1"/"$file ]
        then
            read_dir $1"/"$file 
        else
            echo $1"/"$file 
        fi
    done
}

function compile_shader() {
    for file in `ls $1`
    do 
        if [ -d $1"/"$file ]
        then
            compile_shader $1"/"$file 
        else
            glslangValidator -V $1"/"$file -o $dirDst"/"$file.spv
        fi
    done
}

echo "************** Shader Source .vert/.frag **************"
# read_dir $dirSrc
{
    compile_shader $dirSrc
}
echo "************** Shader Compile .spv ********************"
read_dir $dirDst