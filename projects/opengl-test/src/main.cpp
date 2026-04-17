#include <GL/glew.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>

GLFWwindow* StartGLFW();
void DrawSphere(float radius, int stacks, int sectors);
GLuint CreateShaderProgram(const char* vertexSrc, const char* fragmentSrc);
void ProcessInput(GLFWwindow* window);
void MouseCallback(GLFWwindow* window, double xpos, double ypos);

int screenWidth = 800;
int screenHeight = 600;
GLuint emitterShaderProgram = 0;
GLuint litShaderProgram = 0;

static const float SCALE = 1000000.f;
static const float EARTH_RADIUS_METERS = 6371000.0 / SCALE;
static const float MOON_RADIUS_METERS = 1737100.0 / SCALE;
static const float EARTH_TO_MOON = 384400000.0 / SCALE;

float cameraX = 0.f;
float cameraY = 0.f;
float cameraZ = EARTH_RADIUS_METERS * 2.5f;

float yaw = -90.0f;
float pitch = 0.0f;

float frontX = 0.0f;
float frontY = 0.0f;
float frontZ = -0.2f;

float moveSpeed = 1.f;
float mouseSensitivity = 0.1f;

bool firstMouse = true;
double lastMouseX = screenWidth / 2.0;
double lastMouseY = screenHeight / 2.0;

const char* emitterVertexShaderSource = R"(
#version 120

varying vec3 vNormal;

void main(){
    vNormal = normalize(gl_NormalMatrix * gl_Normal);
    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
}
)";

const char* emitterFragmentShaderSource = R"(
#version 120

varying vec3 vNormal;

void main(){
    vec3 glowColor = vec3(0.0, 0.3, 0.7);

    float fresnel = 1.0 - abs(dot(normalize(vNormal), vec3(0.0, 0.0, 1.0)));
    fresnel = pow(fresnel, 2.0);

    vec3 color = glowColor + glowColor * fresnel * 0.6;

    gl_FragColor = vec4(color, 1.0);
}
)";

const char* litVertexShaderSource = R"(
#version 120

varying vec3 vNormal;
varying vec3 vViewPos;

void main(){
    vec4 viewPos = gl_ModelViewMatrix * gl_Vertex;
    vViewPos = viewPos.xyz;
    vNormal = normalize(gl_NormalMatrix * gl_Normal);
    gl_Position = gl_ProjectionMatrix * viewPos;
}
)";

const char* litFragmentShaderSource = R"(
#version 120

varying vec3 vNormal;
varying vec3 vViewPos;

uniform vec3 lightPosView;

void main(){
    vec3 normal = normalize(vNormal);
    vec3 lightDir = normalize(lightPosView - vViewPos);

    float ndl = max(dot(normal, lightDir), 0.0);
    ndl = pow(ndl, 1.5);

    vec3 baseColor = vec3(1.0, 1.0, 1.0);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);

    vec3 ambient = baseColor * 0.08;
    vec3 diffuse = lightColor * ndl;

    vec3 finalColor = ambient + diffuse;

    gl_FragColor = vec4(finalColor, 1.0);
}
)";

