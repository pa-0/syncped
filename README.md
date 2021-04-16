# syncped

syncped offers a full featured source code text editor.

## Requirements

- [cmake](http://www.cmake.org/)
- [wex lib](https://github.com/antonvw/wex/)
- a `c++20` standard supporting compiler

## Building

```bash
git clone git@gitlab.kitware.com:antonvw/syncped.git
mkdir build && cd build
cmake .. && make
```

When wex is build as a shared lib, add
`-DwexBUILD_SHARED=ON`

For Visual Studio 2019 you should possibly add
`-DCMAKE_INSTALL_PREFIX=c:\program files (x86)\wex`
