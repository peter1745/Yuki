project(Yuki LANGUAGES CXX)

set(YUKI_MODULE_FILES
        Source/Engine/Core/Core.ixx
        Source/Engine/Core/App.ixx
        Source/Engine/Core/EngineTime.ixx
        Source/Engine/Core/Windowing.ixx

        Source/Engine/Rendering/Rendering.ixx
        Source/Engine/Rendering/RHI.ixx)

set(YUKI_MODULE_IMPL_FILES)

if (WIN32)
    list(APPEND YUKI_MODULE_FILES
            Source/Platform/Windows/WindowsCommon.ixx
            Source/Platform/Windows/WindowingImpl.ixx)
endif()

# Ugly hack to get CLion to allow us to add new files into our file lists
# CLion has to see them used in a target function (even one that never runs) in order to pick up on them
if (false)
    add_library(${PROJECT_NAME} ${YUKI_MODULE_FILES} ${YUKI_MODULE_IMPL_FILES})
else()
    add_library(${PROJECT_NAME})
    target_sources(${PROJECT_NAME} PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES ${YUKI_MODULE_FILES})
    target_sources(${PROJECT_NAME} PRIVATE ${YUKI_MODULE_IMPL_FILES})
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC Aura)
