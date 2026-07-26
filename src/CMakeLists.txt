cmake_minimum_required (VERSION 3.15)

find_package(mongocxx CONFIG REQUIRED)
find_package(bsoncxx CONFIG REQUIRED)

add_library(mongodb SHARED
        Client.cpp Client.hpp
        Database.cpp Database.hpp
        Collection.cpp Collection.hpp
        Bulk.cpp Bulk.hpp
        ObjectID.cpp ObjectID.hpp
        Util.cpp Util.hpp
        MongoDB.cpp MongoDB.hpp
        Macros.hpp)

target_compile_features(mongodb PRIVATE cxx_std_23)

# Prefer the static driver targets (vcpkg x64-windows-static-md), fall back to shared.
target_link_libraries(mongodb PRIVATE
        $<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,mongo::mongocxx_static,mongo::mongocxx_shared>
        $<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,mongo::bsoncxx_static,mongo::bsoncxx_shared>)

if(WIN32)
    target_link_libraries(mongodb PRIVATE ws2_32 wsock32 bcrypt crypt32 dnsapi secur32)
endif()

target_link_libraries(mongodb PRIVATE gmod-module-base)

set_gmod_suffix_prefix(mongodb)
