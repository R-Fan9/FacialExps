#ifndef OBJ_H
#define OBJ_H

#define TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#define TINYOBJLOADER_USE_DOUBLE

#include <mapbox/earcut.hpp>
#include <tiny_obj_loader.h>

class Obj
{
public:
  // constructor generates Obj class on the fly
  Obj(const std::string &file_path) : obj_path(file_path)
  {
    tinyobj::ObjReaderConfig config;
    config.triangulate = false;
    config.triangulation_method = "simple";

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(obj_path, config))
    {
      if (!reader.Error().empty())
      {
        std::cerr << "TinyObjReader: " << reader.Error();
      }
      exit(1);
    }

    if (!reader.Warning().empty())
    {
      std::cout << "TinyObjReader: " << reader.Warning();
    }

    attrib = reader.GetAttrib();
    shapes = reader.GetShapes();
    materials = reader.GetMaterials();

    // std::string warn, err;

    // bool bTriangulate = true;
    // bool bSuc = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
    //                              obj_path.c_str(), nullptr, bTriangulate);

    // if (!bSuc)
    // {
    //   throw std::runtime_error("tinyobj error:" + err);
    // }
  }

  std::vector<tinyobj::real_t> getVertices()
  {
    std::vector<tinyobj::real_t> vbuffer;
    // Loop over shapes
    // for (size_t s = 0; s < shapes.size(); s++)
    // {
    //   // Loop over faces(polygon)
    //   size_t index_offset = 0;
    //   for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
    //   {
    //     size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

    //     // Loop over vertices in the face.
    //     for (size_t v = 0; v < fv; v++)
    //     {
    //       // access to vertex
    //       tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

    //       tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
    //       tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
    //       tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

    //       vbuffer.push_back(vx);
    //       vbuffer.push_back(vy);
    //       vbuffer.push_back(vz);
    //     }
    //     index_offset += fv;

    //     // per-face material
    //     shapes[s].mesh.material_ids[f];
    //   }
    // }
    for (auto shape : shapes)
    {
      for (auto face : shape.mesh.indices)
      {
        int vid = face.vertex_index;

        vbuffer.push_back(attrib.vertices[vid * 3]);
        vbuffer.push_back(attrib.vertices[vid * 3 + 1]);
        vbuffer.push_back(attrib.vertices[vid * 3 + 2]);
      }
    }

    return vbuffer;
  }

  std::vector<tinyobj::real_t> getNormals()
  {
    std::vector<tinyobj::real_t> nbuffer;

    for (auto shape : shapes)
    {
      for (auto face : shape.mesh.indices)
      {
        int nid = face.normal_index;

        nbuffer.push_back(attrib.normals[nid * 3]);
        nbuffer.push_back(attrib.normals[nid * 3 + 1]);
        nbuffer.push_back(attrib.normals[nid * 3 + 2]);
      }
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
