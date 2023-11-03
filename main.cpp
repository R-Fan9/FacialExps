#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <shader.h>

static uint32_t ss_id = 0;
const int SCR_WIDTH = 1024;
const int SCR_HEIGHT = 768;
const float HOURS_PER_DAY = 24;
const float SUN_EARTH_DISTANCE = 24.0f;
const float EARTH_MOON_DISTANCE = 12.0f;
const float REVOLVE_DEGREES = 360.0f;
const float SUN_REVOLVE_DAYS = 27.0f;
const float EARTH_REVOLVE_DAYS = 1.0f;
const float EARTH_ORBIT_DAYS = 365.0f;
const float MOON_REVOLVE_DAYS = 28.0f;
const float MOON_ORBIT_DAYS = 28.0f;
const double FRAME_RATE = 60.0;

double prev_time = 0.0f;
double delta_time = 0.0f;

void dump_framebuffer_to_ppm(std::string prefix, uint32_t width, uint32_t height);

void process_input(GLFWwindow *window);

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

float get_sun_rotate_angle_around_itself(float day);

float get_earth_rotate_angle_around_sun(float day);

float get_earth_rotate_angle_around_itself(float day);

float get_moon_rotate_angle_around_earth(float day);

float get_moon_rotate_angle_around_itself(float day);

void get_position_from_angle(float angle, float radius, float &adj_pos, float &opp_pos);

bool should_render();

glm::vec3 draw_sun(float day, Shader *shader);

glm::vec3 draw_earth(float day, glm::vec3 orbit_center, Shader *shader);

glm::vec3 draw_moon(float day, glm::vec3 orbit_center, Shader *shader);

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Solar System", NULL, NULL);

    if (window == NULL) {
        std::cout << "GLFW Window Failed" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "GLAD Initialization Failed" << std::endl;
        return -1;
    }

    // configure global openGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LESS);
//    glEnable(GL_CULL_FACE);
//    glCullFace(GL_BACK);

    // build and compile shader program
    Shader shader("shaders/shader.vs", "shaders/shader.fs");

    // cube vertices
    float vertices[] = {
            // back face, yellow
            -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f,
            1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f,

            // front face, purple
            -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
            -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f,

            // right face, green
            1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
            1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

            // left face, red
            -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
            -1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
            -1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
            -1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
            -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
            -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,

            // bottom face, light blue
            -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f,

            // top face, blue
            -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
    };

    uint32_t VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // bind Vertex Array Object
    glBindVertexArray(VAO);

    // bind vertices array to a vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    float day = 0.0f, inc = 1.0f / HOURS_PER_DAY;

    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 proj = glm::mat4(1.0f);
    proj = glm::perspective(glm::radians(30.0f), (float) 4 / (float) 3, 0.1f, 1000.0f);

    while (!glfwWindowShouldClose(window)) {
        process_input(window);

        if (should_render()) {
            // background color
            glClearColor(0.3f, 0.4f, 0.5f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // activate shader
            shader.use();
            shader.setMat4("view", view);
            shader.setMat4("projection", proj);

            // render container
            glBindVertexArray(VAO);

            glm::vec3 sun_pos = draw_sun(day, &shader);
            glm::vec3 earth_pos = draw_earth(day, sun_pos, &shader);
            glm::vec3 moon_pos = draw_moon(day, earth_pos, &shader);

            view = glm::lookAt(glm::vec3(100.0f, 50.0f, 100.0f), sun_pos, glm::vec3(0.0f, 1.0f, 0.0f));

            day += inc;

            glfwSwapBuffers(window);
        }

        glfwPollEvents();
    }

    //release resource
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}

glm::vec3 draw_sun(float day, Shader *shader) {
    glm::mat4 model = glm::mat4(1.0f);
    float sun_x = 0.0f, sun_y = 0.0f, sun_z = 0.0f;
    glm::mat4 sun = glm::scale(model, glm::vec3(6.0f, 6.0f, 6.0f));
    sun = glm::rotate(sun,
                      (float) glm::radians(get_sun_rotate_angle_around_itself(day)),
                      glm::vec3(0.0f, 1.0f, 0.0f));
    shader->setMat4("model", sun);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    return {sun_x, sun_y, sun_z};
}

