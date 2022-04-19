#include "lightBox_pipeline.h"
#include <fstream>
#include <stdexcept>
#include <iostream>

namespace lightBox {

	lightBox::Pipeline::Pipeline(const std::string vertFilePath, const std::string fragFilePath)
	{
		createGraphicsPipeline(vertFilePath, fragFilePath);
	}

	std::vector<char> lightBox::Pipeline::readFile(const std::string & filepath)
	{
		std::ifstream file{ filepath, std::ios::ate | std::ios::binary };
		if (!file.is_open()) {
			throw std::runtime_error("Failed to open file");
		}

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();
		return buffer;
	}
	void Pipeline::createGraphicsPipeline(const std::string & vertFilepath, const std::string & fragFilePath)
	{
		auto vertCode = readFile(vertFilepath);
		auto fragCode = readFile(fragFilePath);

		std::cout << "Vertex Shader Code Size: " << vertCode.size();
		std::cout << "Fragment Shader Code Size: " << fragCode.size();
	}
}