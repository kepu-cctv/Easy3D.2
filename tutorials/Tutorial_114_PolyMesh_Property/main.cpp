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

#include <easy3d/core/poly_mesh.h>
#include <easy3d/util/logging.h>


using namespace easy3d;


// This example shows how to create and access properties defined on a polyhedral mesh.
// We use per-face properties as example, you should be able to do similarly for per-edge/vertex/cell properties as well.


// the mesh created in the previous tutorial (so you can skip it)
PolyMesh *old_mesh_from_previous_example() {
    // Create a polyhedral mesh
    PolyMesh *mesh = new PolyMesh;

    // Add eight vertices
    PolyMesh::Vertex v0 = mesh->add_vertex(vec3(-1.0, 0.0, 0.0));
    PolyMesh::Vertex v1 = mesh->add_vertex(vec3(0.0, 0.0, 1.0));
    PolyMesh::Vertex v2 = mesh->add_vertex(vec3(1.0, 0.0, 0.0));
    PolyMesh::Vertex v3 = mesh->add_vertex(vec3(0.0, 0.0, -1.0));
    PolyMesh::Vertex v4 = mesh->add_vertex(vec3(0.0, 1.0, 0.0));

    std::vector<PolyMesh::Vertex> vertices;

    // Add faces
    vertices.push_back(v0);
    vertices.push_back(v1);
    vertices.push_back(v4);
    PolyMesh::HalfFace h0 = mesh->add_face(vertices);

    vertices.clear();
    vertices.push_back(v1);
    vertices.push_back(v2);
    vertices.push_back(v4);
    PolyMesh::HalfFace h1 = mesh->add_face(vertices);

    vertices.clear();
    vertices.push_back(v0);
    vertices.push_back(v1);
    vertices.push_back(v2);
    PolyMesh::HalfFace h2 = mesh->add_face(vertices);

    vertices.clear();
    vertices.push_back(v0);
    vertices.push_back(v4);
    vertices.push_back(v2);
    PolyMesh::HalfFace h3 = mesh->add_face(vertices);

    vertices.clear();
    vertices.push_back(v0);
    vertices.push_back(v4);
    vertices.push_back(v3);
    PolyMesh::HalfFace h4 = mesh->add_face(vertices);

    vertices.clear();
    vertices.push_back(v2);
    vertices.push_back(v3);
    vertices.push_back(v4);
    PolyMesh::HalfFace h5 = mesh->add_face(vertices);

    vertices.clear();
    vertices.push_back(v0);
    vertices.push_back(v2);
    vertices.push_back(v3);
    PolyMesh::HalfFace h6 = mesh->add_face(vertices);

    std::vector<PolyMesh::HalfFace> halffaces;

    // Add first tetrahedron
    halffaces.push_back(mesh->opposite(h0));
    halffaces.push_back(mesh->opposite(h1));
    halffaces.push_back(h2);
    halffaces.push_back(mesh->opposite(h3));
    mesh->add_cell(halffaces);

    // Add second tetrahedron
    halffaces.clear();
    halffaces.push_back(mesh->opposite(h4));
    halffaces.push_back(mesh->opposite(h5));
    halffaces.push_back(h3);
    halffaces.push_back(h6);
    mesh->add_cell(halffaces);

    return mesh;
}


int main(int argc, char** argv) {
    // Initialize logging.
    logging::initialize();

    // Create mesh object
    PolyMesh *mesh = old_mesh_from_previous_example();

	// We add a per-face property "f:normal" storing the normal of each face
    PolyMesh::FaceProperty<vec3> center = mesh->add_face_property<vec3>("f:center");

	// for each face, we compute the face center.
	for (auto f : mesh->faces()) {
        vec3 c(0, 0, 0);
        for (auto v : mesh->vertices(f))
            c += mesh->position(v);
        center[f] = c / mesh->vertices(f).size();
	}

    // now we print the face center for each face.
    for (auto f : mesh->faces()) {
        std::cout << "center of " << f << ": " << center[f] << std::endl;
    }

	delete mesh;

    return EXIT_SUCCESS;
}

