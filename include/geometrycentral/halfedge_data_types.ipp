#pragma once

#include "halfedge_data_macros.h"

// === Implementations for datatypes which hold data stored on the mesh

namespace geometrycentral {

// Data on vertices
template <typename T>
VertexData<T>::VertexData(HalfedgeMesh* parentMesh) : mesh(parentMesh) {
  if (parentMesh != nullptr) {
    data.resize(parentMesh->nVertices());
  }
}

template <typename T>
VertexData<T>::VertexData(HalfedgeMesh* parentMesh, T initVal)
    : VertexData(parentMesh) {
  fill(initVal);
}

template <typename T>
VertexData<T>::VertexData(HalfedgeMesh* parentMesh,
                          const Eigen::Matrix<T, Eigen::Dynamic, 1>& vector)
    : VertexData(parentMesh) {
  fromVector(vector);
}

template <typename T>
VertexData<T>::VertexData(HalfedgeMesh* parentMesh,
                          const Eigen::Matrix<T, Eigen::Dynamic, 1>& vector,
                          const VertexData<size_t>& indexer)
    : VertexData(parentMesh) {
  fromVector(vector, indexer);
}

template <typename T>
void VertexData<T>::fill(T val) {
  std::fill(data.begin(), data.end(), val);
}

template <typename T>
Eigen::Matrix<T, Eigen::Dynamic, 1> VertexData<T>::toVector() const {
  Eigen::Matrix<T, Eigen::Dynamic, 1> result(mesh->nVertices());
  VertexData<size_t> ind = mesh->getVertexIndices();
  for (VertexPtr v : mesh->vertices()) {
    result(ind[v]) = (*this)[v];
  }
  return result;
}

template <typename T>
Eigen::Matrix<T, Eigen::Dynamic, 1> VertexData<T>::toVector(
    const VertexData<size_t>& indexer) const {
  size_t outSize = 0;
  for (VertexPtr v : mesh->vertices())
    if (indexer[v] != std::numeric_limits<size_t>::max()) outSize++;
  Eigen::Matrix<T, Eigen::Dynamic, 1> result(outSize);
  for (VertexPtr v : mesh->vertices()) {
    if (indexer[v] != std::numeric_limits<size_t>::max()) {
      result(indexer[v]) = (*this)[v];
    }
  }
  return result;
}

template <typename T>
void VertexData<T>::fromVector(const Eigen::Matrix<T, Eigen::Dynamic, 1>& vector) {
  if ((size_t)vector.rows() != mesh->nVertices())
    throw std::runtime_error("Vector size does not match mesh size.");
  VertexData<size_t> ind = mesh->getVertexIndices();
  for (VertexPtr v : mesh->vertices()) {
    (*this)[v] = vector(ind[v]);
  }
}

template <typename T>
void VertexData<T>::fromVector(const Eigen::Matrix<T, Eigen::Dynamic, 1>& vector,
                               const VertexData<size_t>& indexer) {
  for (VertexPtr v : mesh->vertices()) {
    if (indexer[v] != std::numeric_limits<size_t>::max()) {
      (*this)[v] = vector(indexer[v]);
    }
  }
}

template <typename T>
inline T& VertexData<T>::operator[](VertexPtr v) {
#ifndef NDEBUG
  assert(v->parentMesh == mesh &&
         "Attempted to access data with member from wrong mesh");
#endif
  size_t i = v - mesh->vertex(0);
  return data[i];
}

template <typename T>
inline const T& VertexData<T>::operator[](VertexPtr v) const {
#ifndef NDEBUG
  assert(v->parentMesh == mesh &&
         "Attempted access data with member from wrong mesh");
#endif
  size_t i = v - mesh->vertex(0);
  return data[i];
}

template <typename T>
inline T& VertexData<T>::operator[](size_t v) {
#ifndef NDEBUG
  assert(v < size() && "Attempted to access data with out of bounds index");
#endif
  return data[v];
}

template <typename T>
inline const T& VertexData<T>::operator[](size_t v) const {
#ifndef NDEBUG
  assert(v < size() && "Attempted to access data with out of bounds index");
#endif
  return data[v];
}

template <typename T>
inline size_t VertexData<T>::size() const {
  return data.size();
}

GC_INTERNAL_GENERATE_DATATYPE_OPERATOR_DEFINITIONS(VertexData, mesh)

// Data on edges
template <typename T>
EdgeData<T>::EdgeData(HalfedgeMesh* parentMesh) : mesh(parentMesh) {
  if (parentMesh != nullptr) {
    data.resize(parentMesh->nEdges());
  }
}

template <typename T>
EdgeData<T>::EdgeData(HalfedgeMesh* parentMesh, T initVal)
    : EdgeData(parentMesh) {
  fill(initVal);
}

template <typename T>
EdgeData<T>::EdgeData(HalfedgeMesh* parentMesh,
                      const Eigen::Matrix<T, Eigen::Dynamic, 1>& vector)
    : EdgeData(parentMesh) {
  fromVector(vector);
}

template <typename T>
EdgeData<T>::EdgeData(HalfedgeMesh* parentMesh,
                      const Eigen::Matrix<T, Eigen::Dynamic, 1>& vector,
                      const EdgeData<size_t>& indexer)
    : EdgeData(parentMesh) {
  fromVector(vector, indexer);
}

template <typename T>
void EdgeData<T>::fill(T val) {
  std::fill(data.begin(), data.end(), val);
}

template <typename T>
Eigen::Matrix<T, Eigen::Dynamic, 1> EdgeData<T>::toVector() const {
  Eigen::Matrix<T, Eigen::Dynamic, 1> result(mesh->nEdges());
  EdgeData<size_t> ind = mesh->getEdgeIndices();
  for (EdgePtr e : mesh->edges()) {
    result(ind[e]) = (*this)[e];
  }
  return result;
}

template <typename T>
Eigen::Matrix<T, Eigen::Dynamic, 1> EdgeData<T>::toVector(
    const EdgeData<size_t>& indexer) const {
  size_t outSize = 0;
  for (EdgePtr e : mesh->edges())
    if (indexer[e] != std::numeric_limits<size_t>::max()) outSize++;
  Eigen::Matrix<T, Eigen::Dynamic, 1> result(outSize);
  for (EdgePtr e : mesh->edges()) {
    if (indexer[e] != std::numeric_limits<size_t>::max()) {
      result(indexer[e]) = (*this)[e];
    }
  }
  return result;
}

template <typename T>
void EdgeData<T>::fromVector(const Eigen::Matrix<T, Eigen::Dynamic, 1>& vector) {
  if ((size_t)vector.rows() != mesh->nEdges())
    throw std::runtime_error("Vector size does not match mesh size.");
  EdgeData<size_t> ind = mesh->getEdgeIndices();
  for (EdgePtr e : mesh->edges()) {
    (*this)[e] = vector(ind[e]);
  }
}

template <typename T>
void EdgeData<T>::fromVector(const Eigen::Matrix<T, Eigen::Dynamic, 1>& vector,
                             const EdgeData<size_t>& indexer) {
  for (EdgePtr e : mesh->edges()) {
    if (indexer[e] != std::numeric_limits<size_t>::max()) {
      (*this)[e] = vector(indexer[e]);
    }
  }
}

template <typename T>
inline T& EdgeData<T>::operator[](EdgePtr e) {
#ifndef NDEBUG
  assert(e->parentMesh == mesh &&
         "Attempted access data with member from wrong mesh");
#endif
  size_t i = e - mesh->edge(0);
  return data[i];
}

template <typename T>
inline const T& EdgeData<T>::operator[](EdgePtr e) const {
#ifndef NDEBUG
  assert(e->parentMesh == mesh &&
         "Attempted access data with member from wrong mesh");
#endif
  size_t i = e - mesh->edge(0);
  return data[i];
}

template <typename T>
inline T& EdgeData<T>::operator[](size_t e) {
#ifndef NDEBUG
  assert(e < size() && "Attempted to access data with out of bounds index");
#endif
  return data[e];
}

template <typename T>
inline const T& EdgeData<T>::operator[](size_t e) const {
#ifndef NDEBUG
  assert(e < size() && "Attempted to access data with out of bounds index");
#endif
  return data[e];
}

template <typename T>
inline size_t EdgeData<T>::size() const {
  return data.size();
}


GC_INTERNAL_GENERATE_DATATYPE_OPERATOR_DEFINITIONS(EdgeData, mesh)

// Data on (real) faces
template <typename T>
FaceData<T>::FaceData(HalfedgeMesh* parentMesh) : mesh(parentMesh) {
  if (parentMesh != nullptr) {
    realSize = parentMesh->nFaces();
    size_t imaginarySize = parentMesh->nBoundaryLoops();
    data.resize(realSize + imaginarySize);
  }
}

template <typename T>
FaceData<T>::FaceData(HalfedgeMesh* parentMesh, T initVal)
    : FaceData(parentMesh) {
  fill(initVal);
}

template <typename T>
FaceData<T>::FaceData(HalfedgeMesh* parentMesh,
                      const Eigen::Matrix<T, Eigen::Dynamic, 1>& vector)
    : FaceData(parentMesh) {
  fromVector(vector);
}

template <typename T>
FaceData<T>::FaceData(HalfedgeMesh* parentMesh,
                      const Eigen::Matrix<T, Eigen::Dynamic, 1>& vector,
                      const FaceData<size_t>& indexer)
    : FaceData(parentMesh) {
  fromVector(vector, indexer);
}

template <typename T>
void FaceData<T>::fill(T val) {
  std::fill(data.begin(), data.end(), val);
}

template <typename T>
Eigen::Matrix<T, Eigen::Dynamic, 1> FaceData<T>::toVector() const {
  Eigen::Matrix<T, Eigen::Dynamic, 1> result(mesh->nFaces());
  FaceData<size_t> ind = mesh->getFaceIndices();
  for (FacePtr f : mesh->faces()) {
    result(ind[f]) = (*this)[f];
  }
  return result;
}

template <typename T>
Eigen::Matrix<T, Eigen::Dynamic, 1> FaceData<T>::toVector(
    const FaceData<size_t>& indexer) const {
  size_t outSize = 0;
  for (FacePtr f : mesh->faces())
    if (indexer[f] != std::numeric_limits<size_t>::max()) outSize++;
  Eigen::Matrix<T, Eigen::Dynamic, 1> result(outSize);
  for (FacePtr f : mesh->faces()) {
    if (indexer[f] != std::numeric_limits<size_t>::max()) {
      result(indexer[f]) = (*this)[f];
    }
  }
  return result;
}

template <typename T>
void FaceData<T>::fromVector(const Eigen::Matrix<T, Eigen::Dynamic, 1>& vector) {
  if ((size_t)vector.rows() != mesh->nFaces())
    throw std::runtime_error("Vector size does not match mesh size.");
  FaceData<size_t> ind = mesh->getFaceIndices();
  for (FacePtr f : mesh->faces()) {
    (*this)[f] = vector(ind[f]);
  }
}

template <typename T>
void FaceData<T>::fromVector(const Eigen::Matrix<T, Eigen::Dynamic, 1>& vector,
                             const FaceData<size_t>& indexer) {
  for (FacePtr f : mesh->faces()) {
    if (indexer[f] != std::numeric_limits<size_t>::max()) {
      (*this)[f] = vector(indexer[f]);
    }
  }
}

template <typename T>
inline T& FaceData<T>::operator[](FacePtr f) {
#ifndef NDEBUG
  assert(f->parentMesh == mesh &&
         "Attempted access data with member from wrong mesh");
#endif
  if (f.isReal()) {
    size_t i = f - mesh->face(0);
    return data[i];
  } else {
    size_t i = f - mesh->boundaryLoop(0);
    return data[realSize + i];
  }
}

template <typename T>
inline const T& FaceData<T>::operator[](FacePtr f) const {
#ifndef NDEBUG
  assert(f->parentMesh == mesh &&
         "Attempted access data with member from wrong mesh");
#endif
  if (f.isReal()) {
    size_t i = f - mesh->face(0);
    return data[i];
  } else {
    size_t i = f - mesh->boundaryLoop(0);
    return data[realSize + i];
  }
}

template <typename T>
inline T& FaceData<T>::operator[](size_t f) {
#ifndef NDEBUG
  assert(f < size() && "Attempted to access data with out of bounds index");
#endif
  return data[f];
}

template <typename T>
inline const T& FaceData<T>::operator[](size_t f) const {
#ifndef NDEBUG
  assert(f < size() && "Attempted to access data with out of bounds index");
#endif
  return data[f];
}

template <typename T>
inline size_t FaceData<T>::size() const {
  return data.size();
}

GC_INTERNAL_GENERATE_DATATYPE_OPERATOR_DEFINITIONS(FaceData, mesh)

// Data on (real and imaginary) halfedges
template <typename T>
HalfedgeData<T>::HalfedgeData(HalfedgeMesh* parentMesh) : mesh(parentMesh) {
  if (parentMesh != nullptr) {
    realSize = parentMesh->nHalfedges();
    size_t imaginarySize = parentMesh->nImaginaryHalfedges();
    data.resize(realSize + imaginarySize);
  }
}

template <typename T>
HalfedgeData<T>::HalfedgeData(HalfedgeMesh* parentMesh, T initVal)
    : HalfedgeData(parentMesh) {
  fill(initVal);
}

template <typename T>
HalfedgeData<T>::HalfedgeData(HalfedgeMesh* parentMesh,
                              const Eigen::Matrix<T, Eigen::Dynamic, 1>& vector)
    : HalfedgeData(parentMesh) {
  fromVector(vector);
}

template <typename T>
HalfedgeData<T>::HalfedgeData(HalfedgeMesh* parentMesh,
                              const Eigen::Matrix<T, Eigen::Dynamic, 1>& vector,
                              const HalfedgeData<size_t>& indexer)
    : HalfedgeData(parentMesh) {
  fromVector(vector, indexer);
}

template <typename T>
void HalfedgeData<T>::fill(T val) {
  std::fill(data.begin(), data.end(), val);
}

template <typename T>
Eigen::Matrix<T, Eigen::Dynamic, 1> HalfedgeData<T>::toVector() const {
  Eigen::Matrix<T, Eigen::Dynamic, 1> result(mesh->nHalfedges() + mesh->nImaginaryHalfedges());
  HalfedgeData<size_t> ind = mesh->getHalfedgeIndices();
  for (HalfedgePtr he : mesh->allHalfedges()) {
    result(ind[he]) = (*this)[he];
  }
  return result;
}

template <typename T>
Eigen::Matrix<T, Eigen::Dynamic, 1> HalfedgeData<T>::toVector(
    const HalfedgeData<size_t>& indexer) const {
  size_t outSize = 0;
  for (HalfedgePtr he : mesh->allHalfedges())
    if (indexer[he] != std::numeric_limits<size_t>::max()) outSize++;
  Eigen::Matrix<T, Eigen::Dynamic, 1> result(outSize);
  for (HalfedgePtr he : mesh->allHalfedges()) {
    if (indexer[he] != std::numeric_limits<size_t>::max()) {
      result(indexer[he]) = (*this)[he];
    }
  }
  return result;
}

template <typename T>
void HalfedgeData<T>::fromVector(
    const Eigen::Matrix<T, Eigen::Dynamic, 1>& vector) {
  if ((size_t)vector.rows() != (mesh->nHalfedges() + mesh->nImaginaryHalfedges()))
    throw std::runtime_error("Vector size does not match mesh size.");
  HalfedgeData<size_t> ind = mesh->getHalfedgeIndices();
  for (HalfedgePtr he : mesh->allHalfedges()) {
    (*this)[he] = vector(ind[he]);
  }
}

template <typename T>
void HalfedgeData<T>::fromVector(const Eigen::Matrix<T, Eigen::Dynamic, 1>& vector,
                                 const HalfedgeData<size_t>& indexer) {
  for (HalfedgePtr he : mesh->allHalfedges()) {
    if (indexer[he] != std::numeric_limits<size_t>::max()) {
      (*this)[he] = vector(indexer[he]);
    }
  }
}

template <typename T>
inline T& HalfedgeData<T>::operator[](HalfedgePtr he) {
#ifndef NDEBUG
  assert(he->parentMesh == mesh &&
         "Attempted access data with member from wrong mesh");
#endif
  size_t i = he - mesh->halfedge(0);
  return data[i];
}

template <typename T>
inline const T& HalfedgeData<T>::operator[](HalfedgePtr he) const {
#ifndef NDEBUG
  assert(he->parentMesh == mesh &&
         "Attempted access data with member from wrong mesh");
#endif
  size_t i = he - mesh->halfedge(0);
  return data[i];
}

template <typename T>
inline T& HalfedgeData<T>::operator[](size_t he) {
#ifndef NDEBUG
  assert(he < size() && "Attempted to access data with out of bounds index");
#endif
  return data[he];
}

template <typename T>
inline const T& HalfedgeData<T>::operator[](size_t he) const {
#ifndef NDEBUG
  assert(he < size() && "Attempted to access data with out of bounds index");
#endif
  return data[he];
}

template <typename T>
inline size_t HalfedgeData<T>::size() const {
  return data.size();
}

GC_INTERNAL_GENERATE_DATATYPE_OPERATOR_DEFINITIONS(HalfedgeData, mesh)

// Data on corners
template <typename T>
CornerData<T>::CornerData(HalfedgeMesh* parentMesh) : mesh(parentMesh) {
  if (parentMesh != nullptr) {
    realSize = parentMesh->nHalfedges();
    data.resize(realSize);
  }
}

template <typename T>
CornerData<T>::CornerData(HalfedgeMesh* parentMesh, T initVal)
    : CornerData(parentMesh) {
  fill(initVal);
}

template <typename T>
CornerData<T>::CornerData(HalfedgeMesh* parentMesh,
                          const Eigen::Matrix<T, Eigen::Dynamic, 1>& vector)
    : CornerData(parentMesh) {
  fromVector(vector);
}

template <typename T>
CornerData<T>::CornerData(HalfedgeMesh* parentMesh,
                          const Eigen::Matrix<T, Eigen::Dynamic, 1>& vector,
                          const CornerData<size_t>& indexer)
    : CornerData(parentMesh) {
  fromVector(vector, indexer);
}

template <typename T>
void CornerData<T>::fill(T val) {
  std::fill(data.begin(), data.end(), val);
}

template <typename T>
Eigen::Matrix<T, Eigen::Dynamic, 1> CornerData<T>::toVector() const {
  Eigen::Matrix<T, Eigen::Dynamic, 1> result(mesh->nCorners());
  CornerData<size_t> ind = mesh->getCornerIndices();
  for (CornerPtr c : mesh->corners()) {
    result(ind[c]) = (*this)[c];
  }
  return result;
}

template <typename T>
Eigen::Matrix<T, Eigen::Dynamic, 1> CornerData<T>::toVector(
    const CornerData<size_t>& indexer) const {
  size_t outSize = 0;
  for (CornerPtr c : mesh->corners())
    if (indexer[c] != std::numeric_limits<size_t>::max()) outSize++;
  Eigen::Matrix<T, Eigen::Dynamic, 1> result(outSize);
  for (CornerPtr c : mesh->corners()) {
    if (indexer[c] != std::numeric_limits<size_t>::max()) {
      result(indexer[c]) = (*this)[c];
    }
  }
  return result;
}

template <typename T>
void CornerData<T>::fromVector(const Eigen::Matrix<T, Eigen::Dynamic, 1>& vector) {
  if ((size_t)vector.rows() != mesh->nCorners())
    throw std::runtime_error("Vector size does not match mesh size.");
  CornerData<size_t> ind = mesh->getCornerIndices();
  for (CornerPtr c : mesh->corners()) {
    (*this)[c] = vector(ind[c]);
  }
}

template <typename T>
void CornerData<T>::fromVector(const Eigen::Matrix<T, Eigen::Dynamic, 1>& vector,
                               const CornerData<size_t>& indexer) {
  for (CornerPtr c : mesh->corners()) {
    if (indexer[c] != std::numeric_limits<size_t>::max()) {
      (*this)[c] = vector(indexer[c]);
    }
  }
}

template <typename T>
inline T& CornerData<T>::operator[](CornerPtr c) {
#ifndef NDEBUG
  assert(c->parentMesh == mesh &&
         "Attempted access data with member from wrong mesh");
#endif
  size_t i = c - mesh->corner(0);
  return data[i];
}

template <typename T>
inline const T& CornerData<T>::operator[](CornerPtr c) const {
#ifndef NDEBUG
  assert(c->parentMesh == mesh &&
         "Attempted access data with member from wrong mesh");
#endif
  size_t i = c - mesh->corner(0);
  return data[i];
}

template <typename T>
inline T& CornerData<T>::operator[](size_t c) {
#ifndef NDEBUG
  assert(c < size() && "Attempted to access data with out of bounds index");
#endif
  return data[c];
}

template <typename T>
inline const T& CornerData<T>::operator[](size_t c) const {
#ifndef NDEBUG
  assert(c < size() && "Attempted to access data with out of bounds index");
#endif
  return data[c];
}

template <typename T>
inline size_t CornerData<T>::size() const {
  return data.size();
}

GC_INTERNAL_GENERATE_DATATYPE_OPERATOR_DEFINITIONS(CornerData, mesh)

}  // namespace geometrycentral
