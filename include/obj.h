#ifndef OBJ_H
#define OBJ_H

#include <tiny_obj_loader.h>

class Obj {
public:
  // constructor generates Obj class on the fly
  Obj(const char *file_path) : obj_path(file_path) {
    std::string warn, err;

    bool bTriangulate = true;
    bool bSuc = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                                 file_path, nullptr, bTriangulate);

    if (!bSuc) {
      throw std::runtime_error("tinyobj error:" + err);
    }
  }

  std::vector<float> getVertices() {
    std::vector<float> vbuffer;

    for (auto face : shapes[0].mesh.indices) {
      int vid = face.vertex_index;

      vbuffer.push_back(attrib.vertices[vid * 3]);
      vbuffer.push_back(attrib.vertices[vid * 3 + 1]);
      vbuffer.push_back(attrib.vertices[vid * 3 + 2]);
    }

    return vbuffer;
  }

  std::vector<float> getNormals() {
    std::vector<float> nbuffer;

    for (auto face : shapes[0].mesh.indices) {
      int vid = face.normal_index;

      nbuffer.push_back(attrib.normals[vid * 3]);
      nbuffer.push_back(attrib.normals[vid * 3 + 1]);
      nbuffer.push_back(attrib.normals[vid * 3 + 2]);
    }

    return nbuffer;
  }

private:
  std::string obj_path;
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
};

#endif // !OBJ_H
