#include "engine.h"
#include <algorithm>
#include <limits>
#include <vulkan/vulkan.h>

void Engine::Init() {

    build_glfw_window();
    make_instance();
    make_device();
    make_swapchain();

    make_imageviews();
    create_renderpass();
}

void Engine::Run() {

    create_framebuffers();
    create_commandpool();
    allocate_commandbuffer();
    create_sems();
    create_fence();

    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        render();
        // glfwSwapBuffers(window_);
    }

    glfwDestroyWindow(window_);
}

void Engine::Quit() {

    device_.waitIdle();
    device_.destroySemaphore(imageAvaliable_);
    device_.destroySemaphore(imageDrawFinish_);
    device_.destroyFence(cmdAvaliableFence_);
    device_.freeCommandBuffers(commandPool_, cmdBuf_);
    device_.destroyCommandPool(commandPool_);

    for(auto& framebuffer : framebuffers_){
        device_.destroyFramebuffer(framebuffer);
    }

    device_.destroyRenderPass(renderpass_);
    device_.destroyPipelineLayout(layout_);
    device_.destroyPipeline(pipeline_);

    for (auto &shader : shaderModules_) {
        device_.destroyShaderModule(shader);
    }

    for (auto &view : imageViews_) {
        device_.destroyImageView(view);
    }

    device_.destroySwapchainKHR(swapchain_);
    device_.destroy();
    instance_.destroySurfaceKHR(surface_);
    instance_.destroy();
    // device.destroy();
}

void Engine::make_imageviews() {

    std::vector<vk::ImageView> views(images_.size());
    for (int i = 0; i < views.size(); ++i) {
        vk::ImageViewCreateInfo info;
        info.setImage(images_[i])
            .setFormat(requiredinfo_.format.format)
            .setViewType(vk::ImageViewType::e2D);
        vk::ImageSubresourceRange range;
        range.setBaseMipLevel(0)
            .setLevelCount(1)
            .setBaseArrayLayer(0)
            .setLayerCount(1)
            .setAspectMask(vk::ImageAspectFlagBits::eColor);
        vk::ComponentMapping mapping;
        info.setSubresourceRange(range).setComponents(mapping);

        views[i] = device_.createImageView(info);
    }

    imageViews_ = views;
}

void Engine::make_swapchain() {
    // creat swapchain
    SwapchainRequiredInfo info;

    info.capabilities = physicalDevice_.getSurfaceCapabilitiesKHR(surface_);
    auto formats = physicalDevice_.getSurfaceFormatsKHR(surface_);
    info.format = formats[0];
    for (auto &format : formats) {
        if (format.format == vk::Format::eR8G8B8A8Srgb ||
            format.format == vk::Format::eB8G8R8A8Srgb) {
            info.format = format;
        }
    }

    int w, h;
    glfwGetWindowSize(window_, &w, &h);

    info.extent.width =
        std::clamp<uint32_t>(w, info.capabilities.minImageExtent.width,
                             info.capabilities.maxImageExtent.width);
    info.extent.height =
        std::clamp<uint32_t>(h, info.capabilities.minImageExtent.height,
                             info.capabilities.maxImageExtent.height);

    info.image_count = std::clamp<uint32_t>(2, info.capabilities.minImageCount,
                                            info.capabilities.maxImageCount);

    auto presentModes = physicalDevice_.getSurfacePresentModesKHR(surface_);
    info.present_mode = vk::PresentModeKHR::eFifo;

    for (auto &present : presentModes) {
        if (present == vk::PresentModeKHR::eMailbox) {
            info.present_mode = present;
        }
    }
    requiredinfo_ = info;

    vk::SwapchainCreateInfoKHR createinfo;
    createinfo.setImageColorSpace(requiredinfo_.format.colorSpace)
        .setImageFormat(requiredinfo_.format.format)
        .setMinImageCount(requiredinfo_.image_count)
        .setImageExtent(requiredinfo_.extent)
        .setPresentMode(requiredinfo_.present_mode)
        .setPreTransform(requiredinfo_.capabilities.currentTransform);

    if (queueIndices_.graphicsFamily.value() ==
        queueIndices_.presentFamily.value()) {
        createinfo.setQueueFamilyIndices(queueIndices_.graphicsFamily.value())
            .setImageSharingMode(vk::SharingMode::eExclusive);
    } else {
        std::array<uint32_t, 2> indices{queueIndices_.graphicsFamily.value(),
                                        queueIndices_.presentFamily.value()};
        createinfo.setQueueFamilyIndices(indices).setImageSharingMode(
            vk::SharingMode::eConcurrent);
    }

    createinfo.setClipped(true)
        .setSurface(surface_)
        .setImageArrayLayers(1)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);

    swapchain_ = device_.createSwapchainKHR(createinfo);
    images_ = device_.getSwapchainImagesKHR(swapchain_);

}

