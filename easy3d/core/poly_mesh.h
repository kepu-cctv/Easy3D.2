/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EASY3D_CORE_POLYHEDRAL_MESH_H
#define EASY3D_CORE_POLYHEDRAL_MESH_H

#include <easy3d/core/model.h>
#include <easy3d/core/types.h>
#include <easy3d/core/properties.h>

namespace easy3d {

    /**
     * \brief Data structure representing a polyhedral mesh.
     * \class PolyMesh easy3d/core/poly_mesh.h
     */

    class PolyMesh : public virtual Model
    {

    public: //------------------------------------------------------ topology types


        /// Base class for all topology types (internally it is basically an index)
        /// \sa Vertex, Edge, HalfFace, Cell
        class BaseHandle
        {
        public:

            /// constructor
            explicit BaseHandle(int _idx=-1) : idx_(_idx) {}

            /// Get the underlying index of this handle
            int idx() const { return idx_; }

            /// reset handle to be invalid (index=-1)
            void reset() { idx_=-1; }

            /// return whether the handle is valid, i.e., the index is not equal to -1.
            bool is_valid() const { return idx_ != -1; }

            /// are two handles equal?
            bool operator==(const BaseHandle& _rhs) const {
                return idx_ == _rhs.idx_;
            }

            /// are two handles different?
            bool operator!=(const BaseHandle& _rhs) const {
                return idx_ != _rhs.idx_;
            }

            /// compare operator useful for sorting handles
            bool operator<(const BaseHandle& _rhs) const {
                return idx_ < _rhs.idx_;
            }

            // [Liangliang]: to be able to use std::unordered_map
            struct Hash {
                std::size_t operator()(const BaseHandle& h) const { return h.idx(); }
            };

        private:
            friend class PolyMesh;
            int idx_;
        };


        /// this type represents a vertex (internally it is basically an index)
        ///  \sa Edge, HalfFace, Cell
        struct Vertex : public BaseHandle
        {
            /// default constructor (with invalid index)
            explicit Vertex(int _idx=-1) : BaseHandle(_idx) {}
            std::ostream& operator<<(std::ostream& os) const { return os << 'v' << idx(); }
        };

        /// this type represents an edge (internally it is basically an index)
        /// \sa Vertex, HalfFace, Cell
        struct Edge : public BaseHandle
        {
            /// default constructor (with invalid index)
            explicit Edge(int _idx=-1) : BaseHandle(_idx) {}
            std::ostream& operator<<(std::ostream& os) const { return os << 'e' << idx(); }
        };


        /// this type represents a halfface (internally it is basically an index)
        /// \sa Vertex, Edge, Cell
        struct HalfFace : public BaseHandle
        {
            /// default constructor (with invalid index)
            explicit HalfFace(int _idx=-1) : BaseHandle(_idx) {}
            std::ostream& operator<<(std::ostream& os) const { return os << 'h' << idx(); }
        };

        /// this type represents a halfface (internally it is basically an index)
        /// \sa Vertex, Edge, Cell
        struct Face : public BaseHandle
        {
            /// default constructor (with invalid index)
            explicit Face(int _idx=-1) : BaseHandle(_idx) {}
            std::ostream& operator<<(std::ostream& os) const { return os << 'f' << idx(); }
        };

        /// this type represents a polyhedral cell (internally it is basically an index)
        /// \sa Vertex, Edge, HalfFace
        struct Cell : public BaseHandle
        {
            /// default constructor (with invalid index)
            explicit Cell(int _idx=-1) : BaseHandle(_idx) {}
            std::ostream& operator<<(std::ostream& os) const { return os << 'c' << idx(); }
        };



    public: //-------------------------------------------------- connectivity types

        /// This type stores the vertex connectivity
        /// \sa EdgeConnectivity, HalfFaceConnectivity, CellConnectivity
        struct VertexConnectivity
        {
            std::set<Vertex>     vertices_;
            std::set<Edge>       edges_;
            std::set<HalfFace>   halffaces_;
            std::set<Cell>       cells_;
        };


        /// This type stores the edge connectivity
        /// \sa VertexConnectivity, HalfFaceConnectivity, CellConnectivity
        struct EdgeConnectivity
        {
            std::vector<Vertex>  vertices_;
            std::set<HalfFace>   halffaces_;
            std::set<Cell>       cells_;
        };

        /// This type stores the halfface connectivity
        /// \sa VertexConnectivity, EdgeConnectivity, CellConnectivity
        struct HalfFaceConnectivity
        {
            std::vector<Vertex> vertices_;
            std::set<Edge>   edges_;
            Cell             cell_;
            HalfFace         opposite_;
        };

        /// This type stores the cell connectivity
        /// \sa VertexConnectivity, EdgeConnectivity, HalfFaceConnectivity
        struct CellConnectivity
        {
            std::set<Vertex>     vertices_;
            std::set<Edge>       edges_;
            std::vector<HalfFace>   halffaces_;
        };



    public: //------------------------------------------------------ property types

