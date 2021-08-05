cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_CXX_STANDARD 14)

aux_source_directory(lua LUA_SRC_LIST)
aux_source_directory(bridge ANDROID_BRIDGE_SRC_LIST)
add_library(qLua SHARED ${LUA_SRC_LIST} ${ANDROID_BRIDGE_SRC_LIST} lib/qLua.cpp)
target_include_directories(qLua PRIVATE bridge lua lib)
target_link_libraries(qLua android log)