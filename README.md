# PolyXB

PolyXB is a polyhedral-based compilation framework for in-memory neural network accelerators.

## Get submodules

Before start, get the submodule `isl-polyxb` by:

```bash
git submodule update --init
```

## Build

### Build LLVM

LLVM is one of the dependencies of pet. For other dependencies of pet, see [README of pet](https://repo.or.cz/w/pet.git).

According to the documentation of pet, the preferred version of LLVM is 9.x. We recommand that you install it to a non-default location by using `-DCMAKE_INSTALL_PREFIX=/path/to/llvm/installation` when invoking cmake.

```bash
git clone https://github.com/llvm/llvm-project.git
cd llvm-project
git checkout release/9.x
mkdir build
cd build
cmake -DLLVM_ENABLE_PROJECTS=clang -DCMAKE_INSTALL_PREFIX=/path/to/llvm/installation -G "Unix Makefiles" ../llvm
make # -j is recommended
make install
```

### Build isl-polyxb

It is a customized version of isl. We recommend that you install it to a non-default location by using `--prefix=/path/to/isl/installation` during configuration.

```bash
cd isl-polyxb
./autogen.sh
./configure --prefix=/path/to/isl/installation
make
make install
```

### Build pet

We use the official version of pet.  We recommend that you install it to a non-default location by using `--prefix=/path/to/pet/installation` during configuration. Also, we need to build pet with the installed isl-polyxb by using `--with-isl-prefix=/path/to/isl/installation`during configuration.

```bash
git clone git://repo.or.cz/pet.git
cd pet
./autogen.sh
./configure --prefix=/path/to/pet/installation --with-isl-prefix=/path/to/isl/installation
make
```

### Configure environment variables

Beforing going further, configure the environment variables as follows:

```bash
export PATH=/path/to/llvm/installation/bin:$PATH
export LD_LIBRARY_PATH=/path/to/llvm/installation/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/path/to/pet/installation/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/path/to/isl/installation/lib:$LD_LIBRARY_PATH
```

### Build PolyXB

Current we do not install PolyXB to some locations in the system. It is built and used inside the project root directory.

Before building PolyXB, please first edit the Makefile in the project root directory. Specifically, modify the value of `ISL_PREFIX`,`PET_PREFIX`, and `LLVM_PREFIX` to the actual value used during building of these libs, i.e. `/path/to/isl/installation`, `/path/to/pet/installation`, and `/path/to/llvm/installation`.

After editting the Makefile, simply run:

```bash
make
```

The executable file named `polyxb`should have been built in the project root directory.

To check the building, run:

```bash
./polyxb -t benchmarks/mv.c
```

The output file named `mv.polyxb.c`should be located in the project root directory.

You can also specify the output file path and name by `-o` argument, just like gcc:

```bash
./polyxb -t -o coolname.c benchmarks/mv.c
```

## Run

The basic use of polyxb is:

```bash
./polyxb <options> [-o <output>] input.c
```

Specifying output  filename is optional. The available arguments of polyxb are summaried as follows:

| argument   | description                                                  |
| ---------- | ------------------------------------------------------------ |
| -t         | enable tiling, tiling sizes are specified with -M -N -K      |
| -M, -N, -K | specify tiling sizes, use three for MM, use -M and -N for MV |
| -p         | enable pipeline generation, has no effect when there is only one operator |
| -n         | number of PEs                                                |
| -c         | enable coarse-grained generation                             |

## Test

You can test the generation results by comparing the results of input and output source code:

```bash
cd test
make
```

After the exectution completes, the results of input source code are `ans/<kernel>.ans`, and the results of output source code are `<kernel>.txt`. The corresponding results can be compared with `diff`.

