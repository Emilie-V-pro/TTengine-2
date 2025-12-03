#include "dynamic_renderpass.hpp"

#include <cassert>
#include <cstdint>
#include <glm/fwd.hpp>
#include <vector>

#include "commandBuffer/command_buffer.hpp"
#include "device.hpp"
#include "structs_vk.hpp"
#include "volk.h"

namespace TTe {

DynamicRenderPass::DynamicRenderPass(
    Device* p_device,
    VkExtent2D p_frame_size,
    std::vector<VkFormat> p_image_formats,
    unsigned int p_number_of_frame,
    depthAndStencil p_enable_depth_and_stencil,
    Swapchain* p_swapchain,
    std::vector<Image>* p_external_depth_images)
    : m_number_of_frame(p_number_of_frame),
      m_frame_size(p_frame_size),
      m_depth_and_stencil_mode(p_enable_depth_and_stencil),
      m_swapchain(p_swapchain),
      m_image_formats(p_image_formats),
      m_external_depth_images(p_external_depth_images),
      m_device(p_device) {
    createRessources();
    createAttachmentInfo();
    createRenderpassInfo();
}

DynamicRenderPass::DynamicRenderPass(DynamicRenderPass&& other) {
    m_device = other.m_device;
    m_frame_size = other.m_frame_size;
    m_number_of_frame = other.m_number_of_frame;
    m_depth_and_stencil_mode = other.m_depth_and_stencil_mode;
    m_image_formats = other.m_image_formats;
    m_swapchain = std::move(other.m_swapchain);
    m_external_depth_images = std::move(other.m_external_depth_images);
    m_rendering_infos = other.m_rendering_infos;
    m_attachments = std::move(other.m_attachments);
    m_image_attachement = std::move(other.m_image_attachement);
    m_depth_and_stencil_attachement = std::move(other.m_depth_and_stencil_attachement);
    other.m_device = nullptr;
    other.m_swapchain = nullptr;
    other.m_external_depth_images = nullptr;
}

DynamicRenderPass& DynamicRenderPass::operator=(DynamicRenderPass&& other) {
    if (this != &other) {
        m_device = other.m_device;
        m_frame_size = other.m_frame_size;
        m_number_of_frame = other.m_number_of_frame;
        m_depth_and_stencil_mode = other.m_depth_and_stencil_mode;
        m_image_formats = other.m_image_formats;
        m_swapchain = std::move(other.m_swapchain);
        m_external_depth_images = std::move(other.m_external_depth_images);
        m_rendering_infos = other.m_rendering_infos;
        m_attachments = std::move(other.m_attachments);
        m_image_attachement = std::move(other.m_image_attachement);
        m_depth_and_stencil_attachement = std::move(other.m_depth_and_stencil_attachement);
        other.m_device = nullptr;
        other.m_swapchain = nullptr;
        other.m_external_depth_images = nullptr;
    }
    return *this;
}

DynamicRenderPass::~DynamicRenderPass() {}

void DynamicRenderPass::beginRenderPass(
    CommandBuffer& p_cmd_buffer,
    unsigned p_image_index,
    renderPassModeEnum p_render_pass_mode,
    VkRenderingFlags p_optional_rendering_flag) {
    m_rendering_infos[p_image_index].flags = p_optional_rendering_flag;

    vkCmdBeginRendering(p_cmd_buffer, &m_rendering_infos[p_image_index]);

    m_rendering_infos[p_image_index].flags = 0;
    VkViewport viewport{0, (float)m_frame_size.height, (float)m_frame_size.width, -(float)m_frame_size.height, 0.0, 1.0};
    vkCmdSetViewportWithCount(p_cmd_buffer, 1, &viewport);
    vkCmdSetScissorWithCount(p_cmd_buffer, 1, &m_rendering_infos[p_image_index].renderArea);
    vkCmdSetRasterizerDiscardEnable(p_cmd_buffer, VK_FALSE);
    if (m_depth_and_stencil_mode == DEPTH || (m_depth_and_stencil_mode == DEPTH_AND_STENCIL && !(p_render_pass_mode == ADDIDITIVE))) {
        vkCmdSetDepthTestEnable(p_cmd_buffer, VK_TRUE);
        vkCmdSetDepthWriteEnable(p_cmd_buffer, VK_TRUE);
        vkCmdSetDepthCompareOp(p_cmd_buffer, VK_COMPARE_OP_LESS_OR_EQUAL);
        vkCmdSetDepthBiasEnableEXT(p_cmd_buffer, VK_FALSE);
    } else if (
        m_depth_and_stencil_mode == DEPTH || (m_depth_and_stencil_mode == DEPTH_AND_STENCIL && !(p_render_pass_mode == ADDIDITIVE))) {
        vkCmdSetDepthTestEnable(p_cmd_buffer, VK_FALSE);
        vkCmdSetDepthWriteEnable(p_cmd_buffer, VK_FALSE);
        vkCmdSetDepthCompareOp(p_cmd_buffer, VK_COMPARE_OP_LESS_OR_EQUAL);
        vkCmdSetDepthBiasEnableEXT(p_cmd_buffer, VK_FALSE);

    } else {
        vkCmdSetDepthTestEnable(p_cmd_buffer, VK_FALSE);
        vkCmdSetDepthWriteEnable(p_cmd_buffer, VK_FALSE);
        vkCmdSetDepthBiasEnableEXT(p_cmd_buffer, VK_FALSE);
    }
    if (m_attachments[0].color_attachments.size() != 0) {
        if (p_render_pass_mode == ADDIDITIVE) {
            std::vector<VkBool32> color_blend(m_attachments[0].color_attachments.size(), VK_TRUE);

            vkCmdSetColorBlendEnableEXT(p_cmd_buffer, 0, color_blend.size(), color_blend.data());
            VkColorBlendEquationEXT t{};
            t.colorBlendOp = VK_BLEND_OP_ADD;
            t.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            t.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
            std::vector<VkColorBlendEquationEXT> color_blend_equation(m_attachments[0].color_attachments.size(), t);
            vkCmdSetColorBlendEquationEXT(p_cmd_buffer, 0, color_blend_equation.size(), color_blend_equation.data());

        } else {
            std::vector<VkBool32> color_blend(m_attachments[0].color_attachments.size(), VK_FALSE);
            vkCmdSetColorBlendEnableEXT(p_cmd_buffer, 0, color_blend.size(), color_blend.data());
            VkColorBlendEquationEXT t{};

            std::vector<VkColorBlendEquationEXT> color_blend_equation(m_attachments[0].color_attachments.size(), t);
            vkCmdSetColorBlendEquationEXT(p_cmd_buffer, 0, color_blend_equation.size(), color_blend_equation.data());
        }

        std::vector<VkColorComponentFlags> writeMasks(
            m_attachments[0].color_attachments.size(),
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
        vkCmdSetColorWriteMaskEXT(p_cmd_buffer, 0, writeMasks.size(), writeMasks.data());
    }
    vkCmdSetStencilTestEnable(p_cmd_buffer, VK_FALSE);
}

void DynamicRenderPass::endRenderPass(CommandBuffer& p_cmd_buffer) { vkCmdEndRendering(p_cmd_buffer); }

void DynamicRenderPass::resize(VkExtent2D p_frame_size) {
    this->m_frame_size = p_frame_size;
    m_rendering_infos.clear();
    m_attachments.clear();
    m_image_attachement.clear();
    m_depth_and_stencil_attachement.clear();
    createRessources();
    createAttachmentInfo();
    createRenderpassInfo();
}

void DynamicRenderPass::createRessources() {
    m_image_attachement.resize(m_number_of_frame);
    m_depth_and_stencil_attachement.reserve(m_number_of_frame);
    for (uint32_t i = 0; i < m_number_of_frame; i++) {
        if (m_swapchain != nullptr) {
            m_image_attachement[i].emplace_back(m_swapchain->getSwapChainImage(i));
        }
        std::vector<Image> colorImages;
        ImageCreateInfo image_create_info{};
        image_create_info.image_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        image_create_info.height = m_frame_size.height;
        image_create_info.width = m_frame_size.width;
        image_create_info.usage_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

        int index = 0;
        for (auto& image_format : m_image_formats) {
            image_create_info.format = image_format;
            m_image_attachement[i].emplace_back(m_device, image_create_info);  // construction de l'image
            m_image_attachement[i].back().name = ("DR_" + std::to_string(i) + "_c_att_" + std::to_string(index));
            index++;
        }

        if ((m_depth_and_stencil_mode == DEPTH || m_depth_and_stencil_mode == DEPTH_AND_STENCIL) && !m_external_depth_images) {
            ImageCreateInfo image_create_info{};
            image_create_info.image_layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            image_create_info.usage_flags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            image_create_info.format = m_device->findSupportedFormat(
                {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
            ;
            image_create_info.width = m_frame_size.width;
            image_create_info.height = m_frame_size.height;
            m_depth_and_stencil_attachement.emplace_back(m_device, image_create_info);
            m_depth_and_stencil_attachement.back().name = ("DR_" + std::to_string(i) + "_d_att");
        }

        // m_image_attachement.push_back(colorImages);
    }
}

void DynamicRenderPass::createAttachmentInfo() {
    if (m_external_depth_images != nullptr) {
        assert(
            m_external_depth_images->size() == m_number_of_frame &&
            "number of external depthImage must be the same as number of image of the renderpass");
    }
    for (uint32_t i = 0; i < this->m_number_of_frame; i++) {
        AttachementStruct frame_attachement;
        for (auto& image : m_image_attachement[i]) {
            auto color_attachment_info = make<VkRenderingAttachmentInfo>();
            color_attachment_info.imageView = image;
            color_attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            color_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            color_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            color_attachment_info.clearValue.color = {{0.f, 0.00f, 0.00f, 0.f}};
            frame_attachement.color_attachments.push_back(color_attachment_info);
        }
        if (m_depth_and_stencil_attachement.size() > 0 && m_external_depth_images == nullptr) {
            auto depth_attachment_info = make<VkRenderingAttachmentInfo>();
            depth_attachment_info.imageView = m_depth_and_stencil_attachement[i];
            depth_attachment_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depth_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depth_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            depth_attachment_info.clearValue.depthStencil = {1.0, 0};
            frame_attachement.depth_attachment = depth_attachment_info;
        } else if (m_external_depth_images) {
            auto depth_attachment_info = make<VkRenderingAttachmentInfo>();
            depth_attachment_info.imageView = m_external_depth_images->at(i);
            depth_attachment_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depth_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depth_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            depth_attachment_info.clearValue.depthStencil = {1.0, 0};
            frame_attachement.depth_attachment = depth_attachment_info;
        }

        m_attachments.push_back(frame_attachement);
    }
}

void DynamicRenderPass::createRenderpassInfo() {
    for (uint32_t i = 0; i < this->m_number_of_frame; i++) {
        auto rendering_info = make<VkRenderingInfo>();
        rendering_info.renderArea = {{0, 0}, m_frame_size};
        rendering_info.layerCount = 1;
        rendering_info.colorAttachmentCount = m_attachments[i].color_attachments.size();
        rendering_info.pColorAttachments = m_attachments[i].color_attachments.data();
        if (m_depth_and_stencil_mode == DEPTH || m_depth_and_stencil_mode == DEPTH_AND_STENCIL)
            rendering_info.pDepthAttachment = &m_attachments[i].depth_attachment;
        m_rendering_infos.push_back(rendering_info);
    }
}

void DynamicRenderPass::setClearColor(glm::vec3 p_rgb) {
    for (uint32_t i = 0; i < this->m_number_of_frame; i++) {
        for (auto& a : m_attachments[i].color_attachments) {
            a.clearValue = {{{p_rgb.r, p_rgb.g, p_rgb.b, 0.0}}};
        }
    }
}

void DynamicRenderPass::setClearEnable(bool p_enable) {
    for (uint32_t i = 0; i < this->m_number_of_frame; i++) {
        for (auto& a : m_attachments[i].color_attachments) {
            a.loadOp = p_enable ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        }
    }
}

void DynamicRenderPass::setDepthAndStencil(CommandBuffer& p_cmd_buffer, bool p_enable) {
    if (p_enable) {
        vkCmdSetDepthTestEnable(p_cmd_buffer, VK_TRUE);
        vkCmdSetDepthWriteEnable(p_cmd_buffer, VK_TRUE);

    } else {
        vkCmdSetDepthTestEnable(p_cmd_buffer, VK_FALSE);
        vkCmdSetDepthWriteEnable(p_cmd_buffer, VK_FALSE);
    }
}

void DynamicRenderPass::transitionAttachment(uint32_t p_frame_index, VkImageLayout p_new_layout, CommandBuffer& p_cmd_buffer) {
    for (auto& img : m_image_attachement[p_frame_index]) {
        img.transitionImageLayout(p_new_layout, &p_cmd_buffer);
    }
    m_depth_and_stencil_attachement[p_frame_index].transitionImageLayout(p_new_layout, &p_cmd_buffer);
}

void DynamicRenderPass::transitionColorAttachment(uint32_t frameIndex, VkImageLayout p_new_layout, CommandBuffer& p_cmd_buffer) {
    for (auto& img : m_image_attachement[frameIndex]) {
        img.transitionImageLayout(p_new_layout, &p_cmd_buffer);
    }
}

void DynamicRenderPass::transitionDepthAttachment(uint32_t frameIndex, VkImageLayout p_new_layout, CommandBuffer& p_cmd_buffer) {
    m_depth_and_stencil_attachement[frameIndex].transitionImageLayout(p_new_layout, &p_cmd_buffer);
}

void DynamicRenderPass::savedRenderPass(unsigned p_image_index) {
    for (auto& a : m_image_attachement[p_image_index]) {
        a.saveImageToFile();
    }
    if (m_depth_and_stencil_mode == DEPTH || m_depth_and_stencil_mode == DEPTH_AND_STENCIL) {
        m_depth_and_stencil_attachement[p_image_index].saveImageToFile();
    }
}
}  // namespace TTe