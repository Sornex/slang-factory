#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <slang.h>

#include <optional>
#include <vector>

struct RendererShaderBinaries
{
    slang::IBlob* vertex_shader = nullptr;
    slang::IBlob* fragment_shader = nullptr;
};

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    bool is_complete() const
    {
        return graphics_family.has_value() && present_family.has_value();
    }
};

struct SwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

class VulkanRenderer
{
public:
    VulkanRenderer() = default;
    ~VulkanRenderer();

    bool init(
        const char* window_title,
        int width,
        int height,
        const RendererShaderBinaries& shaders);

    void run();
    void shutdown();

private:
    bool init_window(const char* window_title, int width, int height);
    bool init_vulkan(const RendererShaderBinaries& shaders);

    bool create_instance();
    bool create_surface();

    bool pick_physical_device();
    bool create_logical_device();

    bool create_swapchain();
    bool create_image_views();

    bool create_render_pass();
    bool create_graphics_pipeline(const RendererShaderBinaries& shaders);
    bool create_framebuffers();

    bool create_command_pool();
    bool create_vertex_buffer();
    bool create_command_buffers();
    bool create_sync_objects();

    bool create_descriptor_set_layout();
    bool create_uniform_buffer();
    bool create_descriptor_pool();
    bool create_descriptor_set();
    void update_uniform_buffer();

    void record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index);
    void draw_frame();

    bool is_device_suitable(VkPhysicalDevice device);
    QueueFamilyIndices find_queue_families(VkPhysicalDevice device);

    SwapchainSupportDetails query_swapchain_support(VkPhysicalDevice device);
    VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);
    VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes);
    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);

    VkShaderModule create_shader_module(slang::IBlob* shader_blob);

    uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties);

private:
    GLFWwindow* window_ = nullptr;

    VkInstance instance_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;

    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;

    VkQueue graphics_queue_ = VK_NULL_HANDLE;
    VkQueue present_queue_ = VK_NULL_HANDLE;

    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    std::vector<VkImage> swapchain_images_;
    std::vector<VkImageView> swapchain_image_views_;
    VkFormat swapchain_image_format_ = VK_FORMAT_UNDEFINED;
    VkExtent2D swapchain_extent_{};

    VkRenderPass render_pass_ = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;
    VkPipeline graphics_pipeline_ = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> swapchain_framebuffers_;

    VkCommandPool command_pool_ = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> command_buffers_;

    VkBuffer vertex_buffer_ = VK_NULL_HANDLE;
    VkDeviceMemory vertex_buffer_memory_ = VK_NULL_HANDLE;

    VkSemaphore image_available_semaphore_ = VK_NULL_HANDLE;
    VkSemaphore render_finished_semaphore_ = VK_NULL_HANDLE;
    VkFence in_flight_fence_ = VK_NULL_HANDLE;

    VkDescriptorSetLayout descriptor_set_layout_ = VK_NULL_HANDLE;
    VkDescriptorPool descriptor_pool_ = VK_NULL_HANDLE;
    VkDescriptorSet descriptor_set_ = VK_NULL_HANDLE;

    VkBuffer uniform_buffer_ = VK_NULL_HANDLE;
    VkDeviceMemory uniform_buffer_memory_ = VK_NULL_HANDLE;
};