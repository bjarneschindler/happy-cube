#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include <cstdlib>
#include <iostream>
#include <array>
#include <iomanip>

#include "linmath.h"
#include "LadeShader.h"

#define VERTEX_SHADER_PATH "shaders/DiffuseLightComponent.vert"
#define FRAGMENT_SHADER_PATH "shaders/DiffuseLightComponent.frag"
#define CUBE_VERTICES_COUNT 36
#define GRID_VERTICES_COUNT 32
#define VERTICES_COUNT (CUBE_VERTICES_COUNT + GRID_VERTICES_COUNT)
#define CUBE_EDGES_COUNT 8
#define CUBE_SCALE 0.3

struct Vertex {
    float x, y, z;
    float r, g, b;
    float nx, ny, nz;

    Vertex() { x = y = z = r = g = b = nx = ny = nz = 0; }

    Vertex(const Vertex other, const vec3 normal_vector) {
        x = other.x;
        y = other.y;
        z = other.z;

        r = other.r;
        g = other.g;
        b = other.b;

        nx = normal_vector[0];
        ny = normal_vector[1];
        nz = normal_vector[2];
    }

    Vertex(float x, float y, float z, float r, float g, float b) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->r = r;
        this->g = g;
        this->b = b;
        this->nx = this->ny = this->nz = 0;
    }
};

static Vertex cubeEdges[CUBE_EDGES_COUNT]
        {
                Vertex(+1.0f, +1.0f, +1.0f, 1.0f, 0.0f, 0.0f),
                Vertex(+1.0f, +1.0f, -1.0f, 0.0f, 1.0f, 0.0f),
                Vertex(+1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f),
                Vertex(+1.0f, -1.0f, +1.0f, 1.0f, 0.0f, 1.0f),
                Vertex(-1.0f, +1.0f, +1.0f, 0.0f, 1.0f, 1.0f),
                Vertex(-1.0f, -1.0f, +1.0f, 1.0f, 1.0f, 0.0f),
                Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f),
                Vertex(-1.0f, +1.0f, -1.0f, 0.0f, 0.0f, 0.0f)
        };

static const vec3 outline_color = {1.f, 1.f, 1.f};

static Vertex vertices[VERTICES_COUNT];

static float ambient_factor = 0.05;

static float diffuse_factor = 0.4;

static float spec_factor = 1.3;

static int shininess = 27;

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // Taste A erhöht den Ambient-Faktor, Shift+A verringert ihn
    if (key == GLFW_KEY_A && action != GLFW_PRESS)
        ambient_factor = mods == GLFW_MOD_SHIFT ? ambient_factor + .1f : ambient_factor - .1f;
    // Taste S erhöht den die Shininess, Shift+S verringert ihn
    if (key == GLFW_KEY_S && action != GLFW_PRESS)
        shininess = mods == GLFW_MOD_SHIFT ? shininess + 1 : shininess - 1;
    // Taste D erhöht den Diffuse-Faktor, Shift+D verringert ihn
    if (key == GLFW_KEY_D && action != GLFW_PRESS)
        diffuse_factor = mods == GLFW_MOD_SHIFT ? diffuse_factor + .1f : diffuse_factor - .1f;
    // Taste F erhöht den Specular-Faktor, Shift+F verringert ihn
    if (key == GLFW_KEY_F && action != GLFW_PRESS)
        spec_factor = mods == GLFW_MOD_SHIFT ? spec_factor + .1f : spec_factor - .1f;
    // Escape-Taste beendet das Programm
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void error_callback(int error, const char *description) {
    fprintf(stderr, "(%i) Error: %s\n", error, description);
}

static void scale_cube() {
    for (auto &edge: vertices) {
        edge.x *= CUBE_SCALE;
        edge.y *= CUBE_SCALE;
        edge.z *= CUBE_SCALE;
    }
}

static void build_outlines() {
    std::array<u_short, GRID_VERTICES_COUNT> edge_indexes{
            {
                    0, 1, 1, 7, 7, 4, 4, 0, // TOP
                    2, 3, 3, 5, 5, 6, 6, 2, // BOTTOM
                    0, 1, 1, 2, 2, 3, 3, 0, // RIGHT
                    4, 5, 5, 6, 6, 7, 7, 4  // LEFT
            }};

    for (int i = 0; i < GRID_VERTICES_COUNT; i++) {
        vertices[VERTICES_COUNT - GRID_VERTICES_COUNT + i] = {
                cubeEdges[edge_indexes[i]].x,
                cubeEdges[edge_indexes[i]].y,
                cubeEdges[edge_indexes[i]].z,
                outline_color[0],
                outline_color[1],
                outline_color[2]
        };
    }
}

static void build_cube() {
    const u_short vertices_per_side{6}, sides{6};
    std::array<vec3, sides> normal_vectors{{{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}}};
    std::array<std::array<u_short, vertices_per_side>, sides> side_vertex_indexes{{{0, 1, 2, 0, 3, 2},
                                                                                   {4, 5, 6, 4, 7, 6},
                                                                                   {0, 1, 7, 0, 4, 7},
                                                                                   {2, 3, 5, 2, 6, 5},
                                                                                   {0, 3, 5, 0, 4, 5},
                                                                                   {1, 2, 6, 1, 7, 6}}};
    // Füge jeden Vertex des Würfels in die Vertices-Liste ein und weise ihm den entsprechenden Normalenvektor zu.
    auto insert_index{0};
    for (auto i{0}; i < vertices_per_side; i++) {
        auto side_vertices_indexes{side_vertex_indexes[i]};
        auto normal_vector{normal_vectors[i]};
        for (auto j{0}; j < side_vertices_indexes.size(); j++) {
            auto vertex{cubeEdges[side_vertices_indexes[j]]};
            vertices[insert_index++] = Vertex{vertex, normal_vector};
        }
    }
}

