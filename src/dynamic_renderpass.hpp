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
        std::vector<VkRenderingAttachmentInfo> color_attachments;
        VkRenderingAttachmentInfo depth_attachment;
    };


   public:
   DynamicRenderPass() = default;
    DynamicRenderPass(
        Device *p_device,
        VkExtent2D p_frame_size,
        std::vector<VkFormat> p_image_formats,
        unsigned int p_number_of_frame,
        depthAndStencil p_enable_depth_and_stencil,
        Swapchain *p_swapchain = nullptr,
        std::vector<Image> *p_external_depth_images = {}
        );
    ~DynamicRenderPass();

    // remove copy constructor
    DynamicRenderPass(const DynamicRenderPass &) = delete;
    DynamicRenderPass &operator=(const DynamicRenderPass &) = delete;

    // move constructor
    DynamicRenderPass(DynamicRenderPass &&other);
    DynamicRenderPass &operator=(DynamicRenderPass &&other);

    void beginRenderPass(CommandBuffer & p_cmd_buffer, unsigned p_image_index, renderPassModeEnum p_render_pass_mode = DEFAULT, VkRenderingFlags p_optional_rendering_flag = 0);
    void endRenderPass(CommandBuffer & p_cmd_buffer);

    void savedRenderPass(unsigned p_image_index);
    void resize(VkExtent2D p_frame_size);
    void setClearColor(glm::vec3 p_rgb);
    void setClearEnable(bool p_enable);
    void setDepthAndStencil(CommandBuffer &p_cmd_buffer, bool p_enable);
    void transitionAttachment(uint32_t p_frame_index, VkImageLayout p_new_layout, CommandBuffer &p_cmd_buffer);
    void transitionDepthAttachment(uint32_t p_frame_index, VkImageLayout p_new_laysout, CommandBuffer &p_cmd_buffer);
    void transitionColorAttachment(uint32_t p_frame_index, VkImageLayout p_new_layout, CommandBuffer &cmd_buffer);


    std::vector<Image>& getDepthAndStencilAttachement(){return m_depth_and_stencil_attachement;};
    std::vector<Image>* getDepthAndStencilPtr(){return &m_depth_and_stencil_attachement;}
    std::vector<std::vector<Image>>& getimageAttachement(){return m_image_attachement;};
    VkExtent2D getFrameSize() const {return m_frame_size;};

   private:
    void createRessources();
    void createAttachmentInfo();
    void createRenderpassInfo();

    unsigned int m_number_of_frame = 0;
    VkExtent2D m_frame_size = {0, 0};
    depthAndStencil m_depth_and_stencil_mode = NONE;
    Swapchain *m_swapchain = nullptr;

    std::vector<AttachementStruct> m_attachments;
    std::vector<std::vector<Image>> m_image_attachement;
    std::vector<Image> m_depth_and_stencil_attachement;
    std::vector<VkFormat> m_image_formats;
    std::vector<Image> *m_external_depth_images;
    std::vector<VkRenderingInfo> m_rendering_infos;

    Device *m_device = nullptr;
};
}  // namespace vk_stage