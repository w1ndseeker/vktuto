#pragma once
#include "config.h"

class Engine {

public:

	Engine();
    void Init();
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

	//whether to print debug messages in functions
	bool debugMode = true;

	//glfw-related variables
	int width{ 640 };
	int height{ 480 };
	GLFWwindow* window{ nullptr };

	//instance-related variables
	vk::Instance instance { nullptr };
	vk::DebugUtilsMessengerEXT debugMessenger{ nullptr };
	vk::DispatchLoaderDynamic dldi;
	vk::SurfaceKHR surface;

	//device-related variables
	vk::PhysicalDevice physicalDevice{ nullptr };
	vk::Device device{ nullptr };
	vk::Queue graphicsQueue{ nullptr };
	vk::Queue presentQueue{ nullptr };

    QueueFamilyIndices queueIndices;

	//glfw setup
	void build_glfw_window();

	//instance setup
	void make_instance();

	//device setup
	void make_device();
};