int main(){
    GLFWwindow* window = StartGLFW();
    if (!window) return -1;

    emitterShaderProgram = CreateShaderProgram(emitterVertexShaderSource, emitterFragmentShaderSource);
    if (!emitterShaderProgram){
        glfwTerminate();
        return -1;
    }

    litShaderProgram = CreateShaderProgram(litVertexShaderSource, litFragmentShaderSource);
    if (!litShaderProgram){
        glfwTerminate();
        return -1;
    }

    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    float sphere1X = 0.0f;
    float sphere1Y = 0.0f;
    float sphere1Z = 0.0f;

    float sphere2X = EARTH_TO_MOON;
    float sphere2Y = 0.0f;
    float sphere2Z = 0.0f;

    while (!glfwWindowShouldClose(window)){
        ProcessInput(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        float centerX = cameraX + frontX;
        float centerY = cameraY + frontY;
        float centerZ = cameraZ + frontZ;

        gluLookAt(
            cameraX, cameraY, cameraZ,
            centerX, centerY, centerZ,
            0.0f, 1.0f, 0.0f
        );

        // get sphere1 position in VIEW space
        GLdouble modelview[16];
        glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

        float lightViewX =
            (float)(modelview[0] * sphere1X +
                    modelview[4] * sphere1Y +
                    modelview[8] * sphere1Z +
                    modelview[12]);

        float lightViewY =
            (float)(modelview[1] * sphere1X +
                    modelview[5] * sphere1Y +
                    modelview[9] * sphere1Z +
                    modelview[13]);

        float lightViewZ =
            (float)(modelview[2] * sphere1X +
                    modelview[6] * sphere1Y +
                    modelview[10] * sphere1Z +
                    modelview[14]);

        // sphere 1 = emitter
        glUseProgram(emitterShaderProgram);
        glPushMatrix();
        glTranslatef(sphere1X, sphere1Y, sphere1Z);
        DrawSphere(EARTH_RADIUS_METERS, 30, 30);
        glPopMatrix();

        // sphere 2 = lit by sphere 1
        glUseProgram(litShaderProgram);
        GLint lightPosLoc = glGetUniformLocation(litShaderProgram, "lightPosView");
        glUniform3f(lightPosLoc, lightViewX, lightViewY, lightViewZ);
        glPushMatrix();
        glTranslatef(sphere2X, sphere2Y, sphere2Z);
        DrawSphere(MOON_RADIUS_METERS, 30, 30);
        glPopMatrix();

        glUseProgram(0);

        // simple line connecting the two spheres
        glDisable(GL_LIGHTING);
        glLineWidth(0.5f);

        glBegin(GL_LINES);
        glColor3f(0.2f, 0.2f, 0.2f);
        glVertex3f(sphere1X, sphere1Y, sphere1Z);
        glVertex3f(sphere2X, sphere2Y, sphere2Z);
        glEnd();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(emitterShaderProgram);
    glDeleteProgram(litShaderProgram);
    glfwTerminate();
    return 0;
}

GLFWwindow* StartGLFW(){
    if (!glfwInit()){
        std::cerr << "Failed to init glfw\n";
        return nullptr;
    }

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Sphere", NULL, NULL);
    if (!window){
        std::cerr << "Failed to create window!\n";
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK){
        std::cerr << "Failed to init GLEW\n";
        glfwTerminate();
        return nullptr;
    }

    glViewport(0, 0, screenWidth, screenHeight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float aspect = (float)screenWidth / (float)screenHeight;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
    float fov = 45.f;
    float top = tanf(fov * 0.5f * (float)M_PI / 180.f) * nearPlane;
    float right = top * aspect;

    glFrustum(-right, right, -top, top, nearPlane, farPlane);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.f, 0.f, 0.f, 1.f);

    return window;
}

GLuint CreateShaderProgram(const char* vertexSrc, const char* fragmentSrc){
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSrc, nullptr);
    glCompileShader(vertexShader);

    GLint success = 0;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success){
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex shader compile error:\n" << infoLog << "\n";
        return 0;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSrc, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success){
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment shader compile error:\n" << infoLog << "\n";
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success){
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader link error:\n" << infoLog << "\n";
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

void ProcessInput(GLFWwindow* window){
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }

    float rightX = frontZ;
    float rightY = 0.0f;
    float rightZ = -frontX;

    float rightLength = sqrtf(rightX * rightX + rightY * rightY + rightZ * rightZ);
    if (rightLength > 0.0f){
        rightX /= rightLength;
        rightY /= rightLength;
        rightZ /= rightLength;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        cameraX += frontX * moveSpeed;
        cameraY += frontY * moveSpeed;
        cameraZ += frontZ * moveSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        cameraX -= frontX * moveSpeed;
        cameraY -= frontY * moveSpeed;
        cameraZ -= frontZ * moveSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        cameraX -= rightX * moveSpeed;
        cameraZ -= rightZ * moveSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        cameraX += rightX * moveSpeed;
        cameraZ += rightZ * moveSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS){
        cameraY += moveSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS){
        cameraY -= moveSpeed;
    }
    //std::cout << cameraX << " | " << cameraY << " | " << cameraZ << '\n';
}

void MouseCallback(GLFWwindow* window, double xpos, double ypos){
    if (firstMouse){
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouse = false;
    }

    float xoffset = (float)(xpos - lastMouseX);
    float yoffset = (float)(lastMouseY - ypos);

    lastMouseX = xpos;
    lastMouseY = ypos;

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    float yawRad = yaw * (float)M_PI / 180.0f;
    float pitchRad = pitch * (float)M_PI / 180.0f;

    frontX = cosf(yawRad) * cosf(pitchRad);
    frontY = sinf(pitchRad);
    frontZ = sinf(yawRad) * cosf(pitchRad);

    float length = sqrtf(frontX * frontX + frontY * frontY + frontZ * frontZ);
    if (length > 0.0f){
        frontX /= length;
        frontY /= length;
        frontZ /= length;
    }

    std::cout << "FRONT: " << frontX << " | " << frontY << " | " << frontZ << '\n';
}

void DrawSphere(float radius, int stacks, int sectors){
    for (int i = 0; i < stacks; i++){
        float stackAngle1 = (float)M_PI / 2.f - i * (float)M_PI / stacks;
        float stackAngle2 = (float)M_PI / 2.f - (i + 1) * (float)M_PI / stacks;

        float xy1 = radius * cosf(stackAngle1);
        float z1  = radius * sinf(stackAngle1);

        float xy2 = radius * cosf(stackAngle2);
        float z2  = radius * sinf(stackAngle2);

        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= sectors; j++){
            float sectorAngle = j * 2.f * (float)M_PI / sectors;

            float x1 = xy1 * cosf(sectorAngle);
            float y1 = xy1 * sinf(sectorAngle);

            float x2 = xy2 * cosf(sectorAngle);
            float y2 = xy2 * sinf(sectorAngle);

            glNormal3f(x1 / radius, y1 / radius, z1 / radius);
            glVertex3f(x1, y1, z1);

            glNormal3f(x2 / radius, y2 / radius, z2 / radius);
            glVertex3f(x2, y2, z2);
        }
        glEnd();
    }
}