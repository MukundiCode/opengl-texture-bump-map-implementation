#ifndef GL_WINDOW_H
#define GL_WINDOW_H

#include <GL/glew.h>
#include <glm/glm/gtc/matrix_transform.hpp>
#include "geometry.h"

class OpenGLWindow
{
public:
    OpenGLWindow();

    void initGL();
    void render();
    void resetVariables();
    bool handleEvent(SDL_Event e);
    void handleZoomEvent(SDL_Event e);
    void handleColorEvent(SDL_Event e);
    void handleCameraMovementEvent(SDL_Event e);
    void handleObjectTranslationEvent(SDL_Event e);
    void handleObjectRotationEvent(SDL_Event e);
    void handleObjectScaleEvent(SDL_Event e);
    void addExtraObject(SDL_Event e);
    void handleLightPositionEvent(SDL_Event e);
    void handleTextureChangeEvent(SDL_Event e);
    GLuint loadTexture(const char*,GLuint textureID);
    void cleanup();

private:
    SDL_Window* sdlWin;

    GLuint vao;
    GLuint shader;
    GLuint diffuseMap;
    GLuint normalMap;
    GLuint vertexBuffer;
    GLuint normalBuffer;
    GLuint vertexBuffer2;
    GLuint bitangentBuffer;
    GLuint textures[2];
    GLuint tangentBuffer;
    GLuint textureBuffer;
    GLuint textureNormBuffer;
    GeometryData object;
    GeometryData object2;
    float radian;

    const float cameraSpeed = 0.05f;
    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    glm::vec3 rotateDirection;

    float pan;
    float r,g,b;
    bool extraImage;
    bool addNormalMap;

    float transx;
    float transy;
    float transz;
    float rx;
    float ry;
    float rz;
    float s;
    float lightx;
    float lighty;
    float lightz;
    float lightx2;
    float lighty2;
    float lightz2;
    int textureCount;
};

#endif
