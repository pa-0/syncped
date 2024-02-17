# syncped

syncped offers a full featured source code text editor.

## Requirements

- [cmake](http://www.cmake.org/)
- [wex lib](https://github.com/antonvw/wex/)
- a `c++23` standard supporting compiler

## Building

```bash
git clone git@gitlab.kitware.com:antonvw/syncped.git
```

and for Linux, osx do
  `wex-build-gen.sh`
for Visual Studio do
  `mkdir build && cd build`
  `devenv wex.sln /build Release`,
and for mingw add `-G "MinGW Makefiles"` and do `mingw32-make`.

For Visual Studio 2019 you should possibly add
`-DCMAKE_INSTALL_PREFIX=c:\program files (x86)\wex`
