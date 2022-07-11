## Dev

- install Conan(C++ package manager)
    - `conan profile update settings.compiler.libcxx=libstdc++11 default`
    This is really important, without this you will get "undefined symbols" and other linker errors with dependencies(eg Protobuf).