        /// Vertex property of type T
        /// \sa EdgeProperty, HalfFaceProperty, CellProperty
        template <class T> class VertexProperty : public Property<T>
        {
        public:

            /// default constructor
            explicit VertexProperty() {}
            explicit VertexProperty(Property<T> p) : Property<T>(p) {}

            /// access the data stored for vertex \c v
            typename Property<T>::reference operator[](Vertex v)
            {
                return Property<T>::operator[](v.idx());
            }

            /// access the data stored for vertex \c v
            typename Property<T>::const_reference operator[](Vertex v) const
            {
                return Property<T>::operator[](v.idx());
            }
        };

        /// Edge property of type T
        /// \sa VertexProperty, HalfFaceProperty, CellProperty
        template <class T> class EdgeProperty : public Property<T>
        {
        public:

            /// default constructor
            explicit EdgeProperty() {}
            explicit EdgeProperty(Property<T> p) : Property<T>(p) {}

            /// access the data stored for edge \c e
            typename Property<T>::reference operator[](Edge e)
            {
                return Property<T>::operator[](e.idx());
            }

            /// access the data stored for edge \c e
            typename Property<T>::const_reference operator[](Edge e) const
            {
                return Property<T>::operator[](e.idx());
            }
        };


        /// HalfFace property of type T
        /// \sa VertexProperty, EdgeProperty, CellProperty
        template <class T> class HalfFaceProperty : public Property<T>
        {
        public:

            /// default constructor
            explicit HalfFaceProperty() {}
            explicit HalfFaceProperty(Property<T> p) : Property<T>(p) {}

            /// access the data stored for face \c f
            typename Property<T>::reference operator[](HalfFace f)
            {
                return Property<T>::operator[](f.idx());
            }

            /// access the data stored for face \c f
            typename Property<T>::const_reference operator[](HalfFace f) const
            {
                return Property<T>::operator[](f.idx());
            }
        };



        /// HalfFace property of type T
        /// \sa VertexProperty, EdgeProperty, CellProperty
        template <class T> class FaceProperty : public Property<T>
        {
        public:

            /// default constructor
            explicit FaceProperty() {}
            explicit FaceProperty(Property<T> p) : Property<T>(p) {}

            /// access the data stored for face \c f
            typename Property<T>::reference operator[](Face f)
            {
                return Property<T>::operator[](f.idx());
            }

            /// access the data stored for face \c f
            typename Property<T>::const_reference operator[](Face f) const
            {
                return Property<T>::operator[](f.idx());
            }
        };


        /// Halfedge property of type T
        /// \sa VertexProperty, EdgeProperty, HalfFaceProperty
        template <class T> class CellProperty : public Property<T>
        {
        public:

            /// default constructor
            explicit CellProperty() {}
            explicit CellProperty(Property<T> p) : Property<T>(p) {}

            /// access the data stored for halfedge \c h
            typename Property<T>::reference operator[](Cell t)
            {
                return Property<T>::operator[](t.idx());
            }

            /// access the data stored for halfedge \c h
            typename Property<T>::const_reference operator[](Cell t) const
            {
                return Property<T>::operator[](t.idx());
            }
        };


        /// Mesh property of type T
        /// \sa VertexProperty, CellProperty, EdgeProperty
        template <class T> class ModelProperty : public Property<T>
        {
        public:

            /// default constructor
            explicit ModelProperty() {}
            explicit ModelProperty(Property<T> p) : Property<T>(p) {}

            /// access the data stored for the mesh
            typename Property<T>::reference operator[](size_t idx)
            {
                return Property<T>::operator[](idx);
            }

            /// access the data stored for the mesh
            typename Property<T>::const_reference operator[](size_t idx) const
            {
                return Property<T>::operator[](idx);
            }
        };



    public: //------------------------------------------------------ iterator types

        /// this class iterates linearly over all vertices
        /// \sa vertices_begin(), vertices_end()
        /// \sa CellIterator, EdgeIterator, FaceIterator
        class VertexIterator
        {
        public:

            /// Default constructor
            VertexIterator(Vertex v=Vertex(), const PolyMesh* m=nullptr) : hnd_(v), mesh_(m)
            {
            }

            /// get the vertex the iterator refers to
            Vertex operator*()  const { return  hnd_; }

            /// are two iterators equal?
            bool operator==(const VertexIterator& rhs) const
            {
                return (hnd_==rhs.hnd_);
            }

            /// are two iterators different?
            bool operator!=(const VertexIterator& rhs) const
            {
                return !operator==(rhs);
            }

            /// pre-increment iterator
            VertexIterator& operator++()
            {
                ++hnd_.idx_;
                assert(mesh_);
                return *this;
            }

            /// pre-decrement iterator
            VertexIterator& operator--()
            {
                --hnd_.idx_;
                assert(mesh_);
                return *this;
            }

        private:
            Vertex  hnd_;
            const PolyMesh* mesh_;
        };


        /// this class iterates linearly over all edges
        /// \sa edges_begin(), edges_end()
        /// \sa VertexIterator, CellIterator, FaceIterator
        class EdgeIterator
        {
        public:

            /// Default constructor
            EdgeIterator(Edge e=Edge(), const PolyMesh* m=nullptr) : hnd_(e), mesh_(m)
            {
            }