int main() {
    build_cube();
    build_outlines();
    scale_cube();

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(800, 800, "Happy Cube", nullptr, nullptr);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);

    // Vertex-Buffer erstellen und mit Daten füllen. Der Vertex-Buffer ist ein
    // Speicherbereich auf der Grafikkarte, in dem die Vertex-Daten liegen.
    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Vertex-Shader erstellen und kompilieren. Der Vertex-Shader ist ein
    // Programm, das für jeden Vertex ausgeführt wird. Er bestimmt die
    // Position eines Vertex.
    char *vertex_shader_code = readTextFileIntoString(VERTEX_SHADER_PATH);
    if (vertex_shader_code == nullptr) exit(EXIT_FAILURE);
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_code, nullptr);
    glCompileShader(vertex_shader);

    // Fragment-Shader erstellen und kompilieren. Der Fragment-Shader ist ein
    // Programm, das für jeden Pixel ausgeführt wird. Er bestimmt die Farbe
    // eines Pixels.
    char *fragment_shader_code = readTextFileIntoString(FRAGMENT_SHADER_PATH);
    if (fragment_shader_code == nullptr) exit(EXIT_FAILURE);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_code, nullptr);
    glCompileShader(fragment_shader);

    // Shader-Programm erstellen und die Shader dem Programm zuweisen.
    // Das Shader-Programm wird auf der Grafikkarte ausgeführt.
    auto program{glCreateProgram()};
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    // IDs für die Shader-Parameter abfragen. Ermöglicht das Übertragen
    // von Daten an den Shader.
    auto matrix_access{glGetUniformLocation(program, "matrix")};

    auto ambient_access{glGetUniformLocation(program, "light_params.ambient_factor")};
    auto diffuse_access{glGetUniformLocation(program, "light_params.diffuse_factor")};
    auto spec_access{glGetUniformLocation(program, "light_params.specular_factor")};
    auto shininess_access{glGetUniformLocation(program, "light_params.shininess")};

    auto light_position_x_access{glGetUniformLocation(program, "light_params.x")};
    auto light_position_y_access{glGetUniformLocation(program, "light_params.y")};
    auto light_position_z_access{glGetUniformLocation(program, "light_params.z")};

    auto color_access{glGetAttribLocation(program, "color")};
    auto position_access{glGetAttribLocation(program, "position")};
    auto normal_vector_access{glGetAttribLocation(program, "normal_vector")};

    // Vertex-Array-Objekt erstellen und aktivieren. Das Vertex-Array-Objekt
    // speichert die Konfiguration des Vertex-Buffers. Es wird benötigt, um
    // die Vertex-Daten an den Shader zu übertragen. Z.B. wird hier festgelegt,
    // dass die Positionen der Vertices aus drei Floats bestehen.
    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    glEnableVertexAttribArray(position_access);
    glVertexAttribPointer(position_access, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void *) (offsetof(Vertex, x)));

    glEnableVertexAttribArray(color_access);
    glVertexAttribPointer(color_access, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void *) (offsetof(Vertex, r)));

    glEnableVertexAttribArray(normal_vector_access);
    glVertexAttribPointer(normal_vector_access, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void *) (offsetof(Vertex, nx)));

    glClearDepth(0);
    glDepthFunc(GL_GREATER);
    glEnable(GL_DEPTH_TEST);

    GLuint error;
    if ((error = glGetError()) != GL_NO_ERROR) {
        printf("OpenGL error: %d\n", error);
    }

    auto last_time{glfwGetTime()};

    while (!glfwWindowShouldClose(window)) {
        auto now = glfwGetTime();
        if (now - last_time >= .25) {
            std::cout << "-----------------"
                      << std::fixed << std::setprecision(3) << std::endl
                      << "Ambient:\t" << ambient_factor << std::endl
                      << "Diffuse:\t" << diffuse_factor << std::endl
                      << "Specular:\t" << spec_factor << std::endl
                      << "Shininess:\t" << shininess << std::endl;
            last_time = now;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mat4x4 m;
        mat4x4_identity(m);

        auto light_x{static_cast<float>(5 * M_PI * 30 / 360 * sin((float) glfwGetTime()))};
        auto light_y{static_cast<float>(4 * M_PI * 25 / 360 * cos((float) glfwGetTime()))};
        auto light_z{static_cast<float>(3 * M_PI * 15 / 360 * sin((float) glfwGetTime()))};

        auto rotate_x{static_cast<float>(3 * M_PI * 5 / 360 * sin((float) glfwGetTime()))};
        auto rotate_y{static_cast<float>(3 * M_PI * 5 / 360 * glfwGetTime())};
        mat4x4_rotate_X(m, m, rotate_x);
        mat4x4_rotate_Y(m, m, rotate_y);

        glUseProgram(program);
        glUniformMatrix4fv(matrix_access, 1, GL_FALSE, (const GLfloat *) m);

        glUniform1f(light_position_x_access, -light_x);
        glUniform1f(light_position_y_access, light_y);
        glUniform1f(light_position_z_access, light_z);

        glUniform1f(ambient_access, ambient_factor);
        glUniform1f(diffuse_access, diffuse_factor);
        glUniform1f(spec_access, spec_factor);
        glUniform1i(shininess_access, shininess);

        glDrawArrays(GL_TRIANGLES, 0, VERTICES_COUNT - GRID_VERTICES_COUNT);
        glDrawArrays(GL_LINES, VERTICES_COUNT - GRID_VERTICES_COUNT, GRID_VERTICES_COUNT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
