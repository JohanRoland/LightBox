#pragma once
/*
* ToDo: Taken from tutorial 20, rework and go trhough in detail.
*/


#include "LightBox_device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace lightBox {

    class LightBoxDescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(LightBoxDevice& lightBoxDevice) : lightBoxDevice{ lightBoxDevice } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<LightBoxDescriptorSetLayout> build() const;

        private:
            LightBoxDevice& lightBoxDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        LightBoxDescriptorSetLayout(
            LightBoxDevice& lightBoxDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~LightBoxDescriptorSetLayout();
        LightBoxDescriptorSetLayout(const LightBoxDescriptorSetLayout&) = delete;
        LightBoxDescriptorSetLayout& operator=(const LightBoxDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        LightBoxDevice& lightBoxDevice;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class LightBoxDescriptorWriter;
    };

    class LightBoxDescriptorPool {
    public:
        class Builder {
        public:
            Builder(LightBoxDevice& lightBoxDevice) : lightBoxDevice{ lightBoxDevice } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<LightBoxDescriptorPool> build() const;

        private:
            LightBoxDevice& lightBoxDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        LightBoxDescriptorPool(
            LightBoxDevice& LightBoxDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~LightBoxDescriptorPool();
        LightBoxDescriptorPool(const LightBoxDescriptorPool&) = delete;
        LightBoxDescriptorPool& operator=(const LightBoxDescriptorPool&) = delete;

        bool allocateDescriptorSet(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

    private:
        LightBoxDevice& lightBoxDevice;
        VkDescriptorPool descriptorPool;

        friend class LightBoxDescriptorWriter;
    };

    class LightBoxDescriptorWriter {
    public:
        LightBoxDescriptorWriter(LightBoxDescriptorSetLayout& setLayout, LightBoxDescriptorPool& pool);

        LightBoxDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        LightBoxDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        LightBoxDescriptorSetLayout& setLayout;
        LightBoxDescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

}