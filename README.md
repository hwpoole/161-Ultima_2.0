# Introduction

Ultima 2.0 is a simulated cooperative multi-tasking operating system.

## Building and running

The project uses CMake for its build system. In any given phase's folder, you
will find a `CMakeLists.txt` document. Inside of that phase's folder, you
should run this command:

```bash
mkdir build/ && cd build/ && cmake .. -G Ninja && ninja && ./Ultima
```

This does the following:

1. Makes `build/`
2. Change directory to `build/`
3. Calls `cmake`, pointing it to `../CMakeLists.txt` and specifying `Ninja` as the generator.
4. Uses `ninja` to build all targets.
5. `./Ultima` runs the 161-Ultima 2.0 Demo Program
