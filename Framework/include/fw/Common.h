#pragma once

#include <vulkan/vulkan.h>

#include <string_view>
#include <vector>
#include <functional>

namespace fw
{

void printError(std::string_view msg, const VkResult* result = nullptr);
void printWarning(std::string_view msg);
void printLog(std::string_view msg);

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
    explicit Cleaner(std::function<void()> callback) : m_clean(callback) {}
    ~Cleaner() { m_clean(); }

private:
    std::function<void()> m_clean;
};

template <typename T>
uint32_t ui32size(const T& container)
{
    return static_cast<uint32_t>(container.size());
}

} // namespace fw