            /// get the edge the iterator refers to
            Edge operator*()  const { return  hnd_; }

            /// are two iterators equal?
            bool operator==(const EdgeIterator& rhs) const
            {
                return (hnd_==rhs.hnd_);
            }

            /// are two iterators different?
            bool operator!=(const EdgeIterator& rhs) const
            {
                return !operator==(rhs);
            }

            /// pre-increment iterator
            EdgeIterator& operator++()
            {
                ++hnd_.idx_;
                assert(mesh_);
                return *this;
            }

            /// pre-decrement iterator
            EdgeIterator& operator--()
            {
                --hnd_.idx_;
                assert(mesh_);
                return *this;
            }

        private:
            Edge  hnd_;
            const PolyMesh* mesh_;
        };


        /// this class iterates linearly over all faces
        /// \sa faces_begin(), faces_end()
        /// \sa VertexIterator, CellIterator, EdgeIterator
        class HalffaceIterator
        {
        public:

            /// Default constructor
            HalffaceIterator(HalfFace f=HalfFace(), const PolyMesh* m=nullptr) : hnd_(f), mesh_(m)
            {
            }

            /// get the face the iterator refers to
            HalfFace operator*()  const { return  hnd_; }

            /// are two iterators equal?
            bool operator==(const HalffaceIterator& rhs) const
            {
                return (hnd_==rhs.hnd_);
            }

            /// are two iterators different?
            bool operator!=(const HalffaceIterator& rhs) const
            {
                return !operator==(rhs);
            }

            /// pre-increment iterator
            HalffaceIterator& operator++()
            {
                ++hnd_.idx_;
                assert(mesh_);
                return *this;
            }

            /// pre-decrement iterator
            HalffaceIterator& operator--()
            {
                --hnd_.idx_;
                assert(mesh_);
                return *this;
            }

        private:
            HalfFace  hnd_;
            const PolyMesh* mesh_;
        };


        /// this class iterates linearly over all faces
        /// \sa faces_begin(), faces_end()
        /// \sa VertexIterator, CellIterator, EdgeIterator
        class FaceIterator
        {
        public:

            /// Default constructor
            FaceIterator(Face f=Face(), const PolyMesh* m=nullptr) : hnd_(f), mesh_(m)
            {
            }

            /// get the face the iterator refers to
            Face operator*()  const { return  hnd_; }

            /// are two iterators equal?
            bool operator==(const FaceIterator& rhs) const
            {
                return (hnd_==rhs.hnd_);
            }

            /// are two iterators different?
            bool operator!=(const FaceIterator& rhs) const
            {
                return !operator==(rhs);
            }

            /// pre-increment iterator
            FaceIterator& operator++()
            {
                ++hnd_.idx_;
                assert(mesh_);
                return *this;
            }

            /// pre-decrement iterator
            FaceIterator& operator--()
            {
                --hnd_.idx_;
                assert(mesh_);
                return *this;
            }

        private:
            Face  hnd_;
            const PolyMesh* mesh_;
        };


        /// this class iterates linearly over all halfedges
        /// \sa tetrahedra_begin(), tetrahedra_end()
        /// \sa VertexIterator, EdgeIterator, FaceIterator
        class CellIterator
        {
        public:

            /// Default constructor
            CellIterator(Cell t=Cell(), const PolyMesh* m=nullptr) : hnd_(t), mesh_(m)
            {
            }

            /// get the halfedge the iterator refers to
            Cell operator*()  const { return  hnd_; }

            /// are two iterators equal?
            bool operator==(const CellIterator& rhs) const
            {
                return (hnd_==rhs.hnd_);
            }

            /// are two iterators different?
            bool operator!=(const CellIterator& rhs) const
            {
                return !operator==(rhs);
            }

            /// pre-increment iterator
            CellIterator& operator++()
            {
                ++hnd_.idx_;
                assert(mesh_);
                return *this;
            }

            /// pre-decrement iterator
            CellIterator& operator--()
            {
                --hnd_.idx_;
                assert(mesh_);
                return *this;
            }

        private:
            Cell  hnd_;
            const PolyMesh* mesh_;
        };


    public: //-------------------------- containers for C++11 range-based for loops

        /// this helper class is a container for iterating through all
        /// vertices using C++11 range-based for-loops.
        /// \sa vertices()
        class VertexContainer
        {
        public:
            VertexContainer(VertexIterator _begin, VertexIterator _end) : begin_(_begin), end_(_end) {}
            VertexIterator begin() const { return begin_; }
            VertexIterator end()   const { return end_;   }
        private:
            VertexIterator begin_, end_;
        };


        /// this helper class is a container for iterating through all
        /// edges using C++11 range-based for-loops.
        /// \sa edges()
        class EdgeContainer
        {
        public:
            EdgeContainer(EdgeIterator _begin, EdgeIterator _end) : begin_(_begin), end_(_end) {}
            EdgeIterator begin() const { return begin_; }
            EdgeIterator end()   const { return end_;   }
        private:
            EdgeIterator begin_, end_;
        };


