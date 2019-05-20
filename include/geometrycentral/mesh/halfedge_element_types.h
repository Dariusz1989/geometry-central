#pragma once

#include <cstddef>
#include <functional>
#include <iostream>

#include "geometrycentral/utilities/utilities.h"

namespace geometrycentral {
namespace halfedge_mesh {

// === Types and inline methods for the halfedge mesh pointer and datatypes
class HalfedgeMesh;


// === Types for mesh elements
class Vertex;
class Halfedge;
class Corner;
class Edge;
class Face;
class BoundaryLoop;


// === Forward declare iterator set types (pointers may need to return these to
// support range-based for loops)
class VertexIncomingHalfedgeSet;
class VertexOutgoingHalfedgeSet;
class VertexAdjacentVertexSet;
class VertexAdjacentFaceSet;
class VertexAdjacentEdgeSet;
class VertexAdjacentCornerSet;
class FaceAdjacentHalfedgeSet;
class FaceAdjacentVertexSet;
class FaceAdjacentEdgeSet;
class FaceAdjacentFaceSet;
class FaceAdjacentCornerSet;
class BoundaryLoopAdjacentHalfedgeSet;
class BoundaryLoopAdjacentVertexSet;
class BoundaryLoopAdjacentEdgeSet;
class BoundaryLoopAdjacentFaceSet;
class BoundaryLoopAdjacentCornerSet;


// === Templated helper functions
// clang-format off

// Enums, which we use to template the base case of all actual element types
enum class ElementType { Vertex = 0, Halfedge, Corner, Edge, Face, BoundaryLoop };

// Current count of this element in the mesh
template <typename E> size_t nElements(HalfedgeMesh* mesh) { return INVALID_IND; }

// Capacity of element type in mesh (containers should be at least this big before next resize)
template <typename E> size_t elementCapacity(HalfedgeMesh* mesh) { return INVALID_IND; }

// Canonical index for this element (not always an enumeration)
template <typename E> size_t dataIndexOfElement(HalfedgeMesh* mesh, E e) { return INVALID_IND; }

// The set type used to iterate over all elements of this type
template <typename E> struct ElementSetType { typedef std::tuple<> type; }; // nonsense default value

template <typename E> typename ElementSetType<E>::type iterateElements(HalfedgeMesh* mesh) { return INVALID_IND; }

template <typename E> std::list<std::function<void(size_t)>>& getExpandCallbackList(HalfedgeMesh* mesh);

template <typename E> std::list<std::function<void(const std::vector<size_t>&)>>& getPermuteCallbackList(HalfedgeMesh* mesh);

template <typename E> std::string typeShortName() { return "X"; }
// clang-format on

// ==========================================================
// ================      Base Element      ==================
// ==========================================================


// Need to pre-declare template friends to avoid generating a non-template version
// (see https://isocpp.org/wiki/faq/templates#template-friends)
template <typename T>
class Element;
template <typename T>
std::ostream& operator<<(std::ostream& o, const Element<T>& x);

// Forward-declare dynamic equivalent so we can declare a conversion constructor
template <typename S>
class DynamicElement;

// == Base type for shared logic between elements.
//
// This class uses the "curiously recurring template pattern" (CRTP) because it allows to implement more shared
// functionality in this template. Instantiations of the template will be templted on its child types, like `Vertex :
// public Element<Vertex>`. Because the parent class will know its child type at compile time, it can customize
// functionality based on that child type using the helpers above. This is essentially "compile time polymorphism",
// which allows us to share common functionality without paying a virtual function runtime cost.
template <typename T>
class Element {

public:
  Element();                                 // construct an empty (null) element
  Element(HalfedgeMesh* mesh_, size_t ind_); // construct pointing to the i'th element of that type on a mesh.
  Element(const DynamicElement<T>& e);       // construct from a dynamic element of matching type

  inline bool operator==(const Element<T>& other) const;
  inline bool operator!=(const Element<T>& other) const;
  inline bool operator>(const Element<T>& other) const;
  inline bool operator>=(const Element<T>& other) const;
  inline bool operator<(const Element<T>& other) const;
  inline bool operator<=(const Element<T>& other) const;

