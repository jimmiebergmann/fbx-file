# fbx-file

[![Build Status](https://travis-ci.org/jimmiebergmann/easy-fbx.svg?branch=master)](https://travis-ci.org/jimmiebergmann/easy-fbx) [![](https://img.shields.io/badge/license-MIT-brightgreen.svg)](https://github.com/jimmiebergmann/Guier/blob/master/LICENSE.md)  

fbx-file is an STL-like library for reading and writing FBX binary files.

## Quickstart
```cpp
Fbx::Record file;

// Read
file.read("../models/blender-default.fbx");
auto vertices = *(*(*file.find("Objects"))->find("Geometry"))->find("Vertices");
auto coords = vertices->properties().front()->raw();

// Write
auto customRecord = *file.insert(new Fbx::Record("My custom root record"));
customRecord->properties().insert(new Fbx::Property(std::string("This is a string property")));
file.write("../bin/blender-custom.fbx");
```

## Documentation
~~See [fbx.hpp](https://github.com/jimmiebergmann/easy-fbx/blob/master/fbx.hpp).~~ Nope, not yet.
## Examples
See [examples](https://github.com/jimmiebergmann/easy-fbx/blob/master/examples).

## Build
##### Linux
```
$ make
```
##### Windows
Open builds/fbx.sln and compile solution.