        /// this helper class is a container for iterating through all
        /// faces using C++11 range-based for-loops.
        /// \sa faces()
        class HalffaceContainer
        {
        public:
            HalffaceContainer(HalffaceIterator _begin, HalffaceIterator _end) : begin_(_begin), end_(_end) {}
            HalffaceIterator begin() const { return begin_; }
            HalffaceIterator end()   const { return end_;   }
        private:
            HalffaceIterator begin_, end_;
        };


        /// this helper class is a container for iterating through all
        /// faces using C++11 range-based for-loops.
        /// \sa faces()
        class FaceContainer
        {
        public:
            FaceContainer(FaceIterator _begin, FaceIterator _end) : begin_(_begin), end_(_end) {}
            FaceIterator begin() const { return begin_; }
            FaceIterator end()   const { return end_;   }
        private:
            FaceIterator begin_, end_;
        };

        
        /// this helper class is a container for iterating through all
        /// halfedge using C++11 range-based for-loops.
        /// \sa halfedges()
        class CellContainer
        {
        public:
            CellContainer(CellIterator _begin, CellIterator _end) : begin_(_begin), end_(_end) {}
            CellIterator begin() const { return begin_; }
            CellIterator end()   const { return end_;   }
        private:
            CellIterator begin_, end_;
        };


    public: //-------------------------------------------- constructor / destructor

        /// \name Construct, destruct, assignment
        //@{

        /// default constructor
        PolyMesh();

        // destructor (is virtual, since we inherit from Geometry_representation)
        virtual ~PolyMesh();

        /// copy constructor: copies \c rhs to \c *this. performs a deep copy of all properties.
        PolyMesh(const PolyMesh& rhs) { operator=(rhs); }

        /// assign \c rhs to \c *this. performs a deep copy of all properties.
        PolyMesh& operator=(const PolyMesh& rhs);

        /// assign \c rhs to \c *this. does not copy custom properties.
        PolyMesh& assign(const PolyMesh& rhs);
        //@}

        //!@}
        //! \name File IO
        //!@{

        //! \brief Read a tetrahedral mesh from a ".tet" file \p filename.
        //! Mainly for quick debug purposes. Client code should use PolyMeshIO.
        //! \sa PolyMeshIO.
        bool read_tet(const std::string& filename);

        //! \brief Write a tetrahedral mesh to a ".tet" file \p filename.
        //! Mainly for quick debug purposes. Client code should use PolyMeshIO.
        //! \sa PolyMeshIO.
        bool write_tet(const std::string& filename) const;
        //@}

    public: //----------------------------------------------- add new vertex / face / cell

        /// \name Add new elements by hand
        //@{

        /// add a new vertex with position \c p
        Vertex add_vertex(const vec3& p);

        /// add a new quad connecting vertices \c v1, \c v2, \c v3, \c v4
        /// \sa add_triangle, add_face
        HalfFace add_face(const std::vector<Vertex>& vertices);

        /// add a new quad connecting vertices \c v1, \c v2, \c v3, \c v4
        /// \sa add_triangle, add_face
        HalfFace add_triangle(Vertex v1, Vertex v2, Vertex v3);

        /// add a new quad connecting vertices \c v1, \c v2, \c v3, \c v4
        /// \sa add_triangle, add_face
        HalfFace add_quad(Vertex v1, Vertex v2, Vertex v3, Vertex v4);

        /// add a new quad connecting vertices \c v1, \c v2, \c v3, \c v4
        /// \sa add_triangle, add_face
        Cell add_cell(const std::vector<HalfFace>& faces);

        /// add a new quad connecting vertices \c v1, \c v2, \c v3, \c v4
        /// \sa add_triangle, add_face
        Cell add_tetra(Vertex v1, Vertex v2, Vertex v3, Vertex v4);

        //@}

    public: //--------------------------------------------------- memory management

        /// \name Memory Management
        //@{

        /// returns number of vertices in the mesh
        unsigned int n_vertices() const { return (unsigned int) vprops_.size(); }
        /// returns number of tetrahedra in the mesh
        unsigned int n_cells() const { return (unsigned int) cprops_.size(); }
        /// returns number of edges in the mesh
        unsigned int n_edges() const { return (unsigned int) eprops_.size(); }
        /// returns number of faces in the mesh
        unsigned int n_faces() const { return (unsigned int) fprops_.size(); }
        /// returns number of halffaces in the mesh
        unsigned int n_halffaces() const { return (unsigned int) hprops_.size(); }

        /// \brief Removes all vertices, edges, faces, and properties (and resets garbage state).
        /// \details After calling this method, the mesh is the same as newly constructed. The additional properties
        /// (such as normal vectors) are also removed and must thus be re-added if needed.
        void clear();

        /// reserves memory (mainly used in file readers)
        void reserve(unsigned int nvertices, unsigned int ncells );

        /// resizes space for vertices, halfedges, edges, faces, and their currently
        /// associated properties.
        /// Note: ne is the number of edges. for halfedges, nh = 2 * ne. */
        void resize(unsigned int nv, unsigned int nt) {
            vprops_.resize(nv);
            cprops_.resize(nt);
        }
        

