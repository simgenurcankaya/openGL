#include <ostream>
#include "helper.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
//#include "glm/gtx/rotate_vector.hpp"
//#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

#define SPEEDSTEP 0.01f
#define HEIGHTFACTORSTEP 0.5f
#define PITCHSTEP 0.05f
#define YAWSTEP 0.05f
#define FARPLANE 1000.0f
#define NEARPLANE 0.1f
#define HEIGHTFACTOR 10.0f
#define LIGHTSTEP 5.0f


static GLFWwindow *win = NULL;

GLFWvidmode *mode;
const GLFWmonitor *monitor;
bool isFullScreen = false;
// Shaders
GLuint idProgramShader;
GLuint idFragmentShader;
GLuint idVertexShader;
GLuint idJpegTexture;
GLuint idTexture;
GLuint idMVPMatrix;

int widthTexture, heightTexture;
int widthImagePlane, heightImagePlane;
int widthImagePlaneLast, heightImagePlaneLast;
int g=0;

static void errorCallback(int error, const char *description) {
    fprintf(stderr, "Error: %s\n", description);
}

class Plane {
public:
    glm::vec3 position;
    glm::vec3 up;
    glm::vec3 gaze;
    glm::vec3 u;
    glm::vec3 lightPosition;

    float heightFactor;
    float ppAngle;
    float ppAspectRatio;
    float speed;
    float nearDistance;
    float farDistance;

    glm::mat4 modelM;
    glm::mat4 viewM;
    glm::mat4 projectionM;
    glm::mat4 mv;
    glm::mat4 mvp;

    Plane() {
        this->position = glm::vec3(widthTexture / 2, widthTexture / 10, -(widthTexture / 4));
        this->gaze = glm::vec3(0.0, 0.0, 1.0);
        this->up = glm::vec3(0.0, 1.0, 0.0);
        this->u = glm::vec3(-1.0, 0.0, 0.0);
        this-> lightPosition = glm::vec3(widthTexture/2,100,heightTexture/2);

        this->ppAngle = 45.0;
        this->ppAspectRatio = 1;
        this->speed = 0.0;
        this->heightFactor = HEIGHTFACTOR;
        this->nearDistance = NEARPLANE;
        this->farDistance = FARPLANE;
    }


    void move() {
        this->position = this->position + this->speed * this->gaze;
        this->setShaderData();
    }

    void pitchUp() {
        glm::mat4 rotationM;

        rotationM = glm::rotate(glm::radians(PITCHSTEP), this->u);

        this->gaze = glm::vec3(rotationM * glm::vec4(this->gaze, 1));
        this->gaze = glm::normalize(this->gaze);

        this->up = glm::vec3(rotationM * glm::vec4(this->up, 1));
        this->up = glm::normalize(this->up);
    }

    void pitchDown() {
        glm::mat4 rotationM;

        rotationM = glm::rotate(glm::radians(-PITCHSTEP), this->u);

        this->gaze = glm::vec3(rotationM * glm::vec4(this->gaze, 1));
        this->gaze = glm::normalize(this->gaze);

        this->up = glm::vec3(rotationM * glm::vec4(this->up, 1));
        this->up = glm::normalize(this->up);
    }

    void yawUp() {
        glm::mat4 rotationM;

        rotationM = glm::rotate(glm::radians(YAWSTEP), this->up);

        this->gaze = glm::vec3(rotationM * glm::vec4(this->gaze, 1));
        this->gaze = glm::normalize(this->gaze);

        this->u = glm::vec3(rotationM * glm::vec4(this->u, 1));
        this->u = glm::normalize(this->u);
    }

    void yawDown() {
        glm::mat4 rotationM;

        rotationM = glm::rotate(glm::radians(-YAWSTEP), this->up);

        this->gaze = glm::vec3(rotationM * glm::vec4(this->gaze, 1));
        this->gaze = glm::normalize(this->gaze);

        this->u = glm::vec3(rotationM * glm::vec4(this->u, 1));
        this->u = glm::normalize(this->u);
    }

    void fixCameraVectors() {
        this->u = glm::normalize(glm::cross(this->up, this->gaze));
    }

    void speedUp(){
      this->speed += SPEEDSTEP;
    }

    void speedDown(){
      this->speed -= SPEEDSTEP;
    }

