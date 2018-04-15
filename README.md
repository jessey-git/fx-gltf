# fx-gltf

A C++14/C++17 header-only library for simple, efficient, and robust serialization/deserialization of [glTF 2.0](https://www.khronos.org/gltf/)

## Features
* Complete support for required glTF 2.0 schema elements
    * External and embedded resource loading
    * .glb binary files
    * Serialization (Save) and Deserialization (Load) capability

* Modern C++14/C++17 support
    * Includes usage of std::string_view internally where appropriate for performance
    * Implemented using modern and safe syntax and methodologies

* Small, header-only library
    * ~1500 lines of generously spaced code including whitespace/comments
    * C++20 Module ready (does not leak preprocessor macros beyond its own file)

* Fast and efficient processing

## Usage and Integration

### Installation
* [`gltf.h`](https://github.com/jessey-git/fx-gltf/blob/master/include/fx/gltf.h) is the single required file available in the `include/fx` repo path.

  A typical installation will preserve the directory hierarchy: ```#include <fx/gltf.h>```

Planned: publishing to [vcpkg](https://github.com/Microsoft/vcpkg) for even easier installs within MSVC environments

### Dependencies
* [nlohmann::json](https://github.com/nlohmann/json) (must be referenceable using `#include <nlohmann/json.hpp`)

### Code

```C++
// Single header...
#include <fx/gltf.h>

// Loading...
fx::gltf::Document helmet = fx::gltf::LoadFromText("DamagedHelmet.gltf");

// Manipulation...
helmet.asset.generator = "My cool generator";
helmet.buffers.push_back(fx::gltf::Buffer{});
helmet.buffers.back().byteLength = 678;
helmet.buffers.back().uri = "buffer.bin";

// Saving...
fx::gltf::SaveAsText(helmet, "NewHelmet.gltf");
```

## Safety and Robustness

* Automated, roundtrip testing for all models inside [glTF-Sample-Models](https://github.com/KhronosGroup/glTF-Sample-Models)

| Model Type  | Status |
| ------------| ------ |
| .gltf files w/external resources  | 100% complete and passing  |
| .gltf files w/embedded resources  | 100% complete and passing (2 models excluded due to out-of-spec mimetypes)  |
| .glb files                        | 100% complete and passing  |

* Built-in protection against directory traversal when loading external resource URIs from malicious .gltf files
* Extensive testing of Base64 encoding and decoding
* Strict required vs. optional element loading and saving

* Developed using both clang-tidy and MSVC CppCoreCheck toolsets

## Performance
* Planned: show loading performance comparisons against other glTF loaders
* Planned: show graph of improvement when compiling in C++17 mode (std::string_view etc.)

## Execute unit tests

### Visual Studio

* File->Open->CMake
    * Point to the `test` repo path
* CMake->Rebuild All
* Inside Test Explorer -> Run All

### Command line

```Shell
$ cd test
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
$ ctest --output-on-failure -C [Debug or Release]
```

## Supported Compilers
* Microsoft Visual C++ 2017 15.6 (and possibly earlier)
* Clang 5.0+
* GCC 6.1+

Planned: Ship a C++20 Modules file in addition to the header

## Known Issues
### glTF 2.0 missing support
* Schema: ```Extension``` property bags on all types

### API
* Saving: Binary .glb files
* Saving: Providing options for external or embedded resources

### General (future)
* API: Make manipulation api a bit better by allowing easier creation of objects (C++20 will allow more intuitive aggregate struct initialization so maybe wait until then...)

## License

<img align="right" src="http://opensource.org/trademarks/opensource/OSI-Approved-License-100x137.png">

Licensed under the MIT License <http://opensource.org/licenses/MIT>.

Copyright (c) 2018 Jesse Yurkovich

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

* [Catch2](https://github.com/catchorg/Catch2) for testing
* [glTF-Sample-Models](https://github.com/KhronosGroup/glTF-Sample-Models) for glTF sample models. All 1.2gb of it...