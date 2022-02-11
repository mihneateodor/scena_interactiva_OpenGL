#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

#include <iostream>
#include "SkyBox.hpp"

// proiect
int glWindowWidth = 800;
int glWindowHeight = 600;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;
//

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;


// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;
GLint modelLoc2;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 0.0f, 3.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];

// models
gps::Model3D teapot;
gps::Model3D dog;
gps::Model3D trashbin;
gps::Model3D ground;
gps::Model3D goal;
gps::Model3D plane;
gps::Model3D lamp;
gps::Model3D ball;
gps::Model3D sidewalk;
gps::Model3D fence;
gps::Model3D bush;
gps::Model3D tree;
gps::Model3D bench;
gps::Model3D doghut;
GLfloat angleDog;
GLfloat angleTeapot;

// shaders
gps::Shader myBasicShader;
gps::Shader lightShader;
gps::Shader depthMapShader;

//skybox
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

//rotate camera
bool firstMouse = true;
float lastX = 400.0f;
float lastY = 300.0f;
const float sensitivity = 0.1f;
float yaw = -90.0f;
float pitch = 0.0f;

//shadow
GLuint shadowMapFBO;
GLuint depthMapTexture;
const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

//plane animation
float anglePlane = 0.0f;

//ball animation 
float xBall = 0.0f;
float yBall = 0.0f;
float zBall = 5.0f;

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO	

    glfwGetFramebufferSize(window, &retina_width, &retina_height);

    myBasicShader.useShaderProgram();

    projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    //TODO
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;
    myCamera.rotate(pitch, yaw);
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}