        /// return whether vertex \c v is valid, i.e. the index is stores it within the array bounds.
        bool is_valid(Vertex v) const
        {
            return (0 <= v.idx()) && (v.idx() < (int)n_vertices());
        }
        /// return whether edge \c e is valid, i.e. the index is stores it within the array bounds.
        bool is_valid(Edge e) const
        {
            return (0 <= e.idx()) && (e.idx() < (int)n_edges());
        }
        /// return whether face \c f is valid, i.e. the index is stores it within the array bounds.
        bool is_valid(HalfFace f) const
        {
            return (0 <= f.idx()) && (f.idx() < (int)n_halffaces());
        }
        /// return whether face \c f is valid, i.e. the index is stores it within the array bounds.
        bool is_valid(Face f) const
        {
            return (0 <= f.idx()) && (f.idx() < (int)n_faces());
        }
        /// return whether halfedge \c h is valid, i.e. the index is stores it within the array bounds.
        bool is_valid(Cell t) const
        {
            return (0 <= t.idx()) && (t.idx() < (int)n_cells());
        }

        //@}
        

    public: //--------------------------------------------------- property handling

        /// \name Property handling
        //@{

        /** add a vertex property of type \c T with name \c name and default value \c t.
         fails if a property named \c name exists already, since the name has to be unique.
         in this case it returns an invalid property */
        template <class T> VertexProperty<T> add_vertex_property(const std::string& name, const T t=T())
        {
            return VertexProperty<T>(vprops_.add<T>(name, t));
        }
        /** add a halfedge property of type \c T with name \c name and default value \c t.
         fails if a property named \c name exists already, since the name has to be unique.
         in this case it returns an invalid property */
        template <class T> CellProperty<T> add_tetra_property(const std::string& name, const T t=T())
        {
            return CellProperty<T>(cprops_.add<T>(name, t));
        }
        /** add a edge property of type \c T with name \c name and default value \c t.
         fails if a property named \c name exists already, since the name has to be unique.
         in this case it returns an invalid property */
        template <class T> EdgeProperty<T> add_edge_property(const std::string& name, const T t=T())
        {
            return EdgeProperty<T>(eprops_.add<T>(name, t));
        }
        /** add a face property of type \c T with name \c name and default value \c t.
         fails if a property named \c name exists already, since the name has to be unique.
         in this case it returns an invalid property */
        template <class T> HalfFaceProperty<T> add_halfface_property(const std::string& name, const T t=T())
        {
            return HalfFaceProperty<T>(hprops_.add<T>(name, t));
        }
        /** add a face property of type \c T with name \c name and default value \c t.
         fails if a property named \c name exists already, since the name has to be unique.
         in this case it returns an invalid property */
        template <class T> FaceProperty<T> add_face_property(const std::string& name, const T t=T())
        {
            return FaceProperty<T>(fprops_.add<T>(name, t));
        }
        /** add a model property of type \c T with name \c name and default value \c t.
         fails if a property named \c name exists already, since the name has to be unique.
         in this case it returns an invalid property */
        template <class T> ModelProperty<T> add_model_property(const std::string& name, const T t=T())
        {
            return ModelProperty<T>(mprops_.add<T>(name, t));
        }


        /** get the vertex property named \c name of type \c T. returns an invalid
         VertexProperty if the property does not exist or if the type does not match. */
        template <class T> VertexProperty<T> get_vertex_property(const std::string& name) const
        {
            return VertexProperty<T>(vprops_.get<T>(name));
        }
        /** get the edge property named \c name of type \c T. returns an invalid
         VertexProperty if the property does not exist or if the type does not match. */
        template <class T> EdgeProperty<T> get_edge_property(const std::string& name) const
        {
            return EdgeProperty<T>(eprops_.get<T>(name));
        }
        /** get the face property named \c name of type \c T. returns an invalid
         VertexProperty if the property does not exist or if the type does not match. */
        template <class T> HalfFaceProperty<T> get_halfface_property(const std::string& name) const
        {
            return HalfFaceProperty<T>(hprops_.get<T>(name));
        }
        /** get the face property named \c name of type \c T. returns an invalid
         VertexProperty if the property does not exist or if the type does not match. */
        template <class T> FaceProperty<T> get_face_property(const std::string& name) const
        {
            return FaceProperty<T>(fprops_.get<T>(name));
        }
        /** get the halfedge property named \c name of type \c T. returns an invalid
         VertexProperty if the property does not exist or if the type does not match. */
        template <class T> CellProperty<T> get_cell_property(const std::string& name) const
        {
            return CellProperty<T>(cprops_.get<T>(name));
        }
        /** get the model property named \c name of type \c T. returns an invalid
         ModelProperty if the property does not exist or if the type does not match. */
        template <class T> ModelProperty<T> get_model_property(const std::string& name) const
        {
            return ModelProperty<T>(mprops_.get<T>(name));
        }


