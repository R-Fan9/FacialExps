#define TINYOBJLOADER_IMPLEMENTATION

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <shader.h>
#include <tiny_obj_loader.h>
#include <vector>

void framebufferSizeCallback(GLFWwindow *window, int width, int height);

void processInput(GLFWwindow *window);

bool parseObjFile(const char *obj_path, std::vector<float> *vbuffer,
                  std::vector<float> *nbuffer);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

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
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

  // glad: load all OpenGL function pointers
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // configure global OpenGL state
  glEnable(GL_DEPTH_TEST);

  // build and compile shader program
  Shader shader("shaders/shader.vs", "shaders/shader.fs");

  const char* obj_path = "/home/rfan/code/OpenGL/facialExps/data/cube.obj";

  std::vector<float> vbuffer;
  std::vector<float> nbuffer;

  bool parsed = parseObjFile(obj_path, &vbuffer, &nbuffer);
  if(!parsed){
    std::cout << "Failed to parse OBJ file" << std::endl;
    return -1;
  }


  // render loop
  while (!glfwWindowShouldClose(window)) {
    processInput(window);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved
    // etc.)
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // glfw: terminate, clearing all previously allocated GLFW resources.
  glfwTerminate();
  return 0;
}

bool parseObjFile(const char *obj_path, std::vector<float> *vbuffer,
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
void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}
