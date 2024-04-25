#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "editor_settings.h"
#include "texture.h"
#include "material.h"
#include "shader.h"

#include <string>
#include <vector>
#include <iostream>
using namespace std;

struct Vertex
{
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;

    glm::vec4 Weights;

    glm::ivec4 Influences;
};

class Mesh
{
public:
    // mesh Data
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture2D *> textures;
    unsigned int VAO;
    string name = "mesh";

    // constructor
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture2D *> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        setupMesh();
    }

    Mesh() {
        vertices.clear();
    }

    // Render the mesh
    void Draw(Material *material)
    {
        // Use material shader
        material->Setup(textures);

        // Draw mesh
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        //for (int i = 0; i < vertices.size(); i++) {
        //    std::cout << vertices[i].Influences.x << " " << vertices[i].Influences.y << vertices[i].Influences.z << std::endl;
        //}

        // Always good practice to set everything back to defaults once configured.
        glActiveTexture(GL_TEXTURE0);
    }

    // Draw without material setting (use shader.use() to set render method)
    void Draw()
    {
        // Draw mesh
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Always good practice to set everything back to defaults once configured.
        glActiveTexture(GL_TEXTURE0);
    }

    vector<Vertex>& GetVertices() { return vertices; }
    vector<Vertex> GetVert() { return vertices; }

    void SetSkin(vector<Vertex> _vertices) {
        for (unsigned int i = 0; i < vertices.size(); i++) {
            vertices[i].Weights = _vertices[i].Weights;
            vertices[i].Influences = _vertices[i].Influences;
        }
    }

    void AgainSetUp() {
        //glBindVertexArray(VAO);
        //// Load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        //for (unsigned int i = 0; i < vertices.size(); i++) {
        //    glBufferSubData(GL_ARRAY_BUFFER, offsetof(Vertex, Weights) + i * sizeof(Vertex), sizeof(glm::vec4), glm::value_ptr(vertices[i].Weights));
        //    glBufferSubData(GL_ARRAY_BUFFER, offsetof(Vertex, Influences) + i * sizeof(Vertex), sizeof(glm::ivec4), glm::value_ptr(vertices[i].Influences));
        //}
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
        //glBindVertexArray(0);
    }

private:
    // Render data
    unsigned int VBO, EBO;

    // Initializes all the buffer objects/arrays
    void setupMesh()
    {
        // Create buffers/arrays
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        // Load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // Again translates to 3/2 floats which translates to a byte array.
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // Set the vertex attribute pointers
        // vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, TexCoords));
        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Tangent));
        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Bitangent));
        // weights
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Weights));
        // ids
        glEnableVertexAttribArray(6);
        glVertexAttribIPointer(6, 4, GL_UNSIGNED_INT, sizeof(Vertex), (void *)offsetof(Vertex, Influences));

        glBindVertexArray(0);
    }
};

class MeshRenderer
{
public:
    Material* material;
    Mesh* mesh;
    bool cast_shadow = true;

public:
    MeshRenderer(Material* _material, Mesh* _mesh) : material(_material), mesh(_mesh) {}
    ~MeshRenderer()
    {
        if (material == nullptr) return;
        delete material;
        material = nullptr;
    }

    void SetMaterial(EMaterialType type)
    {
        delete material;
        material = MaterialManager::CreateMaterialByType(type);
    }

    void Draw()
    {
        if (material->IsValid() && mesh != nullptr)
        {
            switch (material->cullface)
            {
            case E_CULL_FACE::culloff:
                glDisable(GL_CULL_FACE);
                break;
            case E_CULL_FACE::cullfront:
                glEnable(GL_CULL_FACE);
                glCullFace(GL_FRONT);
                break;
            case E_CULL_FACE::cullback:
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
                break;
            default:
                glEnable(GL_CULL_FACE);
                break;
            }

            if (EditorSettings::UsePolygonMode)
            {
                // draw mesh
                glBindVertexArray(mesh->VAO);
                glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);

                // always good practice to set everything back to defaults once configured.
                glActiveTexture(GL_TEXTURE0);
            }
            else
            {
                mesh->Draw(material);
            }
        }
    }

    void PureDraw()
    {
        if (mesh != nullptr)
        {
            mesh->Draw();
        }
    }
};