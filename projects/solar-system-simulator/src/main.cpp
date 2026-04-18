#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <string>
#include <iostream>
#include <vector>

// forward declarations
GLFWwindow* StartGLFW();
GLuint CreateShaderProgram(const char* vertexSrc, const char* fragmentSrc);
void ProcessInput(GLFWwindow* window, float deltaTime);
void MouseCallback(GLFWwindow* window, double xpos, double ypos);
void UpdateMouseLook(float deltaTime);

// window
const int WIDTH = 1920;
const int HEIGHT = 1080;
float simRate = 1.f;

// conversions
static const double DISTANCE_RENDER_SCALE = 2e5;
static const double RADIUS_RENDER_SCALE   = 1e5;

static const double SOL_MASS      = 1.989e30;
static const double MERCURY_MASS  = 3.3011e23;
static const double VENUS_MASS    = 4.8675e24;
static const double TERRA_MASS    = 5.9722e24;
static const double MARS_MASS     = 6.4171e23;
static const double JUPITER_MASS  = 1.89813e27;
static const double SATURN_MASS   = 5.6832e26;
static const double URANUS_MASS   = 8.6813e25;
static const double NEPTUNE_MASS  = 1.02431e26;

static const double MERCURY_PERIAPSIS = 4.6001e10;
static const double VENUS_PERIAPSIS   = 1.0748e11;
static const double TERRA_PERIAPSIS   = 1.4710e11;
static const double MARS_PERIAPSIS    = 2.0665e11;
static const double JUPITER_PERIAPSIS = 7.4052e11;
static const double SATURN_PERIAPSIS  = 1.35255e12;
static const double URANUS_PERIAPSIS  = 2.74130e12;
static const double NEPTUNE_PERIAPSIS = 4.44445e12;

//// SATELLITES
static const double MOON_MASS = 7.34767309e22;
static const double MOON_PERIAPSIS = 3.633e8;

static const double PHOBOS_MASS = 1.0659e16;
static const double PHOBOS_PERIAPSIS = 9.23442e6;

static const double DEIMOS_MASS = 1.4762e15;
static const double DEIMOS_PERIAPSIS = 2.34589e7;
////

static const double G = 6.67430e-11; // m^3 kg^-1 s^-2
static const float c = 299792458.0;

// camera
float cameraFov = 45.0f;
int cameraLockIndex = -1;
double cameraDistance = 10000.0;
double cameraX = 0.0;
double cameraY = 0.0;
double cameraZ = 0.0;

std::vector<float> gridVertices{};
double cachedGridHalfSize = -1.0;
double cachedGridStep = -1.0;

double yaw = -90.0;
double pitch = 0.0;

double frontX = 0.0;
double frontY = 0.0;
double frontZ = -1.0;

float moveSpeed = 100000.f;
float mouseSensitivity = 0.1f;

bool firstMouse = true;
double lastMouseX = WIDTH / 2.0;
double lastMouseY = HEIGHT / 2.0;
float pendingMouseX = 0.0f;
float pendingMouseY = 0.0f;

// structs
struct vec3d{
	double x;
	double y;
	double z;
};

struct CelestialBody{
    vec3d position;
    vec3d velocity;
    double mass;
    double radius;
    int parentIndex = -1;
    int stacks = 25;
    int sectors = 25;
    std::vector<vec3d> trail;

    CelestialBody(vec3d pos, vec3d vel, double mass, double density = 3344.0, int parentIndex = -1)
        : position(pos), velocity(vel), mass(mass), parentIndex(parentIndex){
        radius = std::pow((3.0 * mass) / (4.0 * M_PI * density), 1.0 / 3.0);
        trail.emplace_back(position);
    }

    void Accelerate(vec3d vel, float deltaTime){
        velocity.x += vel.x * deltaTime;
        velocity.y += vel.y * deltaTime;
        velocity.z += vel.z * deltaTime;
    }

	void Update(float deltaTime){
		position.x += velocity.x * deltaTime;
		position.y += velocity.y * deltaTime;
		position.z += velocity.z * deltaTime;
        trail.emplace_back(position);
        if (trail.size() > 1000){
            trail.erase(trail.begin());
        }
	}

