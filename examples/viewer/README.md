# glTF Viewer

An application for demonstrating the practical usage of [fx-gltf](https://github.com/jessey-git/fx-gltf) within the context of modern rendering APIs.

## Features
* DirectX 12

![screenshot](screenshots/screenshot00.png)
![screenshot](screenshots/screenshot01.png)
![screenshot](screenshots/screenshot02.png)
![screenshot](screenshots/screenshot03.png)

## Usage
```viewer.exe [--width: width] [--height: height] model```

## Design

| Entry Point      | Engine control | Device Specific Engine    |
| -----------------| -------------- | --------------------------|
| Win32Application | Engine         | D3DEngine or VulkanEngine |

## Supported Compilers
* Microsoft Visual C++ 2017 15.6+ (and possibly earlier)

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

* [d3dx12.h](https://github.com/Microsoft/DirectX-Graphics-Samples/tree/master/Libraries/D3DX12) containing helper structures and functions for D3D12
* [Clara](https://github.com/catchorg/Clara) for command-line parsing
