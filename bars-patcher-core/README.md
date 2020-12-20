## BARS Patcher core

This is the core of the automatic BARS patcher.

### Including

You only need to include the [bars-patcher.h](bars-patcher.h) file.

The code in this file uses platform-specific code, you'll also need to define BARSPATCHER_VERSION_* for the correct platform of your software.

Supported platforms:
- BARSPATCHER_VERSION_PC - Standard POSIX system.

Please note that this code also uses some C++ features and should be included from C++ code.

### Usage

To use this in your own software, you only need to call the barspatcher_run function, and optionally you can also use barspatcher_getErrorString and barspatcher_getVersionString.

See the [bars-patcher.h](bars-patcher.h) file itself for details, and see the [command-line program](/pc/main.cpp) for a simple reference implementation.
