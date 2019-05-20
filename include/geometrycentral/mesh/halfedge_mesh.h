#pragma once

#include <list>
#include <memory>
#include <vector>

#include "geometrycentral/mesh/halfedge_containers.h"
#include "geometrycentral/mesh/halfedge_element_types.h"
#include "geometrycentral/mesh/halfedge_iterators.h"
#include "geometrycentral/utilities/utilities.h"

// NOTE: ipp includes at bottom of file

namespace geometrycentral {
namespace halfedge_mesh {

class HalfedgeMesh {

public:
  HalfedgeMesh();

  // Build a halfedge mesh from polygons, with a list of 0-indexed vertices incident on each face, in CCW order.
  HalfedgeMesh(const std::vector<std::vector<size_t>>& polygons, bool verbose = false);
  ~HalfedgeMesh();


  // Number of mesh elements of each type
  size_t nHalfedges() const;
  size_t nInteriorHalfedges() const;
  size_t nCorners() const;
  size_t nVertices() const;
  size_t nInteriorVertices(); // warning: O(n)
  size_t nEdges() const;
  size_t nFaces() const;
  size_t nBoundaryLoops() const;
  size_t nExteriorHalfedges() const;

  // Methods for range-based for loops
  // Example: for(Vertex v : mesh.vertices()) { ... }
  HalfedgeSet halfedges();
  HalfedgeInteriorSet interiorHalfedges();
  HalfedgeExteriorSet exteriorHalfedges();
  CornerSet corners();
  VertexSet vertices();
  EdgeSet edges();
  FaceSet faces();
  BoundaryLoopSet boundaryLoops();

  // Methods for accessing elements by index
  // only valid when the  mesh is compressed
  Halfedge halfedge(size_t index);
  Corner corner(size_t index);
  Vertex vertex(size_t index);
  Edge edge(size_t index);
  Face face(size_t index);
  BoundaryLoop boundaryLoop(size_t index);

  // Methods that mutate the mesh. Note that these occasionally trigger a resize, which invaliates
  // any outstanding Vertex or MeshData<> objects. See the guide (currently in docs/mutable_mesh_docs.md).
  // TODOs: support adding boundary


  // Flip an edge. Unlike all the other mutation routines, this _does not_ invalidate pointers, though it does break the
  // canonical ordering.
  // Return true if the edge was actually flipped (can't flip boundary or non-triangular edges)
  bool flip(Edge e);

  // Adds a vertex along an edge, increasing degree of faces. Returns ptr along the new edge, with he.vertex() as new
  // vertex and he.edge().halfedge() == he. Preserves canonical direction of edge.halfedge() for both halves of new
  // edge.
  Halfedge insertVertexAlongEdge(Edge e);

  // Split an edge, also splitting adjacent faces. Returns new vertex.
  Vertex splitEdge(Edge e);

  // Split an edge, also splitting adjacent faces. Returns new halfedge which points TOWARDS the new vertex, and is the
  // same direction as e.halfedge() on the original edge. The halfedge direction of the other part of the new split edge
  // is also preserved, as in insertVertexAlongEdge().
  Halfedge splitEdgeReturnHalfedge(Edge e);

  // Add vertex inside face and triangulate. Returns new vertex.
  Vertex insertVertex(Face f);

  // Add an edge connecting two vertices inside the same face. Returns new halfedge with vA at tail. he.twin().face() is
  // the new face.
  Halfedge connectVertices(Vertex vA, Vertex vB);

  // Same as above. Faster if you know the face.
  Halfedge connectVertices(Face face, Vertex vA, Vertex vB);

  // Same as above, but if vertices do not contain shared face or are adajcent returns Halfedge() rather than
  // throwing.
  Halfedge tryConnectVertices(Vertex vA, Vertex vB);

  // Same as above, but you can specify a face to work in
  Halfedge tryConnectVertices(Vertex vA, Vertex vB, Face face);

  // Collapse an edge. Returns the vertex adjacent to that edge which still exists. Returns Vertex() if not
  // collapsible.
  Vertex collapseEdge(Edge e);

