## Dev

~~- install Conan(C++ package manager) **and its tools** eg `pip3 install conan_package_tools`
    - `conan profile new default --detect`
    - IMPORTANT `conan profile update settings.compiler.libcxx=libstdc++11 default`
    This is really important, without this you will get "undefined symbols" and other linker errors with dependencies(eg Protobuf).~~