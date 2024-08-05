#ifndef _BOUNDLESS_TYPES_PLUS_CXX_FILE_
#define _BOUNDLESS_TYPES_PLUS_CXX_FILE_
#include "types.hpp"
namespace BL {
class Attachment {
   protected:
    Image image;
    ImageView imageView;
    Attachment() = default;

   public:
    VkImage get_image() { return image; }
    VkImageView get_image_view() { return imageView; }
    VkImage* get_image_ptr() { return image.getPointer(); }
    VkImageView* get_image_view_ptr() { return imageView.getPointer(); }
    VkDescriptorImageInfo get_descriptor_imageinfo(VkSampler sampler) {
        VkDescriptorImageInfo v {
            .sampler = sampler,
            .imageView = VkImageView(imageView),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
        return v;
    }
};
class ColorAttachment : public Attachment {
   public:
    ColorAttachment() = default;
    ColorAttachment(VkFormat format,
                    VkExtent2D extent,
                    uint32_t layerCount = 1,
                    VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
                    VkImageUsageFlags otherUsages = 0) {
        create(format, extent, layerCount, sampleCount, otherUsages);
    }
    VkResult create(VkFormat format,
                    VkExtent2D extent,
                    uint32_t layerCount = 1,
                    VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
                    VkImageUsageFlags otherUsages = 0) {
        VkImageCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = format,
            .extent = {extent.width, extent.height, 1},
            .mipLevels = 1,
            .arrayLayers = layerCount,
            .samples = sampleCount,
            .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | otherUsages};
        VmaAllocationCreateInfo allocInfo = {
            .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            .priority = 1.0f};
        VkResult result = image.create(createInfo, allocInfo);
        if (result) {
            print_error("ColorAttachment", "Create image Failed!",
                        int32_t(result));
            return result;
        }
        imageView.allocate(image,
                           layerCount > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY
                                          : VK_IMAGE_VIEW_TYPE_2D,
                           format,
                           {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, layerCount});
        if (result) {
            print_error("ColorAttachment", "Create ImageView Failed!",
                        int32_t(result));
        }
        return result;
    }
    // 该函数用于检查某一格式的图像可否被用作颜色附件,supportBlending要求是否检查format可用于融混
    static bool format_availability(VkFormat format,
                                    bool supportBlending = true) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(context.vulkanInfo.phyDevice,
                                            format, &properties);
        return properties.optimalTilingFeatures &
               VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT
                   << uint32_t(supportBlending);
    }
};
class DepthStencilAttachment : public Attachment {
   public:
    DepthStencilAttachment() = default;
    DepthStencilAttachment(
        VkFormat format,
        VkExtent2D extent,
        uint32_t layerCount = 1,
        VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
        VkImageUsageFlags otherUsages = 0,
        bool stencilOnly = false) {
        create(format, extent, layerCount, sampleCount, otherUsages,
               stencilOnly);
    }
    VkResult create(VkFormat format,
                    VkExtent2D extent,
                    uint32_t layerCount = 1,
                    VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
                    VkImageUsageFlags otherUsages = 0,
                    bool stencilOnly = false) {
        VkImageCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = format,
            .extent = {extent.width, extent.height, 1},
            .mipLevels = 1,
            .arrayLayers = layerCount,
            .samples = sampleCount,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT  | otherUsages};
        VmaAllocationCreateInfo allocInfo = {
            .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            .priority = 1.0f};
        VkResult result = image.create(createInfo, allocInfo);
        if (result) {
            print_error("DepthStencilAttachment", "Create Image Failed!",
                        int32_t(result));
            return result;
        }
        VkImageAspectFlags aspectMask =
            (!stencilOnly) * VK_IMAGE_ASPECT_DEPTH_BIT;
        if (format > VK_FORMAT_S8_UINT)
            aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        else if (format == VK_FORMAT_S8_UINT)
            aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        result = imageView.allocate(image,
                                    layerCount > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY
                                                   : VK_IMAGE_VIEW_TYPE_2D,
                                    format, {aspectMask, 0, 1, 0, layerCount});
        if (result) {
            print_error("DepthStencilAttachment", "Create ImageView Failed!",
                        int32_t(result));
        }
        return result;
    }
    // 该函数用于检查某一格式的图像可否被用作深度模板附件
    static bool format_availability(VkFormat format) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(context.vulkanInfo.phyDevice,
                                            format, &properties);
        return properties.optimalTilingFeatures &
               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
};
}  // namespace BL
#endif  //!_BOUNDLESS_TYPES_PLUS_CXX_FILE_