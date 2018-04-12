# fx-gltf

A C++14/C++17 header-only library for simple, efficient, and robust serialization/deserialization of [glTF 2.0](https://www.khronos.org/gltf/)

## Features
* Extensive glTF 2.0 schema support
* Small, header-only library (~1200 lines of generously spaced code including whitespace/comments)
* C++14 and C++17 support (including the use of std::string_view where appropriate)
* Serialization and Deserialization support

## Usage and Integration

### Installation
* [`gltf.h`](https://github.com/jessey-git/fx-gltf/blob/master/include/fx/gltf.h) is the single required file available in the `include/fx` repo path.

* Planned: get published to [vcpkg](https://github.com/Microsoft/vcpkg) for easy install with MSVC environments

### Dependencies
* [nlohmann::json](https://github.com/nlohmann/json) (must be referenceable using `#include <nlohmann/json.hpp`)

### Code
```cpp
// Single header...
#include <fx/gltf.h>

// Loading...
fx::gltf::Document buggyModel = fx::gltf::LoadFromText("Buggy.gltf");

// Manipulation...
buggyModel.asset.generator = "My cool generator";
buggyModel.buffers.push_back(fx::gltf::Buffer{});
buggyModel.buffers.back().byteLength = 678;
buggyModel.buffers.back().uri = "buffer.bin";

// Saving...
#include <nlohmann/json.hpp>

nlohmann::json newModel = buggyModel;

std::ofstream outputFile("example.gltf");
outputFile << newModel;
```

## Safety and Robustness
* Prevention of directory traversal when loading Data URIs from malicious .gltf files
* Strict required vs. optional element loading and saving
* Zero clang-tidy and MSVC CppCoreCheck violations

* Started: Automated, roundtrip testing for all models inside [glTF-Sample-Models](https://github.com/KhronosGroup/glTF-Sample-Models)

## Performance
* Planned: compare with [TinyGLTF](https://github.com/syoyo/tinygltf)
* Planned: show graph of improvement when compiling in C++17 mode (std::string_view etc.)

## Execute unit tests
* Planned: show how to build and run the tests

## Supported Compilers
* Microsoft Visual C++ 2017 15.6 (and possibly earlier)

* Planned: Clang 6.0

## Missing or Potential Features
### glTF 2.0 support
* Loading/Saving: Sparse Accessors
* Loading/Saving: Base64 encoded data URIs
* Loading/Saving: Binary .glb loading and processing

* Saving: Data URIs

### General (future)
* Add a convenience method for saving models (mirroring what is done for loading)
* Make manipulation api a bit better by allowing easier creation of objects (C++20 will allow more intuitive aggregate struct initialization so maybe wait until then...)

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
