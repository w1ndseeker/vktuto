#include "engine.h"
#include "device.h"
#include "instance.h"
#include "logging.h"
#include <algorithm>

Engine::Engine() {

    // if (debugMode) {
    // 	std::cout << "Making a graphics engine\n";
    // }

    // build_glfw_window();

    // make_instance();

    // make_device();
}

void Engine::Init() {

    build_glfw_window();
    make_instance();
    make_device();

    make_swapchain();

    _images = _device.getSwapchainImagesKHR(_swapchain);

}

void Engine::Run() {

    while (!glfwWindowShouldClose(_window)) {
        glfwPollEvents();
        // glfwMakeContextCurrnet(window);
        glfwSwapBuffers(_window);
    }

    glfwDestroyWindow(_window);
}

void Engine::Quit() {

    for(auto & view : _imageViews){
        _device.destroyImageView(view);
    }

    _device.destroySwapchainKHR(_swapchain);
    _device.destroy();
    _instance.destroySurfaceKHR(_surface);
    _instance.destroy();
    // device.destroy();
}

void Engine::make_imageviews(){
    std::vector<vk::ImageView> views(_images.size());
    for(int i = 0; i < views.size(); ++i){
        vk::ImageViewCreateInfo info;
        info.setImage(_images[i]);
        info.setFormat(_requiredinfo.format.format);
        info.setViewType(vk::ImageViewType::e2D);
        vk::ImageSubresourceRange range;
        range.setBaseMipLevel(0);
        range.setLevelCount(1);
        range.setBaseArrayLayer(0);
        range.setLayerCount(1);
        range.setAspectMask(vk::ImageAspectFlagBits::eColor);
        info.setSubresourceRange(range);
        vk::ComponentMapping mapping;
        info.setComponents(mapping);

        views[i] = _device.createImageView(info);
    }

    _imageViews = views;

}

void Engine::make_swapchain() {
    // creat swapchain
    SwapchainRequiredInfo info;

    info.capabilities = _physicalDevice.getSurfaceCapabilitiesKHR(_surface);
    auto formats = _physicalDevice.getSurfaceFormatsKHR(_surface);
    info.format = formats[0];
    for (auto &format : formats) {
        if (format.format == vk::Format::eR8G8B8A8Srgb ||
            format.format == vk::Format::eB8G8R8A8Srgb) {
            info.format = format;
        }
    }

    int w, h;
    glfwGetWindowSize(_window, &w, &h);

    info.extent.width =
        std::clamp<uint32_t>(w, info.capabilities.minImageExtent.width,
                             info.capabilities.maxImageExtent.width);
    info.extent.height =
        std::clamp<uint32_t>(h, info.capabilities.minImageExtent.height,
                             info.capabilities.maxImageExtent.height);

    info.image_count = std::clamp<uint32_t>(2, info.capabilities.minImageCount,
                                            info.capabilities.maxImageCount);

    auto presentModes = _physicalDevice.getSurfacePresentModesKHR(_surface);
    info.present_mode = vk::PresentModeKHR::eFifo;

    for (auto &present : presentModes) {
        if (present == vk::PresentModeKHR::eMailbox) {
            info.present_mode = present;
        }
    }
    _requiredinfo = info;

    vk::SwapchainCreateInfoKHR createinfo;
    createinfo.setImageColorSpace(_requiredinfo.format.colorSpace);
    createinfo.setImageFormat(_requiredinfo.format.format);
    createinfo.setMinImageCount(_requiredinfo.image_count);
    createinfo.setImageExtent(_requiredinfo.extent);
    createinfo.setPresentMode(_requiredinfo.present_mode);
    createinfo.setPreTransform(_requiredinfo.capabilities.currentTransform);

    if (_queueIndices.graphicsFamily.value() ==
        _queueIndices.presentFamily.value()) {
        createinfo.setQueueFamilyIndices(_queueIndices.graphicsFamily.value());
        createinfo.setImageSharingMode(vk::SharingMode::eExclusive);
    } else {
        std::array<uint32_t, 2> indices{_queueIndices.graphicsFamily.value(),
                                        _queueIndices.presentFamily.value()};
        createinfo.setQueueFamilyIndices(indices);
        createinfo.setImageSharingMode(vk::SharingMode::eConcurrent);
    }

    createinfo.setClipped(true);
    createinfo.setSurface(_surface);
    createinfo.setImageArrayLayers(1);
    createinfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
    createinfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);

    _swapchain = _device.createSwapchainKHR(createinfo);

}


