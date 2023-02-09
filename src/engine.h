#pragma once
#include "config.h"
#include <fstream>

class Engine {

  public:
    Engine() = default;
    ~Engine() = default;
    void Init();
    void Run();
    void Quit();
    void CreatePipeline(vk::ShaderModule vertexShader,vk::ShaderModule fragShader);
    vk::ShaderModule CreateShaderModule(const char * filename);

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

    struct Vertex {
        glm::vec2 pos;
        glm::vec3 color;

        static vk::VertexInputBindingDescription getBindingDescription(){
            vk::VertexInputBindingDescription bindingDesc;
            bindingDesc.setBinding(0);
            bindingDesc.setStride(sizeof(Vertex));
            bindingDesc.setInputRate(vk::VertexInputRate::eVertex);
            return bindingDesc;
        }

        static std::array<vk::VertexInputAttributeDescription,2> getAttributeDescription(){
            std::array<vk::VertexInputAttributeDescription,2> attributeDesc;
            attributeDesc[0].setBinding(0)
                            .setLocation(0)
                            .setFormat(vk::Format::eR32G32Sfloat)
                            .setOffset(offsetof(Vertex,pos));

            attributeDesc[1].setBinding(0)
                            .setLocation(1)
                            .setFormat(vk::Format::eR32G32B32Sfloat)
                            .setOffset(offsetof(Vertex,color));

            return attributeDesc;
        }

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

    vk::Pipeline pipeline_;
    vk::PipelineLayout layout_;

    vk::RenderPass renderpass_;

    std::vector<vk::CommandBuffer> cmdBuf_;

    vk::CommandPool commandPool_;

    std::vector<vk::ShaderModule> shaderModules_;

    std::vector<vk::Framebuffer> framebuffers_;

    std::vector<vk::Fence> fences_;

    std::vector<vk::Semaphore> imageAvaliable_;
    std::vector<vk::Semaphore> imageDrawFinish_;

    int max_flight_count_ = 2;
    int cur_frame_ = 0;

    const std::vector<Vertex> vertices_ = {
        {{0.0f, -0.5f}, {1.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };

    vk::Buffer vertexBuffer_;
    vk::DeviceMemory vertexBufferMemory_;
    uint32_t buffer_mem_size_;

    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

    // glfw setup
    void build_glfw_window();

    // instance setup
    void make_instance();

    // device setup
    void make_device();

    void make_swapchain();

    void make_imageviews();

    void create_renderpass();

    void create_framebuffers();

    void create_commandpool();

    void create_fence();

    void allocate_commandbuffer();

    void create_sems();

    void create_buffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                             vk::MemoryPropertyFlags properties,
                             vk::Buffer &buffer,
                             vk::DeviceMemory &bufferMemory);

    void copyBuffer(vk::Buffer srcBuffer,vk::Buffer dstBuffer,vk::DeviceSize size);

    void create_vertexbuffer();

    void render();
};