    void heightFactorUp(){
      this->heightFactor += HEIGHTFACTORSTEP;
    }

    void heightFactorDown(){
      this->heightFactor -= HEIGHTFACTORSTEP;
    }

    void moveLeft(){
      this -> position -= u;
    }
    void moveRight(){
      this->position += u;
    }

    void stop(){
      this->speed = 0;
    }

    void lightUp(){
      this->lightPosition.y  += LIGHTSTEP;
    }

    void lightDown(){
      this->lightPosition.y -= LIGHTSTEP;
    }

    void lightSol(){
      this->lightPosition.x += LIGHTSTEP;
    }

    void lightSag(){
      this->lightPosition.x -= LIGHTSTEP;
    }

    void lightAsagi(){
      this->lightPosition.z -= LIGHTSTEP;
    }

    void lightYukari(){
      this->lightPosition.z += LIGHTSTEP;
    }

    void reset(){
      this->speed = 0;
      this->position = glm::vec3(widthTexture / 2, widthTexture / 10, -(widthTexture / 4));
      this->gaze = glm::vec3(0.0, 0.0, 1.0);
      this->up = glm::vec3(0.0, 1.0, 0.0);
      this->u = glm::vec3(-1.0, 0.0, 0.0);
      this-> lightPosition = glm::vec3(widthTexture/2,100,heightTexture/2);
      this->ppAngle = 45.0;
      this->ppAspectRatio = 1;
      this->speed = 0.0;
      this->heightFactor = HEIGHTFACTOR;
      this->nearDistance = NEARPLANE;
      this->farDistance = FARPLANE;
    }

    void setShaderData() {
        glViewport(0, 0, widthImagePlane, heightImagePlane);

        this->projectionM = glm::perspective(
                this->ppAngle,
                this->ppAspectRatio,
                this->nearDistance,
                this->farDistance
        );

        this->viewM = glm::lookAt(
                this->position,
                glm::vec3(
                        this->position.x + this->gaze.x * this->nearDistance,
                        this->position.y + this->gaze.y * this->nearDistance,
                        this->position.z + this->gaze.z * this->nearDistance
                ),
                this->up
        );

        this->modelM = glm::mat4(1.0f);

        this->mv = this->viewM * this->modelM;
        this->mvp = this->projectionM * this->viewM * this->modelM;

        GLint shaderLocation = glGetUniformLocation(idProgramShader, "MV");
        glUniformMatrix4fv(shaderLocation, 1, GL_FALSE, &mv[0][0]);

        shaderLocation = glGetUniformLocation(idProgramShader, "MVP");
        glUniformMatrix4fv(shaderLocation, 1, GL_FALSE, &mvp[0][0]);

        glm::vec4 lightPosition4 = glm::vec4(
                this->lightPosition.x,
                this->lightPosition.y,
                this->lightPosition.z,
                1);
        shaderLocation = glGetUniformLocation(idProgramShader, "lightPosition");
        glUniform4fv(shaderLocation, 1, &lightPosition4.x);

        glm::vec4 cameraPosition4 = glm::vec4(this->position, 1);
        shaderLocation = glGetUniformLocation(idProgramShader, "cameraPosition");
        glUniform4fv(shaderLocation, 1, &cameraPosition4.x);

        shaderLocation = glGetUniformLocation(idProgramShader, "heightFactor");
        glUniform1f(shaderLocation, this->heightFactor);

        shaderLocation = glGetUniformLocation(idProgramShader, "widthTexture");
        glUniform1i(shaderLocation, widthTexture);

        shaderLocation = glGetUniformLocation(idProgramShader, "heightTexture");
        glUniform1i(shaderLocation, heightTexture);

    }
};

Plane *plane;
glm::vec3 *vertices;
int triangleCount;

void prepareVertexData(int w, int h) {
    triangleCount = 2 * w * h;
    vertices = new glm::vec3[triangleCount * 3];
    glm::vec3 vertex1, vertex2, vertex3, vertex4;
    int k = 0;
    for (int i = 0; i < w-1; ++i) {
        for (int j = 0; j < h-1; ++j) {
            vertex1 = glm::vec3(i, 0, j);
            vertex2 = glm::vec3(i + 1, 0, j);
            vertex3 = glm::vec3(i, 0, j + 1);
            vertex4 = glm::vec3(i + 1, 0, j + 1);

            vertices[k++] = vertex1;
            vertices[k++] = vertex4;
            vertices[k++] = vertex2;

            vertices[k++] = vertex1;
            vertices[k++] = vertex3;
            vertices[k++] = vertex4;

        }
    }


}