	void Render(){
		glPushMatrix();
        glTranslatef(position.x / DISTANCE_RENDER_SCALE, position.y / DISTANCE_RENDER_SCALE, position.z / DISTANCE_RENDER_SCALE);
		float scaledRadius = radius / RADIUS_RENDER_SCALE;
        for (int i = 0; i < stacks; i++){
			float stackAngle1 = (float)M_PI / 2.f - i * (float)M_PI / stacks;
			float stackAngle2 = (float)M_PI / 2.f - (i + 1) * (float)M_PI / stacks;
			float xy1 = scaledRadius * cosf(stackAngle1);
			float z1  = scaledRadius * sinf(stackAngle1);
			float xy2 = scaledRadius * cosf(stackAngle2);
			float z2  = scaledRadius * sinf(stackAngle2);
			glBegin(GL_TRIANGLE_STRIP);
			for (int j = 0; j <= sectors; j++){
				float sectorAngle = j * 2.f * (float)M_PI / sectors;
				float x1 = xy1 * cosf(sectorAngle);
				float y1 = xy1 * sinf(sectorAngle);
				float x2 = xy2 * cosf(sectorAngle);
				float y2 = xy2 * sinf(sectorAngle);
				glNormal3f(x1 / scaledRadius, y1 / scaledRadius, z1 / scaledRadius);
				glVertex3f(x1, y1, z1);
				glNormal3f(x2 / scaledRadius, y2 / scaledRadius, z2 / scaledRadius);
				glVertex3f(x2, y2, z2);
			}
			glEnd();
		}
		glPopMatrix();

        glBegin(GL_LINE_STRIP);
        for (const vec3d& pos : trail){
            glVertex3d(pos.x / DISTANCE_RENDER_SCALE, pos.y / DISTANCE_RENDER_SCALE, pos.z / DISTANCE_RENDER_SCALE);
        }
        glEnd();
	}
};

CelestialBody MakeSatellite(const CelestialBody& parent,int parentIndex,double periapsis,double relativeSpeed,double mass,double density=3344.0){
    return CelestialBody(
        vec3d{parent.position.x + periapsis, parent.position.y, parent.position.z},
        vec3d{parent.velocity.x, parent.velocity.y, parent.velocity.z + relativeSpeed},
        mass,
        density,
        parentIndex
    );
}

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

void DrawGrid(double halfSize,double step);
void DrawWarpGrid(double halfSize,double step,const CelestialBody& sol,const std::vector<CelestialBody>& bodies);
double GetWarpHeight(double x,double z,const CelestialBody& sol,const std::vector<CelestialBody>& bodies);
void RebuildGridCache(double halfSize,double step,const CelestialBody& sol,const std::vector<CelestialBody>& bodies);

void doGravity(CelestialBody& sol, std::vector<CelestialBody>& bodies, float deltaTime){
    for (int i = 0; i < (int)bodies.size(); i++){
        CelestialBody& body = bodies[i];

        double dx = sol.position.x - body.position.x;
        double dy = sol.position.y - body.position.y;
        double dz = sol.position.z - body.position.z;
        double distance = std::sqrt(dx * dx + dy * dy + dz * dz);

        if (distance > 0.0){
            vec3d direction = { dx / distance, dy / distance, dz / distance };
            double acc = (G * sol.mass) / (distance * distance);
            vec3d acceleration = {
                direction.x * acc,
                direction.y * acc,
                direction.z * acc
            };
            body.Accelerate(acceleration, deltaTime);
        }

        if (body.parentIndex >= 0){
            CelestialBody& parent = bodies[body.parentIndex];

            double pdx = parent.position.x - body.position.x;
            double pdy = parent.position.y - body.position.y;
            double pdz = parent.position.z - body.position.z;
            double pdistance = std::sqrt(pdx * pdx + pdy * pdy + pdz * pdz);

            if (pdistance > 0.0){
                vec3d pdir = { pdx / pdistance, pdy / pdistance, pdz / pdistance };
                double pacc = (G * parent.mass) / (pdistance * pdistance);
                vec3d pacceleration = {
                    pdir.x * pacc,
                    pdir.y * pacc,
                    pdir.z * pacc
                };
                body.Accelerate(pacceleration, deltaTime);
            }
        }
    }
}

