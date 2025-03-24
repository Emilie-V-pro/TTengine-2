#pragma once


#include "commandBuffer/command_buffer.hpp"
#include "volk.h"


#include <cstdint>
#include <glm/fwd.hpp>
#include <vector>


#include "device.hpp"
#include "GPU_data/image.hpp"

#include "swapchain.hpp"
namespace TTe {
enum depthAndStencil {
    NONE,
    DEPTH,
    DEPTH_AND_STENCIL,
};


enum renderPassModeEnum{
    DEFAULT = 0,
    ADDIDITIVE = 1,
};
class DynamicRenderPass  {
    struct AttachementStruct{
        std::vector<VkRenderingAttachmentInfo> colorAttachments;
        VkRenderingAttachmentInfo depthAttachment;
    };


   public:
   DynamicRenderPass() {};
    DynamicRenderPass(
        Device *device,
        VkExtent2D frameSize,
        std::vector<VkFormat> imageFormats,
        unsigned int numberOfFrame,
        depthAndStencil enableDepthAndStencil,
        SwapChain *swapChain = nullptr,
        std::vector<Image> *externalDepthImages = {}
        );
    ~DynamicRenderPass();

    // remove copy constructor
    DynamicRenderPass(const DynamicRenderPass &) = delete;
    DynamicRenderPass &operator=(const DynamicRenderPass &) = delete;

    // move constructor
    DynamicRenderPass(DynamicRenderPass &&other);
    DynamicRenderPass &operator=(DynamicRenderPass &&other);

    void beginRenderPass(CommandBuffer & commandBuffer, unsigned imageIndex, renderPassModeEnum renderPassMode = DEFAULT, VkRenderingFlags optionalRenderingFlag = 0);
    void endRenderPass(CommandBuffer & commandBuffer);

    void resize(VkExtent2D frameSize);
    void setClearColor(glm::vec3 rgb);
    void setClearEnable(bool enable);
    void setDepthAndStencil(CommandBuffer &cmdbuffer, bool enable);
    void transitionColorAttachment(uint32_t frameIndex, VkImageLayout newLayout, CommandBuffer &commandBuffer);


    std::vector<Image>& getDepthAndStencilAttachement(){return depthAndStencilAttachement;};
    std::vector<Image>* getDepthAndStencilPtr(){return &depthAndStencilAttachement;}
    std::vector<std::vector<Image>>& getimageAttachement(){return imageAttachement;};
    VkExtent2D getFrameSize(){return frameSize;};

   private:
    void createRessources();
    void createAttachmentInfo();
    void createRenderpassInfo();

    unsigned int numberOfFrame = 0;
    VkExtent2D frameSize = {0, 0};
    depthAndStencil enableDepthAndStencil = NONE;
    SwapChain *swapChain = nullptr;

    std::vector<AttachementStruct> attachments;
    std::vector<std::vector<Image>> imageAttachement;
    std::vector<Image> depthAndStencilAttachement;
    std::vector<VkFormat> imageFormats;
    std::vector<Image> *externalDepthImages;
    std::vector<VkRenderingInfo> renderingInfos;

    Device *device = nullptr;
};
}  // namespace vk_stage