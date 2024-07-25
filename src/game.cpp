#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <tuple>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <shader.h>
#include <ball.h>
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

#define MAXBUFLEN 1000

typedef std::tuple<bool, glm::vec3, glm::vec3> Collision;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
Collision checkCollision(Platform &p, Ball &b);

const unsigned int SCRN_WIDTH = 800;
const unsigned int SCRN_HEIGHT = 600;

glm::vec3 cameraPos   = glm::vec3(10.0f, 10.0f, 10.0f);
glm::vec3 cameraFront = glm::vec3(glm::normalize(glm::vec3(0,0,0) - cameraPos));
// glm::vec3 cameraFront = glm::vec3(0,0,-1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

float speedFactor = 0.1f;

Platform platform(5.0f, 5.0f, 1.0f, 0.2f);
Ball ball(0.5f, 20, 20);

int main() 
{   

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Balance Ball", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);  
    
    Shader shaderProgram("../shaders/shader.vs", "../shaders/shader.frag");

    glViewport(0, 0, SCRN_WIDTH, SCRN_HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    unsigned int VBO[2], VAO[2], EBO[2];
    glGenVertexArrays(2, VAO);
    glGenBuffers(2, VBO);
    glGenBuffers(2, EBO);

    glBindVertexArray(VAO[0]);

    std::vector<float> platformVertexData = platform.getInterleavedVertexData();

    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * platformVertexData.size(), platformVertexData.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * platform.indices.size(), platform.indices.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    std::vector<float> ballVertexData = ball.getInterleavedVertexData();

    glBindVertexArray(VAO[1]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * ballVertexData.size(), ballVertexData.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * ball.indices.size(), ball.indices.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    shaderProgram.use();
    glm::mat4 projection    = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(45.0f), (float)SCRN_WIDTH / (float)SCRN_HEIGHT, 0.1f, 100.0f);
    unsigned int projectLoc = glGetUniformLocation(shaderProgram.ID, "projection");
    glUniformMatrix4fv(projectLoc, 1, GL_FALSE, glm::value_ptr(projection));

    ball.y = 2.0f;
    ball.vy = -0.05f; // simulate gravity

    glm::mat4 platform_model = glm::mat4(1.0f);
    glm::mat4 ball_model = glm::mat4(1.0f);

    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        
        glClearColor(0.0f, 0.1f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderProgram.use();

        // ----- PLATFORM ------ 

        platform.update(glfwGetTime());

        platform_model = glm::mat4(1.0f);

        platform.rotate(platform_model);
        platform_model = glm::translate(platform_model, glm::vec3(platform.x, platform.y, platform.z));
        unsigned int modelLoc = glGetUniformLocation(shaderProgram.ID, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(platform_model));

        glBindVertexArray(VAO[0]);
        glDrawElements(GL_TRIANGLES, platform.indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // ----- BALL ------

        if (ball.y <= -10.0f) {
            ball.y = 2.5f;
            ball.z = 0;
            ball.x = 0;
        }

        ball.vx = ball.vz = 0;

        bool collided;
        glm::vec3 translation;
        glm::vec3 collisionNormal;
        std::tie(collided, translation, collisionNormal) = checkCollision(platform, ball);

        ball.x += ball.vx;
        ball.y += ball.vy;
        ball.z += ball.vz; 

        

        ball_model = glm::mat4(1.0f);
        glm::vec4 ball_trans = platform_model * glm::vec4(translation, 1.0f);
        ball_model = glm::translate(ball_model, glm::vec3(ball.x, ball.y, ball.z));
        if (collided) {
            ball_model = glm::translate(ball_model, glm::vec3(ball_trans.x, ball_trans.y, ball_trans.z));
            ball.x += ball_trans.x;
            ball.y += ball_trans.y;
            ball.z += ball_trans.z;
        }
        collisionNormal = platform_model * glm::vec4(collisionNormal, 1.0f);
        ball.update(glfwGetTime(), ball_model, collisionNormal);

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(ball_model));

        glBindVertexArray(VAO[1]);
        glDrawElements(GL_TRIANGLES, ball.indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // ----- CAMERA ------ 

        glm::mat4 view;
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        unsigned int viewLoc  = glGetUniformLocation(shaderProgram.ID, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(2, VAO);
    glDeleteBuffers(2, VBO);
    glDeleteBuffers(2, EBO);

    glfwTerminate();

    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}


// OBB - Sphere collision checker 
// 
Collision checkCollision(Platform &p, Ball &b) {
    // first transform the sphere coordinate system to the OBB's local space
    // the OBB in local space is axis aligned, making it into an AABB
    // thus this becomes an AABB - Sphere case which is simpler than dealing with OBBs directly
    glm::vec4 localSphereCenter = glm::inverse(p.rotationMatrix) * glm::vec4(b.x, b.y, b.z, 1.0f);
    glm::vec4 localBoxCenter = glm::inverse(p.rotationMatrix) * glm::vec4(p.x, p.y, p.z, 1.0f);
    glm::vec3 b_center(localSphereCenter.x, localSphereCenter.y, localSphereCenter.z);
    glm::vec3 p_center(localBoxCenter.x, localBoxCenter.y, localBoxCenter.z);
    glm::vec3 difference = b_center - p_center;
    glm::vec3 p_half_extents(p.width / 2.0f, p.height / 2.0f, p.length / 2.0f);
    glm::vec3 clamped = glm::clamp(difference, -p_half_extents, p_half_extents);
    glm::vec3 closest = p_center + clamped;
    glm::vec3 sphere_to_closest = b_center - closest;
    float distance = glm::length(sphere_to_closest);
    if (distance < b.radius) {
        glm::vec3 collisionNormal = glm::normalize(sphere_to_closest);
        glm::vec3 penetrationDepth = collisionNormal * (b.radius - distance);
        return std::make_tuple(true, penetrationDepth, collisionNormal);
    } else {
        return std::make_tuple(false, glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));
    }
}


void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 0.2f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}