#****************************************************************************
# test_vulkan_samples - Copyright (C) 2022 by LostPeter
# 
# Author: LostPeter
# Time:   2022-11-05
#
# This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
#****************************************************************************/

debug=${1}

name=glfw-3.3.6
if [ "$debug" == "debug" ]; then
    name_project=$name"_d"
    name_lib="lib"$name"_d.a"
else
    name_project=$name
    name_lib="lib"$name".a"
fi

rm -rf "../Build/MacOS/"$name_project
mkdir -p "../Build/MacOS/"$name_project

rm -rf "../Lib/MacOS/"$name_lib
mkdir -p "../Lib/MacOS"

cd ..
cd Build
cd MacOS
cd $name_project

#lib
if [ "$debug" == "debug" ]; then
    cmake -DDEBUG=1 -DCMAKE_BUILD_TYPE=Debug ../../../Sources/$name/
else
    cmake ../../../Sources/$name/
fi
make

cd ..
cd ..
cd ..
cd Compile

cp -rf "../Build/MacOS/"$name_project"/src/libglfw3.a" "../Lib//MacOS/"$name_lib

folderSrc="../Sources/$name/include/GLFW"
folderDst="../Include/MacOS/"$name"/GLFW"
rm -rf $folderDst
mkdir -p $folderDst

for file in ${folderSrc}/*.h
do 
    cp -rf $file $folderDst
done