void renderFunction() {
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices);

    glDrawArrays(GL_TRIANGLES, 0, triangleCount * 3);

    glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays

}


static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
            case GLFW_KEY_I:   // reset plane
                plane->reset();
                break;
            case GLFW_KEY_X:   // stop the plane
                plane->stop();
                break;
            case GLFW_KEY_Q:   // move plane to left
                plane->moveLeft();
                break;
            case GLFW_KEY_E:   // move plane to right
                plane->moveRight();
                break;
            case GLFW_KEY_W:   // Increase pitch
                plane->pitchUp();
                break;
            case GLFW_KEY_S:   // Decrease pitch
                plane->pitchDown();
                break;
            case GLFW_KEY_A:   // Increase yaw
                plane->yawUp();
                break;
            case GLFW_KEY_D:   // Decrease yaw
                plane->yawDown();
                break;
            case GLFW_KEY_Y:   // Increase speed
                plane->speedUp();
                break;
            case GLFW_KEY_H:   // Decrease speed
                plane->speedDown();
                break;
            case GLFW_KEY_R:   // Increase heightFactor
                plane->heightFactorUp();
                break;
            case GLFW_KEY_F:   // Decrease heightFactor
                plane->heightFactorDown();
                break;
            case GLFW_KEY_T:
                plane->lightUp();
                break;
            case GLFW_KEY_G:
                plane->lightDown();
                break;
            case GLFW_KEY_UP:
                plane->lightYukari();
                break;
            case GLFW_KEY_LEFT:
                plane->lightSol();
                break;
            case GLFW_KEY_RIGHT:
                plane->lightSag();
                break;
            case GLFW_KEY_DOWN:
                plane->lightAsagi();
                break;

            case GLFW_KEY_P:   // Switch to fullscreen mode
                if (isFullScreen) {
                    glfwSetWindowMonitor(window, nullptr, 0, 0, widthImagePlaneLast, heightImagePlaneLast, 0);
                    isFullScreen = false;
                } else {
                    widthImagePlaneLast = widthImagePlane;
                    heightImagePlaneLast = heightImagePlane;
                    glfwSetWindowMonitor(window, const_cast<GLFWmonitor *>(monitor), 0, 0, mode->width, mode->height,
                                         mode->refreshRate);
                    isFullScreen = true;
                }

                break;
            default:
//                std::cout << "Pressed something unhandled." << std::endl;
                break;
        }
    }
}

void windowSizeCallback(GLFWwindow *win, int width, int height) {
    widthImagePlane = width;
    heightImagePlane = height;
}

int main(int argc, char *argv[]) {

    // if (argc != 2) {
    //     printf("Please provide only a texture image\n");
    //     exit(-1);
    // }

    glfwSetErrorCallback(errorCallback);

    if (!glfwInit()) {
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    widthImagePlane = 1000;
    heightImagePlane = 1000;
    win = glfwCreateWindow(widthImagePlane, heightImagePlane, "CENG477 - HW3", NULL, NULL);

    if (!win) {
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(win);

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));

        glfwTerminate();
        exit(-1);
    }

    glfwSetKeyCallback(win, keyCallback);
    glfwSetWindowSizeCallback(win, windowSizeCallback);

    monitor = glfwGetPrimaryMonitor();
    mode = const_cast<GLFWvidmode *>(glfwGetVideoMode(const_cast<GLFWmonitor *>(monitor)));


    initShaders();
    glUseProgram(idProgramShader);
    initTexture(argv[1], argv[2], & widthTexture, & heightTexture,idProgramShader);

    prepareVertexData(widthTexture, heightTexture);

    plane = new Plane();
    plane->setShaderData();

    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(win)) {

        renderFunction();

        glfwSwapBuffers(win);
        glfwPollEvents();
        plane->move();

    }


    glfwDestroyWindow(win);
    glfwTerminate();

    return 0;
}