        /** if a vertex property of type \c T with name \c name exists, it is returned.
         otherwise this property is added (with default value \c t) */
        template <class T> VertexProperty<T> vertex_property(const std::string& name, const T t=T())
        {
            return VertexProperty<T>(vprops_.get_or_add<T>(name, t));
        }
        /** if an edge property of type \c T with name \c name exists, it is returned.
         otherwise this property is added (with default value \c t) */
        template <class T> EdgeProperty<T> edge_property(const std::string& name, const T t=T())
        {
            return EdgeProperty<T>(eprops_.get_or_add<T>(name, t));
        }
        /** if a face property of type \c T with name \c name exists, it is returned.
         otherwise this property is added (with default value \c t) */
        template <class T> HalfFaceProperty<T> halfface_property(const std::string& name, const T t=T())
        {
            return HalfFaceProperty<T>(hprops_.get_or_add<T>(name, t));
        }
        /** if a face property of type \c T with name \c name exists, it is returned.
         otherwise this property is added (with default value \c t) */
        template <class T> FaceProperty<T> face_property(const std::string& name, const T t=T())
        {
            return FaceProperty<T>(fprops_.get_or_add<T>(name, t));
        }
        /** if a halfedge property of type \c T with name \c name exists, it is returned.
         otherwise this property is added (with default value \c t) */
        template <class T> CellProperty<T> cell_property(const std::string& name, const T t=T())
        {
            return CellProperty<T>(cprops_.get_or_add<T>(name, t));
        }
         /** if a model property of type \c T with name \c name exists, it is returned.
         otherwise this property is added (with default value \c t) */
        template <class T> ModelProperty<T> model_property(const std::string& name, const T t=T())
        {
            return ModelProperty<T>(mprops_.get_or_add<T>(name, t));
        }


        /// remove the vertex property \c p
        template<class T>
        bool remove_vertex_property(VertexProperty<T> &p) { return vprops_.remove(p); }

        /// remove the vertex property named \c n
        bool remove_vertex_property(const std::string &n) { return vprops_.remove(n); }

        /// remove the edge property \c p
        template<class T>
        bool remove_edge_property(EdgeProperty<T> &p) { return eprops_.remove(p); }

        /// remove the edge property named \c n
        bool remove_edge_property(const std::string &n) { return eprops_.remove(n); }

        /// remove the face property \c p
        template<class T>
        bool remove_halfface_property(HalfFaceProperty<T> &p) { return hprops_.remove(p); }

        /// remove the face property named \c n
        bool remove_halfface_property(const std::string &n) { return hprops_.remove(n); }

        /// remove the face property \c p
        template<class T>
        bool remove_face_property(FaceProperty<T> &p) { return fprops_.remove(p); }

        /// remove the face property named \c n
        bool remove_face_property(const std::string &n) { return fprops_.remove(n); }

        /// remove the halfedge property \c p
        template<class T>
        bool remove_cell_property(CellProperty<T> &p) { return cprops_.remove(p); }

        /// remove the halfedge property named \c n
        bool remove_cell_property(const std::string &n) { return cprops_.remove(n); }

        /// remove the model property \c p
        template<class T>
        bool remove_model_property(ModelProperty<T> &p) { return mprops_.remove(p); }

        /// remove the model property named \c n
        bool remove_model_property(const std::string &n) { return mprops_.remove(n); }

        /// rename a vertex property given its name
        bool rename_vertex_property(const std::string &old_name, const std::string &new_name) {
            return vprops_.rename(old_name, new_name);
        }

        /// rename a face property given its name
        bool rename_halfface_property(const std::string &old_name, const std::string &new_name) {
            return hprops_.rename(old_name, new_name);
        }

        /// rename a face property given its name
        bool rename_face_property(const std::string &old_name, const std::string &new_name) {
            return fprops_.rename(old_name, new_name);
        }

        /// rename an edge property given its name
        bool rename_edge_property(const std::string &old_name, const std::string &new_name) {
            return eprops_.rename(old_name, new_name);
        }

        /// rename a halfedge property given its name
        bool rename_cell_property(const std::string &old_name, const std::string &new_name) {
            return cprops_.rename(old_name, new_name);
        }

        /// rename a model property given its name
        bool rename_model_property(const std::string &old_name, const std::string &new_name) {
            return mprops_.rename(old_name, new_name);
        }


        /** get the type_info \c T of vertex property \p name. returns an typeid(void)
         if the property does not exist or if the type does not match. */
        const std::type_info& get_vertex_property_type(const std::string& name) const
        {
            return vprops_.get_type(name);
        }
        /** get the type_info \c T of edge property \p name. returns an typeid(void)
         if the property does not exist or if the type does not match. */
        const std::type_info& get_edge_property_type(const std::string& name) const
        {
            return eprops_.get_type(name);
        }
        /** get the type_info \c T of face property \p name. returns an typeid(void)
         if the property does not exist or if the type does not match. */
        const std::type_info& get_halfface_property_type(const std::string& name) const
        {
            return hprops_.get_type(name);
        }
        /** get the type_info \c T of face property \p name. returns an typeid(void)
         if the property does not exist or if the type does not match. */
        const std::type_info& get_face_property_type(const std::string& name) const
        {
            return fprops_.get_type(name);
        }
        /** get the type_info \c T of halfedge property \p name. returns an typeid(void)
         if the property does not exist or if the type does not match. */
        const std::type_info& get_cell_property_type(const std::string& name) const
        {
            return cprops_.get_type(name);
        }
        /** get the type_info \c T of model property \p name. returns an typeid(void)
         if the property does not exist or if the type does not match. */
        const std::type_info& get_model_property_type(const std::string& name) const
        {
            return mprops_.get_type(name);
        }


