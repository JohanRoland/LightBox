#include "first_app.hpp"

#include "lightBox_buffer.hpp"
#include "renderSystems/simple_render_system.hpp"
#include "renderSystems/point_light_system.hpp"
#include "keyboard_movement_controller.hpp"

#include <stdexcept>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm.hpp>
#include <gtc/constants.hpp>


#include <math.h>

#include <chrono>
#include <iostream>
#include <cgltf.h>
#include <lightBox_texture.hpp>
#include <ddsLoaderImplementation.hpp>


namespace lightBox {
	/*****
	* The last component of the light vectors control light intensity.
	* 
	*/
	struct GlobalUbo {
		
		glm::mat4 projectionMatrix{ 1.0f };
		glm::mat4 viewMatrix{ 1.0f };
		glm::vec4 ambientLightColor{ 1.0f, 1.0f, 1.0f, 0.02f };
		glm::vec3 lightPosition{ -1.0f };
		glm::vec3 cameraPosition{ -1.0f };
		alignas(16) glm::vec4 lightColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	};

	FirstApp::FirstApp()
	{
		gameObjects = LightBoxGameObject::GameObjectMap();
		globalPool = LightBoxDescriptorPool::Builder(lightBoxDevice)
			.setMaxSets(LightBoxSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LightBoxSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();
		/*
		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		auto tCB = lightBoxDevice.beginSingleTimeCommands();
		vkBeginCommandBuffer(tCB, &beginInfo);

		VkImageMemoryBarrier2 preBarrier = imageBarrier(image.image,
			0, 0, VK_IMAGE_LAYOUT_UNDEFINED,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		pipelineBarrier(tCB, 0, 0, nullptr, 1, &preBarrier);
		lightBoxDevice.endSingleTimeCommands(tCB);
		*/

		loadSceen(std::filesystem::path(R"(E:\OldInstallFiles\Users\Johan Ekdahl\source\repos\Resources\Bistro_v5_2.gltf)"));
		std::cout << "Done Loading Sceen!\n";
		//loadGameObjects();
		//loadTextures();
		//mapTextures();
	}
	FirstApp::~FirstApp() {}
	void lightBox::FirstApp::run()
	{
		std::cout << "Starting Run!\n";
		std::vector<std::unique_ptr<LightBoxBuffer>> uboBuffers(LightBoxSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++) {
			uboBuffers[i] = std::make_unique< LightBoxBuffer>(lightBoxDevice,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT); // | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			uboBuffers[i]->map();
		}

		auto globalSetLayout = 
			LightBoxDescriptorSetLayout::Builder(lightBoxDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(LightBoxSwapChain::MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < globalDescriptorSets.size(); i++) {
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			LightBoxDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

		SimpleRenderSystem simpleRenderSystem{ 
			lightBoxDevice, lightBoxRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
		PointLightSystem pointLightSystem{ 
			lightBoxDevice, lightBoxRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };

		auto currentTime = std::chrono::high_resolution_clock::now();

		bool stillRunning = true;
		unsigned int priviusTick = SDL_GetTicks();
		std::cout << "Starting loop!\n";
		while (stillRunning) {
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				unsigned int currentTick = SDL_GetTicks();
				auto delta = currentTick - priviusTick;

				auto newTime = std::chrono::high_resolution_clock::now();
				float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
				currentTime = newTime;

				switch (event.type) 
				{
				case SDL_KEYDOWN:
					cameraController.moveInPlaneXZ(event.key.keysym.sym, frameTime, viewerObject);
					camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
					
					break;
				case SDL_WINDOWEVENT:
					switch (event.window.event) 
					{
					case SDL_WINDOWEVENT_RESIZED:
					case SDL_WINDOWEVENT_SIZE_CHANGED:
					case  SDL_WINDOWEVENT_RESTORED:
						{
							VkExtent2D windowSize = lightBoxWindow.getExtent();							
							camera.updatePerspectivAspect((float)windowSize.height / windowSize.width);
						}
						break;
					default:
						break;
					}
					break;
				case SDL_QUIT:
					stillRunning = false;
					break;
				default:
					// Do nothing.
					break;
				}

				if (delta > 1000 / 60.0)
				{
					float aspect = lightBoxRenderer.getAspectRatio();
					

					priviusTick = currentTick;
					auto commandBuffer = lightBoxRenderer.beginFrame();
					if (commandBuffer != nullptr) {
						int frameIndex = lightBoxRenderer.getFrameIndex();
						FrameInfo frameInfo{
							frameIndex,
							frameTime,
							commandBuffer,
							camera,
							globalDescriptorSets[frameIndex],
							gameObjects
						};

						// setup buffers
						GlobalUbo ubo{};
						ubo.projectionMatrix = camera.getProjection();
						ubo.viewMatrix =camera.getView();

						uboBuffers[frameIndex]->writeToBuffer(&ubo);
						uboBuffers[frameIndex]->flush();


						// render 
						lightBoxRenderer.beginSwapChainRenderPass(commandBuffer);
						simpleRenderSystem.renderGameObjects(frameInfo);
						pointLightSystem.render(frameInfo);
						lightBoxRenderer.endSwapChainRenderPass(commandBuffer);
						lightBoxRenderer.endFrame();
					}
				}
			}
		}

		vkDeviceWaitIdle(lightBoxDevice.device());
	}


	struct Vertex { // TODO: temp

	};
	/*
	cgltf_accessor* findAccessor(const cgltf_primitive const * primitive, cgltf_attribute_type type, cgltf_int index = 0)
	{
		for (size_t i = 0; i < primitive->attributes_count; i++)
		{
			cgltf_attribute& attrib = primitive->attributes[i];
			if (attrib.type == type && attrib.index == index)
			{
				return attrib.data;
			}
		}
		return NULL;
	}
	*/

	void decomposeTransform(float translation[3], float rotation[4], float scale[3], const float* transform)
	{
		float m[4][4] = {};
		memcpy(m, transform, 16 * sizeof(float));

		// extract translation from last row
		translation[0] = m[3][0];
		translation[1] = m[3][1];
		translation[2] = m[3][2];

		// compute determinant to determine handedness
		float det =
			m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2]) -
			m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
			m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

		float sign = (det < 0.f) ? -1.f : 1.f;

		// recover scale from axis lengths
		scale[0] = sqrtf(m[0][0] * m[0][0] + m[0][1] * m[0][1] + m[0][2] * m[0][2]) * sign;
		scale[1] = sqrtf(m[1][0] * m[1][0] + m[1][1] * m[1][1] + m[1][2] * m[1][2]) * sign;
		scale[2] = sqrtf(m[2][0] * m[2][0] + m[2][1] * m[2][1] + m[2][2] * m[2][2]) * sign;

		// normalize axes to get a pure rotation matrix
		float rsx = (scale[0] == 0.f) ? 0.f : 1.f / scale[0];
		float rsy = (scale[1] == 0.f) ? 0.f : 1.f / scale[1];
		float rsz = (scale[2] == 0.f) ? 0.f : 1.f / scale[2];

		float r00 = m[0][0] * rsx, r10 = m[1][0] * rsy, r20 = m[2][0] * rsz;
		float r01 = m[0][1] * rsx, r11 = m[1][1] * rsy, r21 = m[2][1] * rsz;
		float r02 = m[0][2] * rsx, r12 = m[1][2] * rsy, r22 = m[2][2] * rsz;

		// "branchless" version of Mike Day's matrix to quaternion conversion
		int qc = r22 < 0 ? (r00 > r11 ? 0 : 1) : (r00 < -r11 ? 2 : 3);
		float qs1 = qc & 2 ? -1.f : 1.f;
		float qs2 = qc & 1 ? -1.f : 1.f;
		float qs3 = (qc - 1) & 2 ? -1.f : 1.f;

		float qt = 1.f - qs3 * r00 - qs2 * r11 - qs1 * r22;
		float qs = 0.5f / sqrtf(qt);

		rotation[qc ^ 0] = qs * qt;
		rotation[qc ^ 1] = qs * (r01 + qs1 * r10);
		rotation[qc ^ 2] = qs * (r20 + qs2 * r02);
		rotation[qc ^ 3] = qs * (r12 + qs3 * r21);
	}


	bool FirstApp::loadSceen(std::filesystem::path filePath)
	{
		cgltf_options option = {};
		cgltf_data *data = NULL;

		cgltf_result res = cgltf_parse_file(&option, filePath.string().c_str(), &data); // Temporary filePath might cause trubble
		if (res != cgltf_result_success)
		{
			std::cout << "Failed to parse of glft file";
			return false;
		}

		res = cgltf_load_buffers(&option, data, filePath.string().c_str());
		if (res != cgltf_result_success)
		{
			std::cout << "Failed to load buffers of glft file";
			cgltf_free(data);
			return false;
		}

		res = cgltf_validate(data);
		if (res != cgltf_result_success)
		{
			std::cout << "Failed to validate glft file";
			cgltf_free(data);
			return false;
		}

		if (data->cameras_count > 1)
		{
			viewerObject.transform.translation.x = 0.0f;
			viewerObject.transform.translation.y = 0.0f;
			viewerObject.transform.translation.z = 0.0f;
			for (int i = 0; i < data->nodes_count; i++)
			{
				const cgltf_node* currentNode_p = &data->nodes[i];
				if (currentNode_p->camera != NULL)
				{
					float outCameraWorldMatix[16];
					cgltf_node_transform_world(currentNode_p, outCameraWorldMatix);
					float translation[3] = {};
					float rotation[4] = {};
					float scale[3] = {};
					decomposeTransform(translation, rotation, scale, outCameraWorldMatix);
					viewerObject.transform.translation.x = translation[0];
					viewerObject.transform.translation.y = translation[1];
					viewerObject.transform.translation.z = translation[2];
					viewerObject.transform.quaternionRotation = { rotation[3], rotation[0] ,rotation[1] , rotation[2] };
					break;
				}
			}
			auto& firstCamera = data->cameras[0];
			if (cgltf_camera_type_perspective == firstCamera.type) 
			{
				camera.setPerspectiveProjection(
					firstCamera.data.perspective.yfov,
					firstCamera.data.perspective.aspect_ratio,
					firstCamera.data.perspective.znear,
					firstCamera.data.perspective.zfar);
			}
			else
			{
				std::cout << "Unsuported Camera type, setting up deafault";
				camera.setPerspectiveProjection(glm::radians(50.0f), lightBoxRenderer.getAspectRatio(), 0.1f, 200.0f);
			}

		}
		else 
		{
			camera.setPerspectiveProjection(glm::radians(50.0f), lightBoxRenderer.getAspectRatio(), 0.1f, 200.0f);
			viewerObject.transform.translation.x = 50.0f;
			viewerObject.transform.translation.y = 10.0f;
			viewerObject.transform.translation.z = 0.0f;
			viewerObject.transform.rotation = {0.0f, 0.0f , 0.0f };
		}

		for (size_t i = 0; i < data->meshes_count; i++)
		{
			cgltf_mesh& mesh = data->meshes[i];
			const cgltf_primitive& primitive = mesh.primitives[0];
			const size_t vertexCount = primitive.attributes[0].data->count;
			const size_t indicesCount = primitive.indices->count;
			
			assert(primitive.type == cgltf_primitive_type_triangles);
			assert(primitive.indices);
			const cgltf_accessor* possition = cgltf_find_accessor(&primitive, cgltf_attribute_type_position, 0);
			const cgltf_accessor* normal = cgltf_find_accessor(&primitive, cgltf_attribute_type_normal, 0);
			const cgltf_accessor* textureCoord = cgltf_find_accessor(&primitive, cgltf_attribute_type_texcoord, 0);
			const cgltf_accessor* color = cgltf_find_accessor(&primitive, cgltf_attribute_type_color, 0);

			const bool constraintHasPossitionsAndNormals = possition && normal;
			const bool constraintNumberOfPrimitievs = mesh.primitives_count == 1;
			if (!constraintNumberOfPrimitievs) std::cout << "primitives_count != 1\n";

			const auto numPos = (possition ? cgltf_num_components(possition->type) : 0);
			const bool constraintNumberOfPossitions = 3 == numPos;
			const auto numNorm = (normal ? cgltf_num_components(normal->type) : 0);
			const bool constraintNumberOfNormals = 3 == numNorm;

			if ( constraintNumberOfPossitions && constraintNumberOfNormals && constraintHasPossitionsAndNormals)
			{

				LightBoxModel::Builder builderStruct = {
					.vertices =std::vector<LightBoxModel::Vertex>(vertexCount * 4),
					.indices = {std::vector<uint32_t>(indicesCount)}};

				std::vector<float> scrachBuffer(vertexCount * 4);


				//Possitions
				if (NULL != possition)
				{
					cgltf_accessor_unpack_floats(possition, scrachBuffer.data(), vertexCount * 3);

					for (size_t j = 0; j < vertexCount; j++)
					{
						builderStruct.vertices[j].position.x = scrachBuffer[j * 3 + 0];
						builderStruct.vertices[j].position.y = scrachBuffer[j * 3 + 1];
						builderStruct.vertices[j].position.z = scrachBuffer[j * 3 + 2];
					}
				}
				//Normals

				if (NULL != normal)
				{

					cgltf_accessor_unpack_floats(normal, scrachBuffer.data(), vertexCount * 3);

					for (size_t j = 0; j < vertexCount; j++)
					{
						builderStruct.vertices[j].normal.x = scrachBuffer[j * 3 + 0]; // *127f+127.5 ?? //TODO: Check
						builderStruct.vertices[j].normal.y = scrachBuffer[j * 3 + 1]; // *127f+127.5 ??
						builderStruct.vertices[j].normal.z = scrachBuffer[j * 3 + 2]; // *127f+127.5 ??
					}
				}

				//Texture Coordinates
				const bool hasColor = color != NULL;
				if (hasColor)
				{
					cgltf_accessor_unpack_floats(color, scrachBuffer.data(), vertexCount * 3);

					for (size_t j = 0; j < vertexCount; j++)
					{
						builderStruct.vertices[j].color = { scrachBuffer[j * 3 + 0], scrachBuffer[j * 3 + 1], scrachBuffer[j * 3 + 2]};
					}
				}

				//Texture Coordinates
				const bool hasTextureCoord = textureCoord != NULL;
				const auto numTexCord = (hasTextureCoord ? cgltf_num_components(textureCoord->type) : 0);
				const bool constraintNumberOfTexCoords = 2 == numTexCord;
				if (hasTextureCoord && constraintNumberOfTexCoords)
				{
					cgltf_accessor_unpack_floats(textureCoord, scrachBuffer.data(), vertexCount * 2);

					for (size_t j = 0; j < vertexCount; j++)
					{
						builderStruct.vertices[j].uv.x = scrachBuffer[j * 2 + 0];
						builderStruct.vertices[j].uv.y = scrachBuffer[j * 2 + 1];
					}
				}


				for (size_t j = 0; j < vertexCount; j++)
				{
					builderStruct.vertices[j].color = { 255.0f,0.0f,0.0f };// TODO: Fix so that color is set correctly
				}

				builderStruct.indices.reserve(primitive.indices->count);
				cgltf_accessor_unpack_indices(primitive.indices, builderStruct.indices.data(), sizeof(uint32_t), builderStruct.indices.size());

				const cgltf_node* node = &data->nodes[i];

				cgltf_float outWorldMatix[16];
				cgltf_node_transform_world(node, outWorldMatix);

				float translation[3] = {};
				float rotation[4] = {};
				float scale[3] = {};

				decomposeTransform(translation, rotation, scale, outWorldMatix);


				std::shared_ptr<LightBoxModel> lightBoxModel = std::make_unique<LightBoxModel>(lightBoxDevice, builderStruct);
				auto gameObject = LightBoxGameObject::createGameObject();
				gameObject.model = lightBoxModel;
				gameObject.transform.hardCodedMatrixEnabled = false;
				/*
				gameObject.transform.hardCodedMatrix = glm::mat4(
					outWorldMatix[0], outWorldMatix[1], outWorldMatix[2], outWorldMatix[3],
					outWorldMatix[4], outWorldMatix[5], outWorldMatix[6], outWorldMatix[7],
					outWorldMatix[8], outWorldMatix[9], outWorldMatix[10], outWorldMatix[11],
					outWorldMatix[12], outWorldMatix[13], outWorldMatix[14], outWorldMatix[15]);*/




				gameObject.transform.translation = { translation[0],translation[1],translation[2]};
				gameObject.transform.scale = { scale[0], scale[1], scale [2]};
				gameObject.transform.rotation = { 0.0f,0.0f, 0.0f }; //rotation[0], rotation[1], rotation[2]}; // Wrong TODO: fix to use qurturnions proper
				gameObject.transform.quaternionRotation = { rotation[3],  rotation[0], rotation[1], rotation[2]};
	
				/*
				std::cout << "Loaded mesh sucessfully ID: " << i << " Places at : x:" << gameObject.transform.translation.x << " y:" <<
					gameObject.transform.translation.y << " z: " << gameObject.transform.translation.z;
				*/
				gameObjects.emplace(gameObject.getId(), std::move(gameObject));


			}
			else {

				std::cout << "Skiping mesh: " << i << " due to failing to meet constraints.\n"
					<<"constraintNumberOfPrimitievs: " << constraintNumberOfPrimitievs << "\n"
					<< "constraintNumberOfPossitions: " << constraintNumberOfPossitions << "\n"
					<< "constraintNumberOfNormals: " << constraintNumberOfNormals << "\n"
					<< "constraintAccessorsAreNotNull: " << constraintHasPossitionsAndNormals << "\n";

			}
		}
/*
		for (size_t i = 0; i < data->textures_count; ++i)
		{
			cgltf_texture* texture = &data->textures[i];
			assert(texture->image);

			cgltf_image* image = texture->image;
			assert(image->uri);

			std::string ipath = path;
			std::string::size_type pos = ipath.find_last_of('/');
			if (pos == std::string::npos)
				ipath = "";
			else
				ipath = ipath.substr(0, pos + 1);

			std::string uri = image->uri;
			uri.resize(cgltf_decode_uri(&uri[0]));

			std::string::size_type dot = uri.find_last_of('.');
			if (dot != std::string::npos)
				uri.replace(dot, uri.size() - dot, ".dds");

			texturePaths.push_back(ipath + uri);
		}
*/

		imageLoader::DdsLoaderImplementation ddsLoader;
		auto device = lightBoxDevice.device();
		int j=0;
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		lightBoxDevice.createBuffer(128 * 1024 * 1024, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		for (int i = 0; i < data->textures_count; i++)
		{


			cgltf_texture* texture = &data->textures[i];
			if (!texture) {
				std::cout << "texture indexis null, index: " << i << "\n";
				continue;
			}
			cgltf_image* image = texture->image;
			
			if (!image)
			{
				std::cout << "image is null for: " << texture->name << "\n";
				continue;
			}
			//const uint8_t *dataBuffer = nullptr;
			std::unique_ptr<uint8_t> buffer_p;
			//ToDo: find the dds version of the embedded textures
			if (image->uri) {
				std::filesystem::path filePath = std::filesystem::path(std::string(R"(E:\OldInstallFiles\Users\Johan Ekdahl\source\repos\Resources\)") + std::string(image->uri));

				//dataBuffer = cgltf_buffer_view_data(image->buffer_view);
				std::string s = std::string(image->mime_type);

				if (".dds" == filePath.extension()) {
					/*if (3 == j || 4 == j || 6 == j || 7 == j || 10 == j || 11 == j || 12 == j) {
						j++;
						continue;
					}*/
					// SUPER SLOW!!!
					VkPhysicalDeviceMemoryProperties memoryProperties;
					vkGetPhysicalDeviceMemoryProperties(lightBoxDevice.getPhysicalDevice(), &memoryProperties);
					VkResult res = vkResetCommandPool(device, lightBoxDevice.getCommandPool(), 0);
					if (VK_SUCCESS != res)
					{
						std::cout << "vkResetCommandPool failed";
					}
					try {
						vKImage::Texture newTexture;
						imageLoader::createTextureImage(
							filePath,
							memoryProperties,
							lightBoxDevice,
							lightBoxDevice.graphicsQueue(),
							newTexture,
							stagingBuffer,
							stagingBufferMemory);
						//newTexture.filename = image->name;
						textures.push_back(std::move(newTexture));
					} catch(std::format_error e)
					{
						std::cout << e.what() << std::endl;
						continue;
					}
				} else {
					std::cout << image->name << "." << image->mime_type << "\n";
				}
			} else if (image->uri) {
				std::string url = image->uri;
				url.resize(cgltf_decode_uri(&url[0]));
				std::cout << url << "\n";
				//Do stuff
			}

		}
		vkDestroyBuffer(lightBoxDevice.device_, stagingBuffer, nullptr);
		vkFreeMemory(lightBoxDevice.device_, stagingBufferMemory, nullptr);

		std::cout << "loading glft file compleat\n";
		cgltf_free(data);
		return true;
	}

	void FirstApp::loadGameObjects()
	{
		std::shared_ptr<LightBoxModel> lightBoxModel = LightBoxModel::createModelFromFile(lightBoxDevice, "models/smooth_vase.obj");

		auto gameObjectVaseSmooth = LightBoxGameObject::createGameObject();
		gameObjectVaseSmooth.model = lightBoxModel;
		gameObjectVaseSmooth.transform.translation = { -0.25f, 0.0f, 0.0f };
		gameObjectVaseSmooth.transform.scale = { 0.5f, 0.5f, 0.5f };
		gameObjects.emplace(gameObjectVaseSmooth.getId(), std::move(gameObjectVaseSmooth));

		lightBoxModel = LightBoxModel::createModelFromFile(lightBoxDevice, "models/flat_vase.obj");
		auto gameObjectVaseFlat = LightBoxGameObject::createGameObject();
		gameObjectVaseFlat.model = lightBoxModel;
		gameObjectVaseFlat.transform.translation = { 0.5f, 0.0f, 0.0f };
		gameObjectVaseFlat.transform.scale = { 0.5f, 0.25f, 0.5f };
		gameObjects.emplace(gameObjectVaseFlat.getId(), std::move(gameObjectVaseFlat));

		lightBoxModel = LightBoxModel::createModelFromFile(lightBoxDevice, "models/quad.obj");
		auto gameObjecPlane = LightBoxGameObject::createGameObject();
		gameObjecPlane.model = lightBoxModel;
		gameObjecPlane.transform.translation = { 0.0f, 0.0f, 0.0f };
		gameObjecPlane.transform.scale = { 4.0f, 1.0f, 3.0f };
		gameObjects.emplace(gameObjecPlane.getId(), std::move(gameObjecPlane));
	}
	void FirstApp::loadTextures() {



	}
	void FirstApp::mapTextures() {

	}


}