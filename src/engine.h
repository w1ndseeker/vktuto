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

    struct SwapchainRequiredInfo{
        vk::SurfaceCapabilitiesKHR capabilities;
        vk::Extent2D extent;
        vk::SurfaceFormatKHR format;
        vk::PresentModeKHR present_mode;
        uint32_t image_count;
    };

	//whether to print debug messages in functions
	bool debugMode = true;

	//glfw-related variables
	int width{ 640 };
	int height{ 480 };
	GLFWwindow* _window { nullptr };

	//instance-related variables
	vk::Instance _instance { nullptr };
	vk::DebugUtilsMessengerEXT debugMessenger{ nullptr };
	vk::DispatchLoaderDynamic dldi;
	vk::SurfaceKHR _surface;

    std::vector<vk::Image> _images;
    std::vector<vk::ImageView> _imageViews;

	//device-related variables
	vk::PhysicalDevice _physicalDevice { nullptr };
	vk::Device _device { nullptr };
	vk::Queue _graphicsQueue { nullptr };
	vk::Queue _presentQueue { nullptr };

    QueueFamilyIndices _queueIndices;
    SwapchainRequiredInfo _requiredinfo;
    vk::SwapchainKHR _swapchain {nullptr};

	//glfw setup
	void build_glfw_window();

	//instance setup
	void make_instance();

	//device setup
	void make_device();

	void make_swapchain();

    void make_imageviews();

};