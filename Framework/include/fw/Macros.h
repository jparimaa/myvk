#pragma once

#include <vulkan/vulkan.h>

#include <cassert>
#include <iostream>

// clang-format off
#define VK_CHECK(f)                                                                                            \
{                                                                                                              \
    VkResult res = (f);                                                                                        \
    if (res != VK_SUCCESS)                                                                                     \
    {                                                                                                          \
        std::cerr << "Vulkan Error : VkResult is \"" << res << "\" in " << __FILE__ << ":" << __LINE__ << "\n" \
					<< #f << "\n";                                                                             \
		abort();																							   \
	}																										   \
}

#define CHECK(f)                                                                       \
{	                                                                                   \
    if (!(f))                                                                          \
    {                                                                                  \
        std::cerr << "Assert failure in " << __FILE__ << ":" << __LINE__ << "\n";	   \
        std::cerr << #f << "\n";                                                       \
        abort();			                                                           \
    }                                                                                  \
}
// clang-format on