# Push constant

Push constants can be used instead of uniform buffers to upload data to shaders. Push constants are meant for frequently changing data since it's probably faster to update push constants. The downside is that there is a size limit for push constants which can be queried with `vkGetPhysicalDeviceProperties`. Push constants have to represented in the graphics pipeline layout with `VkPushConstantRange`. This demo uploads a single color variable to shaders.

![push_constant](push_constant.png?raw=true "push_constant")