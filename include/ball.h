#ifndef BALL_H
#define BALL_H

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>

class Ball 
{

unsigned int stacks;
unsigned int sectors;
float prev_time;

public:
    float radius;
    float x,y,z;
    float vx, vy, vz;
    std::vector<float> vertices;
    std::vector<int> indices;
    std::vector<float> colors;
    Ball(float radius, unsigned int stackCount, unsigned int sectorCount);
    std::vector<float> getInterleavedVertexData();
    void update(float time, glm::mat4 &model, glm::vec3 &normal);
};

#endif