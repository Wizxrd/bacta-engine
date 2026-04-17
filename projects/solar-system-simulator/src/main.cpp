#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <string>
#include <iostream>


// forward declarations
GLFWwindow* StartGLFW();
GLuint CreateShaderProgram(const char* vertexSrc, const char* fragmentSrc);
void ProcessInput(GLFWwindow* window);
void MouseCallback(GLFWwindow* window, double xpos, double ypos);
void DrawGrid(float halfSize, float step);

// window
const int WIDTH = 1920;
const int HEIGHT = 1080;

// conversions
static const double MASS_SCALE = 1e12;
static const double DISTANCE_SCALE = 5e2;

static const double SOL_MASS      = 1.989e30   / MASS_SCALE;
static const double MERCURY_MASS  = 3.3011e23  / MASS_SCALE;
static const double VENUS_MASS    = 4.8675e24  / MASS_SCALE;
static const double TERRA_MASS    = 5.9722e24  / MASS_SCALE;

static const double VENUS_PERIAPSIS   = 1.0748e8 / DISTANCE_SCALE;
static const double MERCURY_PERIAPSIS = 4.6001e7 / DISTANCE_SCALE;
static const double TERRA_PERIAPSIS   = 1.4710e8 / DISTANCE_SCALE;

// camera
float cameraFov = 45.0f;
float cameraX = 0.f;
float cameraY = 0.f;
float cameraZ = 0.f;

float yaw = -90.0f;
float pitch = 0.0f;

float frontX = 0.0f;
float frontY = 0.0f;
float frontZ = -0.1f;

float moveSpeed = 5000.f;
float mouseSensitivity = 0.1f;

bool firstMouse = true;
double lastMouseX = WIDTH / 2.0;
double lastMouseY = HEIGHT / 2.0;

// structs
struct vec3f{
	float x;
	float y;
	float z;
};

struct CelestialBody{
	vec3f position;
	vec3f velocity;
	double mass;
	double radius;
	int stacks = 25;
	int sectors = 25;
	CelestialBody(vec3f pos, vec3f vel, double mass, double density = 3344.0) : position(pos), velocity(vel), mass(mass){
		double v = mass/density;
		radius = std::pow((3.0 * mass) / (4.0 * M_PI * density), 1.0/3.0);
	}

	void Update(float deltaTime){
		position.x += velocity.x * deltaTime;
		position.y += velocity.y * deltaTime;
		position.z += velocity.z * deltaTime;
	}

	void Render(){
		glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
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
		glPopMatrix();
	}
};

// shader programs
GLuint emitterShaderProgram = 0;
GLuint litShaderProgram = 0;

// shaders
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
    vec3 normal = normalize(vNormal);

    float facing = max(dot(normal, vec3(0.0, 0.0, 1.0)), 0.0);

    vec3 edgeColor = vec3(1.0, 0.45, 0.0);
    vec3 midColor = vec3(1.0, 0.75, 0.2);
    vec3 coreColor = vec3(1.0, 0.95, 0.8);

    vec3 color = mix(edgeColor, midColor, facing);
    color = mix(color, coreColor, pow(facing, 4.0));

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
uniform vec3 baseColor;

void main(){
    vec3 normal = normalize(vNormal);
    vec3 lightDir = normalize(lightPosView - vViewPos);

    float ndl = max(dot(normal, lightDir), 0.0);
    ndl = pow(ndl, 1.5);

    vec3 ambient = baseColor * 0.08;
    vec3 diffuse = baseColor * ndl;

    vec3 finalColor = ambient + diffuse;

    gl_FragColor = vec4(finalColor, 1.0);
}
)";

