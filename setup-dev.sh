#!/bin/sh

cd $(dirname $0)

ENV_NAME="flim"
ENV_YML="debug-env.yml"

eval "$(conda shell.bash hook)"
conda env list | grep $ENV_NAME >/dev/null
if [ $? -ne 0 ]; then
  echo "CREATING ENV '${ENV_NAME}' USING ENV '$ENV_YML'"
  conda env create -n $ENV_NAME -f $ENV_YML 
  conda activate $ENV_NAME
  conda env config vars set VK_LAYER_PATH=${CONDA_PREFIX}/share/vulkan/explicit_layer.d
fi

conda activate $ENV_NAME

cmake . -B build -DCMAKE_BUILD_TYPE=Debug
rm compile_commands.json
rm -rf ./build/textures
ln -s ./textures/ ./build/textures/
ln -s ./build/compile_commands.json compile_commands.json

echo "Setup DONE"
echo "You can now compile the project using:"
echo "\$ conda activate flim"
echo "\$ cd build"
echo "\$ make -j"
echo ""
echo "The project can then be run using:"
echo "\$ ./engine"