  // Remove a face which is adjacent to the boundary of the mesh (along with its edge on the boundary).
  // Face must have exactly one boundary edge.
  // Returns true if could remove
  bool removeFaceAlongBoundary(Face f);


  // Set e.halfedge() == he. he must be adjacent.
  void setEdgeHalfedge(Edge e, Halfedge he);

  // Triangulate in a face, returns all subfaces
  std::vector<Face> triangulate(Face face);


  // Methods for obtaining canonical indices for mesh elements
  // (Note that in some situations, custom indices might instead be needed)
  VertexData<size_t> getVertexIndices();
  VertexData<size_t> getInteriorVertexIndices();
  FaceData<size_t> getFaceIndices();
  EdgeData<size_t> getEdgeIndices();
  HalfedgeData<size_t> getHalfedgeIndices();
  CornerData<size_t> getCornerIndices();

  // == Utility functions
  bool isTriangular();           // returns true if and only if all faces are triangles [O(n)]
  int eulerCharacteristic();     // compute the Euler characteristic [O(1)]
  int genus();     								// compute the genus [O(1)]
  size_t nConnectedComponents(); // compute number of connected components [O(n)]

  std::vector<std::vector<size_t>> getFaceVertexList();
  std::unique_ptr<HalfedgeMesh> copy();

  // Compress the mesh
  bool isCompressed();
  void compress();

  // Canonicalize the element ordering to be the same indexing convention as after construction from polygon soup.
  bool isCanonical();
  void canonicalize();

  // == Callbacks that will be invoked on mutation to keep containers/iterators/etc valid.
  // Expansion callbacks
  // Argument is the new size of the element list. Elements up to this index (aka offset from base) may now be used (but
  // _might_ not be)
  std::list<std::function<void(size_t)>> vertexExpandCallbackList;
  std::list<std::function<void(size_t)>> faceExpandCallbackList;
  std::list<std::function<void(size_t)>> edgeExpandCallbackList;
  std::list<std::function<void(size_t)>> halfedgeExpandCallbackList;

  // Compression callbacks
  // Argument is a permutation to a apply, such that d_new[i] = d_old[p[i]]. New size may be smaller, in which case
  // capacity may be shrunk to that size.
  std::list<std::function<void(const std::vector<size_t>&)>> vertexPermuteCallbackList;
  std::list<std::function<void(const std::vector<size_t>&)>> facePermuteCallbackList;
  std::list<std::function<void(const std::vector<size_t>&)>> edgePermuteCallbackList;
  std::list<std::function<void(const std::vector<size_t>&)>> halfedgePermuteCallbackList;

  // Mesh delete callbacks
  // (this unfortunately seems to be necessary; objects which have registered their callbacks above
  // need to know not to try to de-register them if the mesh has been deleted)
  std::list<std::function<void()>> meshDeleteCallbackList;

  // Check capacity. Needed when implementing expandable containers for mutable meshes to ensure the contain can
  // hold a sufficient number of elements before the next resize event.
  size_t nHalfedgesCapacity() const;
  size_t nVerticesCapacity() const;
  size_t nEdgesCapacity() const;
  size_t nFacesCapacity() const;
  size_t nBoundaryLoopsCapacity() const;

  // Performs a sanity checks on halfedge structure; throws on fail
  void validateConnectivity();


private:
  // Core arrays which hold the connectivity
  std::vector<size_t> heNext;    // he.next()
  std::vector<size_t> heVertex;  // he.vertex()
  std::vector<size_t> heFace;    // he.face()
  std::vector<size_t> vHalfedge; // v.halfedge()
  std::vector<size_t> fHalfedge; // f.halfedge()

  // Implicit connectivity relationships
  static size_t heTwin(size_t iHe);   // he.twin()
  static size_t heEdge(size_t iHe);   // he.edge()
  static size_t eHalfedge(size_t iE); // e.halfedge()

  // Other implicit relationships
  bool heIsInterior(size_t iHe) const;
  bool faceIsBoundaryLoop(size_t iF) const;
  size_t faceIndToBoundaryLoopInd(size_t iF) const;
  size_t boundaryLoopIndToFaceInd(size_t iF) const;

