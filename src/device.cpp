
#include "device.hpp"

namespace TTe {

Device::Device(Window &window) {
    createInstance();
    selectPhysicalDevice(window);
}

void Device::createInstance() {
    vkb::InstanceBuilder instance_builder;
    instance_builder.set_app_name("TTEngine 2.0").set_engine_name("TTEngine 2.0").require_api_version(1, 3, 0);

    if (enableValidationLayers) {
        instance_builder.request_validation_layers()
            .use_default_debug_messenger();  //.add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT);
    }
    auto instance_builder_return = instance_builder.build();

    if (!instance_builder_return) {
        throw std::runtime_error(instance_builder_return.error().message());
    }
    vkbInstance = std::move(instance_builder_return.value());
    volkInitialize();
    volkLoadInstance(vkbInstance.instance);
}

void Device::selectPhysicalDevice(Window &window) {
    vkb::PhysicalDeviceSelector phys_device_selector(vkbInstance);
    phys_device_selector.set_surface(window.getSurface(vkbInstance));
}

Device::~Device() {
    // volkFinalize();
    vkb::destroy_instance(vkbInstance);
}

const VkDevice &Device::operator()() { return device; }

}  // namespace TTe