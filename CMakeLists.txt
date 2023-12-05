cmake_minimum_required(VERSION 3.28)
project(YukiD3D12)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Yuki)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Yuki-D3D12)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/EngineTester)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/Aura)
