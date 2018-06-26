# milkdrop-shader-converter

To build with CMake:

```bash
cd milkdrop-shader-converter

#out of source builds, please :)
mkdir build; cd build

#To build with a Makefile (default on Linux systems)
cmake .. && make

#To build instead with Ninja (if you have it installed)
cmake -G Ninja .. && ninja
```

On Ubuntu 16.04, I had to install the following packages
* libglew-dev
* bison
* flex
* freeglut3-dev

---

Unused include-what-you-use snippet for CMakeLists.txt:

```CMake
find_program(iwyu_path NAMES include-what-you-use iwyu)
if(iwyu_path)
    set(target_properties
        ${target_properties}
        CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})
else()
    message(WARNING "Couldn't find 'include-what-you-use'")
endif()

set_target_properties(
    MilkdropShaderConverter
    PROPERTIES
    ${target_properties}
    COMPILE_OPTIONS
    "${target_compile_options}")
```
