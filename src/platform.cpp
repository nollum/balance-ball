#include <platform.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
#include <gyro.h>
#include <unistd.h>  
#include <vector>

Platform::Platform(float width, float length, float height, int deltaT) : width(width), length(length), height(height), gyro(deltaT), pitch(0), roll(0), x(0), y(0), z(0)
{    
    sleep(3); // wait for gyroscope to finish setup 

    float half_width = width / 2;
    float half_height = height / 2;
    float half_length = length / 2;
    vertices = {
        -half_width, half_height, half_length, // front top left 
        half_width, half_height, half_length, // front top right 
        -half_width, -half_height, half_length, // front bottom left 
        half_width, -half_height, half_length, // front bottom right 

        -half_width, half_height, -half_length, // back top left 
        half_width, half_height, -half_length, // back top right 
        -half_width, -half_height, -half_length, // back bottom left 
        half_width, -half_height, -half_length, // back bottom right 
    };

    indices = {
        // front face
        0, 1, 3, 
        0, 2, 3,
        
        // top face
        0, 4, 5,
        0, 1, 5,

        // bottom face
        2, 3, 6,
        3, 7, 6,

        // right face 
        1, 3, 7,
        1, 5, 7,

        // left face
        0, 4, 6,
        0, 2, 6,

        // back face
        4, 5, 7,
        4, 6, 7
    };

    color = {
        0.0f, 1.0f, 1.0f, // front top left - cyan
        1.0f, 0.0f, 1.0f,  // front top right - magenta
        1.0f, 1.0f, 0.0f, // front bottom left - yellow
        0.5f, 0.5f, 0.5f, // front bottom right - grey

        0.0f, 1.0f, 0.0f, // back top left - green
        1.0f, 0.5f, 0.0f,  // back top right - orange
        0.0f, 0.0f, 1.0f, // back bottom left - blue
        1.0f, 0.0f, 0.0f, // back bottom right - red
    };

}

void Platform::update(float time) {
    gyro.cur_time = time;
    gyro.updateData();
}

void Platform::rotate(glm::mat4 &model) {
    glm::quat quaternion; 
    glm::vec3 euler(gyro.pitch, 0, gyro.roll);
    quaternion = glm::quat(euler);
    rotationMatrix = glm::toMat4(quaternion);
    model = rotationMatrix * model;
}

std::vector<float> Platform::getInterleavedVertexData() {
    std::vector<float> data;
    for (int i = 0; i < vertices.size(); i += 3) {
        data.push_back(vertices[i]);
        data.push_back(vertices[i+1]);
        data.push_back(vertices[i+2]);
        data.push_back(color[i]);
        data.push_back(color[i+1]);
        data.push_back(color[i+2]);
    }
    return data;
}

