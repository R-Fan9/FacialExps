#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <obj.h>
#include <shader.h>
#include <sstream>
#include <string>
#include <vector>
#include <assert.h>

void dump_framebuffer_to_ppm(std::string prefix, uint32_t width,
                             uint32_t height);

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void process_input(GLFWwindow *window);

std::vector<Obj> load_face_objs(const std::string faces_path);
std::vector<tinyobj::real_t> get_weights(const char *file_path);
std::vector<tinyobj::real_t> blend_shape(Obj base_obj, std::vector<Obj> face_objs,
                                std::vector<tinyobj::real_t> weights);

static uint32_t ss_id = 0;

const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

int main()
{
  // initialize and configure
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
  if (window == NULL)
  {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // load all OpenGL function pointers
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // configure global OpenGL state
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // build and compile shader program
  Shader shader("shaders/shader.vs", "shaders/shader.fs");

  // load base and file objs
  Obj base_obj("data/faces/base.obj");
  std::vector<Obj> face_objs = load_face_objs("data/faces/");
  std::vector<tinyobj::real_t> weights = get_weights("data/weights/12.weights");

  // blend shpae
  std::vector<tinyobj::real_t> vbuffer = blend_shape(base_obj, face_objs, weights);
  // std::vector<tinyobj::real_t> vbuffer = face_objs[16].getVertices();
  std::vector<tinyobj::real_t> nbuffer = base_obj.getNormals();

  GLuint VAO, VBO_vertices, VBO_normals;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  // bind vertex array to vertex buffer
  glGenBuffers(1, &VBO_vertices);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_vertices);
  glBufferData(GL_ARRAY_BUFFER, vbuffer.size() * sizeof(tinyobj::real_t), &vbuffer[0],
               GL_STATIC_DRAW);

  // position attribute
  GLuint vertex_loc = shader.getAttribLocation("aPos");
  glVertexAttribPointer(vertex_loc, 3, GL_DOUBLE, GL_FALSE, 3 * sizeof(tinyobj::real_t),
                        (void *)0);
  glEnableVertexAttribArray(vertex_loc);

  // bind normal array to normal buffer
  glGenBuffers(1, &VBO_normals);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_normals);
  glBufferData(GL_ARRAY_BUFFER, nbuffer.size() * sizeof(tinyobj::real_t), &nbuffer[0],
               GL_STATIC_DRAW);

  // normal attribute
  GLuint normal_loc = shader.getAttribLocation("aNormal");
  glVertexAttribPointer(normal_loc, 3, GL_DOUBLE, GL_FALSE, 3 * sizeof(tinyobj::real_t),
                        (void *)0);
  glEnableVertexAttribArray(normal_loc);

  glm::mat4 model = glm::mat4(1.0f);
  glm::mat4 view = glm::lookAt(glm::vec3(30, 90, 80), glm::vec3(0, 90, 0),
                               glm::vec3(0, 1, 0));
  glm::mat4 proj =
      glm::perspective(glm::radians(60.0f), 4.0f / 3.0f, 0.1f, 1000.0f);

  // render loop
  while (!glfwWindowShouldClose(window))
  {
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
    glDrawArrays(GL_TRIANGLES, 0, vbuffer.size() / 3);

    // swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // terminate, clearing all previously allocated GLFW resources.
  glfwTerminate();
  return 0;
}

std::vector<Obj> load_face_objs(const std::string faces_path)
{
  const int num_of_faces = 35;

  std::vector<Obj> face_objs;
  for (int i = 0; i < num_of_faces; i++)
  {
    std::string file_name = faces_path + std::to_string(i) + ".obj";
    Obj obj(file_name);
    face_objs.push_back(obj);
  }

  return face_objs;
}

std::vector<tinyobj::real_t> get_weights(const char *file_path)
{
  std::vector<tinyobj::real_t> weights;

  std::ifstream weights_file(file_path);

  std::string line;
  while (std::getline(weights_file, line))
  {
    std::istringstream line_stream(line);
    tinyobj::real_t weight;

    while (line_stream >> weight)
    {
      weights.push_back(weight);
    }
  }

  weights_file.close();
  return weights;
}

std::vector<tinyobj::real_t> subtract_vertices(const std::vector<tinyobj::real_t> v1,
                                      const std::vector<tinyobj::real_t> v2)
{
  assert(v1.size() == v2.size());

  std::vector<tinyobj::real_t> vertices_diff(v1.size());
  for (int i = 0; i < v1.size(); i++)
  {
    vertices_diff[i] = v1[i] - v2[i];
  }

  return vertices_diff;
}

void add_vertices(std::vector<tinyobj::real_t> &v1, const std::vector<tinyobj::real_t> v2)
{
  assert(v1.size() == v2.size());

  for (int i = 0; i < v1.size(); i++)
  {
    v1[i] += v2[i];
  }
}

void scale_vertices(std::vector<tinyobj::real_t> &v, const tinyobj::real_t value)
{
  for (int i = 0; i < v.size(); i++)
  {
    v[i] *= value;
  }
}

std::vector<tinyobj::real_t> blend_shape(Obj base_obj, std::vector<Obj> face_objs,
                                std::vector<tinyobj::real_t> weights)
{
  std::vector<tinyobj::real_t> base_vertices = base_obj.getVertices();
  std::vector<tinyobj::real_t> result_vertices = base_vertices;
  for (size_t i = 0; i < face_objs.size(); i++)
  {
    std::vector<tinyobj::real_t> obj_vertices = face_objs[i].getVertices();

    assert(obj_vertices.size() == base_vertices.size());
    for (size_t j = 0; j < obj_vertices.size(); j++)
    {

      result_vertices[j] += weights[i] * (obj_vertices[j] - base_vertices[j]);
    }
  }

  return result_vertices;

  // std::vector<tinyobj::real_t> base_vertices = base_obj.getVertices();
  // std::vector<tinyobj::real_t> new_shape = base_vertices;

  // for (int i = 0; i < face_objs.size(); i++)
  // {
  //   std::vector<tinyobj::real_t> vertices_diff =
  //       subtract_vertices(face_objs[i].getVertices(), base_vertices);

  //   scale_vertices(vertices_diff, weights[i]);
  //   add_vertices(new_shape, vertices_diff);
  // }

  // return new_shape;
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
void process_input(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  // press p to capture screen
  if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
  {
    std::cout << "Capture Window " << ss_id << std::endl;
    int buffer_width, buffer_height;
    glfwGetFramebufferSize(window, &buffer_width, &buffer_height);
    dump_framebuffer_to_ppm("tmp", buffer_width, buffer_height);
  }
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}

void dump_framebuffer_to_ppm(std::string prefix, uint32_t width,
                             uint32_t height)
{
  int pixelChannel = 3;
  int totalPixelSize = pixelChannel * width * height * sizeof(GLubyte);
  GLubyte *pixels = new GLubyte[totalPixelSize];
  glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

  std::string fileName = prefix + std::to_string(ss_id) + ".ppm";
  std::filesystem::path filePath = std::filesystem::current_path() / fileName;
  std::ofstream fout(filePath.string());

  fout << "P3\n"
       << width << " " << height << "\n"
       << 255 << std::endl;
  for (size_t i = 0; i < height; i++)
  {
    for (size_t j = 0; j < width; j++)
    {
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
