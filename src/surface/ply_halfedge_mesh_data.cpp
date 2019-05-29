#include "geometrycentral/surface/ply_halfedge_mesh_data.h"

#include "geometrycentral/surface/meshio.h"
#include "geometrycentral/surface/polygon_soup_mesh.h"

#include <cstring>
#include <fstream>
#include <iostream>


using std::cerr;
using std::cout;
using std::endl;

namespace geometrycentral {
namespace surface {


PlyHalfedgeMeshData::PlyHalfedgeMeshData(HalfedgeMesh& mesh_, std::string filename_, bool verbose_)
    : mesh(mesh_), plyData(filename_, verbose_), verbose(verbose_) {}

PlyHalfedgeMeshData::PlyHalfedgeMeshData(HalfedgeMesh& mesh_, bool verbose_)
    : mesh(mesh_), plyData(), verbose(verbose_) {
  // Write connectiviy as indices
  std::vector<std::vector<size_t>> faceIndices = mesh.getFaceVertexList();
  plyData.addFaceIndices(faceIndices);
}

std::tuple<std::unique_ptr<HalfedgeMesh>, std::unique_ptr<PlyHalfedgeMeshData>>
PlyHalfedgeMeshData::loadMeshAndData(std::string filename, bool verbose) {

  // First, just load the connectivity
  std::unique_ptr<HalfedgeMesh> mesh = loadConnectivity(filename, verbose, "ply");

  // Now, open file on that mesh
  std::unique_ptr<PlyHalfedgeMeshData> data(new PlyHalfedgeMeshData(*mesh, filename, verbose));

  return std::make_tuple(std::move(mesh), std::move(data));
}

void PlyHalfedgeMeshData::addGeometry(const Geometry<Euclidean>& geometry) {

  // separate x/y/z coordinates
  VertexData<double> x = getVertexProperty<double>("x");
  VertexData<double> y = getVertexProperty<double>("y");
  VertexData<double> z = getVertexProperty<double>("z");

  for (Vertex v : mesh.vertices()) {
    Vector3 p = geometry[v];
    x[v] = p.x;
    y[v] = p.y;
    z[v] = p.z;
  }

  addVertexProperty("x", x);
  addVertexProperty("y", y);
  addVertexProperty("z", z);
}


std::unique_ptr<Geometry<Euclidean>> PlyHalfedgeMeshData::getGeometry() {

  // Get the vertex positions as a float or double
  std::unique_ptr<Geometry<Euclidean>> geom(new Geometry<Euclidean>(mesh));

  // Get x/y/z coordinates
  VertexData<double> x = getVertexProperty<double>("x");
  VertexData<double> y = getVertexProperty<double>("y");
  VertexData<double> z = getVertexProperty<double>("z");

  for (Vertex v : mesh.vertices()) {
    (*geom)[v] = Vector3{x[v], y[v], z[v]};
  }

  return geom;
}


VertexData<Vector3> PlyHalfedgeMeshData::getVertexColors() {

  VertexData<Vector3> color(mesh);

  try {
    // Try uchar first
    VertexData<unsigned char> r = getVertexProperty<unsigned char>("red");
    VertexData<unsigned char> g = getVertexProperty<unsigned char>("green");
    VertexData<unsigned char> b = getVertexProperty<unsigned char>("blue");
    for (Vertex v : mesh.vertices()) {
      color[v][0] = r[v] / 255.0;
      color[v][1] = g[v] / 255.0;
      color[v][2] = b[v] / 255.0;
    }
    return color;

  } catch (std::runtime_error orig_e) {

    // If that doesn't work, try float
    try {
      VertexData<double> r = getVertexProperty<double>("red");
      VertexData<double> g = getVertexProperty<double>("green");
      VertexData<double> b = getVertexProperty<double>("blue");
      for (Vertex v : mesh.vertices()) {
        color[v][0] = r[v];
        color[v][1] = g[v];
        color[v][2] = b[v];
      }
      return color;
    } catch (std::runtime_error second_e) {
      throw std::runtime_error("Could not find vertex colors in PLY file, as uchar or float");
    }
  }
}

void PlyHalfedgeMeshData::write(std::string filename) { plyData.write(filename, outputFormat); }

} // namespace surface
} // namespace geometrycentral