        /// returns the names of all vertex properties
        std::vector<std::string> vertex_properties() const
        {
            return vprops_.properties();
        }
        /// returns the names of all edge properties
        std::vector<std::string> edge_properties() const
        {
            return eprops_.properties();
        }
        /// returns the names of all face properties
        std::vector<std::string> halfface_properties() const
        {
            return hprops_.properties();
        }
        /// returns the names of all face properties
        std::vector<std::string> face_properties() const
        {
            return fprops_.properties();
        }
        /// returns the names of all halfedge properties
        std::vector<std::string> cell_properties() const
        {
            return cprops_.properties();
        }
        /// returns the names of all model properties
        std::vector<std::string> model_properties() const
        {
            return mprops_.properties();
        }

        /// prints the names of all properties to an output stream (e.g., std::cout).
        void property_stats(std::ostream &output) const;

        //@}


    public: //--------------------------------------------- iterators & circulators

        /// \name Iterators & Circulators
        //@{

        /// returns start iterator for vertices
        VertexIterator vertices_begin() const
        {
            return VertexIterator(Vertex(0), this);
        }

        /// returns end iterator for vertices
        VertexIterator vertices_end() const
        {
            return VertexIterator(Vertex(n_vertices()), this);
        }

        /// returns vertex container for C++11 range-based for-loops
        VertexContainer vertices() const
        {
            return VertexContainer(vertices_begin(), vertices_end());
        }

        /// returns start iterator for edges
        EdgeIterator edges_begin() const
        {
            return EdgeIterator(Edge(0), this);
        }

        /// returns end iterator for edges
        EdgeIterator edges_end() const
        {
            return EdgeIterator(Edge(n_edges()), this);
        }

        /// returns edge container for C++11 range-based for-loops
        EdgeContainer edges() const
        {
            return EdgeContainer(edges_begin(), edges_end());
        }

        /// returns start iterator for faces
        HalffaceIterator halffaces_begin() const
        {
            return HalffaceIterator(HalfFace(0), this);
        }

        /// returns end iterator for faces
        HalffaceIterator halffaces_end() const
        {
            return HalffaceIterator(HalfFace(n_halffaces()), this);
        }

        /// returns face container for C++11 range-based for-loops
        HalffaceContainer halffaces() const
        {
            return HalffaceContainer(halffaces_begin(), halffaces_end());
        }

        /// returns start iterator for faces
        FaceIterator faces_begin() const
        {
            return FaceIterator(Face(0), this);
        }

        /// returns end iterator for faces
        FaceIterator faces_end() const
        {
            return FaceIterator(Face(n_faces()), this);
        }

        /// returns face container for C++11 range-based for-loops
        FaceContainer faces() const
        {
            return FaceContainer(faces_begin(), faces_end());
        }

        /// returns start iterator for halfedges
        CellIterator cells_begin() const
        {
            return CellIterator(Cell(0), this);
        }

        /// returns end iterator for halfedges
        CellIterator cells_end() const
        {
            return CellIterator(Cell(n_cells()), this);
        }

        /// returns halfedge container for C++11 range-based for-loops
        CellContainer cells() const
        {
            return CellContainer(cells_begin(), cells_end());
        }
        //@}

    public: //--------------------------------------------- adjacency access

        /// \name Adjacency access
        //@{

        /// returns circulator for vertices around vertex \c v
        const std::set<Vertex>& vertices(Vertex v) const
        {
            return vconn_[v].vertices_;
        }

        /// returns the \c i'th halfedge of edge \c e. \c i has to be 0 or 1.
        HalfFace halfface(Face f, unsigned int i) const
        {
            assert(i<=1);
            return HalfFace((f.idx() << 1) + i);
        }

        HalfFace opposite(HalfFace f) const
        {
            return hconn_[f].opposite_;
        }

        /// returns the \c i'th vertex of edge \c e. \c i has to be 0 or 1.
        Vertex vertex(Edge e, unsigned int i) const
        {
            assert(i<=1);
            return econn_[e].vertices_[i];
        }

        /// returns circulator for vertices around vertex \c v
        const std::vector<Vertex>& vertices(HalfFace f) const
        {
            return hconn_[f].vertices_;
        }

        /// returns circulator for vertices around vertex \c v
        const std::vector<Vertex>& vertices(Face f) const
        {
            return vertices(halfface(f, 0));
        }

        /// returns circulator for vertices around vertex \c v
        const std::set<Vertex>& vertices(Cell t) const
        {
            return cconn_[t].vertices_;
        }

        /// returns circulator for vertices around vertex \c v
        const std::set<Edge>& edges(Vertex v) const
        {
            return vconn_[v].edges_;
        }

        /// returns circulator for vertices around vertex \c v
        const std::set<Edge>& edges(HalfFace f) const
        {
            return hconn_[f].edges_;
        }

        /// returns circulator for vertices around vertex \c v
        const std::set<Edge>& edges(Cell c) const
        {
            return cconn_[c].edges_;
        }
        
