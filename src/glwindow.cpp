#include <iostream>
#include <stdio.h>

#include "SDL.h"
#include <GL/glew.h>
#include <GL/glu.h>

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

#include "glwindow.h"
#include "geometry.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

const char* glGetErrorString(GLenum error)
{
    switch(error)
    {
    case GL_NO_ERROR:
        return "GL_NO_ERROR";
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
    default:
        return "UNRECOGNIZED";
    }
}

void glPrintError(const char* label="Unlabelled Error Checkpoint", bool alwaysPrint=false)
{
    GLenum error = glGetError();
    if(alwaysPrint || (error != GL_NO_ERROR))
    {
        printf("%s: OpenGL error flag is %s\n", label, glGetErrorString(error));
    }
}

GLuint loadShader(const char* shaderFilename, GLenum shaderType)
{
    FILE* shaderFile = fopen(shaderFilename, "r");
    if(!shaderFile)
    {
        return 0;
    }

    fseek(shaderFile, 0, SEEK_END);
    long shaderSize = ftell(shaderFile);
    fseek(shaderFile, 0, SEEK_SET);

    char* shaderText = new char[shaderSize+1];
    size_t readCount = fread(shaderText, 1, shaderSize, shaderFile);
    shaderText[readCount] = '\0';
    fclose(shaderFile);

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, (const char**)&shaderText, NULL);
    glCompileShader(shader);

    delete[] shaderText;

    return shader;
}

