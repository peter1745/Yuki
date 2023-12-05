project(Yuki-D3D12 LANGUAGES CXX)

set(YUKI_D3D12_MODULE_FILES
        Source/D3D12RHI.ixx
		Source/D3D12RHIHandles.ixx
		Source/D3D12Context.ixx
		Source/D3D12Queue.ixx
		Source/D3D12Fence.ixx
		Source/D3D12Swapchain.ixx
		Source/D3D12CommandList.ixx
		Source/D3D12ShaderLibrary.ixx
		Source/D3D12GraphicsPipeline.ixx)

set(YUKI_D3D12_MODULE_IMPL_FILES)

# Ugly hack to get CLion to allow us to add new files into our file lists
# CLion has to see them used in a target function (even one that never runs) in order to pick up on them
if (false)
    add_library(${PROJECT_NAME} ${YUKI_D3D12_MODULE_FILES} ${YUKI_D3D12_MODULE_IMPL_FILES})
else()
    add_library(${PROJECT_NAME})
    target_sources(${PROJECT_NAME} PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES ${YUKI_D3D12_MODULE_FILES})
    target_sources(${PROJECT_NAME} PRIVATE ${YUKI_D3D12_MODULE_IMPL_FILES})
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
		../ThirdParty/AgilitySDK/include/
		../ThirdParty/DXC/inc/)

target_link_directories(${PROJECT_NAME} PUBLIC ../ThirdParty/DXC/lib/x64/)

target_link_libraries(${PROJECT_NAME} PUBLIC
		Aura
		Yuki
		D3d12
		DXGI
		dxcompiler)
