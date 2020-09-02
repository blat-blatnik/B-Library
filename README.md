# B-Library

A collection of personal single-header C/C++ libraries similar to the [stb libraries](https://github.com/nothings/stb). They are all public domain (alternatively MIT licenced).

library                      | latest version | category    | language | LoC  | description
:--------------------------- |:--------------:|:-----------:|:--------:| ----:|:----------------------------------------------
**[bmath.hpp](./bmath.hpp)** | `0.22`         | math        | C++03    | 3518 | type generic 2, 3 and 4D vector, matrix and quaternion algebra - alternative to [GLM](https://glm.g-truc.net/0.9.9/index.html)
**[bmem.h](./bmem.h)**       | `0.2`          | utility     | C99      |  598 | quick & dirty memory leak-checking and temporary storage implementation
**[bdebug.h](./bdebug.h)**   | `1.0`          | utility     | C99      |  263 | assertion macro and logging function
**[bfile.h](./bfile.h)**     | `0.1`          | utility     | C99      |  259 | linux/windows file utilities - dynamically track file changes
**[brng.h](./brng.h)**       | `0.9`          | utility     | C99      |  212 | simple [PCG generator](https://www.pcg-random.org/index.html) implementation

All C libraries will compile as C++.

All libraries rely only on the standard library and so they should be multi-platform. The exception to this is [bfile.h](./bfile.h) which relies on the OS-specific `sys/stat.h` header.