GLuint loadShaderProgram(const char* vertShaderFilename,
                       const char* fragShaderFilename)
{
    GLuint vertShader = loadShader(vertShaderFilename, GL_VERTEX_SHADER);
    GLuint fragShader = loadShader(fragShaderFilename, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if(linkStatus != GL_TRUE)
    {
        GLsizei logLength = 0;
        GLchar message[1024];
        glGetProgramInfoLog(program, 1024, &logLength, message);
        cout << "Shader load error: " << message << endl;
        return 0;
    }

    return program;
}

OpenGLWindow::OpenGLWindow()
{
}

GLuint OpenGLWindow::loadTexture(const char* filename, GLuint textureID){
    
    glBindTexture(GL_TEXTURE_2D, textureID);

    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load and generate the texture
    int width, height, nrChannels;
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else{
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    return textureID;
}

void OpenGLWindow::initGL()
{
    // We need to first specify what type of OpenGL context we need before we can create the window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    sdlWin = SDL_CreateWindow("OpenGL Prac 1",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              640, 480, SDL_WINDOW_OPENGL);
    if(!sdlWin)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", "Unable to create window", 0);
    }
    SDL_GLContext glc = SDL_GL_CreateContext(sdlWin);
    SDL_GL_MakeCurrent(sdlWin, glc);
    SDL_GL_SetSwapInterval(1);

    glewExperimental = false;
    GLenum glewInitResult = glewInit();
    glGetError(); // Consume the error erroneously set by glewInit()
    if(glewInitResult != GLEW_OK)
    {
        const GLubyte* errorString = glewGetErrorString(glewInitResult);
        cout << "Unable to initialize glew: " << errorString;
    }

    int glMajorVersion;
    int glMinorVersion;
    glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
    cout << "Loaded OpenGL " << glMajorVersion << "." << glMinorVersion << " with:" << endl;
    cout << "\tVendor: " << glGetString(GL_VENDOR) << endl;
    cout << "\tRenderer: " << glGetString(GL_RENDERER) << endl;
    cout << "\tVersion: " << glGetString(GL_VERSION) << endl;
    cout << "\tGLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0,0,0,1);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    this->extraImage = false;
    
    resetVariables();
    // Note that this path is relative to your working directory
    // when running the program (IE if you run from within build
    // then you need to place these files in build as well)
    shader = loadShaderProgram("simple.vert", "simple.frag");
    glUseProgram(shader);

    int vertexLoc = glGetAttribLocation(shader, "position");
    int normalLoc = glGetAttribLocation(shader, "normal");
    int textureLoc = glGetAttribLocation(shader, "texture");

    int tangentLoc = 3;
    glBindAttribLocation(shader,tangentLoc, "tangent");
    int bitangentLoc = 4;
    glBindAttribLocation(shader,bitangentLoc, "bitangent");


    glGenTextures(2, textures);
    glActiveTexture(GL_TEXTURE0);
    diffuseMap = loadTexture("metal.jpg",textures[0]);
    glActiveTexture(GL_TEXTURE1);
    normalMap = loadTexture("metal_normal.jpg",textures[1]);
    glUniform1i(glGetUniformLocation(shader, "ourTexture"), 0); 
    glUniform1i(glGetUniformLocation(shader, "ourTextureMap"), 1); 

    std::cout << textureLoc << " " << bitangentLoc << std::endl;
    this->object = GeometryData();
    object.GeometryData::loadFromOBJFile("objFiles/suzanne.obj");
    //this->object2 = GeometryData();
    //object2.GeometryData::loadFromOBJFile("objFiles/suzanne.obj");


    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, object.vertexCount()*sizeof(float)*3, object.vertexData(), GL_STATIC_DRAW);
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(vertexLoc);

    //normal buffer
    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, object.vertexCount()*sizeof(float)*3, object.normalData(), GL_STATIC_DRAW);
    glVertexAttribPointer(normalLoc, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(normalLoc);

    //binding textures
    glGenBuffers(1, &textureBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
    glBufferData(GL_ARRAY_BUFFER, object.vertexCount()*sizeof(float)*3, object.textureCoordData(), GL_STATIC_DRAW);
    glVertexAttribPointer(textureLoc, 2, GL_FLOAT, false,0, (void*)(0 * sizeof(float)));
    glEnableVertexAttribArray(textureLoc);

    //binding tangent
    glGenBuffers(1, &tangentBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, tangentBuffer);
    glBufferData(GL_ARRAY_BUFFER, object.vertexCount()*sizeof(float)*3, object.tangentData(), GL_STATIC_DRAW);
    glVertexAttribPointer(tangentLoc, 3, GL_FLOAT, false,0, (void*)(0 * sizeof(float)));
    glEnableVertexAttribArray(tangentLoc);

    //binding bitangent
    glGenBuffers(1, &bitangentBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, bitangentBuffer);
    glBufferData(GL_ARRAY_BUFFER, object.vertexCount()*sizeof(float)*3, object.bitangentData(), GL_STATIC_DRAW);
    glVertexAttribPointer(bitangentLoc, 3, GL_FLOAT, false,0, (void*)(0 * sizeof(float)));
    glEnableVertexAttribArray(bitangentLoc);


    //glGenBuffers(1, &vertexBuffer2);
    //glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer2);
    //glBufferData(GL_ARRAY_BUFFER, object2.vertexCount()*sizeof(float)*3, object2.vertexData(), GL_STATIC_DRAW);
    //glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, false, 0, 0);
    glPrintError("Setup complete", true);
}

void OpenGLWindow::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  

    int colorLoc = glGetUniformLocation(shader, "objectColor");
    glUniform3f(colorLoc,r,g,b);
    int lightColorLoc = glGetUniformLocation(shader, "lightColor");
    glUniform3f(lightColorLoc,1.0f, 0.0f, 0.0f);

    int lightColorLoc2 = glGetUniformLocation(shader, "lightColor2");
    glUniform3f(lightColorLoc2,0.0f, 0.0f, 1.0f);

    int lightPos = glGetUniformLocation(shader, "lightPos");
    glUniform3f(lightPos,lightx,lighty,lightz);

    int lightPos2 = glGetUniformLocation(shader, "lightPos2");
    glUniform3f(lightPos2,lightx2,lighty2,lightz2);
    //glUniform3f(lightPos2,lightx2,lighty2,lightz2);

    int ambient = glGetUniformLocation(shader, "ambientStrength");
    glUniform1f(ambient, 0.1f);

    int viewPos = glGetUniformLocation(shader, "viewPos");
    glUniform3fv(viewPos,1,&cameraPos[0]);

    glUniform1i(glGetUniformLocation(shader, "addNormalMap"), addNormalMap);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(this->pan), rotateDirection); 
    unsigned int modelLoc = glGetUniformLocation(shader, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

    float radius = 10.0f;
    glm::mat4 view = glm::mat4(1.0f);
    // note that we're translating the scene in the reverse direction of where we want to move
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(this->radian), 800.0f / 600.0f, 0.1f, 100.0f); //change distance from camera

    //matrix to do a translation
    glm::mat4 trans = glm::mat4(1.0f);
    trans = glm::translate(trans, glm::vec3(transx,transy,transz)); //x,y,z
    //x y z matrics to do rotations
    glm::mat4 rotatex = glm::mat4(1.0f);
    rotatex = glm::rotate(rotatex,glm::radians(this->rx),glm::vec3(1.0f,0.0f,0.0f));
    glm::mat4 rotatey = glm::mat4(1.0f);
    rotatey = glm::rotate(rotatey,glm::radians(this->ry),glm::vec3(0.0f,1.0f,0.0f));
    glm::mat4 rotatez = glm::mat4(1.0f);
    rotatez = glm::rotate(rotatez,glm::radians(this->rz),glm::vec3(0.0f,0.0f,1.0f));
    //matrix to do scaling
    glm::mat4 scale = glm::mat4(1.0f);
    scale = glm::scale(scale,glm::vec3(s,s,s));
   
    glm::mat4 MVP = projection * view * model;  

    MVP = MVP*trans * rotatex * rotatey * rotatez * scale;  
    unsigned int MVPLoc = glGetUniformLocation(shader, "mvp");
    glUniformMatrix4fv(MVPLoc, 1, GL_FALSE, &MVP[0][0]);

    glm::mat4 transform = trans * rotatex * rotatey * rotatez * scale;
    unsigned int transLoc = glGetUniformLocation(shader, "trans");
    glUniformMatrix4fv(transLoc, 1, GL_FALSE, &transform[0][0]);

    // Swap the front and back buffers on the window, effectively putting what we just "drew"
    // onto the screen (whereas previously it only existed in memory)
    glDrawArrays(GL_TRIANGLES, 0, object.vertexCount());

        //drawing the second object

    glm::mat4 trans2 = glm::mat4(1.0f);
    trans2 = glm::translate(trans2, glm::vec3(2.0f,0.0f,0.0f)); //x,y,z
    glUniform3f(colorLoc,1.0f,1.0f,1.0f);
    glUniform1f(ambient, 1.0f);
    MVP = MVP * trans2; 
    MVPLoc = glGetUniformLocation(shader, "mvp");

   // if (extraImage == 1){
   //  glUniformMatrix4fv(MVPLoc, 1, GL_FALSE, &MVP[0][0]);
   //  }
   //  glDrawArrays(GL_TRIANGLES, 0, object2.vertexCount());


     SDL_GL_SwapWindow(sdlWin);
    
}


