#include <ball.h>
#include <vector>
#include <cmath>
#include <random>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>

#define PI 3.14
#define deltaT 0.05

static float theta = 0;
static glm::mat4 orientation = glm::mat4(1.0f);

Ball::Ball(float radius, unsigned int stackCount, unsigned int sectorCount) : x(0), y(0), z(0), vx(0), vy(0), vz(0), radius(radius) {
    srand(time(NULL));
    float sectorStep = 2 * PI / sectorCount; 
    float stackStep = PI / stackCount;
    float sectorAngle, stackAngle;
    float x,y,z;

    // generate vertices based on stack and sector counts
    for (int i = 0; i <= sectorCount; i++) {

        stackAngle = PI / 2 - i * stackStep;
        z = radius * sin(stackAngle);

        for (int j = 0; j <= stackCount; j++) {

            sectorAngle = j * sectorStep;

            x = radius * cos(sectorAngle) * cos(stackAngle);
            y = radius * cos(stackAngle) * sin(sectorAngle);

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            colors.push_back(r);
            r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            colors.push_back(r);
            r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            colors.push_back(r);
        }
    }

    // generate index list for sphere vertices
    // top and bottom stacks only require one triangle but inner stacks require 2

    std::vector<int> lineIndices; // optional

    int p1, p2;
    for (int i = 0; i < stackCount; i++) {
        p1 = i * (sectorCount + 1);
        p2 = p1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; j++, p1++, p2++) {

            if (i != 0) {
                indices.push_back(p1);
                indices.push_back(p2);
                indices.push_back(p1 + 1);
            }

            if (i != (stackCount-1)) {
                indices.push_back(p1 + 1);
                indices.push_back(p2);
                indices.push_back(p2 + 1);
            }

            lineIndices.push_back(p1);
            lineIndices.push_back(p2);
            if (i != 0)  
            {
                lineIndices.push_back(p1);
                lineIndices.push_back(p1 + 1);
            }
        }
    }

}

void Ball::update(float time, glm::mat4 &model, glm::vec3 &collision_normal) {

    if (collision_normal != glm::vec3(0,0,0) & collision_normal != glm::vec3(0, 1.0f, 0)) {
        glm::vec3 direction = collision_normal - glm::vec3(0, 1.0f, 0);
        direction = glm::normalize(direction);
        
        float velocityMagnitude = acos(glm::dot(glm::vec3(0, 1.0f, 0), collision_normal));

        glm::vec3 new_velocity = direction * velocityMagnitude;
        vx = new_velocity.x;
        vz = new_velocity.z;

        theta = sqrt(vx * vx + vy * vy + vz * vz) / radius;
        theta *= 0.05f;

        glm::vec3 rotationAxis = glm::cross(glm::vec3(0, 1.0f, 0), collision_normal);

        if (glm::length(rotationAxis) != 0.0f) {
            rotationAxis = glm::normalize(rotationAxis);
            glm::mat4 incrementalRotation = glm::rotate(glm::mat4(1.0f), theta, rotationAxis);
            orientation = incrementalRotation * orientation;
        }
    }
    

    model *= orientation;
    
}

std::vector<float> Ball::getInterleavedVertexData() {
    std::vector<float> data;
    for (int i = 0; i < vertices.size(); i += 3) {
        data.push_back(vertices[i]);
        data.push_back(vertices[i+1]);
        data.push_back(vertices[i+2]);
        data.push_back(colors[i]);
        data.push_back(colors[i+1]);
        data.push_back(colors[i+2]);
    }
    return data;
}