  // Get the "index" associated with the element.
  // Note that these are not always a dense enumeration, and generally should not be accessed by "users" unless you are
  // monkeying around the HalfedgeMesh datastructure in some deep and scary way. Generally prefer
  // `HalfedgeMesh::getVertexIndices()` (etc) if you are looking for a set of indices for a linear algebra problem or
  // something.
  size_t getIndex() const;

  // Get the halfedge mesh on which the element is defined.
  HalfedgeMesh* getMesh() const;

protected:
  HalfedgeMesh* mesh = nullptr;
  size_t ind = INVALID_IND;

  // Friends
  friend std::ostream& operator<<<>(std::ostream& output, const Element<T>& e);
  friend struct std::hash<T>;
};

template <typename T>
std::ostream& operator<<(std::ostream& output, const Element<T>& e);

// The equivalent dynamic pointers. These should be rarely used, but are guaranteed to be preserved through _all_ mesh
// operations, including compress().
template <typename S>
class DynamicElement {
public:
  DynamicElement();                                 // construct an empty (null) element
  DynamicElement(HalfedgeMesh* mesh_, size_t ind_); // construct from an index as usual
  DynamicElement(const S& e);                       // construct from a non-dynamic element

  ~DynamicElement();

  // returns a new plain old static element. Useful for chaining.
  S decay() const;

private:
  // References to the callbacks which keep the element valid. Keep these around to de-register on destruction.
  std::list<std::function<void(const std::vector<size_t>&)>>::iterator permuteCallbackIt;
  std::list<std::function<void()>>::iterator deleteCallbackIt;
};


// == Base range iterator
// All range iterators have the form "advance through indices, skipping invalid elements". The two classes below
// encapsulate that functionality, allowing us to just specify the element type and "valid" function for each.
template <typename F>
class RangeIteratorBase {

public:
  RangeIteratorBase(HalfedgeMesh* mesh_, size_t iStart_, size_t iEnd_);
  const RangeIteratorBase& operator++();
  bool operator==(const RangeIteratorBase& other) const;
  bool operator!=(const RangeIteratorBase& other) const;
  typename F::Etype operator*() const;

private:
  HalfedgeMesh* mesh;
  size_t iCurr;
  size_t iEnd;
};

template <typename F>
class RangeSetBase {
public:
  RangeSetBase(HalfedgeMesh* mesh_, size_t iStart_, size_t iEnd_);
  RangeIteratorBase<F> begin() const;
  RangeIteratorBase<F> end() const;

private:
  HalfedgeMesh* mesh;
  size_t iStart, iEnd;
};


// ==========================================================
// ================        Vertex          ==================
// ==========================================================

class Vertex : public Element<Vertex> {
public:
  // Constructors (inherit from base)
  using Element<Vertex>::Element;

  // Navigators
  Halfedge halfedge() const;
  Corner corner() const;

  // Properties
  bool isBoundary() const;
  size_t degree();
  size_t faceDegree();

  // Iterators
  VertexIncomingHalfedgeSet incomingHalfedges() const;
  VertexOutgoingHalfedgeSet outgoingHalfedges() const;
  VertexAdjacentVertexSet adjacentVertices() const;
  VertexAdjacentFaceSet adjacentFaces() const;
  VertexAdjacentEdgeSet adjacentEdges() const;
  VertexAdjacentCornerSet adjacentCorners() const;
};

using DynamicVertex = DynamicElement<Vertex>;

// == Range iterators

// All vertices
struct VertexRangeF {
  static bool elementOkay(const HalfedgeMesh& mesh, size_t ind);
  typedef Vertex Etype;
};
typedef RangeSetBase<VertexRangeF> VertexSet;

// ==========================================================
// ================        Halfedge        ==================
// ==========================================================

class Halfedge : public Element<Halfedge> {
public:
  // Constructors (inherit from base)
  using Element<Halfedge>::Element;

  // Navigators
  Halfedge twin() const;
  Halfedge next() const;
  Corner corner() const;
  Vertex vertex() const;
  Edge edge() const;
  Face face() const;