glm::vec3 draw_earth(float day, glm::vec3 orbit_center, Shader *shader) {
    glm::mat4 model = glm::mat4(1.0f);
    float earth_x = 0.0f, earth_y = 0.0f, earth_z = 0.0f;
    float earth_orbit_degree = get_earth_rotate_angle_around_sun(day);
    get_position_from_angle(earth_orbit_degree, SUN_EARTH_DISTANCE, earth_x, earth_z);

    glm::mat4 earth = glm::translate(
            model,
            glm::vec3(orbit_center[0] + earth_x, orbit_center[1] + earth_y, orbit_center[2] + earth_z));
    earth = glm::scale(earth, glm::vec3(3.0f, 3.0f, 3.0f));
    earth = glm::rotate(earth, (float) glm::radians(-23.4), glm::vec3(0.0f, 0.0f, 1.0f));
    earth = glm::rotate(earth,
                        (float) glm::radians(get_earth_rotate_angle_around_itself(day)),
                        glm::vec3(0.0f, 1.0f, 0.0f));

    shader->setMat4("model", earth);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    return {earth_x, earth_y, earth_z};

}

glm::vec3 draw_moon(float day, glm::vec3 orbit_center, Shader *shader) {
    glm::mat4 model = glm::mat4(1.0f);
    float moon_x = 0.0f, moon_y = 0.0f, moon_z = 0.0f;
    float moon_orbit_degree = get_moon_rotate_angle_around_earth(day);
    get_position_from_angle(moon_orbit_degree, EARTH_MOON_DISTANCE, moon_x, moon_z);

    glm::mat4 moon = glm::scale(moon, glm::vec3(1.5f, 1.5f, 1.5f));
    moon = glm::translate(
            model,
            glm::vec3(orbit_center[0] + moon_x, orbit_center[1] + moon_y, orbit_center[2] + moon_z));
    moon = glm::rotate(moon,
                       (float) glm::radians(get_moon_rotate_angle_around_itself(day)),
                       glm::vec3(0.0f, 1.0f, 0.0f));

    shader->setMat4("model", moon);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    return {moon_x, moon_y, moon_z};
}

void get_position_from_angle(float angle, float radius, float &adj_pos, float &opp_pos) {
    adj_pos = radius * (float) cos(glm::radians(angle));
    opp_pos = -radius * (float) sin(glm::radians(angle));
}

float get_sun_rotate_angle_around_itself(float day) {
    float revolveDegPerDay = REVOLVE_DEGREES / SUN_REVOLVE_DAYS;
    return day * revolveDegPerDay;
}

float get_earth_rotate_angle_around_sun(float day) {
    float revolveDegPerDay = REVOLVE_DEGREES / EARTH_ORBIT_DAYS;
    return day * revolveDegPerDay;
}

float get_earth_rotate_angle_around_itself(float day) {
    float revolveDegPerDay = REVOLVE_DEGREES / EARTH_REVOLVE_DAYS;
    return day * revolveDegPerDay;
}

float get_moon_rotate_angle_around_earth(float day) {
    float revolveDegPerDay = REVOLVE_DEGREES / MOON_ORBIT_DAYS;
    return day * revolveDegPerDay;
}

float get_moon_rotate_angle_around_itself(float day) {
    float revolveDegPerDay = REVOLVE_DEGREES / MOON_REVOLVE_DAYS;
    return day * revolveDegPerDay;
}

bool should_render() {
    double cur_time = glfwGetTime();
    delta_time += (cur_time - prev_time);
    prev_time = cur_time;

    // check if the time elapsed is greater than or equal the frame rate
    if (delta_time >= 1.0 / FRAME_RATE) {
        delta_time -= 1.0 / FRAME_RATE;
        return true;
    }
    return false;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

void process_input(GLFWwindow *window) {
    //press escape to exit
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    //press p to capture screen
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        std::cout << "Capture Window " << ss_id << std::endl;
        int buffer_width, buffer_height;
        glfwGetFramebufferSize(window, &buffer_width, &buffer_height);
        dump_framebuffer_to_ppm("Assignment0-ss", buffer_width, buffer_height);
    }
}

void dump_framebuffer_to_ppm(std::string prefix, uint32_t width, uint32_t height) {
    int pixelChannel = 3;
    int totalPixelSize = pixelChannel * width * height * sizeof(GLubyte);
    GLubyte *pixels = new GLubyte[totalPixelSize];
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    std::string fileName = prefix + std::to_string(ss_id) + ".ppm";
    std::filesystem::path filePath = std::filesystem::current_path() / fileName;
    std::ofstream fout(filePath.string());

    fout << "P3\n" << width << " " << height << "\n" << 255 << std::endl;
    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width; j++) {
            size_t cur = pixelChannel * ((height - i - 1) * width + j);
            fout << (int) pixels[cur] << " " << (int) pixels[cur + 1] << " " << (int) pixels[cur + 2] << " ";
        }
        fout << std::endl;
    }

    ss_id++;

    delete[] pixels;
    fout.flush();
    fout.close();
}
