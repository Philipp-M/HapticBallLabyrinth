
#pragma once

#include <memory>

#define GLM_FORCE_RADIANS

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "tiny_obj_loader.hpp"
#include "ShaderProgram.hpp"
#include "MaterialManager.hpp"

/**
 * Class representing the graphical objects in the scene.
 */
class GraphicsModel {
public:
    /**
     * Struct representing a vertex of a model.
     */
    struct Vertex {
        /**
         * Constructor for a vertex.
         * @param vertex Vertex position.
         * @param normal Vertex normal.
         * @param textureCoordinate Texture coordinate of vertex.
         */
        Vertex(const glm::vec3 &vertex = glm::vec3(), const glm::vec3 &normal = glm::vec3(),
               const glm::vec2 &textureCoordinate = glm::vec2()) :
                vertex(vertex), normal(normal), textureCoordinate(textureCoordinate) {}

        glm::vec3 vertex; /**< Vertex position. */
        glm::vec3 normal; /**< Vertex normal. */
        glm::vec2 textureCoordinate; /**< Texture coordinate (currently not used). */
    };

private:
    GLuint vbo; /**< Vertex Buffer Object, vertex data of the model on the GPU. */
    GLuint ibo; /**< Index Buffer Object, index data of the model on the GPU. */

    std::string name; /**< Model name. */
    std::vector<GraphicsModel::Vertex> vertexData; /**< Container containing all the vertex data of the model. */
    std::vector<GLuint> indexData; /**< Container containing all the indices of the model. */
    glm::vec3 centroid; /**< Center of the model. */
    std::shared_ptr<Material> material; /**< Material of the model. */
    glm::mat4 translationMatrix; /**< Absolute translation matrix. */
    glm::mat4 rotationMatrixAxis; /**< Rotation matrix for rotations after model translation. */
    glm::mat4 rotationMatrixModelOrigin; /**< Rotation matrix for rotations around the model origin. */
    glm::mat4 modelMatrix; /**< Model matrix = rotationMatrixAxis * translationMatrix * rotationMatrixModelOrigin. */

    /**
     * Computes all vertices using the input from tinyobj.
     * @param mesh Input mesh from tinyobj.
     */
    void computeVertices(tinyobj::mesh_t &mesh);

    /**
     * Calculation of vertex normals, if not available in .obj file.
     */
    void calculateVertexNormals();

    /**
     * Write vbo and ibo to graphics card.
     */
    void initializeBuffers();

    /**
     * Updates model matrix.
     * Model matrix = rotationMatrixAxis * translationMatrix * rotationMatrixModelOrigin.
     */
    void updateModelMatrix();

public:
    /**
     * Constructor for a graphics model.
     * @param mesh tinyobj mesh containing the vertices and indices of the model.
     * @param name Model name..
     * @param material Model material.
     */
    GraphicsModel(tinyobj::mesh_t &mesh, std::string &name, std::shared_ptr<Material> material = nullptr);

    /**
     * Destructor for graphics model.
     * Deletes vbo and ibo.
     */
    ~GraphicsModel();

    /**
     * Draw model (All necessary opengl commands).
     * @param shaderProgram Shader program used to draw model.
     */
    void draw(ShaderProgram &shaderProgram);


    /**
     * Get model name.
     * @return Model name.
     */
    const std::string &getName() const;

    /**
     * Get model centroid position.
     * @return Model centroid position.
     */
    const glm::vec3 &getCentroid() const;

    /**
     * Get model material.
     * @return Model material.
     */
    const std::shared_ptr<Material> &getMaterial() const;

    /**
     * Get model matrix.
     * @return Model matrix.
     */
    const glm::mat4 &getModelMatrix() const;

    /**
     * Rotate model around x axis (after translation).
     * @param angle Rotation angle.
     */
    void rotateAroundAxisX(float angle);

    /**
     * Rotate model around z axis (after translation).
     * @param angle Rotation angle.
     */
    void rotateAroundAxisZ(float angle);

    /**
     * Rotate model around its origin (before translation).
     * @param rotationMat Rotation matrix for rotating the model around its origin.
     */
    void rotateAroundModelOrigin(glm::mat4 &rotationMat);

    /**
     * Translate model.
     * @param translationVec Translation from absolute position of the model.
     */
    void translate(glm::vec3 &translationVec);

    /**
     * Mirror model around axis.
     * @param axis Vector pointing along the axis around which the model is mirrored.
     */
    void mirror(glm::vec4 &axis);

    /**
     * Reset rotation matrix to identity.
     */
    void resetRotationMatrixAxis();

    /**
     * Reset rotation matrix around model origin to identity.
     */
    void resetRotationMatrixModelOrigin();

    /**
     * Reset translation matrix to identitiy.
     */
    void resetTranslationMatrix();
};



