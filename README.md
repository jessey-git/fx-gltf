# fx-gltf

A C++14/C++17 header-only library for simple, efficient, and robust serialization/deserialization of [glTF 2.0](https://www.khronos.org/gltf/)

[![Build Status](https://travis-ci.com/jessey-git/fx-gltf.svg?branch=master)](https://travis-ci.com/jessey-git/fx-gltf)
[![Build status](https://ci.appveyor.com/api/projects/status/hqrjm0fweyc0dod6/branch/master?svg=true)](https://ci.appveyor.com/project/jessey-git/fx-gltf/branch/master)
[![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/jessey-git/fx-gltf/master/LICENSE)
[![GitHub Releases](https://img.shields.io/github/release/jessey-git/fx-gltf.svg)](https://github.com/jessey-git/fx-gltf/releases)

## Features
* Complete support for required glTF 2.0 schema elements
    * Full serialization (save) and deserialization (load) capability
    * External and embedded resource loading
    * Binary .glb files
    * Load as text, save as binary etc.

* Modern C++14/C++17 support
    * Includes usage of std::string_view internally where appropriate for performance
    * Implemented using modern and safe syntax and methodologies

* Small, header-only library
    * <2000 lines of high-level, straightforward, generously spaced code including whitespace/comments
    * C++20 Module ready (does not leak preprocessor defines/macros beyond its own file)

* Fast, Efficient, and Safe processing
    * [Safety and Robustness](#safety-and-robustness)
    * [Performance](#performance)

* Useful examples
    * DirectX 12 enabled [viewer](examples/viewer)

## Usage and Integration

### Installation
* [`gltf.h`](https://github.com/jessey-git/fx-gltf/blob/master/include/fx/gltf.h) is the single required file available in the `include/fx` repo path.

  A typical installation will preserve the directory hierarchy: ```#include <fx/gltf.h>```

### Dependencies
* [nlohmann::json](https://github.com/nlohmann/json) (must be referenceable using `#include <nlohmann/json.hpp>`)

### Code

Example: General usage

```C++
// Single header...
#include <fx/gltf.h>

// Loading...
fx::gltf::Document helmet = fx::gltf::LoadFromText("DamagedHelmet.gltf");

// Manipulation...
helmet.asset.generator = "My cool generator";
helmet.buffers.push_back(fx::gltf::Buffer{});
helmet.buffers.back().byteLength = 678;
helmet.buffers.back().data = ...;
helmet.buffers.back().SetAsEmbeddedResource();

// Saving back as text...
fx::gltf::Save(helment, "NewHelmet.gltf", false);
```

Example: Placing a quota on how large files and buffers can be; e.g. when loading files from potentially hostile sources

```C++
#include <fx/gltf.h>

// Load at most 3 buffers in total, each as large as 8mb...
// additionally, place a quota on the file size as well
fx::gltf::ReadQuotas readQuotas{};
readQuotas.MaxBufferCount = 3;                     // default: 8
readQuotas.MaxBufferByteLength = 8 * 1024 * 1024;  // default: 32mb
readQuotas.MaxFileSize = 8 * 1024 * 1024;          // default: 32mb (applies to binary .glb only)

fx::gltf::Document docFromInternet = fx::gltf::LoadFromBinary("untrusted.glb", readQuotas);
```

### Applied Integration
See the DirectX 12 enabled [viewer](examples/viewer) example for a demonstration of how to leverage ```fx-gltf``` in a full application context.

## Safety and Robustness

* Robust automated testing

    * Roundtrip testing for all models inside [glTF-Sample-Models](https://github.com/KhronosGroup/glTF-Sample-Models)
    * Testing of Base64 encoding and decoding routines, including invalid Base64 inputs
    * Strict required vs. optional element loading and saving

| Model Type  | Status: glTF-Sample-Models |
| ------------| -------------------------- |
| .gltf files w/external resources  | 100% complete and passing  |
| .gltf files w/embedded resources  | 100% complete and passing (2 models excluded due to out-of-spec mimetypes)  |
| .gltf files w/Draco extension     | 100% complete and passing  |
| .glb files                        | 100% complete and passing  |

* Safety
    * Built-in protection against directory traversal when loading external resource URIs from malicious files
    * Enforced quotas on maximum files sizes and buffers to prevent DOS's from malicious files

* Developed using both clang-tidy and MSVC CppCoreCheck toolsets

## Performance
* Faster than other full-featured loaders

  * Improvement ranges from 1.1x to 2x (!) faster depending on the model.

```
-------------------------------------------------------------------------------------
Benchmark                                               Time           CPU Iterations
-------------------------------------------------------------------------------------
fxgltf_Test/External_Box_mean                         261 us        261 us       2358
fxgltf_Test/External_Box_median                       253 us        258 us       2358
fxgltf_Test/External_Box_stddev                        27 us         28 us       2358
tinygltf_Test/External_Box_mean                       321 us        319 us       2133
tinygltf_Test/External_Box_median                     312 us        308 us       2133
tinygltf_Test/External_Box_stddev                      20 us         20 us       2133

fxgltf_Test/Embedded_Box_mean                         192 us        192 us       4073
fxgltf_Test/Embedded_Box_median                       192 us        188 us       4073
fxgltf_Test/Embedded_Box_stddev                        14 us         16 us       4073
tinygltf_Test/Embedded_Box_mean                       274 us        273 us       3200
tinygltf_Test/Embedded_Box_median                     281 us        278 us       3200
tinygltf_Test/Embedded_Box_stddev                      30 us         30 us       3200

fxgltf_Test/External_2CylinderEngine_mean            6503 us       6563 us        100
fxgltf_Test/External_2CylinderEngine_median          7065 us       7188 us        100
fxgltf_Test/External_2CylinderEngine_stddev           900 us        931 us        100
tinygltf_Test/External_2CylinderEngine_mean          8664 us       8681 us         90
tinygltf_Test/External_2CylinderEngine_median        8490 us       8507 us         90
tinygltf_Test/External_2CylinderEngine_stddev        1034 us       1005 us         90

fxgltf_Test/Embedded_2CylinderEngine_mean           72760 us      72727 us         11
fxgltf_Test/Embedded_2CylinderEngine_median         74147 us      73864 us         11
fxgltf_Test/Embedded_2CylinderEngine_stddev          8219 us       8246 us         11
tinygltf_Test/Embedded_2CylinderEngine_mean        110179 us     110268 us          7
tinygltf_Test/Embedded_2CylinderEngine_median      109267 us     109375 us          7
tinygltf_Test/Embedded_2CylinderEngine_stddev        5324 us       6431 us          7

fxgltf_Test/External_ReciprocatingSaw_mean          12221 us      12333 us         56
fxgltf_Test/External_ReciprocatingSaw_median        11875 us      11998 us         56
fxgltf_Test/External_ReciprocatingSaw_stddev          717 us        666 us         56
tinygltf_Test/External_ReciprocatingSaw_mean        17273 us      17313 us         50
tinygltf_Test/External_ReciprocatingSaw_median      18154 us      18125 us         50
tinygltf_Test/External_ReciprocatingSaw_stddev       2095 us       2205 us         50

fxgltf_Test/Embedded_ReciprocatingSaw_mean         143724 us     144792 us          6
fxgltf_Test/Embedded_ReciprocatingSaw_median       147001 us     148438 us          6
fxgltf_Test/Embedded_ReciprocatingSaw_stddev        14070 us      14096 us          6
tinygltf_Test/Embedded_ReciprocatingSaw_mean       211632 us     210417 us          3
tinygltf_Test/Embedded_ReciprocatingSaw_median     206052 us     208333 us          3
tinygltf_Test/Embedded_ReciprocatingSaw_stddev      20525 us      21663 us          3
```

* More memory efficient than other full-featured loaders

  * Greater than 50% reduction in number of allocations across a variety of models. Sometimes as high as 70%!

  * Greater than 40% reduction in total allocation size across a variety of models. Sometimes as high as 67%!

```
----------------------------------------------------------------------------------------------------------
Benchmark                                        Time           CPU Iterations TotalNewCalls TotalNewSize
----------------------------------------------------------------------------------------------------------
fxgltf_Test/External_Box                       222 us          0 us          1        201    12.646k
tinygltf_Test/External_Box                     664 us          0 us          1        428    36.709k

fxgltf_Test/Embedded_Box                       660 us          0 us          1        217    19.958k
tinygltf_Test/Embedded_Box                     670 us          0 us          1        452    49.667k

fxgltf_Test/External_2CylinderEngine         11398 us          0 us          1     4.915k   2.15445M
tinygltf_Test/External_2CylinderEngine       11598 us      15625 us          1    14.942k    5.1615M

fxgltf_Test/Embedded_2CylinderEngine         57782 us      62500 us          1     4.968k   23.0764M
tinygltf_Test/Embedded_2CylinderEngine       80368 us      78125 us          1    15.021k   43.5535M

fxgltf_Test/External_ReciprocatingSaw        17415 us      15625 us          1     9.784k    4.2078M
tinygltf_Test/External_ReciprocatingSaw      13825 us      15625 us          1    31.806k   10.5618M

fxgltf_Test/Embedded_ReciprocatingSaw       126205 us     125000 us          1      9.84k   42.5873M
tinygltf_Test/Embedded_ReciprocatingSaw     236703 us     234375 us          1    31.889k   79.4386M
```

* Using C++17 yields some additional benefits over C++14

  * Total number of allocations drops 2-3% and total allocation size drops 1% for external resources and 5-10% for embedded resources!

C++14

```
--------------------------------------------------------------------------------------------------------
Benchmark                                      Time           CPU Iterations TotalNewCalls TotalNewSize
--------------------------------------------------------------------------------------------------------
fxgltf_Test/External_Box                     648 us          0 us          1        206    12.806k
fxgltf_Test/Embedded_Box                     608 us          0 us          1        223        21k
fxgltf_Test/External_2CylinderEngine       11017 us      15625 us          1     5.052k   2.15883M
fxgltf_Test/Embedded_2CylinderEngine       54864 us      62500 us          1     5.106k   25.4737M
fxgltf_Test/External_ReciprocatingSaw      10434 us          0 us          1    10.045k   4.21615M
fxgltf_Test/Embedded_ReciprocatingSaw     106756 us     109375 us          1    10.102k    47.258M
```

C++17

```
--------------------------------------------------------------------------------------------------------
Benchmark                                      Time           CPU Iterations TotalNewCalls TotalNewSize
--------------------------------------------------------------------------------------------------------
fxgltf_Test/External_Box                     236 us          0 us          1        201    12.646k
fxgltf_Test/Embedded_Box                     193 us          0 us          1        217    19.958k
fxgltf_Test/External_2CylinderEngine        6076 us      15625 us          1     4.915k   2.15445M
fxgltf_Test/Embedded_2CylinderEngine       63823 us      78125 us          1     4.968k   23.0764M
fxgltf_Test/External_ReciprocatingSaw      17033 us      15625 us          1     9.784k    4.2078M
fxgltf_Test/Embedded_ReciprocatingSaw     122586 us     109375 us          1      9.84k   42.5873M
```

## Execute unit tests

### Visual Studio

* File->Open->CMake
    * Point to the `test` repo path
* CMake->Rebuild All
* Inside Test Explorer -> Run All

### Command line

```Shell
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
$ ctest --output-on-failure -C [Debug or Release]
```

## Supported Compilers
* Microsoft Visual C++ 2017 15.3+ (and possibly earlier)
* Clang 5.0+
* GCC 6.1+

## Known Issues and TODOs
### glTF 2.0 support
* No known issues or missing features

### API
* No known issues

### Packaging
 * TODO: Publish to [vcpkg](https://github.com/Microsoft/vcpkg) for easier package maintenance within MSVC environments
 * TODO: Ship a C++20 Modules file in addition to the header

### General (future)
* API: Improvement: Make creation of objects easier when building documents by hand (C++20 will allow aggregate struct initialization so maybe that's all that is needed...)

## License

<img align="right" src="http://opensource.org/trademarks/opensource/OSI-Approved-License-100x137.png">

Licensed under the MIT License <http://opensource.org/licenses/MIT>.

Copyright (c) 2018-2021 Jesse Yurkovich

Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## Used third-party tools

This software would not be possible without the help of these great resources. Thanks a lot!

* [nlohmann::json](https://github.com/nlohmann/json) for JSON serialization
* [Catch2](https://github.com/catchorg/Catch2) for testing
* [glTF-Sample-Models](https://github.com/KhronosGroup/glTF-Sample-Models) for glTF sample models. All 1.2gb of it...