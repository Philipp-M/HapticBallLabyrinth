#pragma once

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#endif
#include <string>

enum ShaderType
{
	VERTEX, FRAGMENT, GEOMETRY, COMPUTE
};

class Shader
{
private:
	GLuint shaderId;

	std::string name;

	ShaderType type;

	GLuint load(const std::string &filename);

public:
	Shader(const std::string &name, const std::string &filename, ShaderType type);

	ShaderType const getType() const;

	GLuint getShaderId() const;

	const std::string &getName() const;

	void setName(std::string &name);
};

