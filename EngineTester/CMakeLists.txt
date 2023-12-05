project(EngineTester LANGUAGES CXX)

set(ENGINE_TESTER_MODULE_FILES Source/Main.ixx)
set(ENGINE_TESTER_MODULE_IMPL_FILES)

# Ugly hack to get CLion to allow us to add new files into our file lists
# CLion has to see them used in a target function (even one that never runs) in order to pick up on them
if (false)
    add_executable(${PROJECT_NAME} ${ENGINE_TESTER_MODULE_FILES} ${ENGINE_TESTER_MODULE_IMPL_FILES})
else()
    add_executable(${PROJECT_NAME})
    target_sources(${PROJECT_NAME} PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES ${ENGINE_TESTER_MODULE_FILES})
    target_sources(${PROJECT_NAME} PRIVATE ${ENGINE_TESTER_MODULE_IMPL_FILES})
endif()

target_link_directories(${PROJECT_NAME} PUBLIC ../ThirdParty/DXC/lib/x64/)

target_link_libraries(${PROJECT_NAME} PUBLIC
        Yuki
        Yuki-D3D12
        D3d12
        DXGI
        dxcompiler)
