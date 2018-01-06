#pragma once

#include "Framework.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string_view>
#include <vector>
#include <functional>

namespace fw
{

template <typename T>
int runApplication()
{
    fw::Framework fw;
    int status = 1;
    if (fw.initialize()) {
        T app;
        if (app.initialize()) {
            fw.setApplication(&app);
            fw.execute();
            status = 0;
        }
    }

    return status;
}

void printError(std::string_view msg, const VkResult* result = nullptr);

struct QueueFamilyIndices
{
    int graphicsFamily = -1;
    int presentFamily = -1;
    bool hasGraphicsAndPresentFamily() const;
};

QueueFamilyIndices getQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

VkShaderModule createShaderModule(const std::string& filename);

bool findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, uint32_t& typeIndex);

class Cleaner {
public:
    Cleaner() = delete;
    explicit Cleaner(std::function<void()> callback) : clean(callback) {}
    ~Cleaner() { clean(); }

private:
    std::function<void()> clean;
};
    
} // namespace fw