int main(){
	// DEBUG

	CelestialBody sol({0.f,0.f,0.f}, {0.f, 0.f, 0.f}, SOL_MASS, 1408.0);
	sol.position.y += sol.radius*2.0;
	//cameraZ = sol.radius*25.f;

	CelestialBody mercury({MERCURY_PERIAPSIS,0.f,0.f}, {0.f, 0.f, 0.f}, MERCURY_MASS, 5429.0);
	mercury.position.y += sol.radius*2.0;

	CelestialBody venus({VENUS_PERIAPSIS,0.f, mercury.radius*2.0f}, {0.f, 0.f, 0.f}, VENUS_MASS, 5243.0);
	venus.position.y += sol.radius*2.0;

	CelestialBody terra({TERRA_PERIAPSIS,0.f, venus.radius*2.0f}, {0.f, 0.f, 0.f}, TERRA_MASS, 5243.0);
	terra.position.y += sol.radius*2.0;

	GLFWwindow* window = StartGLFW();
	if (!window){ return -1; }

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
	float lastTime = (float)glfwGetTime();
	while (!glfwWindowShouldClose(window)){
		float currentTime = (float)glfwGetTime();
		float deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		//updates
		sol.Update(deltaTime);
		mercury.Update(deltaTime);
		venus.Update(deltaTime);
		terra.Update(deltaTime);

		//input
		ProcessInput(window);

		//rendering
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

		DrawGrid(1250000.f, 62500.f);

		// get sphere1 position in VIEW space
        GLdouble modelview[16];
        glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

		float lightViewX =
            (float)(modelview[0] * sol.position.x +
                    modelview[4] * sol.position.y +
                    modelview[8] * sol.position.z +
                    modelview[12]);

        float lightViewY =
            (float)(modelview[1] * sol.position.x +
                    modelview[5] * sol.position.y +
                    modelview[9] * sol.position.z +
                    modelview[13]);

        float lightViewZ =
            (float)(modelview[2] * sol.position.x +
                    modelview[6] * sol.position.y +
                    modelview[10] * sol.position.z +
                    modelview[14]);

		// sol, the emiiter.
        glUseProgram(emitterShaderProgram);
		sol.Render();

		glUseProgram(litShaderProgram);
		GLint lightPosLoc = glGetUniformLocation(litShaderProgram, "lightPosView");
		GLint baseColorLoc = glGetUniformLocation(litShaderProgram, "baseColor");
        glUniform3f(lightPosLoc, lightViewX, lightViewY, lightViewZ);
		glUniform3f(baseColorLoc, 0.6f, 0.6f, 0.6f);
		mercury.Render();
		glUseProgram(0);

		glUseProgram(litShaderProgram);
        glUniform3f(lightPosLoc, lightViewX, lightViewY, lightViewZ);
		glUniform3f(baseColorLoc, 0.9f, 0.7f, 0.4f);
		venus.Render();
		glUseProgram(0);

		glUseProgram(litShaderProgram);
        glUniform3f(lightPosLoc, lightViewX, lightViewY, lightViewZ);
		glUniform3f(baseColorLoc, 0.2f, 0.4f, 1.0f);
		terra.Render();
		glUseProgram(0);


        // simple line connecting the two spheres
        glDisable(GL_LIGHTING);
        glLineWidth(0.5f);

        glBegin(GL_LINES);
        glColor3f(0.2f, 0.2f, 0.2f);
        glVertex3f(sol.position.x, sol.position.y, sol.position.z);
        glVertex3f(mercury.position.x, mercury.position.y, mercury.position.z);
        glEnd();

		glBegin(GL_LINES);
        glColor3f(0.2f, 0.2f, 0.2f);
        glVertex3f(sol.position.x, sol.position.y, sol.position.z);
        glVertex3f(venus.position.x, venus.position.y, venus.position.z);
        glEnd();

		glBegin(GL_LINES);
        glColor3f(0.2f, 0.2f, 0.2f);
        glVertex3f(sol.position.x, sol.position.y, sol.position.z);
        glVertex3f(terra.position.x, terra.position.y, terra.position.z);
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
		std::cerr << "ERROR: failed to init glfw\n";
		glfwTerminate();
		return nullptr;
	}

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "solar system", NULL, NULL);
	if (!window){
		std::cerr << "ERROR: failed to create window\n";
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK){
		std::cerr << "ERROR: failed to init glew\n";
		return nullptr;
	}

	glViewport(0, 0, WIDTH, HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

    float aspect = (float)WIDTH / (float)HEIGHT;
	float nearPlane = 100.0f;
	float farPlane = 2000000.0f;
    float top = tanf(cameraFov * 0.5f * (float)M_PI / 180.f) * nearPlane;
    float right = top * aspect;

    glFrustum(-right, right, -top, top, nearPlane, farPlane);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

void UpdateProjection(){
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float aspect = (float)WIDTH / (float)HEIGHT;
    float nearPlane = 100.0f;
    float farPlane = 2000000.0f;

    float top = tanf(cameraFov * 0.5f * (float)M_PI / 180.0f) * nearPlane;
    float right = top * aspect;

    glFrustum(-right, right, -top, top, nearPlane, farPlane);

    glMatrixMode(GL_MODELVIEW);
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

	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS){
		cameraFov -= 0.5f;
		if (cameraFov < 10.0f){
			cameraFov = 10.0f;
		}
		UpdateProjection();
	}

	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS){
		cameraFov += 0.5f;
		if (cameraFov > 200.0f){
			cameraFov = 200.0f;
		}
		UpdateProjection();
	}
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
}

void DrawGrid(float halfSize, float step){
    glUseProgram(0);

    glDisable(GL_LIGHTING);
    glColor3f(0.25f, 0.25f, 0.25f);

    glBegin(GL_LINES);

    for (float i = -halfSize; i <= halfSize; i += step){
        // lines parallel to Z
        glVertex3f(i, 0.0f, -halfSize);
        glVertex3f(i, 0.0f,  halfSize);

        // lines parallel to X
        glVertex3f(-halfSize, 0.0f, i);
        glVertex3f( halfSize, 0.0f, i);
    }

    glEnd();

    // world center axes highlight
	glLineWidth(2.0f);
    glBegin(GL_LINES);
	// x
	glColor3f(0.0f, 0.2f, 1.0f);
    glVertex3f(-halfSize, 0.0f, 0.0f);
    glVertex3f( halfSize, 0.0f, 0.0f);
	//z
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, -halfSize);
    glVertex3f(0.0f, 0.0f,  halfSize);
    glEnd();
}