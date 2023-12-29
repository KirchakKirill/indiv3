#ifndef MESH_H
#define MESH_H
#include "Headers.h"

struct Vertex//��������� ��� ������
{
    glm::vec3 Positions;
    glm::vec3 Normals;
    glm::vec2 TextureCoordinats;
};

struct Texture //��������� ��� ��������
{
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh 
{
public:
    // ������ ��� ������� ����
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture>      textures;
    unsigned int VAO, VBO, EBO;;

   
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        
        setupMesh();
    }

    void setupMesh()
    {
        // ��������� ������
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        // �������� ������� � �����
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        //�������� �������
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        //������������� ��������� ������ ��� ���������� ����.
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normals));
        
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TextureCoordinats));

        glBindVertexArray(0);
    }

    void Draw(GLuint shader_id, GLint count)
    {
        //��������� ��������
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;

        for (unsigned int i = 0; i < textures.size(); i++)
        {
            //���������� ������ ��������
            glActiveTexture(GL_TEXTURE0 + i); 

            
            std::string number;
            std::string name = textures[i].type;
           
            // �������� � ������
            glUniform1i(glGetUniformLocation(shader_id, (name + number).c_str()), i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        glBindVertexArray(VAO);
        glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0, count);
        glBindVertexArray(0);

        // ��������� ��������
        glActiveTexture(GL_TEXTURE0);
    }
};

#endif