void OpenGLWindow::resetVariables(){
    this->radian = 45.0f;
    this->r = 1.0f;
    this->g = 1.0f;
    this->b = 1.0f;
    this->transx = 0.0f;
    this->transy = 0.0f;
    this->transz = 0.0f;
    this->lightx = 2.0f;
    this->lighty = 2.0f;
    this->lightz = 2.0f;
    this->lightx2 = -2.0f;
    this->lighty2 = 2.0f;
    this->lightz2 = 2.0f;
    this->pan = 0.0f;
    this-> rx = 0.0f;
    this-> ry = 0.0f;
    this-> rz = 0.0f;
    this-> s = 1.0f;
    this-> textureCount = 1;
    this->addNormalMap = false;
    
    cameraPos   = glm::vec3(0.0f, 0.0f,  5.0f);
    cameraFront = glm::vec3(0.0f, 0.0f, -3.0f);
    cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
    rotateDirection = glm::vec3(1.0f, 0.0f, 0.0f);

}
// The program will exit if this function returns false
bool OpenGLWindow::handleEvent(SDL_Event e)
{
    // A list of keycode constants is available here: https://wiki.libsdl.org/SDL_Keycode
    // Note that SDL provides both Scancodes (which correspond to physical positions on the keyboard)
    // and Keycodes (which correspond to symbols on the keyboard, and might differ across layouts)
    if(e.type == SDL_KEYDOWN)
    {
        if(e.key.keysym.sym == SDLK_ESCAPE)
        {
            return false;
        }
    }
    return true;
}




