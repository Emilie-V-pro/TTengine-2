
#include "dynamic_renderpass.hpp"
#include <vulkan/vulkan_core.h>



#include <cassert>
#include <glm/fwd.hpp>
#include <vector>

#include "commandBuffer/command_buffer.hpp"
#include "device.hpp"

#include "structs_vk.hpp"
#include "volk.h"

namespace TTe {

DynamicRenderPass::DynamicRenderPass(
    Device *device,
    VkExtent2D frameSize,
    std::vector<VkFormat> imageFormats,
    unsigned int numberOfFrame,
    depthAndStencil enableDepthAndStencil,
    SwapChain *swapChain,
    std::vector<Image> *externalDepthImages)
    : device(device),
      frameSize(frameSize),
      numberOfFrame(numberOfFrame),
      enableDepthAndStencil(enableDepthAndStencil),
      imageFormats(imageFormats),
      swapChain(swapChain),
      externalDepthImages(externalDepthImages) {
    createRessources();
    createAttachmentInfo();
    createRenderpassInfo();
}


DynamicRenderPass::DynamicRenderPass(DynamicRenderPass &&other) {
    device = other.device;
    frameSize = other.frameSize;
    numberOfFrame = other.numberOfFrame;
    enableDepthAndStencil = other.enableDepthAndStencil;
    imageFormats = other.imageFormats;
    swapChain = std::move(other.swapChain);
    externalDepthImages = std::move(other.externalDepthImages);
    renderingInfos = other.renderingInfos;
    attachments = std::move(other.attachments);
    imageAttachement = std::move(other.imageAttachement);
    depthAndStencilAttachement = std::move(other.depthAndStencilAttachement);
    other.device = nullptr;
    other.swapChain = nullptr;
    other.externalDepthImages = nullptr;
}

DynamicRenderPass &DynamicRenderPass::operator=(DynamicRenderPass &&other) {
    if (this != &other) {
        device = other.device;
        frameSize = other.frameSize;
        numberOfFrame = other.numberOfFrame;
        enableDepthAndStencil = other.enableDepthAndStencil;
        imageFormats = other.imageFormats;
        swapChain = std::move(other.swapChain);
        externalDepthImages = std::move(other.externalDepthImages);
        renderingInfos = other.renderingInfos;
        attachments = std::move(other.attachments);
        imageAttachement = std::move(other.imageAttachement);
        depthAndStencilAttachement = std::move(other.depthAndStencilAttachement);
        other.device = nullptr;
        other.swapChain = nullptr;
        other.externalDepthImages = nullptr;
    }
    return *this;
}

DynamicRenderPass::~DynamicRenderPass() {
   
}

void DynamicRenderPass::beginRenderPass(
    CommandBuffer &commandBuffer, unsigned imageIndex, renderPassModeEnum renderPassMode, VkRenderingFlags optionalRenderingFlag) {
    renderingInfos[imageIndex].flags = optionalRenderingFlag;

    auto externaldepthAttachmentInfo = make<VkRenderingAttachmentInfo>();
    if (externalDepthImages != nullptr) {
        externaldepthAttachmentInfo.imageView = externalDepthImages->at(imageIndex);
        externaldepthAttachmentInfo.imageLayout = externalDepthImages->at(imageIndex);
        externaldepthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        externaldepthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        externaldepthAttachmentInfo.clearValue.depthStencil = {1.0, 0};
        renderingInfos[imageIndex].pDepthAttachment = &externaldepthAttachmentInfo;
    }

    vkCmdBeginRendering(commandBuffer, &renderingInfos[imageIndex]);

    renderingInfos[imageIndex].flags = 0;
    VkViewport viewport{0, (float)frameSize.height, (float)frameSize.width, -(float)frameSize.height, 0.0, 1.0};
    vkCmdSetViewportWithCount(commandBuffer, 1, &viewport);
    vkCmdSetScissorWithCount(commandBuffer, 1, &renderingInfos[imageIndex].renderArea);
    vkCmdSetRasterizerDiscardEnable(commandBuffer, VK_FALSE);
    if (enableDepthAndStencil == DEPTH || enableDepthAndStencil == DEPTH_AND_STENCIL && !(renderPassMode == ADDIDITIVE)) {
        vkCmdSetDepthTestEnable(commandBuffer, VK_TRUE);
        vkCmdSetDepthWriteEnable(commandBuffer, VK_TRUE);
        vkCmdSetDepthCompareOp(commandBuffer, VK_COMPARE_OP_LESS_OR_EQUAL);
        vkCmdSetDepthBiasEnableEXT(commandBuffer, VK_FALSE);
    } else if (enableDepthAndStencil == DEPTH || enableDepthAndStencil == DEPTH_AND_STENCIL && !(renderPassMode == ADDIDITIVE)) {
        vkCmdSetDepthTestEnable(commandBuffer, VK_FALSE);
        vkCmdSetDepthWriteEnable(commandBuffer, VK_FALSE);
        vkCmdSetDepthCompareOp(commandBuffer, VK_COMPARE_OP_LESS_OR_EQUAL);
        vkCmdSetDepthBiasEnableEXT(commandBuffer, VK_FALSE);

    } else {
        vkCmdSetDepthTestEnable(commandBuffer, VK_FALSE);
        vkCmdSetDepthWriteEnable(commandBuffer, VK_FALSE);
        vkCmdSetDepthBiasEnableEXT(commandBuffer, VK_FALSE);
    }

    if (renderPassMode == ADDIDITIVE) {
        std::vector<VkBool32> colorBlend(attachments[0].colorAttachments.size(), VK_TRUE);
        vkCmdSetColorBlendEnableEXT(commandBuffer, 0, colorBlend.size(), colorBlend.data());
        VkColorBlendEquationEXT t{};
        t.colorBlendOp = VK_BLEND_OP_ADD;
        t.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        t.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
        std::vector<VkColorBlendEquationEXT> colorBlendEquation(attachments[0].colorAttachments.size(), t);
        vkCmdSetColorBlendEquationEXT(commandBuffer, 0, colorBlendEquation.size(), colorBlendEquation.data());

    } else {
        std::vector<VkBool32> colorBlend(attachments[0].colorAttachments.size(), VK_FALSE);
        vkCmdSetColorBlendEnableEXT(commandBuffer, 0, colorBlend.size(), colorBlend.data());
        VkColorBlendEquationEXT t{};
        std::vector<VkColorBlendEquationEXT> colorBlendEquation(attachments[0].colorAttachments.size(), t);
        vkCmdSetColorBlendEquationEXT(commandBuffer, 0, colorBlendEquation.size(), colorBlendEquation.data());
    }
    std::vector<VkColorComponentFlags> writeMasks(
        attachments[0].colorAttachments.size(),
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
    vkCmdSetColorWriteMaskEXT(commandBuffer, 0, writeMasks.size(), writeMasks.data());

    vkCmdSetStencilTestEnable(commandBuffer, VK_FALSE);
}

void DynamicRenderPass::endRenderPass(CommandBuffer &commandBuffer) { vkCmdEndRendering(commandBuffer); }

void DynamicRenderPass::resize(VkExtent2D frameSize) {
    this->frameSize = frameSize;
    renderingInfos.clear();
    attachments.clear();
    imageAttachement.clear();
    depthAndStencilAttachement.clear();
    createRessources();
    createAttachmentInfo();
    createRenderpassInfo();
}

void DynamicRenderPass::createRessources() {
    imageAttachement.resize(numberOfFrame);
    depthAndStencilAttachement.reserve(numberOfFrame);
    for (int i = 0; i < numberOfFrame; i++) {
        if (swapChain != nullptr) {
            imageAttachement[i].emplace_back(swapChain->getSwapChainImage(i));
        }
        std::vector<Image> colorImages;
        ImageCreateInfo imageCreateInfo{};
        imageCreateInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageCreateInfo.height = frameSize.height;
        imageCreateInfo.width = frameSize.width;
        imageCreateInfo.usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        for (auto &imageFormat : imageFormats) {
            imageCreateInfo.format = imageFormat;
            imageAttachement[i].emplace_back(device, imageCreateInfo);  // construction de l'image
        }
        if (enableDepthAndStencil == DEPTH || enableDepthAndStencil == DEPTH_AND_STENCIL) {
            ImageCreateInfo imageCreateInfo{};
            imageCreateInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            imageCreateInfo.usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            imageCreateInfo.format = device->findSupportedFormat(
                {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
            ;
            imageCreateInfo.width = frameSize.width;
            imageCreateInfo.height = frameSize.height;

            depthAndStencilAttachement.emplace_back(device, imageCreateInfo);
        }

        // imageAttachement.push_back(colorImages);
    }
}

void DynamicRenderPass::createAttachmentInfo() {
    if (externalDepthImages != nullptr) {
        assert(
            externalDepthImages->size() == numberOfFrame &&
            "number of external depthImage must be the same as number of image of the renderpass");
    }
    for (int i = 0; i < this->numberOfFrame; i++) {
        AttachementStruct frameAttachement;
        for (auto &image : imageAttachement[i]) {
            auto colorAttachmentInfo = make<VkRenderingAttachmentInfo>();
            colorAttachmentInfo.imageView = image;
            colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachmentInfo.clearValue.color = {0.f, 0.00f, 0.00f, 0.f};
            frameAttachement.colorAttachments.push_back(colorAttachmentInfo);
        }
        if (depthAndStencilAttachement.size() > 0 && externalDepthImages == nullptr) {
            auto depthAttachmentInfo = make<VkRenderingAttachmentInfo>();
            depthAttachmentInfo.imageView = depthAndStencilAttachement[i];
            depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            depthAttachmentInfo.clearValue.depthStencil = {1.0, 0};
            frameAttachement.depthAttachment = depthAttachmentInfo;
        }
        attachments.push_back(frameAttachement);
    }
}

void DynamicRenderPass::createRenderpassInfo() {
    for (int i = 0; i < this->numberOfFrame; i++) {
        auto renderingInfo = make<VkRenderingInfo>();
        renderingInfo.renderArea = {0, 0, frameSize};
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = attachments[i].colorAttachments.size();
        renderingInfo.pColorAttachments = attachments[i].colorAttachments.data();
        if (enableDepthAndStencil == DEPTH || enableDepthAndStencil == DEPTH_AND_STENCIL)
            renderingInfo.pDepthAttachment = &attachments[i].depthAttachment;
        renderingInfos.push_back(renderingInfo);
    }
}

void DynamicRenderPass::setClearColor(glm::vec3 rgb) {
    for (int i = 0; i < this->numberOfFrame; i++) {
        for (auto &a : attachments[i].colorAttachments) {
            a.clearValue = {rgb.r, rgb.g, rgb.b, 0.0};
        }
    }
}

void DynamicRenderPass::transitionColorAttachment(uint32_t frameIndex, VkImageLayout newLayout, CommandBuffer& commandBuffer) {
    for (auto &img : imageAttachement[frameIndex]) {
        img.transitionImageLayout(newLayout, &commandBuffer);
    }
}

}  // namespace vk_stage