void Engine::build_glfw_window() {

    glfwInit();
    // no default rendering client, we'll hook vulkan up
    // to the window later
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // resizing breaks the swapchain, we'll disable it for now
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window_ = glfwCreateWindow(800, 600, "vulkan demo", nullptr, nullptr);
}

void Engine::make_instance() {

    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char *> extensions(glfwExtensions,
                                         glfwExtensions + glfwExtensionCount);

    for (auto &ext : extensions) {
        std::cout << ext << std::endl;
    }

    extensions.push_back("VK_KHR_get_physical_device_properties2");

    // validation layers
    std::array<const char *, 1> layers{"VK_LAYER_KHRONOS_validation"};
    vk::InstanceCreateInfo info;

#ifdef __APPLE__
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    info.setFlags(vk::InstanceCreateFlags(VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR));
#endif

    info.setPEnabledExtensionNames(extensions);
    info.setPEnabledLayerNames(layers);

    instance_ = vk::createInstance(info);
    if (!instance_) {
        throw std::runtime_error("instace create failed");
    }

    VkSurfaceKHR c_style_surface;
    glfwCreateWindowSurface(instance_, window_, nullptr, &c_style_surface);
    surface_ = c_style_surface;
}

void Engine::make_device() {

    // device
    auto physical_devices = instance_.enumeratePhysicalDevices();
    physicalDevice_ = physical_devices[0];

    std::cout << physicalDevice_.getProperties().deviceName << std::endl;
    QueueFamilyIndices indices;

    auto families = physicalDevice_.getQueueFamilyProperties();
    uint32_t idx = 0;
    for (auto &family : families) {
        if (family.queueFlags | vk::QueueFlagBits::eGraphics) {
            indices.graphicsFamily = idx;
        }
        if (physicalDevice_.getSurfaceSupportKHR(idx, surface_)) {
            indices.presentFamily = idx;
        }
        if (indices.graphicsFamily && indices.presentFamily) {
            break;
        }
        idx++;
    }
    queueIndices_ = indices;

    // create device
    std::vector<vk::DeviceQueueCreateInfo> queueinfos;

    if (queueIndices_.graphicsFamily.value() ==
        queueIndices_.presentFamily.value()) {
        vk::DeviceQueueCreateInfo dqinfo;
        float priority = 1.0;
        dqinfo.setQueuePriorities(priority)
            .setQueueFamilyIndex(queueIndices_.graphicsFamily.value())
            .setQueueCount(1);
        queueinfos.push_back(dqinfo);
    } else {
        vk::DeviceQueueCreateInfo dqinfo1;
        float priority = 1.0;
        dqinfo1.setQueuePriorities(priority)
            .setQueueFamilyIndex(queueIndices_.graphicsFamily.value())
            .setQueueCount(1);

        vk::DeviceQueueCreateInfo dqinfo2;
        dqinfo2.setQueuePriorities(priority)
            .setQueueFamilyIndex(queueIndices_.presentFamily.value())
            .setQueueCount(1);

        queueinfos.push_back(dqinfo1);
        queueinfos.push_back(dqinfo2);
    }

#ifdef __APPLE__
    std::array<const char *, 2> subset_ext{"VK_KHR_portability_subset",
                                           VK_KHR_SWAPCHAIN_EXTENSION_NAME};
#else
    std::array<const char *, 1> subset_ext{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
#endif

    vk::DeviceCreateInfo deviceinfo;
    deviceinfo.setPEnabledExtensionNames(subset_ext);
    deviceinfo.setQueueCreateInfos(queueinfos);

    device_ = physicalDevice_.createDevice(deviceinfo);

    graphicsQueue_ = device_.getQueue(queueIndices_.graphicsFamily.value(), 0);
    presentQueue_ = device_.getQueue(queueIndices_.presentFamily.value(), 0);
}

void Engine::CreatePipeline(vk::ShaderModule vertexShader,
                            vk::ShaderModule fragShader) {

    vk::GraphicsPipelineCreateInfo info;
    // info.setPVertexInputState();
    // info.setPInputAssemblyState();

    std::array<vk::PipelineShaderStageCreateInfo, 2> stageInfos;
    stageInfos[0]
        .setModule(vertexShader)
        .setStage(vk::ShaderStageFlagBits::eVertex)
        .setPName("main");

    stageInfos[1]
        .setModule(fragShader)
        .setStage(vk::ShaderStageFlagBits::eFragment)
        .setPName("main");

    info.setStages(stageInfos);

    vk::PipelineVertexInputStateCreateInfo vertexInput;
    info.setPVertexInputState(&vertexInput);

    vk::PipelineInputAssemblyStateCreateInfo inputAsm;
    inputAsm.setPrimitiveRestartEnable(false).setTopology(
        vk::PrimitiveTopology::eTriangleList);

    info.setPInputAssemblyState(&inputAsm);

    vk::PipelineLayoutCreateInfo layoutInfo;
    layout_ = device_.createPipelineLayout(layoutInfo);

    info.setLayout(layout_);

    vk::PipelineViewportStateCreateInfo viewportInfo;
    vk::Viewport viewport(0, 0, requiredinfo_.extent.width,
                          requiredinfo_.extent.height, 0, 1);

    vk::Rect2D scissor({0, 0}, requiredinfo_.extent);
    viewportInfo.setViewports(viewport).setScissors(scissor);

    info.setPViewportState(&viewportInfo);

    vk::PipelineRasterizationStateCreateInfo rastInfo;
    rastInfo.setCullMode(vk::CullModeFlagBits::eFront)
        .setFrontFace(vk::FrontFace::eCounterClockwise)
        .setDepthClampEnable(false)
        .setLineWidth(1)
        .setPolygonMode(vk::PolygonMode::eFill)
        .setRasterizerDiscardEnable(false);
    info.setPRasterizationState(&rastInfo);

    vk::PipelineMultisampleStateCreateInfo multisample;
    multisample.setSampleShadingEnable(false).setRasterizationSamples(
        vk::SampleCountFlagBits::e1);

    info.setPMultisampleState(&multisample);

    info.setPDepthStencilState(nullptr);

    vk::PipelineColorBlendStateCreateInfo colorBlend;
    vk::PipelineColorBlendAttachmentState atBlendState;
    atBlendState.setColorWriteMask(
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
    colorBlend.setLogicOpEnable(false).setAttachments(atBlendState);

    info.setPColorBlendState(&colorBlend);

    info.setRenderPass(renderpass_);

    pipeline_ = device_.createGraphicsPipeline(nullptr, info).value;
}

void Engine::create_renderpass() {
    vk::RenderPassCreateInfo info;

    vk::AttachmentDescription attachDesc;
    attachDesc.setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setFormat(requiredinfo_.format.format)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    info.setAttachments(attachDesc);

    vk::SubpassDescription subpassDesc;
    vk::AttachmentReference refer;
    refer.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
    refer.setAttachment(0);
    subpassDesc.setColorAttachments(refer).setPipelineBindPoint(
        vk::PipelineBindPoint::eGraphics);

    info.setSubpasses(subpassDesc);

    renderpass_ = device_.createRenderPass(info);
}

void Engine::create_framebuffers() {

    for (int i = 0; i < imageViews_.size(); ++i) {
        vk::FramebufferCreateInfo info;
        info.setRenderPass(renderpass_)
            .setLayers(1)
            .setWidth(requiredinfo_.extent.width)
            .setHeight(requiredinfo_.extent.height)
            .setAttachments(imageViews_[i]);
        framebuffers_.push_back(device_.createFramebuffer(info));
    }
}

void Engine::create_commandpool() {
    vk::CommandPoolCreateInfo info;
    info.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    commandPool_ = device_.createCommandPool(info);
}

void Engine::allocate_commandbuffer() {
    vk::CommandBufferAllocateInfo info;
    info.setCommandPool(commandPool_)
        .setCommandBufferCount(1)
        .setLevel(vk::CommandBufferLevel::ePrimary);

    cmdBuf_ = device_.allocateCommandBuffers(info)[0];
}

void Engine::render() {

    auto result = device_.acquireNextImageKHR(
        swapchain_, std::numeric_limits<uint64_t>::max(),imageAvaliable_);

    if (result.result != vk::Result::eSuccess) {
        std::cout << "aquire next image failed!" << std::endl;
    }

    auto imageIndex = result.value;
    cmdBuf_.reset();

    vk::CommandBufferBeginInfo begin_info;
    begin_info.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmdBuf_.begin(begin_info);

    vk::ClearValue clearValue;
    clearValue.setColor(vk::ClearColorValue(std::array<float, 4>{0.1, 0.1, 0.1, 1}));
    vk::RenderPassBeginInfo renderPassBegin;
    renderPassBegin.setRenderPass(renderpass_)
                   .setFramebuffer(framebuffers_[imageIndex])
                   .setClearValues(clearValue)
                   .setRenderArea(vk::Rect2D({}, vk::Extent2D(requiredinfo_.extent.width,requiredinfo_.extent.height)));

    cmdBuf_.beginRenderPass(renderPassBegin,{});
    cmdBuf_.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_);
    cmdBuf_.draw(3,1,0,0);
    cmdBuf_.endRenderPass();

    cmdBuf_.end();

    device_.resetFences(cmdAvaliableFence_);

    vk::SubmitInfo submit_info;

    vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    submit_info.setCommandBuffers(cmdBuf_)
               .setWaitSemaphores(imageAvaliable_)
               .setSignalSemaphores(imageDrawFinish_)
               .setWaitDstStageMask(waitStages);

    graphicsQueue_.submit(submit_info, cmdAvaliableFence_);

    auto wait_ret = device_.waitForFences(cmdAvaliableFence_, true,
                                          std::numeric_limits<uint64_t>::max());
    if (wait_ret != vk::Result::eSuccess) {
        std::cout << "Fence wait failed!" << std::endl;
    }

    vk::PresentInfoKHR present_info;
    present_info.setImageIndices(imageIndex)
                .setSwapchains(swapchain_)
                .setWaitSemaphores(imageDrawFinish_);

    auto ret = presentQueue_.presentKHR(present_info);

    if (ret != vk::Result::eSuccess) {
        std::cout << "present failed!" << std::endl;
    }


}

void Engine::create_fence() {
    vk::FenceCreateInfo info;
    info.setFlags(vk::FenceCreateFlagBits::eSignaled);
    cmdAvaliableFence_ = device_.createFence(info);
}

void Engine::create_sems() {
    vk::SemaphoreCreateInfo info;
    imageAvaliable_ = device_.createSemaphore(info);
    imageDrawFinish_ = device_.createSemaphore(info);

}

vk::ShaderModule Engine::CreateShaderModule(const char *filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::in);
    std::vector<char> content((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
    file.close();
    vk::ShaderModuleCreateInfo info;
    info.pCode = (uint32_t *)content.data();
    info.codeSize = content.size();

    shaderModules_.push_back(device_.createShaderModule(info));
    return shaderModules_.back();
}