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

#include <easy3d/viewer/viewer.h>
#include <easy3d/viewer/camera.h>
#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/fileio/resources.h>
#include <easy3d/core/types.h>
#include <easy3d/util/logging.h>


using namespace easy3d;


// The use of drawables for visualization is quite flexible.
// Drawables are typically created for rendering 3D models
// (e.g., point clouds, meshes, graphs) and a 3D model is
// usually loaded from a file or generated by an algorithm.
// Easy3D also allows to visualize drawables without a model.

// This example shows how to
//      - visualize 3D data without explicitly defining a model,
//        (i.e., rendering drawables directly);
//		- create a drawable for a specific rendering purpose;
//		- use the viewer to visualize the drawable.


int main(int argc, char** argv) {
    // Initialize logging.
    logging::initialize();

    try {
        // Create the default Easy3D viewer.
        // Note: a viewer must be created before creating any drawables.
        Viewer viewer("Tutorial_401_Drawable");

        // We use the points and indices of the bunny model.
        const std::vector<vec3>& vertices = resource::bunny_vertices;
        // Each consecutive 3 indices represent a triangle.
        const std::vector<unsigned int>& indices = resource::bunny_indices;

        // To create a TrianglesDrawable to visualize the surface, we need to send
        // the point positions and the vertex indices of the faces to the GPU.
		TrianglesDrawable* surface = new TrianglesDrawable("faces");
		// Upload the vertex positions of the surface to the GPU.
		surface->update_vertex_buffer(vertices);
		// Upload the vertex indices of the surface to the GPU.
        surface->update_index_buffer(indices);

        // Add the drawable to the viewer
        viewer.add_drawable(surface);

        //-------------------------------------------------------------
        // Of course you can create as many drawables as you need.
        // Here, we show how to create a LinesDrawable to visualize the
        // bounding box of the bunny model.
        LinesDrawable* bbox_drawable = new LinesDrawable("bbox");
        const Box3& box = geom::bounding_box<Box3, std::vector<vec3>::const_iterator>(vertices.begin(), vertices.end());
        float xmin = box.min(0);	float xmax = box.max(0);
        float ymin = box.min(1);	float ymax = box.max(1);
        float zmin = box.min(2);	float zmax = box.max(2);
        // The eight vertices of the bounding box.
        const std::vector<vec3> bbox_points = {
            vec3(xmin, ymin, zmax), vec3(xmax, ymin, zmax),
            vec3(xmin, ymax, zmax),	vec3(xmax, ymax, zmax),
            vec3(xmin, ymin, zmin),	vec3(xmax, ymin, zmin),
            vec3(xmin, ymax, zmin),	vec3(xmax, ymax, zmin)
        };
        const std::vector<unsigned int> bbox_indices = {
            0, 1, 2, 3, 4, 5, 6, 7,
            0, 2, 4, 6, 1, 3, 5, 7,
            0, 4, 2, 6, 1, 5, 3, 7
        };

        // Upload the vertex positions of the bounding box to the GPU.
        bbox_drawable->update_vertex_buffer(bbox_points);
        // Upload the vertex indices of the bounding box to the GPU.
        bbox_drawable->update_index_buffer(bbox_indices);
        bbox_drawable->set_default_color(vec3(1.0f, 0.0f, 0.0f));	// red color
        bbox_drawable->set_line_width(5.0f);

		// Add the drawable to the viewer
		viewer.add_drawable(bbox_drawable);
		viewer.fit_screen();

        // Run the viewer
        viewer.run();

        // Delete the mesh (i.e., release memory)? No. The viewer will do this.
        // delete mesh;
    } catch (const std::runtime_error &e) {
        LOG(ERROR) << "caught a fatal error: " + std::string(e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

