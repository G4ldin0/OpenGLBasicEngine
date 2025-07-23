#pragma once
#include <glad/glad.h>
#include <sstream>
#include <string>
#include <fstream>

class Shader
{
	public:
		GLuint ID;

		Shader(const char* shaderPath, const std::string& vertexSection, const std::string& fragmentSection);
		Shader(const char* vertexPath, const char* fragmentPath);
		void Use() const;
		void SetBool(const std::string& name, bool value) const;
		void SetInt(const std::string& name, int value) const;
		void SetFloat(const std::string& name, float value) const;
};

