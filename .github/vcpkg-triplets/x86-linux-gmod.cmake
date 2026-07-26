set(VCPKG_TARGET_ARCHITECTURE x86)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Linux)

# 32-bit build on a 64-bit host: compile every dependency with -m32.
set(VCPKG_C_FLAGS "-m32")
set(VCPKG_CXX_FLAGS "-m32")

# Static libs get linked into a shared GMod module, so build them position-independent.
set(VCPKG_CMAKE_CONFIGURE_OPTIONS "-DCMAKE_POSITION_INDEPENDENT_CODE=ON")
