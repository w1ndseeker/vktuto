#include "engine.h"
#include "device.h"
#include "instance.h"
#include "logging.h"

Engine::Engine() {

    // if (debugMode) {
    // 	std::cout << "Making a graphics engine\n";
    // }

    // build_glfw_window();

    // make_instance();

    // make_device();
}

void Engine::Init() {
    // build_glfw_window();

    glfwInit();

    // no default rendering client, we'll hook vulkan up
    // to the window later
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // resizing breaks the swapchain, we'll disable it for now
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(800, 600, "vulkan demo", nullptr, nullptr);

    // bool isquit = false;

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
    instance = vk::createInstance(info);
    if (!instance) {
        throw std::runtime_error("instace create failed");
    }
    VkSurfaceKHR c_style_surface;

    glfwCreateWindowSurface(instance, window, nullptr, &c_style_surface);
    surface = c_style_surface;

    // device
    auto physical_devices = instance.enumeratePhysicalDevices();
    physicalDevice = physical_devices[0];

    std::cout << physicalDevice.getProperties().deviceName << std::endl;

    // // logic device
    // device = vkInit::create_logical_device(physicalDevice, surface,
    // debugMode); std::array<vk::Queue, 2> queues =
    //     vkInit::get_queues(physicalDevice, device, surface, debugMode);
    // graphicsQueue = queues[0];
    // presentQueue = queues[1];

    QueueFamilyIndices indices;

    auto families = physicalDevice.getQueueFamilyProperties();
    uint32_t idx = 0;
    for (auto &family : families) {
        if (family.queueFlags | vk::QueueFlagBits::eGraphics) {
            indices.graphicsFamily = idx;
        }
        if (physicalDevice.getSurfaceSupportKHR(idx, surface)) {
            indices.presentFamily = idx;
        }
        if (indices.graphicsFamily && indices.presentFamily) {
            break;
        }
        idx++;
    }
    queueIndices = indices;

    // create device
    std::vector<vk::DeviceQueueCreateInfo> queueinfos;

    if (queueIndices.graphicsFamily.value() ==
        queueIndices.presentFamily.value()) {
        vk::DeviceQueueCreateInfo dqinfo;
        float priority = 1.0;
        dqinfo.setQueuePriorities(priority);
        dqinfo.setQueueFamilyIndex(queueIndices.graphicsFamily.value());
        dqinfo.setQueueCount(1);
        queueinfos.push_back(dqinfo);
    } else {
        vk::DeviceQueueCreateInfo dqinfo1;
        float priority = 1.0;
        dqinfo1.setQueuePriorities(priority);
        dqinfo1.setQueueFamilyIndex(queueIndices.graphicsFamily.value());
        dqinfo1.setQueueCount(1);

        vk::DeviceQueueCreateInfo dqinfo2;
        dqinfo2.setQueuePriorities(priority);
        dqinfo2.setQueueFamilyIndex(queueIndices.presentFamily.value());
        dqinfo2.setQueueCount(1);

        queueinfos.push_back(dqinfo1);
        queueinfos.push_back(dqinfo2);
    }

    std::array<const char *, 1> subset_ext{"VK_KHR_portability_subset"};

    vk::DeviceCreateInfo deviceinfo;
    deviceinfo.setPEnabledExtensionNames(subset_ext);
    deviceinfo.setQueueCreateInfos(queueinfos);

    device =  physicalDevice.createDevice(deviceinfo);

    graphicsQueue = device.getQueue(queueIndices.graphicsFamily.value(),0);
    presentQueue = device.getQueue(queueIndices.presentFamily.value(),0);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        // glfwMakeContextCurrnet(window);
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);

    // make_instance();
}

void Engine::Quit() {
    device.destroy();
    instance.destroySurfaceKHR(surface);
    instance.destroy();
    // device.destroy();
}

void Engine::build_glfw_window() {

    // initialize glfw
    glfwInit();

    // no default rendering client, we'll hook vulkan up
    // to the window later
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // resizing breaks the swapchain, we'll disable it for now
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // GLFWwindow* glfwCreateWindow (int width, int height, const char *title,
    // GLFWmonitor *monitor, GLFWwindow *share)
    if ((window =
             glfwCreateWindow(width, height, "ID Tech 12", nullptr, nullptr))) {
        if (debugMode) {
            std::cout << "Successfully made a glfw window called \"ID Tech "
                         "12\", width: "
                      << width << ", height: " << height << '\n';
        }
    } else {
        if (debugMode) {
            std::cout << "GLFW window creation failed\n";
        }
    }
}

void Engine::make_instance() {

    // instance = vkInit::make_instance(debugMode, "ID Tech 12");
    dldi = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);
    if (debugMode) {
        debugMessenger = vkInit::make_debug_messenger(instance, dldi);
    }
    VkSurfaceKHR c_style_surface;
    if (glfwCreateWindowSurface(instance, window, nullptr, &c_style_surface) !=
        VK_SUCCESS) {
        if (debugMode) {
            std::cout << "Failed to abstract glfw surface for Vulkan\n";
        }
    } else if (debugMode) {
        std::cout << "Successfully abstracted glfw surface for Vulkan\n";
    }
    // copy constructor converts to hpp convention
    surface = c_style_surface;
}

void Engine::make_device() {

    // physicalDevice = vkInit::choose_physical_device(instance, debugMode);
    // device = vkInit::create_logical_device(physicalDevice, surface,
    // debugMode); std::array<vk::Queue, 2> queues =
    //     vkInit::get_queues(physicalDevice, device, surface, debugMode);
    // graphicsQueue = queues[0];
    // presentQueue = queues[1];
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