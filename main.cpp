#define TINYOBJLOADER_IMPLEMENTATION

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

#include <filesystem>
#include <iostream>
#include <shader.h>
#include <tiny_obj_loader.h>
#include <vector>

void dump_framebuffer_to_ppm(std::string prefix, uint32_t width,
                             uint32_t height);

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void process_input(GLFWwindow *window);

bool parse_obj_file(const char *obj_path, std::vector<float> *vbuffer,
                    std::vector<float> *nbuffer);

static uint32_t ss_id = 0;

const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

int main() {
  // glfw: initialize and configure
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // glfw window creation
  GLFWwindow *window =
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Facial Expressions", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // glad: load all OpenGL function pointers
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // configure global OpenGL state
  glEnable(GL_DEPTH_TEST);
  // glEnable(GL_CULL_FACE);
  // glCullFace(GL_BACK);

  // build and compile shader program
  Shader shader("shaders/shader.vs", "shaders/shader.fs");

  const char *obj_path = "data/cube.obj";

  std::vector<float> vbuffer;
  std::vector<float> nbuffer;

  bool parsed = parse_obj_file(obj_path, &vbuffer, &nbuffer);
  if (!parsed) {
    return -1;
  }

  GLuint VAO, VBO_vertices, VBO_normals;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  // bind vertex array to vertex buffer
  glGenBuffers(1, &VBO_vertices);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_vertices);
  glBufferData(GL_ARRAY_BUFFER, vbuffer.size() * sizeof(float), vbuffer.data(),
               GL_STATIC_DRAW);

  // position attribute
  GLuint vertex_loc = shader.getAttribLocation("aPos");
  glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                        (void *)0);
  glEnableVertexAttribArray(vertex_loc);

  // bind normal array to normal buffer
  glGenBuffers(1, &VBO_normals);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_normals);
  glBufferData(GL_ARRAY_BUFFER, nbuffer.size() * sizeof(float), nbuffer.data(),
               GL_STATIC_DRAW);

  // normal attribute
  GLuint normal_loc = shader.getAttribLocation("aNormal");
  glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                        (void *)0);
  glEnableVertexAttribArray(normal_loc);

  glm::mat4 model = glm::mat4(1.0f);
  glm::mat4 view =
      glm::lookAt(glm::vec3(-2, 6, 5), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
  glm::mat4 proj =
      glm::perspective(glm::radians(60.0f), 4.0f / 3.0f, 0.1f, 1000.0f);

  // render loop
  while (!glfwWindowShouldClose(window)) {
    process_input(window);

    // background color
    glClearColor(0.3f, 0.4f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // activate shader
    shader.use();
    shader.setMat4("model", model);
    shader.setMat4("view", view);
    shader.setMat4("projection", proj);

    // render container
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved
    // etc.)
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // glfw: terminate, clearing all previously allocated GLFW resources.
  glfwTerminate();
  return 0;
}

bool parse_obj_file(const char *obj_path, std::vector<float> *vbuffer,
                    std::vector<float> *nbuffer) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn, err;

  bool bTriangulate = true;
  bool bSuc = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                               obj_path, nullptr, bTriangulate);

  if (!bSuc) {
    std::cout << "tinyobj error:" << err.c_str() << std::endl;
    return false;
  }

  for (auto face : shapes[0].mesh.indices) {
    int vid = face.vertex_index;
    int nid = face.normal_index;

    vbuffer->push_back(attrib.vertices[vid * 3]);
    vbuffer->push_back(attrib.vertices[vid * 3 + 1]);
    vbuffer->push_back(attrib.vertices[vid * 3 + 2]);

    nbuffer->push_back(attrib.normals[nid * 3]);
    nbuffer->push_back(attrib.normals[nid * 3 + 1]);
    nbuffer->push_back(attrib.normals[nid * 3 + 2]);
  }

  return true;
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
void process_input(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  // press p to capture screen
  if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
    std::cout << "Capture Window " << ss_id << std::endl;
    int buffer_width, buffer_height;
    glfwGetFramebufferSize(window, &buffer_width, &buffer_height);
    dump_framebuffer_to_ppm("tmp", buffer_width, buffer_height);
  }
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}

void dump_framebuffer_to_ppm(std::string prefix, uint32_t width,
                             uint32_t height) {
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
      fout << (int)pixels[cur] << " " << (int)pixels[cur + 1] << " "
           << (int)pixels[cur + 2] << " ";
    }
    fout << std::endl;
  }

  ss_id++;

  delete[] pixels;
  fout.flush();
  fout.close();
}
