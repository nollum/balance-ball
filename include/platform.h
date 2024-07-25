#ifndef PLATFORM_H 
#define PLATFORM_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
#include <gyro.h>
#include <vector>

class Platform 
{

Gyro gyro;

public:
    float pitch;
    float roll;
    float x,y,z;
    float width, height, length;
    glm::mat4 rotationMatrix;
    std::vector<float> vertices;
    std::vector<int> indices;
    std::vector<float> color;
    Platform(float width, float length, float height, int deltaT);
    void rotate(glm::mat4 &model);
    void update(float time);
    std::vector<float> getInterleavedVertexData();
};

#endif