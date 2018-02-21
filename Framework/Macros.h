#pragma once

#include <vulkan/vulkan.h>

#include <iostream>

#define VK_CHECK(f)                                                                                                          \
{															 		                    									 \
	VkResult res = (f);																	                    				 \
	if (res != VK_SUCCESS) {                                                                                                 \
		std::cerr << "Vulkan Error : VkResult is \"" << res << "\" in " << __FILE__ << " at line " << __LINE__ << std::endl; \
		return false;                                                                                                        \
	}																									                     \
}