  // Auxilliary arrays which cache other useful information

  // Track element counts (can't rely on rawVertices.size() after deletions have made the list sparse). These are the
  // actual number of valid elements, not the size of the buffer that holds them.
  size_t nHalfedgesCount = 0;
  size_t nInteriorHalfedgesCount = 0;
  size_t nVerticesCount = 0;
  size_t nEdgesCount = 0;
  size_t nFacesCount = 0;
  size_t nBoundaryLoopsCount = 0;

  // == Track the capacity and fill size of our buffers.
  // These give the capacity of the currently allocated buffer.
  // Note that this is _not_ defined to be std::vector::capacity(), it's the largest size such that arr[i] is legal.
  size_t nVerticesCapacityCount = 0;
  size_t nHalfedgesCapacityCount = 0; // will always be even
  size_t nEdgesCapacityCount() const;
  size_t nFacesCapacityCount = 0;     // capacity for faces _and_ boundary loops

  // These give the number of filled elements in the currently allocated buffer. This will also be the maximal index of
  // any element (except the weirdness of boundary loop faces). As elements get marked dead, nVerticesCount decreases
  // but nVertexFillCount does not (etc), so it denotes the end of the region in the buffer where elements have been
  // stored.
  size_t nVerticesFillCount = 0;
  size_t nHalfedgesFillCount = 0; // must always be even
  size_t nEdgesFillCount() const;
  size_t nFacesFillCount = 0;         // where the real faces stop, and empty/boundary loops begin
  size_t nBoundaryLoopsFillCount = 0; // remember, these fill from the back of the face buffer

  bool isCanonicalFlag = true;
  bool isCompressedFlag = true;

  // Hide copy and move constructors, we don't wanna mess with that
  HalfedgeMesh(const HalfedgeMesh& other) = delete;
  HalfedgeMesh& operator=(const HalfedgeMesh& other) = delete;
  HalfedgeMesh(HalfedgeMesh&& other) = delete;
  HalfedgeMesh& operator=(HalfedgeMesh&& other) = delete;

  // Used to resize the halfedge mesh. Expands and shifts vectors as necessary.
  Halfedge* getNewHalfedge(bool interior = true);
  Vertex* getNewVertex();
  Edge* getNewEdge();
  Face* getNewFace();

  // Detect dead elements
  bool vertexIsDead(size_t iV) const;
  bool halfedgeIsDead(size_t iHe) const;
  bool edgeIsDead(size_t iE) const;
  bool faceIsDead(size_t iF) const;

  // Deletes leave tombstones, which can be cleaned up with compress()
  void deleteElement(Halfedge he);
  void deleteElement(Edge e);
  void deleteElement(Vertex v);
  void deleteElement(Face f);

  // Compression helpers
  void compressHalfedges();
  void compressEdges();
  void compressFaces();
  void compressVertices();


  // Helpers for mutation methods
  void ensureVertexHasBoundaryHalfedge(Vertex v); // impose invariant that v.halfedge is start of half-disk
  Vertex collapseEdgeAlongBoundary(Edge e);


  // Elements need direct access in to members to traverse
  friend class Vertex;
  friend class Halfedge;
  friend class Corner;
  friend class Edge;
  friend class Face;
  friend class BoundaryLoop;

  // friend class VertexRangeIterator;
  friend struct VertexRangeF;
  friend struct HalfedgeRangeF;
  friend struct HalfedgeInteriorRangeF;
  friend struct HalfedgeExteriorRangeF;
  friend struct CornerRangeF;
  friend struct EdgeRangeF;
  friend struct FaceRangeF;
  friend struct BoundaryLoopRangeF;
};

} // namespace halfedge_mesh
} // namespace geometrycentral

// clang-format off
// preserve ordering
#include "geometrycentral/mesh/halfedge_containers.ipp"
#include "geometrycentral/mesh/halfedge_iterators.ipp"
#include "geometrycentral/mesh/halfedge_element_types.ipp"
#include "geometrycentral/mesh/halfedge_logic_templates.ipp"
#include "geometrycentral/mesh/halfedge_mesh.ipp"
// clang-format on
