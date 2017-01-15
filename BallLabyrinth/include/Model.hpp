
#pragma once

#define GLM_FORCE_RADIANS
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <memory>
#include "tiny_obj_loader.hpp"
#include "ShaderProgram.hpp"
#include "MaterialManager.hpp"

class Model {
public:
	struct Vertex {
		Vertex(const glm::vec3 &vertex = glm::vec3(), const glm::vec3 &normal = glm::vec3(), const glm::vec2 &textureCoordinate = glm::vec2()) :
				vertex(vertex), normal(normal), textureCoordinate(textureCoordinate)
		{}

		glm::vec3 vertex;
		glm::vec3 normal;
		glm::vec2 textureCoordinate;
	};

private:
	GLuint vbo;
	GLuint ibo;

	bool bufferObjectsLoaded;

	std::string name;
	std::vector<Model::Vertex> vertexData;
	std::vector<GLuint> indexData;
	glm::vec3 centroid;
    std::shared_ptr<Material> material;
	glm::mat4 objectTransformationMatrix;

	void computeVertices(tinyobj::mesh_t &mesh);
	void calculateVertexNormals();

public:
	Model(tinyobj::mesh_t &mesh, std::string &name, std::shared_ptr<Material> material = nullptr);

	void draw(ShaderProgram &shaderProgram);

	void refreshBuffers();

    const glm::vec3 &getCentroid() const;

    const std::shared_ptr<Material> &getMaterial() const;

    const glm::mat4 &getObjectTransformationMatrix() const;

	void rotateAroundX(float angle);

    void rotateAroundZ(float angle);
};



