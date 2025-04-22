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
#include <easy3d/core/point_cloud.h>
#include <easy3d/viewer/drawable_points.h>
#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/fileio/resources.h>
#include <easy3d/util/logging.h>



using namespace easy3d;


// This example shows how to
//		- rendering a vector field defined on a point cloud;
//		- use the viewer to visualize the drawable.


int main(int argc, char** argv) {
    // Initialize logging.
    logging::initialize(argv[0]);

    try {
        // Create the default Easy3D viewer.
        // Note: a viewer must be created before creating any drawables.
        Viewer viewer("Tutorial_403_VectorField");

        // Load point cloud data from a file
        const std::string file_name = resource::directory() + "/data/polyhedron.bin";
        PointCloud* model = dynamic_cast<PointCloud*>(viewer.add_model(file_name, true));
        if (!model) {
            LOG(ERROR) << "Error: failed to load model. Please make sure the file exists and format is correct.";
            return EXIT_FAILURE;
        }

        // The drawable created by default.
        auto points_drawable = model->points_drawable("vertices");
        points_drawable->set_point_size(6.0f);

        // Now let's create a drawable to visualize the point normals.
        auto normals = model->get_vertex_property<vec3>("v:normal");
        if (normals) {
            auto points = model->get_vertex_property<vec3>("v:point");

            // Get the bounding box of the model. Then we defined the length of the
            // normal vectors to be 5% of the bounding box diagonal.
            const Box3& box = model->bounding_box();
            float length = norm(box.max() - box.min()) * 0.05f;

            // Now let collects the two end points of each normal vector. So from
            // these points we can create a drawable to visualize the normal vectors.

            // Every consecutive two points represent a normal vector.
            std::vector<vec3> normal_points;
            for (auto v : model->vertices()) {
                const vec3& s = points[v];
                vec3 n = normals[v];
                n.normalize();
                const vec3& t = points[v] + n * length;
                normal_points.push_back(s);
                normal_points.push_back(t);
            }

            // Create a drawable for rendering the normal vectors.
            auto normals_drawable = model->add_lines_drawable("normals");
            // Upload the data to the GPU.
            normals_drawable->update_vertex_buffer(normal_points);
            // We will draw the normal vectors in green color
            normals_drawable->set_per_vertex_color(false);
            normals_drawable->set_default_color(vec3(0.0f, 1.0f, 0.0f));
        }
        else {
            LOG(ERROR) << "This point cloud does not have normal information. "
                "No vector field will be visualized.";
        }

        // Run the viewer
        viewer.run();
    }
    catch (const std::runtime_error &e) {
        LOG(ERROR) << "caught a fatal error: " + std::string(e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

