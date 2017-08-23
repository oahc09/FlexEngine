#pragma once

#include <string>
#include <vector>

#include <vulkan\vulkan.h>

#include "ShaderUtils.h"
#include "VulkanBuffer.h"
#include "VertexBufferData.h"
#include "VDeleter.h"

namespace flex
{
	std::string VulkanErrorString(VkResult errorCode);

#ifndef VK_CHECK_RESULT
#define VK_CHECK_RESULT(f)																				\
{																										\
	VkResult res = (f);																					\
	if (res != VK_SUCCESS)																				\
	{																									\
		std::cerr << "Vulkan fatal error: VkResult is \"" << VulkanErrorString(res) << "\" in " << __FILE__ << " at line " << __LINE__ << std::endl; \
		assert(res == VK_SUCCESS);																		\
	}																									\
}
#endif // VK_CHECK_RESULT

	namespace Vulkan
	{
		VkVertexInputBindingDescription GetVertexBindingDescription(VertexBufferData* vertexBufferData);

		void GetVertexAttributeDescriptions(VertexBufferData* vertexBufferData,
			std::vector<VkVertexInputAttributeDescription>& attributeDescriptions);

	} // namespace Vulkan

	struct QueueFamilyIndices
	{
		int graphicsFamily = -1;
		int presentFamily = -1;

		bool IsComplete()
		{
			return graphicsFamily >= 0 && presentFamily >= 0;
		}
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct UniformBufferObjectData
	{
		Uniform::Type elements;
		float* data = nullptr;
		glm::uint size;
	};

	struct UniformBuffer
	{
		UniformBuffer(const VDeleter<VkDevice>& device);

		VulkanBuffer constantBuffer;
		VulkanBuffer dynamicBuffer;
		UniformBufferObjectData constantData;
		UniformBufferObjectData dynamicData;
	};

	struct VulkanTexture
	{
		VulkanTexture(const VDeleter<VkDevice>& device);

		VDeleter<VkImage> image;
		VDeleter<VkDeviceMemory> imageMemory;
		VDeleter<VkImageView> imageView;
		VDeleter<VkSampler> sampler;
		std::string filePath;
	};

	struct RenderObject
	{
		RenderObject(const VDeleter<VkDevice>& device);

		VkPrimitiveTopology topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		RenderID renderID;

		glm::uint VAO;
		glm::uint VBO;
		glm::uint IBO;

		VertexBufferData* vertexBufferData = nullptr;
		glm::uint vertexOffset = 0;

		bool indexed = false;
		std::vector<glm::uint>* indices = nullptr;
		glm::uint indexOffset = 0;

		std::string fragShaderFilePath;
		std::string vertShaderFilePath;

		glm::uint descriptorSetLayoutIndex;

		glm::uint shaderIndex;

		std::string diffuseTexturePath;
		VulkanTexture* diffuseTexture = nullptr;

		std::string normalTexturePath;
		VulkanTexture* normalTexture = nullptr;

		std::string specularTexturePath;
		VulkanTexture* specularTexture = nullptr;

		VkDescriptorSet descriptorSet;

		VDeleter<VkPipelineLayout> pipelineLayout;
		VDeleter<VkPipeline> graphicsPipeline;
	};

	typedef std::vector<RenderObject*>::iterator RenderObjectIter;


	VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);

	void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);
} // namespace flex