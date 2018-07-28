#pragma once

#include <vulkan/vulkan.h>

#include <cassert>
#include <iostream>

#define VK_CHECK(f)                                                                                        \
    {                                                                                                      \
        VkResult res = (f);                                                                                \
        if (res != VK_SUCCESS)                                                                             \
        {                                                                                                  \
            std::cerr << "Vulkan Error : VkResult is \"" << res << "\" in " << __FILE__ << ":" << __LINE__ \
                      << std::endl;                                                                        \
            assert(false);                                                                                 \
        }                                                                                                  \
    }

#define CHECK(f)                                                                           \
    {                                                                                      \
        if (!(f))                                                                          \
        {                                                                                  \
            std::cerr << "Assert failure in " << __FILE__ << ":" << __LINE__ << std::endl; \
            assert(false);                                                                 \
        }                                                                                  \
    }
