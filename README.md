# Flim

Flim is a simulation and rendering project with a structured CMake-based build system. It includes support for shaders, textures, and various simulations.

## Setup and Development

### Prerequisites
To compile Flim, you need:
- [Mamba](https://mamba.readthedocs.io/en/latest/installation/mamba-installation.html) or [Conda](https://docs.conda.io/projects/conda/en/latest/user-guide/install/index.html)
- [Kokkos](https://github.com/kokkos/kokkos) compiled for your target architecture 

### Environment Setup
To set up the development environment, use the `setup-dev.sh` script. This script will install required dependencies using Mamba or Conda and setup the cmake `build/` folder.

```sh
$ ./setup-dev.sh
```

Alternatively, you can manually create the environment using `env.yml`:

```sh
$ mamba env create -f env.yml  # Or use conda instead of mamba
```

Activate the environment:
```sh
$ mamba activate flim-env  # Replace with `conda activate flim-env` if using Conda
```

## Building the Project
Flim uses CMake for building. To build the project, run:

```sh
$ mkdir -p build && cd build
$ cmake ..
$ make -j$(nproc)
```


## Contributing
Feel free to open issues or submit pull requests to improve Flim!


## Notes

If facing an issue with libpthread on archlinux while trying to compile the project, please check https://bbs.archlinux.org/viewtopic.php?id=244500.
