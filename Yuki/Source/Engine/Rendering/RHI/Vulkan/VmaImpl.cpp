#include <spdlog/fmt/fmt.h>

namespace Yuki {
    inline static thread_local char s_FormatBuffer[1024]{0};
}

#define VMA_DEBUG_LOG(format, ...)\
    sprintf(Yuki::s_FormatBuffer, format __VA_OPT__(,) __VA_ARGS__);\
    Yuki::LogError("[VMA]: {}", Yuki::s_FormatBuffer)

#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
