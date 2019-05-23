#include "spch.h"

#include "Render.h"
#include "Surface/IO.h"
#include <glad/glad.h>

namespace Surface {
namespace Render {

	void Domain::LoadShader()
	{
		vertexShaderSrc = ReadFile("assets/shaders/basic.vert");
		fragmentShaderSrc = ReadFile("assets/shaders/basic.frag");
	}

	void Domain::DrawTriangle()
	{
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		const char* vert_src = vertexShaderSrc.c_str();
		glShaderSource(vertexShader, 1, &vert_src, NULL);
		glCompileShader(vertexShader);

		GLint vertex_compiled;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertex_compiled);
		if (vertex_compiled != GL_TRUE)
		{
			GLchar message[1024];
			glGetShaderInfoLog(vertexShader, 1024, NULL, message);

			SURF_CORE_ERROR("Vertex shader failed to compile: {0}", message);
		}

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		const char* frag_src = fragmentShaderSrc.c_str();
		glShaderSource(fragmentShader, 1, &frag_src, NULL);
		glCompileShader(fragmentShader);

		GLint fragment_compiled;
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragment_compiled);
		if (fragment_compiled != GL_TRUE)
		{
			GLchar message[1024];
			glGetShaderInfoLog(fragmentShader, 1024, NULL, message);

			SURF_CORE_ERROR("Fragment shader failed to compile: {0}", message);
		}

		GLuint program = glCreateProgram();
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		glLinkProgram(program);

		GLint program_linked;
		glGetProgramiv(program, GL_LINK_STATUS, &program_linked);
		if (program_linked != GL_TRUE)
		{
			GLchar message[1024];
			glGetProgramInfoLog(program, 1024, NULL, message);

			SURF_CORE_ERROR("Shader program failed to link: {0}", message);
		}

		GLint position_location = glGetAttribLocation(program, "vPosition");
		GLint color_location = glGetAttribLocation(program, "vColor");

		glUseProgram(program);
		
		float points[] = {
		//    x      y     z       r     g     b     a
			 0.0f,  0.5f, 0.f,    1.0f, 0.2f, 0.2f, 1.0f,
			 0.5f, -0.5f, 0.f,    0.2f, 1.0f, 0.2f, 1.0f,
			-0.5f, -0.5f, 0.f,    0.2f, 0.2f, 1.0f, 1.0f
		};
		
		// Vertex Buffer Initialization
		GLuint vertexBuffer;
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

		// Vertex Array Initialization
		GLuint vertexArray;
		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		glEnableVertexAttribArray(position_location);
		glEnableVertexAttribArray(color_location);

		glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);
		glVertexAttribPointer(color_location, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));

		glDrawArrays(GL_TRIANGLES, 0, 3);

		glDisableVertexAttribArray(position_location);
		glDisableVertexAttribArray(color_location);

		glDeleteVertexArrays(1, &vertexArray);
	}

}
}
