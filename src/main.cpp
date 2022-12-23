#include "engine.h"
#include <memory>

int main() {

    auto graphicsEngine = std::make_unique<Engine>();
    graphicsEngine->Init();
    auto vertexShader = graphicsEngine->CreateShaderModule("../shader/vert.spv");
    auto fragShader = graphicsEngine->CreateShaderModule("../shader/frag.spv");

    graphicsEngine->CreatePipeline(vertexShader,fragShader);

    graphicsEngine->Run();
    graphicsEngine->Quit();

    return 0;
}

// #include <iostream>
// #include <vector>
// #include <vulkan/vulkan.hpp>

// constexpr int NUM_ELEMENTS = 1024;

// // Structure to hold the input data
// struct InputData {
//     int num1;
//     int num2;
// };

// // Structure to hold the output data
// struct OutputData {
//     int sum;
// };

// int main() {
//     // Initialize the input data
//     InputData inputData = { 5, 7 };

//     // Create the Vulkan instance
//     vk::Instance instance;
//     vk::InstanceCreateInfo instanceCreateInfo;
//     instance = vk::createInstance(instanceCreateInfo);

//     // Choose the first physical device
//     std::vector<vk::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();
//     vk::PhysicalDevice physicalDevice = physicalDevices[0];

//     // Choose the first queue family that supports compute
//     std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();
//     uint32_t queueFamilyIndex = 0;
//     for (; queueFamilyIndex < queueFamilies.size(); ++queueFamilyIndex) {
//         if (queueFamilies[queueFamilyIndex].queueCount > 0 && (queueFamilies[queueFamilyIndex].queueFlags & vk::QueueFlagBits::eCompute)) {
//             break;
//         }
//     }

//     // Create the logical device and compute queue
//     float queuePriority = 1.0f;
//     vk::DeviceQueueCreateInfo queueCreateInfo;
//     queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
//     queueCreateInfo.queueCount = 1;
//     queueCreateInfo.pQueuePriorities = &queuePriority;
//     vk::DeviceCreateInfo deviceCreateInfo;
//     deviceCreateInfo.queueCreateInfoCount = 1;
//     deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
//     vk::Device device = physicalDevice.createDevice(deviceCreateInfo);
//     vk::Queue queue = device.getQueue(queueFamilyIndex, 0);

//     // Create the input and output buffers
//     vk::Buffer inputBuffer;
//     vk::Buffer outputBuffer;
//     vk::DeviceMemory inputBufferMemory;
//     vk::DeviceMemory outputBufferMemory;
//     vk::BufferCreateInfo bufferCreateInfo;
//     bufferCreateInfo.size = sizeof(InputData);
//     bufferCreateInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer;
//     inputBuffer = device.createBuffer(bufferCreateInfo);
//     bufferCreateInfo.size = sizeof(OutputData);
//     outputBuffer = device.createBuffer(bufferCreateInfo);
//     vk::MemoryRequirements memoryRequirements;
//     memoryRequirements = device.getBufferMemoryRequirements(inputBuffer);
//     vk::MemoryAllocateInfo memoryAllocateInfo;
//     memoryAllocateInfo.allocationSize = memoryRequirements.size;
//     memoryAllocateInfo.memoryTypeIndex = 0;
//     inputBufferMemory = device.allocateMemory(memoryAllocateInfo);
//     device.bindBufferMemory(inputBuffer, inputBufferMemory, 0);


//     memoryRequirements = device.getBufferMemoryRequirements(outputBuffer);
//     memoryAllocateInfo.allocationSize = memoryRequirements.size;
//     outputBufferMemory = device.allocateMemory(memoryAllocateInfo);
//     device.bindBufferMemory(outputBuffer, outputBufferMemory, 0);

//     // Copy the input data to the input buffer
//     void* inputDataPtr = device.mapMemory(inputBufferMemory, 0, sizeof(InputData));
//     std::memcpy(inputDataPtr, &inputData, sizeof(InputData));
//     device.unmapMemory(inputBufferMemory);

//     // Create the compute pipeline
//     vk::PipelineLayout pipelineLayout;
//     vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
//     pipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);
//     vk::ShaderModule shaderModule;
//     vk::ShaderModuleCreateInfo shaderModuleCreateInfo;
//     std::vector<uint32_t> shaderCode = {
//         // Compute shader code goes here
//     };
//     shaderModuleCreateInfo.codeSize = shaderCode.size() * sizeof(uint32_t);
//     shaderModuleCreateInfo.pCode = shaderCode.data();
//     shaderModule = device.createShaderModule(shaderModuleCreateInfo);
//     vk::PipelineShaderStageCreateInfo pipelineShaderStageCreateInfo;
//     pipelineShaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eCompute;
//     pipelineShaderStageCreateInfo.module = shaderModule;
//     pipelineShaderStageCreateInfo.pName = "main";
//     vk::ComputePipelineCreateInfo computePipelineCreateInfo;
//     computePipelineCreateInfo.stage = pipelineShaderStageCreateInfo;
//     computePipelineCreateInfo.layout = pipelineLayout;
//     vk::Pipeline pipeline = device.createComputePipeline(nullptr, computePipelineCreateInfo).value;

//     // Create the command buffer
//     vk::CommandPool commandPool;
//     vk::CommandPoolCreateInfo commandPoolCreateInfo;
//     commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
//     commandPool = device.createCommandPool(commandPoolCreateInfo);
//     vk::CommandBuffer commandBuffer;
//     vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
//     commandBufferAllocateInfo.commandPool = commandPool;
//     commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
//     commandBufferAllocateInfo.commandBufferCount = 1;
//     device.allocateCommandBuffers(&commandBufferAllocateInfo, &commandBuffer);


//     // Record the command buffer
//     commandBuffer.begin(vk::CommandBufferBeginInfo());
//     commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);
//     commandBuffer.bindbuffer(0, vk::PipelineStageFlagBits::eComputeShader, inputBuffer, 0, sizeof(InputData));
//     commandBuffer.bindBuffer(1, vk::PipelineStageFlagBits::eComputeShader, outputBuffer, 0, sizeof(OutputData));
//     commandBuffer.dispatch(NUM_ELEMENTS, 1, 1);
//     commandBuffer.end();

//     // Submit the command buffer
//     vk::SubmitInfo submitInfo;
//     submitInfo.commandBufferCount = 1;
//     submitInfo.pCommandBuffers = &commandBuffer;
//     queue.submit(1, &submitInfo, nullptr);
//     queue.waitIdle();

//     // Copy the output data from the output buffer
//     OutputData outputData;
//     void* outputDataPtr = device.mapMemory(outputBufferMemory, 0, sizeof(OutputData));
//     std::memcpy(&outputData, outputDataPtr, sizeof(OutputData));
//     device.unmapMemory(outputBufferMemory);

//     // Print the result
//     std::cout << "Sum: " << outputData.sum << std::endl;

//     // Clean up
//     device.destroyBuffer(outputBuffer);
//     device.freeMemory(outputBufferMemory);
//     device.destroyBuffer(inputBuffer);
//     device.freeMemory(inputBufferMemory);
//     device.destroyPipeline(pipeline);
//     device.destroyShaderModule(shaderModule);
//     device.destroyPipelineLayout(pipelineLayout);
//     device.destroyCommandPool(commandPool);
//     device.destroy();
//     instance.destroy();

//     return 0;
// }


