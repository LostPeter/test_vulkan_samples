#****************************************************************************
# test_vulkan_samples - Copyright (C) 2022 by LostPeter
# 
# Author: LostPeter
# Time:   2022-11-05
#
# This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
#****************************************************************************/

name_sample=${1}
debug=${2}

rm -rf "../Build/MacOS/"$name_sample
mkdir -p "../Build/MacOS/"$name_sample

if [ "$debug" == "debug" ]; then
    rm -rf "../../Bin/MacOS/"$name_sample"_d"
else
    rm -rf "../../Bin/MacOS/"$name_sample
fi
mkdir -p "../../Bin/MacOS"


cd ..
cd Build
cd MacOS
cd $name_sample

if [ "$debug" == "debug" ]; then
    cmake -DDEBUG=1 ../../../$name_sample/
else
    cmake ../../../$name_sample/
fi
make

cd ..
cd ..
cd ..
cd Compile

if [ "$debug" == "debug" ]; then
    cp -rf "../Build/MacOS/$name_sample/$name_sample" "../../Bin/MacOS/"$name_sample"_d"
else
    cp -rf "../Build/MacOS/$name_sample/$name_sample" "../../Bin/MacOS"
fi