  // Properties
  bool isInterior() const;
};

using DynamicHalfedge = DynamicElement<Halfedge>;

// == Range iterators

// All halfedges
struct HalfedgeRangeF {
  static bool elementOkay(const HalfedgeMesh& mesh, size_t ind);
  typedef Halfedge Etype;
};
typedef RangeSetBase<HalfedgeRangeF> HalfedgeSet;

// Interior halfedges
struct HalfedgeInteriorRangeF {
  static bool elementOkay(const HalfedgeMesh& mesh, size_t ind);
  typedef Halfedge Etype;
};
typedef RangeSetBase<HalfedgeInteriorRangeF> HalfedgeInteriorSet;

// Exterior halfedges
struct HalfedgeExteriorRangeF {
  static bool elementOkay(const HalfedgeMesh& mesh, size_t ind);
  typedef Halfedge Etype;
};
typedef RangeSetBase<HalfedgeExteriorRangeF> HalfedgeExteriorSet;

// ==========================================================
// ================        Corner          ==================
// ==========================================================

// Implmentation note: The `ind` parameter for a corner will be the index of a halfedge, which should always be real.

class Corner : public Element<Corner> {
public:
  // Constructors (inherit from base)
  using Element<Corner>::Element;

  // Navigators
  Halfedge halfedge() const;
  Vertex vertex() const;
  Face face() const;
};

using DynamicCorner = DynamicElement<Corner>;

// == Range iterators

// All corners
struct CornerRangeF {
  static bool elementOkay(const HalfedgeMesh& mesh, size_t ind);
  typedef Corner Etype;
};
typedef RangeSetBase<CornerRangeF> CornerSet;

// ==========================================================
// ================          Edge          ==================
// ==========================================================

class Edge : public Element<Edge> {
public:
  // Constructors (inherit from base)
  using Element<Edge>::Element;

  // Navigators
  Halfedge halfedge() const;

  // Properties
  bool isBoundary() const;
};

using DynamicEdge = DynamicElement<Edge>;

// == Range iterators

// All edges
struct EdgeRangeF {
  static bool elementOkay(const HalfedgeMesh& mesh, size_t ind);
  typedef Edge Etype;
};
typedef RangeSetBase<EdgeRangeF> EdgeSet;

// ==========================================================
// ================          Face          ==================
// ==========================================================

// Implmentation note: The `ind` parameter for a face might correspond to a boundary loop. The boundary loops have face
// IDs which are at the very end of the face buffer, but can still index in to face-valued arrays/functions in
// HalfedgeMesh (they _cannot_ index in to FaceData<> containers).

class Face : public Element<Face> {
public:
  // Constructors (inherit from base)
  using Element<Face>::Element;

  // Navigators
  Halfedge halfedge() const;
  BoundaryLoop asBoundaryLoop() const;

  // Properties
  bool isBoundaryLoop() const;
  size_t degree() const;

  // Iterators
  FaceAdjacentHalfedgeSet adjacentHalfedges() const;
  FaceAdjacentVertexSet adjacentVertices() const;
  FaceAdjacentFaceSet adjacentFaces() const;
  FaceAdjacentEdgeSet adjacentEdges() const;
  FaceAdjacentCornerSet adjacentCorners() const;
};

using DynamicFace = DynamicElement<Face>;

// == Range iterators

// All faces
struct FaceRangeF {
  static bool elementOkay(const HalfedgeMesh& mesh, size_t ind);
  typedef Face Etype;
};
typedef RangeSetBase<FaceRangeF> FaceSet;

// ==========================================================
// ================     Boundary Loop      ==================
// ==========================================================

// Implementation note: the `ind` parameter for a boundary loop is index from the back of the face index space, from [0,
// nBoundaryLoopFillCount).

class BoundaryLoop : public Element<BoundaryLoop> {
public:
  // Constructors (inherit from base)
  using Element<BoundaryLoop>::Element;

  Halfedge halfedge() const;
  Face asFace() const;

  // Properties
  size_t degree() const;

  // Iterators
  BoundaryLoopAdjacentHalfedgeSet adjacentHalfedges() const;
  BoundaryLoopAdjacentVertexSet adjacentVertices() const;
  BoundaryLoopAdjacentEdgeSet adjacentEdges() const;
};

using DynamicBoundaryLoop = DynamicElement<BoundaryLoop>;

// == Range iterators

// All boundary loops
struct BoundaryLoopRangeF {
  static bool elementOkay(const HalfedgeMesh& mesh, size_t ind);
  typedef BoundaryLoop Etype;
};
typedef RangeSetBase<BoundaryLoopRangeF> BoundaryLoopSet;


} // namespace halfedge_mesh
} // namespace geometrycentral
