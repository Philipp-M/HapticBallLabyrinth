#define GLM_ENABLE_EXPERIMENTAL

#include <tiny_obj_loader.hpp>
#include <iostream>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include "GraphicsModel.hpp"


void
GraphicsModel::computeVertices(tinyobj::mesh_t& mesh)
{
    indexData = mesh.indices;

    for (int i = 0; i < mesh.positions.size() / 3; i++)
    {
        vertexData[i] = Vertex(
            glm::vec3(mesh.positions[i * 3], mesh.positions[i * 3 + 1], mesh.positions[i * 3 + 2]),
            glm::vec3());

        if (mesh.texcoords.size() >= i * 2 + 1)
        {
            /** u and v coordinates,
             *  1.0 - mesh.textcoords[i * 2 + 1]: OpenGL y texture coordinates opposite to the
             * coordinates used with meshes.
             */
            vertexData[i].textureCoordinate
                = glm::vec2(mesh.texcoords[i * 2], 1.0 - mesh.texcoords[i * 2 + 1]);
        }

        centroid += vertexData[i].vertex;
    }

    centroid = centroid / (float) (vertexData.size());

    if (mesh.normals.size() != mesh.positions.size())
    {
        calculateVertexNormals();
    }
    else
    {
        for (int i = 0; i < mesh.normals.size() / 3; i++)
        {
            vertexData[i].normal = glm::normalize(
                glm::vec3(mesh.normals[i * 3], mesh.normals[i * 3 + 1], mesh.normals[i * 3 + 2]));
        }
    }
}

void
GraphicsModel::calculateVertexNormals()
{
    for (int f = 0; f < indexData.size(); f += 3)
    {
        Vertex& v0 = vertexData[indexData[f]];
        Vertex& v1 = vertexData[indexData[f + 1]];
        Vertex& v2 = vertexData[indexData[f + 2]];

        // calculateFaceNormal
        glm::vec3 fn = glm::normalize(glm::cross(v2.vertex - v1.vertex, v0.vertex - v1.vertex));
        v0.normal += fn;
        v1.normal += fn;
        v2.normal += fn;
    }

    for (auto& v : vertexData)
    {
        v.normal = glm::normalize(v.normal);
    }
}

void
GraphicsModel::initializeBuffers()
{
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vertex), &vertexData[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(GLint), &indexData[0], GL_STATIC_DRAW);
}

void
GraphicsModel::updateModelMatrix()
{
    modelMatrix = rotationMatrixAxis * translationMatrix * rotationMatrixModelOrigin;
}

GraphicsModel::GraphicsModel(tinyobj::mesh_t&          mesh,
                             std::string&              name,
                             std::shared_ptr<Material> material)
: name(name)
, centroid(glm::vec3(0))
, vertexData(mesh.positions.size() / 3)
, material(material)
, translationMatrix(glm::mat4(1.0f))
, rotationMatrixAxis(glm::mat4(1.0f))
, rotationMatrixModelOrigin(glm::mat4(1.0f))
, modelMatrix(1.0f)
{
    computeVertices(mesh);

    std::cout << "Added model: " << name << " - number vertices: " << vertexData.size()
              << " - centroid: " << centroid.x << "/" << centroid.y << "/" << centroid.z
              << " - material: " << material->name << std::endl;
    initializeBuffers();
}

GraphicsModel::~GraphicsModel()
{
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
}

void
GraphicsModel::draw(ShaderProgram& shaderProgram)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(shaderProgram.attributeLocation("position"));
    shaderProgram.vertexAttribPointer("position", 3, GL_FLOAT, sizeof(Vertex), nullptr, false);
    glEnableVertexAttribArray(shaderProgram.attributeLocation("normal"));
    shaderProgram.vertexAttribPointer(
        "normal", 3, GL_FLOAT, sizeof(Vertex), (void*) (sizeof(glm::vec3)), false);
    //  glEnableVertexAttribArray(shaderProgram.attributeLocation("texCoord"));
    //  shaderProgram.vertexAttribPointer("texCoord", 2, GL_FLOAT, sizeof(Vertex3), (void *) (3 *
    //  sizeof(glm::vec3)), false);
    shaderProgram.setMatrixUniform4f("modelMatrix", getModelMatrix());
    shaderProgram.setMatrixUniform3f("normalMatrix",
                                     glm::mat3(glm::transpose(glm::inverse(getModelMatrix()))));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    if (material != nullptr)
    {
        shaderProgram.setUniform3f("diffuseColor", material->diffuseColor.rgb);
        shaderProgram.setUniform3f("specularColor", material->specularColor.rgb);
        shaderProgram.setUniform1f("shininess", material->shininess);
    }

    GLint size;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    glDrawElements(GL_TRIANGLES, (GLsizei)(size / sizeof(GLuint)), GL_UNSIGNED_INT, nullptr);
}

const std::string&
GraphicsModel::getName() const
{
    return name;
}

const glm::vec3&
GraphicsModel::getCentroid() const
{
    return centroid;
}

const std::shared_ptr<Material>&
GraphicsModel::getMaterial() const
{
    return material;
}

const glm::mat4&
GraphicsModel::getModelMatrix() const
{
    return modelMatrix;
}

void
GraphicsModel::rotateAroundAxisX(float angle)
{
    //    float cosA = std::cos(glm::radians(angle));
    //    float sinA = std::sin(glm::radians(angle));
    //    glm::mat4 rotationMatX(1.0, 0.0, 0.0, 0.0,
    //                           0.0, cosA, -sinA, 0.0,
    //                           0.0, sinA, cosA, 0.0,
    //                           0.0, 0.0, 0.0, 1.0);
    //    rotationMatrixAxis *= rotationMatX;
    rotationMatrixAxis
        *= glm::rotate(rotationMatrixAxis, glm::radians(angle), glm::vec3(1.0f, 0.0f, 0.0f));
    updateModelMatrix();
}

void
GraphicsModel::rotateAroundAxisZ(float angle)
{
    //    float cosA = std::cos(glm::radians(angle));
    //    float sinA = std::sin(glm::radians(angle));
    //    glm::mat4 rotationMatZ(cosA, -sinA, 0.0, 0.0,
    //                           sinA, cosA, 0.0, 0.0,
    //                           0.0, 0.0, 1.0, 0.0,
    //                           0.0, 0.0, 0.0, 1.0);
    //    rotationMatrixAxis *= rotationMatZ;
    rotationMatrixAxis
        *= glm::rotate(rotationMatrixAxis, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
    updateModelMatrix();
}

void
GraphicsModel::rotateAroundModelOrigin(glm::mat4& rotationMat)
{
    rotationMatrixModelOrigin = rotationMat * rotationMatrixModelOrigin;
    updateModelMatrix();
}

void
GraphicsModel::translate(glm::vec3& translationVec)
{
    translationMatrix = glm::translate(translationMatrix, translationVec);
    updateModelMatrix();
}

void
GraphicsModel::mirror(glm::vec4& axis)
{
    translationMatrix *= axis;
}

void
GraphicsModel::resetRotationMatrixAxis()
{
    rotationMatrixAxis = glm::mat4(1.0);
    updateModelMatrix();
}

void
GraphicsModel::resetRotationMatrixModelOrigin()
{
    rotationMatrixModelOrigin = glm::mat4(1.0);
    updateModelMatrix();
}

void
GraphicsModel::resetTranslationMatrix()
{
    translationMatrix = glm::mat4(1.0);
    updateModelMatrix();
}
