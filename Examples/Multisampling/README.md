# Multisampling

Multisampling in Vulkan is done by creating an image (`VkImageCreateInfo`) with the desired sample count and using it as a render target. The image is then resolved to a separate image that can be displayed. The resolving is specified during the render pass creation using `VkSubpassDescription`. In addition color attachment (in `VkAttachmentDescription`) of a render pass must also have the correct sample count. The sample count must also be specified in the used graphics pipeline (`VkPipelineMultisampleStateCreateInfo`). Maximum supported sample count can be queried with `vkGetPhysicalDeviceProperties`.

For more info: https://vulkan-tutorial.com/Multisampling

1 sample

![1](1.png?raw=true "1")

4 samples

![4](4.png?raw=true "4")
