#pragma once
#include "config.h"

class Engine {

  public:
    Engine();
    void Init();
    void Run();
    void Quit();

    ~Engine();

  private:
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapchainRequiredInfo {
        vk::SurfaceCapabilitiesKHR capabilities;
        vk::Extent2D extent;
        vk::SurfaceFormatKHR format;
        vk::PresentModeKHR present_mode;
        uint32_t image_count;
    };

    // whether to print debug messages in functions
    bool debugMode = true;

    // glfw-related variables
    int width{640};
    int height{480};
    GLFWwindow *window_{nullptr};

    // instance-related variables
    vk::Instance instance_{nullptr};
    vk::DebugUtilsMessengerEXT debugMessenger{nullptr};
    vk::DispatchLoaderDynamic dldi;
    vk::SurfaceKHR surface_;

    std::vector<vk::Image> images_;
    std::vector<vk::ImageView> imageViews_;

    // device-related variables
    vk::PhysicalDevice physicalDevice_{nullptr};
    vk::Device device_{nullptr};
    vk::Queue graphicsQueue_{nullptr};
    vk::Queue presentQueue_{nullptr};

    QueueFamilyIndices queueIndices_;
    SwapchainRequiredInfo requiredinfo_;
    vk::SwapchainKHR swapchain_{nullptr};

    // glfw setup
    void build_glfw_window();

    // instance setup
    void make_instance();

    // device setup
    void make_device();

    void make_swapchain();

    void make_imageviews();
};