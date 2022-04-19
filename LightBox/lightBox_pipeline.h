#pragma once

#include <string>
#include <vector>

namespace lightBox {
	class Pipeline {
	public:
		Pipeline(const std::string vertFilePath, const std::string fragFilePath);
	private:
		static std::vector<char> readFile(const std::string& filepath);

		void createGraphicsPipeline(const std::string& vetFilepath, const std::string& fragFilePath);

	};
}