void OpenGLWindow::handleCameraMovementEvent(SDL_Event e){

    if(e.type == SDL_KEYDOWN){
        switch (e.key.keysym.sym){
            case SDLK_q: //Move camera to the left
                cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
                return;
            case SDLK_e: //Move camera to the right
                cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
                return;
            case SDLK_w: //Rotate camera about the y axis 
                rotateDirection = glm::vec3(1.0f, 0.0f, 0.0f);
                pan += 1.0f;
                return;
            case SDLK_s: //Rotate camera about the y axis down
                rotateDirection = glm::vec3(1.0f, 0.0f, 0.0f);
                pan -= 1.0f;
                return;
            case SDLK_d: //Rotate camera about the object to the left
                rotateDirection = glm::vec3(0.0f, 1.0f, 0.0f);
                pan += 1.0f;
                return;
            case SDLK_a: //Rotate camera about the object to the left
                rotateDirection = glm::vec3(0.0f, 1.0f, 0.0f);
                pan -= 1.0f;
                return;
         }
      }
} 

void OpenGLWindow::handleColorEvent(SDL_Event e){

    if(e.type == SDL_KEYDOWN){
        switch (e.key.keysym.sym){
            case SDLK_z: //color one
                r = 0.5f;
                g = 0.0f;
                b = 0.0f; 
                return;
            case SDLK_x: //color two
                r = 0.1f;
                g = 0.5f;
                b = 0.0f;
                return;
            case SDLK_c: //color three
                r = 0.5f;
                g = 0.0f;
                b = 0.5f; 
                return;
            }

    }

}

void OpenGLWindow::handleObjectTranslationEvent(SDL_Event e){

    if(e.type == SDL_KEYDOWN){
        switch (e.key.keysym.sym){
            case SDLK_h: //move up y axis
                transy += 0.1f;
                return;
            case SDLK_n: //move down y axis
                transy -= 0.1f;
                return;
            case SDLK_m: //move left x axis
                transx += 0.1f;
                return;
            case SDLK_b: //move right x axis
                transx -= 0.1f;
                return;
            case SDLK_g: //move out z axis
                transz -= 0.1f;
                return;
            case SDLK_j: //move in z axis
                transz += 0.1f;
                return;
            }

    }

}

void OpenGLWindow::handleObjectRotationEvent(SDL_Event e){

    if(e.type == SDL_KEYDOWN){
        switch (e.key.keysym.sym){
            case SDLK_RIGHT: //move up y axis
                ry += 0.4f;
                return;
            case SDLK_DOWN: //move down y axis
                rx += 0.4f;
                return;
            case SDLK_LEFT: //move left x axis
                rz += 0.4f;
                return;
            
            }

    }

}

void OpenGLWindow::handleObjectScaleEvent(SDL_Event e){

    if(e.type == SDL_KEYDOWN){
        switch (e.key.keysym.sym){
            case SDLK_y: //move up y axis
                s += 0.2f;
                return;
            case SDLK_u: //move down y axis
                s -= 0.2f;
                return;
            }

    }

}

void OpenGLWindow::addExtraObject(SDL_Event e){

    if(e.type == SDL_KEYDOWN){
        switch (e.key.keysym.sym){
            case SDLK_t: 
                //resetVariables();
                this->addNormalMap = !addNormalMap;
                return;

            }

    }

}

void OpenGLWindow::handleZoomEvent(SDL_Event e){

    if(e.type == SDL_KEYDOWN){
        switch (e.key.keysym.sym){
            case SDLK_i: // zoom in
                cameraPos += cameraSpeed * cameraFront;
                return;
            case SDLK_o: //zoom out
                cameraPos -= cameraSpeed * cameraFront;
                return;
          
            }

    }


}

