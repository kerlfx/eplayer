#ifndef VULKAN_VIEW_H_
#define VULKAN_VIEW_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// #define STB_IMAGE_IMPLEMENTATION
// #include <stb/stb_image.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
// #include <fstream>
#include <iostream>
#include <mutex>
// #include <set>
#include <stdexcept>
#include <thread>
#include <vector>

#include "elog/elog.h"

const uint32_t WIDTH = 1920;
const uint32_t HEIGHT = 1080;

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

// #ifdef NDEBUG

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator);
struct QueueFamilyIndices
{
    uint32_t graphicsFamily = -1;
    uint32_t presentFamily = -1;
    // std::optional<uint32_t> graphicsFamily;
    // std::optional<uint32_t> presentFamily;

    bool isComplete() { return graphicsFamily != -1 && presentFamily != -1; }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

typedef struct VkImageData
{
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
} VkImageData;

class VulkanView
{
public:
    bool framebufferResized = false;

    VulkanView() {}
    /**
     * 接受 bgra 格式的 image数据
     * 返回0 ： 成功
     * */
    int updateTextureImage(uint8_t *frame, int width, int height);
    int updateTexture(uint8_t *frame, int width, int height);
    int updateCommandBuffers();

    void run()
    {
        initWindow();
        initVulkan();
        // mainLoop();
        // cleanup();
        drawLoop();
    }

    int isRun() { return isrun; }

    void createWindow(const std::string &window_title, uint32_t width = WIDTH,
                      uint32_t height = HEIGHT)
    {
        initWindow(window_title, width, height);
        return;
    }
    void initVulkan();

    void mainLoop()
    {
        isrun = 1;
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            drawFrame();
        }

        isrun = 0;
        vkDeviceWaitIdle(device);
    }

    void drawLoop()
    {
        drawFrame();
        isrun = 1;
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            drawFrame();
        }

        isrun = 0;
        vkDeviceWaitIdle(device);

        cleanup();
    }

private:
    int isrun = 0;

    std::mutex queue_mutex;

    GLFWwindow *window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;

    VkSurfaceKHR surface;
    uint32_t surface_height;
    uint32_t surface_width;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkCommandPool commandPool;

    VkImage textureImage;
    VkDeviceMemory textureImageMemory;

    VkImageView textureImageView;

    VkSampler textureSampler;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;

    void initWindow();

    //
    void initWindow(const std::string &window_title, uint32_t width,
                    uint32_t height);

    void initVulkan(uint8_t *frame, int width, int height);

    void cleanupSwapChain();

    void cleanup();

    void recreateSwapChain();

    void createInstance();

    void populateDebugMessengerCreateInfo(
        VkDebugUtilsMessengerCreateInfoEXT &createInfo);

    void setupDebugMessenger();

    void createSurface();

    void pickPhysicalDevice();

    void createLogicalDevice();
    void createSwapChain();
    void createImageViews();

    void createRenderPass();

    void createDescriptorSetLayout();

    void createGraphicsPipeline();

    void createFramebuffers();

    void createCommandPool();

    VkImageData createTextureImage(uint8_t *frame, int width, int height);
    void createTextureImage();
    VkImageView createTextureImageView()
    {

        return createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
    }

    void createTextureSampler();

    VkImageView createImageView(VkImage image, VkFormat format);

    void createImage(uint32_t width, uint32_t height, VkFormat format,
                     VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage &image,
                     VkDeviceMemory &imageMemory);

    void transitionImageLayout(VkImage image, VkFormat format,
                               VkImageLayout oldLayout,
                               VkImageLayout newLayout);

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width,
                           uint32_t height);

    void createVertexBuffer();

    void createIndexBuffer();

    void createUniformBuffers();

    void createDescriptorPool();

    void createDescriptorSets();

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties, VkBuffer &buffer,
                      VkDeviceMemory &bufferMemory);

    VkCommandBuffer beginSingleTimeCommands();

    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    uint32_t findMemoryType(uint32_t typeFilter,
                            VkMemoryPropertyFlags properties);

    void createCommandBuffers();

    void createSyncObjects();

    void updateUniformBuffer(uint32_t currentImage);

    void drawFrame();

    VkShaderModule createShaderModule(const std::vector<char> &code);

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR> &availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR> &availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    bool isDeviceSuitable(VkPhysicalDevice device);

    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    std::vector<const char *> getRequiredExtensions();

    bool checkValidationLayerSupport();
};

#endif