double GetCircularOrbitSpeed(double parentMass, double radius){
    return std::sqrt((G * parentMass) / radius);
}

int main(){
	CelestialBody sol({0.0,0.0,0.0}, {0.0, 0.0, 0.0}, SOL_MASS, 1408.0);

    std::vector<CelestialBody> bodies{};
    bodies.emplace_back(vec3d{MERCURY_PERIAPSIS, 0.0, 0.0}, vec3d{0.0, 0.0, 58985.0}, MERCURY_MASS, 5429.0);
    bodies.emplace_back(vec3d{-VENUS_PERIAPSIS,   0.0, 0.0}, vec3d{0.0, 0.0, 35000.0}, VENUS_MASS,   5243.0);
    bodies.emplace_back(vec3d{TERRA_PERIAPSIS,   0.0, TERRA_PERIAPSIS}, vec3d{0.0, 0.0, 30290.0}, TERRA_MASS,   5514.0);
    bodies.emplace_back(vec3d{-MARS_PERIAPSIS,    0.0, 0.0}, vec3d{0.0, 0.0, 26500.0}, MARS_MASS,    3933.0);
    bodies.emplace_back(vec3d{JUPITER_PERIAPSIS, 0.0, 0.0}, vec3d{0.0, 0.0, 13720.0}, JUPITER_MASS, 1326.0);
    bodies.emplace_back(vec3d{0.0,  0.0, SATURN_PERIAPSIS}, vec3d{0.0, 0.0, 10180.0}, SATURN_MASS,  687.0);
    bodies.emplace_back(vec3d{-URANUS_PERIAPSIS,  0.0, 0.0}, vec3d{0.0, 0.0, 7110.0},  URANUS_MASS,  1271.0);
    bodies.emplace_back(vec3d{NEPTUNE_PERIAPSIS, 0.0, 0.0}, vec3d{0.0, 0.0, 5500.0},  NEPTUNE_MASS, 1638.0);

    double moonSpeed   = GetCircularOrbitSpeed(TERRA_MASS, MOON_PERIAPSIS);
    double phobosSpeed = GetCircularOrbitSpeed(MARS_MASS, PHOBOS_PERIAPSIS);
    double deimosSpeed = GetCircularOrbitSpeed(MARS_MASS, DEIMOS_PERIAPSIS);
    bodies.emplace_back(MakeSatellite(bodies[2], 2,  MOON_PERIAPSIS,    moonSpeed,   MOON_MASS,   3344.0));
    bodies.emplace_back(MakeSatellite(bodies[3], 3,  PHOBOS_PERIAPSIS,  phobosSpeed, PHOBOS_MASS, 1860.0));
    bodies.emplace_back(MakeSatellite(bodies[3], 3, DEIMOS_PERIAPSIS, deimosSpeed, DEIMOS_MASS, 1471.0));

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
		float realDeltaTime = currentTime - lastTime;
        lastTime = currentTime;
        float deltaTime = realDeltaTime * simRate;

        int physicsSubsteps = 32;
        float stepDt = deltaTime / physicsSubsteps;

        for (int step = 0; step < physicsSubsteps; step++){
            doGravity(sol, bodies, stepDt);

            sol.Update(stepDt);
            for (CelestialBody& body : bodies){
                body.Update(stepDt);
            }
        }

		//input
		ProcessInput(window, realDeltaTime);
        UpdateMouseLook(realDeltaTime);

        if (cameraLockIndex == -2){
            cameraX = (sol.position.x / DISTANCE_RENDER_SCALE) - frontX * cameraDistance;
            cameraY = (sol.position.y / DISTANCE_RENDER_SCALE) - frontY * cameraDistance;
            cameraZ = (sol.position.z / DISTANCE_RENDER_SCALE) - frontZ * cameraDistance;
        }
        else if (cameraLockIndex >= 0 && cameraLockIndex < (int)bodies.size()){
            cameraX = (bodies[cameraLockIndex].position.x / DISTANCE_RENDER_SCALE) - frontX * cameraDistance;
            cameraY = (bodies[cameraLockIndex].position.y / DISTANCE_RENDER_SCALE) - frontY * cameraDistance;
            cameraZ = (bodies[cameraLockIndex].position.z / DISTANCE_RENDER_SCALE) - frontZ * cameraDistance;
        }

		//rendering
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        double centerX = cameraX + frontX;
        double centerY = cameraY + frontY;
        double centerZ = cameraZ + frontZ;

		gluLookAt(
            cameraX, cameraY, cameraZ,
            centerX, centerY, centerZ,
            0.0f, 1.0f, 0.0f
        );

		//DrawWarpGrid(2500000.0, 10000.0, sol, bodies);//DrawWarpGrid(25000000.0, 50000.0, sol, bodies);
        DrawGrid(25000000.0, 100000.0);
		// get sphere1 position in VIEW space
        GLdouble modelview[16];
        glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

        float lightViewX =
            (float)(modelview[0] * (sol.position.x / DISTANCE_RENDER_SCALE) +
                    modelview[4] * (sol.position.y / DISTANCE_RENDER_SCALE) +
                    modelview[8] * (sol.position.z / DISTANCE_RENDER_SCALE) +
                    modelview[12]);

        float lightViewY =
            (float)(modelview[1] * (sol.position.x / DISTANCE_RENDER_SCALE) +
                    modelview[5] * (sol.position.y / DISTANCE_RENDER_SCALE) +
                    modelview[9] * (sol.position.z / DISTANCE_RENDER_SCALE) +
                    modelview[13]);

        float lightViewZ =
            (float)(modelview[2] * (sol.position.x / DISTANCE_RENDER_SCALE) +
                    modelview[6] * (sol.position.y / DISTANCE_RENDER_SCALE) +
                    modelview[10] * (sol.position.z / DISTANCE_RENDER_SCALE) +
                    modelview[14]);

		// sol, the emiiter.
        glUseProgram(emitterShaderProgram);
		sol.Render();

        glUseProgram(litShaderProgram);
        GLint lightPosLoc = glGetUniformLocation(litShaderProgram, "lightPosView");
        GLint baseColorLoc = glGetUniformLocation(litShaderProgram, "baseColor");
        glUniform3f(lightPosLoc, lightViewX, lightViewY, lightViewZ);
        glUniform3f(baseColorLoc, 0.6f, 0.6f, 0.6f);
        bodies[0].Render();
        glUniform3f(baseColorLoc, 0.9f, 0.7f, 0.4f);
        bodies[1].Render();
        glUniform3f(baseColorLoc, 0.2f, 0.4f, 1.0f);
        bodies[2].Render();
        glUniform3f(baseColorLoc, 0.9f, 0.3f, 0.2f);
        bodies[3].Render();
        glUniform3f(baseColorLoc, 0.9f, 0.8f, 0.6f);
        bodies[4].Render();
        glUniform3f(baseColorLoc, 0.9f, 0.8f, 0.5f);
        bodies[5].Render();
        glUniform3f(baseColorLoc, 0.5f, 0.9f, 0.9f);
        bodies[6].Render();
        glUniform3f(baseColorLoc, 0.2f, 0.5f, 0.9f);
        bodies[7].Render();
        glUniform3f(baseColorLoc, 0.6f, 0.6f, 0.6f);
        bodies[8].Render();
        glUniform3f(baseColorLoc, 0.2f, 0.5f, 0.9f);
        bodies[9].Render();
        glUniform3f(baseColorLoc, 0.6f, 0.6f, 0.6f);
        bodies[10].Render();
        glUseProgram(0);

        glDisable(GL_LIGHTING);
        // glLineWidth(0.5f);

        // for (CelestialBody& body : bodies){
        //     glBegin(GL_LINES);
        //     glColor3f(0.2f, 0.2f, 0.2f);
        //     glVertex3f(sol.position.x / DISTANCE_RENDER_SCALE,
        //             sol.position.y / DISTANCE_RENDER_SCALE,
        //             sol.position.z / DISTANCE_RENDER_SCALE);

        //     glVertex3f(body.position.x / DISTANCE_RENDER_SCALE,
        //             body.position.y / DISTANCE_RENDER_SCALE,
        //             body.position.z / DISTANCE_RENDER_SCALE);
        //     glEnd();
        // }

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
    if (glfwRawMouseMotionSupported()){
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }

	if (glewInit() != GLEW_OK){
		std::cerr << "ERROR: failed to init glew\n";
		return nullptr;
	}

	glViewport(0, 0, WIDTH, HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

    float aspect = (float)WIDTH / (float)HEIGHT;
    float nearPlane = 5000.0f;
    float farPlane = 1000000000.0f;
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
    float nearPlane = 5000.0f;
    float farPlane = 1000000000.0f;

    float top = tanf(cameraFov * 0.5f * (float)M_PI / 180.0f) * nearPlane;
    float right = top * aspect;

    glFrustum(-right, right, -top, top, nearPlane, farPlane);

    glMatrixMode(GL_MODELVIEW);
}

void ProcessInput(GLFWwindow* window, float deltaTime){
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }

    double rightX = frontZ;
    double rightY = 0.0;
    double rightZ = -frontX;

    double rightLength = sqrt(rightX * rightX + rightY * rightY + rightZ * rightZ);
    if (rightLength > 0.0f){
        rightX /= rightLength;
        rightY /= rightLength;
        rightZ /= rightLength;
    }

    double speed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) ? moveSpeed*50 : moveSpeed;
    double cameraStep = speed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        cameraX += frontX * cameraStep;
        cameraY += frontY * cameraStep;
        cameraZ += frontZ * cameraStep;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        cameraX -= frontX * cameraStep;
        cameraY -= frontY * cameraStep;
        cameraZ -= frontZ * cameraStep;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        cameraX -= rightX * cameraStep;
        cameraZ -= rightZ * cameraStep;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        cameraX += rightX * cameraStep;
        cameraZ += rightZ * cameraStep;
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS){
        cameraY += cameraStep;
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS){
        cameraY -= cameraStep;
    }

	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS){
		cameraFov -= 0.5f;
		if (cameraFov < 1.0f){
			cameraFov = 1.0f;
		}
		UpdateProjection();
	}

	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS){
		cameraFov += 0.5f;
		if (cameraFov > 120.0f){
			cameraFov = 120.0f;
		}
		UpdateProjection();
	}

    if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS){
        simRate *= 2.0f;
        if (simRate > 2048.0f*10000){
            simRate = 2048.0f*10000;
        }
        std::cout << simRate << '\n';
    }

    if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS){
        simRate = 1.f;
    }

    if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS){
        simRate *= 0.5f;
        if (simRate < 0.125f){
            simRate = 0.125f;
        }
        std::cout << simRate << '\n';
    }
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS){
        cameraLockIndex = -1;
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS){
        cameraDistance = 50000.0;
        cameraLockIndex = -2;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS){
        cameraDistance = 7500.0;
        cameraLockIndex = 0;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS){
        cameraDistance = 10000.0;
        cameraLockIndex = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS){
        cameraDistance = 10000.0;
        cameraLockIndex = 2;
    }
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS){
        cameraDistance = 10000.0;
        cameraLockIndex = 3;
    }
    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS){
        cameraDistance = 10000.0;
        cameraLockIndex = 4;
    }
    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS){
        cameraDistance = 10000.0;
        cameraLockIndex = 5;
    }
    if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS){
        cameraDistance = 10000.0;
        cameraLockIndex = 6;
    }
    if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS){
        cameraDistance = 10000.0;
        cameraLockIndex = 7;
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

    pendingMouseX += xoffset;
    pendingMouseY += yoffset;
}