        /// returns circulator for faces around vertex \c v
        const std::set<HalfFace>& halffaces(Vertex v) const
        {
            return vconn_[v].halffaces_;
        }

        /// returns circulator for faces around vertex \c v
        const std::set<HalfFace>& halffaces(Edge e) const
        {
            return econn_[e].halffaces_;
        }

        /// returns circulator for faces around vertex \c v
        const std::vector<HalfFace>& halffaces(Cell t) const
        {
            return cconn_[t].halffaces_;
        }

        /// returns circulator for vertices around vertex \c v
        const std::set<Cell>& cells(Vertex v) const
        {
            return vconn_[v].cells_;
        }

        /// returns circulator for vertices around vertex \c v
        const std::set<Cell>& cells(Edge e) const
        {
            return econn_[e].cells_;
        }

        /// returns circulator for vertices around vertex \c v
        Cell cell(HalfFace f) const
        {
            return hconn_[f].cell_;
        }
        //@}

    public: //--------------------------------------------- higher-level operations

        /// \name Higher-level Topological Operations
        //@{

        /// returns whether the mesh a tetrahedral mesh.
        bool is_tetraheral_mesh() const;

        /// returns whether \c f is a boundary face, i.e., it is incident to only one cell.
        bool is_border(Face f) const
        {
            return is_border(halfface(f, 0)) || is_border(halfface(f, 1));
        }

        /// returns whether \c f is a boundary face, i.e., it is incident to only one cell.
        bool is_border(HalfFace h) const
        {
            return (!cell(h).is_valid());
        }

        /// find the edge (a,b)
        Edge find_edge(Vertex a, Vertex b) const;

        HalfFace find_face(const std::vector<Vertex>& vertices) const;

        /// returns whether \c f is degenerate
        bool is_degenerate(HalfFace f) const;
        //@}

    public: //------------------------------------------ geometry-related functions

        /// \name Geometry-related Functions
        //@{

        /// position of a vertex (read only)
        const vec3& position(Vertex v) const { return vpoint_[v]; }

        /// vector of vertex positions (read only)
        const std::vector<vec3>& points() const { return vpoint_.vector(); }

        /// compute face normals by calling compute_face_normal(HalfFace) for each face.
        void update_face_normals();

        /// compute normal vector of face \c f.
        vec3 compute_face_normal(HalfFace f) const;

        /// compute the length of edge \c e.
        float edge_length(Edge e) const;

        //@}


    private: //---------------------------------------------- allocate new elements

        /// allocate a new vertex, resize vertex properties accordingly.
        Vertex new_vertex()
        {
            vprops_.push_back();
            return Vertex(n_vertices()-1);
        }

        /// allocate a new edge, resize edge and halfedge properties accordingly.
        Edge new_edge(Vertex s, Vertex t)
        {
            assert(s != t);
            eprops_.push_back();
            Edge e = Edge(n_edges() - 1);
            econn_[e].vertices_ = {s, t};
            vconn_[s].edges_.insert(e);
            vconn_[t].edges_.insert(e);
            vconn_[s].vertices_.insert(t);
            vconn_[t].vertices_.insert(s);
            return e;
        }

        /// allocate a new face, resize face properties accordingly.
        HalfFace new_face()
        {
            fprops_.push_back();
            hprops_.push_back();
            hprops_.push_back();
            HalfFace f0(n_halffaces()-2);
            HalfFace f1(n_halffaces()-1);

            hconn_[f0].opposite_ = f1;
            hconn_[f1].opposite_ = f0;

            return f0;
        }

        /// allocate a new face, resize face properties accordingly.
        Cell new_cell()
        {
            cprops_.push_back();
            return Cell(n_cells()-1);
        }

    private: //------------------------------------------------------- private data

        PropertyContainer vprops_;
        PropertyContainer eprops_;
        PropertyContainer hprops_;
        PropertyContainer fprops_;
        PropertyContainer cprops_;
        PropertyContainer mprops_;

        VertexProperty<VertexConnectivity>      vconn_;
        EdgeProperty<EdgeConnectivity>          econn_;
        HalfFaceProperty<HalfFaceConnectivity>  hconn_;
        CellProperty<CellConnectivity>          cconn_;

        VertexProperty<vec3>    vpoint_;
        HalfFaceProperty<vec3>  fnormal_;
    };


    //------------------------------------------------------------ output operators

    inline std::ostream& operator<<(std::ostream& os, PolyMesh::Vertex v)
    {
        return (os << 'v' << v.idx());
    }
    
    inline std::ostream& operator<<(std::ostream& os, PolyMesh::Edge e)
    {
        return (os << 'e' << e.idx());
    }

    inline std::ostream& operator<<(std::ostream& os, PolyMesh::HalfFace h)
    {
        return (os << 'h' << h.idx());
    }

    inline std::ostream& operator<<(std::ostream& os, PolyMesh::Face f)
    {
        return (os << 'f' << f.idx());
    }

    inline std::ostream& operator<<(std::ostream& os, PolyMesh::Cell c)
    {
        return (os << 'c' << c.idx());
    }

} // namespace easy3d

#endif // EASY3D_CORE_POLYHEDRAL_MESH_H