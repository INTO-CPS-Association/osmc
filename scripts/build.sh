#!/bin/bash
set -e

target=$1
install_name=$2
dockcross_url=$3

echo "Configuration Target=${target}, DependencyInstallName=${install_name}, Dockcross image url=${dockcross_url}"

repo=$(git rev-parse --show-toplevel)
current_dir=$(pwd)
cd $repo


working_dir=build/$target

mkdir -p $working_dir

script=$working_dir/${target}-dockcross


echo Creating env

docker run --rm $dockcross_url > $script
chmod +x $script


echo Build dependencies

echo Running CMake
./$script cmake -B$working_dir -H.

echo Compiling

./$script make -C$working_dir -j8
./$script make -C$working_dir/oosmc-fmu install
cd $current_dir

echo Done