void processMovement() {
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

    if (pressedKeys[GLFW_KEY_Q]) {
        angleDog -= 1.0f;
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angleDog += 1.0f;
        // update model matrix for teapot
        //model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        //normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }
    if (pressedKeys[GLFW_KEY_R]) {
        xBall = 0.0f;
        yBall = 0.0f;
        zBall = 5.0f;
    }
    if (zBall >= -6.2f) {
        if (pressedKeys[GLFW_KEY_B]) {
            yBall += 0.0071f;
            zBall -= 0.05f;
            xBall += 0.00668f;
        }
    }
}

void initSkybox() {
    std::vector<const GLchar*> faces;
 
    faces.push_back("skybox/cloudtop_rt.tga");
    faces.push_back("skybox/cloudtop_lf.tga");
    faces.push_back("skybox/cloudtop_up.tga");
    faces.push_back("skybox/cloudtop_dn.tga");
    faces.push_back("skybox/cloudtop_bk.tga");
    faces.push_back("skybox/cloudtop_ft.tga");
    
    mySkyBox.Load(faces);
}

void initFBO() {
    //generate FBO ID
    glGenFramebuffers(1, &shadowMapFBO);

    //create depth texture for FBO
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    //attach texture to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initOpenGLWindow() {
    myWindow.Create(800, 600, "OpenGL Project Core");
    
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwMakeContextCurrent(myWindow.getWindow());

    glfwSwapInterval(1);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
 
    teapot.LoadModel("models/teapot/teapot20segUT.obj");
    dog.LoadModel("models/12228_Dog_v1_L2.obj");
    trashbin.LoadModel("models/bin/bin.obj");
    ground.LoadModel("models/ground/ground.obj");
    goal.LoadModel("models/FootballGoal/football_goal.obj");
    plane.LoadModel("models/airplane/11805_airplane_v2_L2.obj");
    lamp.LoadModel("models/street_lamp_obj/street_lamp.obj");
    ball.LoadModel("models/soccerb/football-obj.obj");
    sidewalk.LoadModel("models/sidewalk/untitled.obj");
    fence.LoadModel("models/fence/fence_wood.obj");
    bush.LoadModel("models/plant1/plant_combined.obj");
    tree.LoadModel("models/TreeOBJ/TreeOBJ.obj");
    doghut.LoadModel("models/doghut/doghouse0908.obj");
    bench.LoadModel("models/bench/bench.obj");
}

void initShaders() {
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();

    lightShader.loadShader("shaders/light.vert", "shaders/light.frag");
    lightShader.useShaderProgram();

    depthMapShader.loadShader("shaders/depthMapShader.vert", "shaders/depthMaoShader.frag");
    depthMapShader.useShaderProgram();
    
    myBasicShader.loadShader("shaders/basic.vert", "shaders/basic.frag");
    myBasicShader.useShaderProgram();
    
}

void initUniforms() {
    
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

    projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    
    // create model matrix for teapot
    model = glm::mat4(1.0f);
    //model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    //model = glm::translate(model, glm::vec3(2, 0, 0));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
    projection = glm::perspective(glm::radians(45.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        // 0.1f, 20.0f);
        0.1f, 50.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
    
    /// ///////////////////////////////////////////////////////////////
    
    
}

glm::mat4 computeLightSpaceTrMatrix() {
    //TODO - Return the light-space transformation matrix
   // glm::mat4 lightView = glm::lookAt(glm::mat3(lightRotation) * lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 lightView = glm::lookAt( lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    const GLfloat near_plane = 0.1f, far_plane = 5.0f;
    glm::mat4 lightProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, near_plane, far_plane);

    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
    return lightSpaceTrMatrix;
}

void ballAnimation(float* x, float* y, float* z) {
    *x += 0.0175f;
    *y -= 0.0325f;
    *z -= 0.03f;
}

void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    myBasicShader.useShaderProgram();

    //dog
    model = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 7.0f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(5.0f), glm::vec3(1, 0, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    dog.Draw(myBasicShader);


    //goal
    model = glm::translate(glm::mat4(1.0f), glm::vec3(2.5f, -0.05f, -7.0f));
    model = glm::scale(model, glm::vec3(0.01, 0.01, 0.01));
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    goal.Draw(myBasicShader);


    //ground
    model = glm::mat4(1.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    ground.Draw(myBasicShader);

    //plane
    model = glm::mat4(1.0f);   
    model = glm::rotate(model, glm::radians(anglePlane), glm::vec3(0, 1, 0));
    model = glm::translate(model, glm::vec3(0, 10, 10));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 0, 1));
    model = glm::rotate(model, glm::radians(-20.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(0.003f, 0.003f, 0.003f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    plane.Draw(myBasicShader);
    anglePlane += 0.1f;

    //lamp
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-0.75f, 0.0f,8.0f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(1.0f, 1.5f, 1.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    lamp.Draw(myBasicShader);

    //ball
    //pozitie initiala 0.0 0.0 5.0
    //pozitie2 1.5 1.6 -6.2
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(xBall, yBall, zBall));
    model = glm::scale(model, glm::vec3(0.0035f, 0.0035f, 0.0035f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    ball.Draw(myBasicShader);
    if(zBall < -6.2f)
        if(zBall >= -7.7f)
            ballAnimation(&xBall, &yBall, &zBall);

    //sidewalk
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(6.5f, 0.0f, -1.5f));
    model = glm::scale(model, glm::vec3(2.0f, 1.0f, 1.7f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    sidewalk.Draw(myBasicShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-6.5f, 0.0f, 1.5f));
    model = glm::scale(model, glm::vec3(2.0f, 1.0f, 1.7f));
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    sidewalk.Draw(myBasicShader);

    //fence
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(3.5f, 0.0f, -7.0f));
    model = glm::scale(model, glm::vec3(1.3f, 1.0f, 1.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    fence.Draw(myBasicShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-3.8f, 0.0f, -7.0f));
    model = glm::scale(model, glm::vec3(1.3f, 1.0f, 1.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    fence.Draw(myBasicShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-5.0f, 0.0f, -6.0f));
    model = glm::scale(model, glm::vec3(1.3f, 1.0f, 1.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    fence.Draw(myBasicShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(4.85f, 0.0f, -6.0f));
    model = glm::scale(model, glm::vec3(1.3f, 1.0f, 1.0f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    fence.Draw(myBasicShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-5.0f, 0.0f, -4.5f));
    model = glm::scale(model, glm::vec3(1.3f, 1.0f, 1.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    fence.Draw(myBasicShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(4.85f, 0.0f, -4.5f));
    model = glm::scale(model, glm::vec3(1.3f, 1.0f, 1.0f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    fence.Draw(myBasicShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-5.0f, 0.0f, -3.0f));
    model = glm::scale(model, glm::vec3(1.3f, 1.0f, 1.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    fence.Draw(myBasicShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-5.0f, 0.0f, -1.5f));
    model = glm::scale(model, glm::vec3(1.3f, 1.0f, 1.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    fence.Draw(myBasicShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-5.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.3f, 1.0f, 1.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    fence.Draw(myBasicShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-5.0f, 0.0f, 4.0f));
    model = glm::scale(model, glm::vec3(1.3f, 1.0f, 1.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    fence.Draw(myBasicShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-5.0f, 0.0f, 5.5f));
    model = glm::scale(model, glm::vec3(1.3f, 1.0f, 1.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    fence.Draw(myBasicShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-5.0f, 0.0f, 7.0f));
    model = glm::scale(model, glm::vec3(1.3f, 1.0f, 1.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    fence.Draw(myBasicShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-5.0f, 0.0f, 8.5f));
    model = glm::scale(model, glm::vec3(1.3f, 1.0f, 1.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    fence.Draw(myBasicShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(4.85f, 0.0f, -3.0f));
    model = glm::scale(model, glm::vec3(1.3f, 1.0f, 1.0f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    fence.Draw(myBasicShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(4.85f, 0.0f, -1.5f));
    model = glm::scale(model, glm::vec3(1.3f, 1.0f, 1.0f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    fence.Draw(myBasicShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(4.85f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.3f, 1.0f, 1.0f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    fence.Draw(myBasicShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(4.85f, 0.0f, 4.0f));
    model = glm::scale(model, glm::vec3(1.3f, 1.0f, 1.0f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    fence.Draw(myBasicShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(4.85f, 0.0f, 5.5f));
    model = glm::scale(model, glm::vec3(1.3f, 1.0f, 1.0f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    fence.Draw(myBasicShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(4.85f, 0.0f, 7.0f));
    model = glm::scale(model, glm::vec3(1.3f, 1.0f, 1.0f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    fence.Draw(myBasicShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(4.85f, 0.0f, 8.5f));
    model = glm::scale(model, glm::vec3(1.3f, 1.0f, 1.0f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    fence.Draw(myBasicShader);
    
    //bush
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(9.0f, 0.0f, -6.0f));
    model = glm::scale(model, glm::vec3(0.08f, 0.08f, 0.08f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    bush.Draw(myBasicShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-9.0f, 0.0f, -6.0f));
    model = glm::scale(model, glm::vec3(0.08f, 0.08f, 0.08f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    bush.Draw(myBasicShader);

    //trees
    model = glm::mat4(1.0f);
    model = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -0.05f, -9.0f));
    model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    tree.Draw(myBasicShader);

    model = glm::mat4(1.0f);
    model = glm::translate(glm::mat4(1.0f), glm::vec3(9.0f, 0.0f, 4.5f));
    model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    tree.Draw(myBasicShader);

    model = glm::mat4(1.0f);
    model = glm::translate(glm::mat4(1.0f), glm::vec3(-9.0f, 0.0f, 4.5f));
    model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    tree.Draw(myBasicShader);

    //doghut
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(3.5f, 0.0f, 8.0f));
    model = glm::scale(model, glm::vec3(1.3f, 1.3f, 1.3f));
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    doghut.Draw(myBasicShader);

    //bench
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(1.0f, 0.0f, 8.0f));
    model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    bench.Draw(myBasicShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-2.5f, 0.0f, 8.0f));
    model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    bench.Draw(myBasicShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-4.0f, 0.0f, 5.0f));
    model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    bench.Draw(myBasicShader);

    //trash
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-4.5f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    trashbin.Draw(myBasicShader);

    mySkyBox.Draw(skyboxShader, view, projection);
    
}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
    glDeleteTextures(1, &depthMapTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &shadowMapFBO);
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
   
    initModels();
    
    initShaders();
    
    initSkybox();
    
	initUniforms();
    
    setWindowCallbacks();

    initFBO();
    
	
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
	    renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