void UpdateMouseLook(float deltaTime){
    float xoffset = pendingMouseX;
    float yoffset = pendingMouseY;

    pendingMouseX = 0.0f;
    pendingMouseY = 0.0f;

    float mouseScale = mouseSensitivity * 60.0f * deltaTime;
    xoffset *= mouseScale;
    yoffset *= mouseScale;

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

double GetWarpHeight(double x,double z,const CelestialBody& sol,const std::vector<CelestialBody>& bodies){
    double warp = 0.0;

    auto addWarp = [&](const CelestialBody& body,double depth,double width){
        double bx = body.position.x / DISTANCE_RENDER_SCALE;
        double bz = body.position.z / DISTANCE_RENDER_SCALE;
        double dx = x - bx;
        double dz = z - bz;
        double dist = std::sqrt(dx * dx + dz * dz);
        warp -= depth / (1.0 + dist / width);
    };

    addWarp(sol, 120000.0, 25000.0);

    for (const CelestialBody& body : bodies){
        double depth = body.mass / 1e20;
        double width = body.radius / RADIUS_RENDER_SCALE;

        if (depth < 3000.0){
            depth = 3000.0;
        }
        if (depth > 40000.0){
            depth = 40000.0;
        }

        if (width < 4000.0){
            width = 4000.0;
        }
        if (width > 20000.0){
            width = 20000.0;
        }

        addWarp(body, depth, width);
    }

    return warp;
}

void RebuildGridCache(double halfSize,double step,const CelestialBody& sol,const std::vector<CelestialBody>& bodies){
    cachedGridHalfSize = halfSize;
    cachedGridStep = step;

    int count = (int)((halfSize * 2.0) / step) + 1;
    if (count < 2){
        gridVertices.clear();
        return;
    }

    std::vector<float> heights(count * count);

    double baseOffset =
        (GetWarpHeight(-halfSize,-halfSize,sol,bodies) +
         GetWarpHeight( halfSize,-halfSize,sol,bodies) +
         GetWarpHeight(-halfSize, halfSize,sol,bodies) +
         GetWarpHeight( halfSize, halfSize,sol,bodies)) * 0.25;

    for (int ix = 0; ix < count; ix++){
        double x = -halfSize + ix * step;
        for (int iz = 0; iz < count; iz++){
            double z = -halfSize + iz * step;
            heights[ix * count + iz] = (float)(GetWarpHeight(x, z, sol, bodies) - baseOffset);
        }
    }

    gridVertices.clear();
    gridVertices.reserve((size_t)(count - 1) * count * 2 * 2 * 3);

    for (int ix = 0; ix < count; ix++){
        float x = (float)(-halfSize + ix * step);
        for (int iz = 0; iz < count - 1; iz++){
            float z1 = (float)(-halfSize + iz * step);
            float z2 = (float)(-halfSize + (iz + 1) * step);
            float y1 = heights[ix * count + iz];
            float y2 = heights[ix * count + (iz + 1)];

            gridVertices.push_back(x);
            gridVertices.push_back(y1);
            gridVertices.push_back(z1);

            gridVertices.push_back(x);
            gridVertices.push_back(y2);
            gridVertices.push_back(z2);
        }
    }

    for (int iz = 0; iz < count; iz++){
        float z = (float)(-halfSize + iz * step);
        for (int ix = 0; ix < count - 1; ix++){
            float x1 = (float)(-halfSize + ix * step);
            float x2 = (float)(-halfSize + (ix + 1) * step);
            float y1 = heights[ix * count + iz];
            float y2 = heights[(ix + 1) * count + iz];

            gridVertices.push_back(x1);
            gridVertices.push_back(y1);
            gridVertices.push_back(z);

            gridVertices.push_back(x2);
            gridVertices.push_back(y2);
            gridVertices.push_back(z);
        }
    }
}

void DrawGrid(double halfSize,double step){
    glUseProgram(0);
    glDisable(GL_LIGHTING);
    glColor3f(0.25f, 0.25f, 0.25f);

    glBegin(GL_LINES);

    for (double x = -halfSize; x <= halfSize; x += step){
        glVertex3f((float)x, 0.0f, (float)-halfSize);
        glVertex3f((float)x, 0.0f, (float) halfSize);
    }

    for (double z = -halfSize; z <= halfSize; z += step){
        glVertex3f((float)-halfSize, 0.0f, (float)z);
        glVertex3f((float) halfSize, 0.0f, (float)z);
    }

    glEnd();
}

void DrawWarpGrid(double halfSize,double step,const CelestialBody& sol,const std::vector<CelestialBody>& bodies){
    if (gridVertices.empty() || cachedGridHalfSize != halfSize || cachedGridStep != step){
        RebuildGridCache(halfSize, step, sol, bodies);
    }
    else{
        RebuildGridCache(halfSize, step, sol, bodies);
    }

    glUseProgram(0);
    glDisable(GL_LIGHTING);
    glColor3f(0.25f, 0.25f, 0.25f);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, gridVertices.data());
    glDrawArrays(GL_LINES, 0, (GLsizei)(gridVertices.size() / 3));
    glDisableClientState(GL_VERTEX_ARRAY);
}