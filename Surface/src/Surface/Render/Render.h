#pragma once

namespace Surface {
namespace Render {

	class Domain
	{
	private:
		std::string vertexShaderSrc;
		std::string fragmentShaderSrc;
	public:
		Domain() {}
		~Domain() {}

		void LoadShader();
		void DrawTriangle();

	};

}
}