void OpenGLWindow::handleLightPositionEvent(SDL_Event e){

    if(e.type == SDL_KEYDOWN){
        switch (e.key.keysym.sym){
            case SDLK_1: //move 1 + x axis
                lightx += 0.1f;
                return;
            case SDLK_2: //move 1 - x axis
                lightx -= 0.1f;
                return;
            case SDLK_3: //move 1 + y axis
                lighty += 0.1f;
                return;
            case SDLK_4: //move 1 - x axis
                lighty -= 0.1f;
                return;
            case SDLK_5: //move 1 + z axis
                lightx += 0.1f;
                return;
            case SDLK_6: //move 1 - z axis
                lighty -= 0.1f;
                return;
            case SDLK_g: //move 1 + y axis
                lightx2 += 0.1f;
                return;
            case SDLK_h: //move 1 - x axis
                lighty2 += 0.1f;
                return;
            case SDLK_j: //move 1 + z axis
                lightz2 += 0.1f;
                return;
            case SDLK_b: //move 1 + y axis
                lightx2 -= 0.1f;
                return;
            case SDLK_n: //move 1 - x axis
                lighty2 -= 0.1f;
                return;
            case SDLK_m: //move 1 + z axis
                lightz2 -= 0.1f;
                return;
            
            }

    }

}

void OpenGLWindow::handleTextureChangeEvent(SDL_Event e){

    if(e.type == SDL_KEYDOWN){
        switch (e.key.keysym.sym){
            case SDLK_l: //color one
                switch (textureCount){
                    case 0:
                        glActiveTexture(GL_TEXTURE0);
                        diffuseMap = loadTexture("metal.jpg",textures[0]);
                        glActiveTexture(GL_TEXTURE1);
                        normalMap = loadTexture("metal_normal.jpg",textures[1]);
                        glUniform1i(glGetUniformLocation(shader, "ourTexture"), 0); 
                        glUniform1i(glGetUniformLocation(shader, "ourTextureMap"), 1);
                        textureCount += 1;
                        return;
                    case 1:
                        glActiveTexture(GL_TEXTURE0);
                        diffuseMap = loadTexture("Abstract.jpg",textures[0]);
                        glActiveTexture(GL_TEXTURE1);
                        normalMap = loadTexture("Abstract_normal.jpg",textures[1]);
                        glUniform1i(glGetUniformLocation(shader, "ourTexture"), 0); 
                        glUniform1i(glGetUniformLocation(shader, "ourTextureMap"), 1);
                        textureCount += 1;
                        return;
                    case 2:
                        glActiveTexture(GL_TEXTURE0);
                        diffuseMap = loadTexture("thatch.jpg",textures[0]);
                        glActiveTexture(GL_TEXTURE1);
                        normalMap = loadTexture("thatch_normal.jpg",textures[1]);
                        glUniform1i(glGetUniformLocation(shader, "ourTexture"), 0); 
                        glUniform1i(glGetUniformLocation(shader, "ourTextureMap"), 1);
                        textureCount +=1;
                        return;
                    case 3:
                        glActiveTexture(GL_TEXTURE0);
                        diffuseMap = loadTexture("water.jpg",textures[0]);
                        glActiveTexture(GL_TEXTURE1);
                        normalMap = loadTexture("water_normal.jpg",textures[1]);
                        glUniform1i(glGetUniformLocation(shader, "ourTexture"), 0); 
                        glUniform1i(glGetUniformLocation(shader, "ourTextureMap"), 1);
                        textureCount +=1;
                        return;
                    case 4:
                        glActiveTexture(GL_TEXTURE0);
                        diffuseMap = loadTexture("metal2.jpg",textures[0]);
                        glActiveTexture(GL_TEXTURE1);
                        normalMap = loadTexture("metal2_normal.jpg",textures[1]);
                        glUniform1i(glGetUniformLocation(shader, "ourTexture"), 0); 
                        glUniform1i(glGetUniformLocation(shader, "ourTextureMap"), 1);
                        textureCount =0;
                        return;

                }

            }

    }

}

void OpenGLWindow::cleanup()
{
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &vertexBuffer2);
    glDeleteBuffers(1, &normalBuffer);
    glDeleteBuffers(1, &bitangentBuffer);
    glDeleteBuffers(1, &tangentBuffer);
    glDeleteVertexArrays(1, &vao);
    SDL_DestroyWindow(sdlWin);
}
