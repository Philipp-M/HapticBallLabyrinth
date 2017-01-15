#version 330

uniform mat4 MVPMatrix;
uniform mat4 modelMatrix;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 vNormal;
out vec3 vPosition;

void main()
{
	gl_Position = MVPMatrix * vec4(position.x, position.y, position.z, 1.0);
	vPosition = position;
	vNormal = normal;
}
/*#version 330

uniform mat4 MVPMatrix;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

//out vec3 vNormal;
//out vec3 vPosition;
//out vec3 vPosView;

void main()
{

	gl_Position = MVPMatrix * vec4(position.x, position.y, position.z, 1.0);
	//vNormal = normal;
}*/
