set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Linux)

# Static libs get linked into a shared GMod module, so build them position-independent.
set(VCPKG_CMAKE_CONFIGURE_OPTIONS "-DCMAKE_POSITION_INDEPENDENT_CODE=ON")