void Engine::build_glfw_window() {

    glfwInit();

    // no default rendering client, we'll hook vulkan up
    // to the window later
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // resizing breaks the swapchain, we'll disable it for now
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    _window = glfwCreateWindow(800, 600, "vulkan demo", nullptr, nullptr);
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
    info.setPEnabledExtensionNames(extensions);
    info.setPEnabledLayerNames(layers);
    _instance = vk::createInstance(info);
    if (!_instance) {
        throw std::runtime_error("instace create failed");
    }

    VkSurfaceKHR c_style_surface;
    glfwCreateWindowSurface(_instance, _window, nullptr, &c_style_surface);
    _surface = c_style_surface;
}

void Engine::make_device() {

    // device
    auto physical_devices = _instance.enumeratePhysicalDevices();
    _physicalDevice = physical_devices[0];

    std::cout << _physicalDevice.getProperties().deviceName << std::endl;

    // // logic device
    // device = vkInit::create_logical_device(physicalDevice, surface,
    // debugMode); std::array<vk::Queue, 2> queues =
    //     vkInit::get_queues(physicalDevice, device, surface, debugMode);
    // graphicsQueue = queues[0];
    // presentQueue = queues[1];

    QueueFamilyIndices indices;

    auto families = _physicalDevice.getQueueFamilyProperties();
    uint32_t idx = 0;
    for (auto &family : families) {
        if (family.queueFlags | vk::QueueFlagBits::eGraphics) {
            indices.graphicsFamily = idx;
        }
        if (_physicalDevice.getSurfaceSupportKHR(idx, _surface)) {
            indices.presentFamily = idx;
        }
        if (indices.graphicsFamily && indices.presentFamily) {
            break;
        }
        idx++;
    }
    _queueIndices = indices;

    // create device
    std::vector<vk::DeviceQueueCreateInfo> queueinfos;

    if (_queueIndices.graphicsFamily.value() ==
        _queueIndices.presentFamily.value()) {
        vk::DeviceQueueCreateInfo dqinfo;
        float priority = 1.0;
        dqinfo.setQueuePriorities(priority);
        dqinfo.setQueueFamilyIndex(_queueIndices.graphicsFamily.value());
        dqinfo.setQueueCount(1);
        queueinfos.push_back(dqinfo);
    } else {
        vk::DeviceQueueCreateInfo dqinfo1;
        float priority = 1.0;
        dqinfo1.setQueuePriorities(priority);
        dqinfo1.setQueueFamilyIndex(_queueIndices.graphicsFamily.value());
        dqinfo1.setQueueCount(1);

        vk::DeviceQueueCreateInfo dqinfo2;
        dqinfo2.setQueuePriorities(priority);
        dqinfo2.setQueueFamilyIndex(_queueIndices.presentFamily.value());
        dqinfo2.setQueueCount(1);

        queueinfos.push_back(dqinfo1);
        queueinfos.push_back(dqinfo2);
    }

    std::array<const char *, 2> subset_ext{"VK_KHR_portability_subset",
                                           VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    vk::DeviceCreateInfo deviceinfo;
    deviceinfo.setPEnabledExtensionNames(subset_ext);
    deviceinfo.setQueueCreateInfos(queueinfos);

    _device = _physicalDevice.createDevice(deviceinfo);

    _graphicsQueue = _device.getQueue(_queueIndices.graphicsFamily.value(), 0);
    _presentQueue = _device.getQueue(_queueIndices.presentFamily.value(), 0);
}

Engine::~Engine() {

    // if (debugMode) {
    // 	std::cout << "Goodbye see you!\n";
    // }

    // device.destroy();

    // instance.destroySurfaceKHR(surface);
    // if (debugMode) {
    // 	instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dldi);
    // }
    /*
    * from vulkan_funcs.hpp:
    *
    * void Instance::destroy( Optional<const
    VULKAN_HPP_NAMESPACE::AllocationCallbacks> allocator = nullptr, Dispatch
    const & d = ::vk::getDispatchLoaderStatic())
    */
    // instance.destroy();

    // terminate glfw
    //  glfwTerminate();
}