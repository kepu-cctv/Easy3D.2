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
#include <easy3d/fileio/resources.h>
#include <easy3d/util/logging.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/gui/picker_model.h>

using namespace easy3d;


// This example shows how to
//		- override the file loading function of the default easy3d viewer to visualize textured meshes;




int main(int argc, char **argv) {
    // Initialize logging.
    logging::initialize();

    const float w_a1 = 1.0/3.0;
    const float w_a2 = 1.0/3.0;
    const float w_a3 = 1.0/3.0;
    const float w_assignments = 0.4;
    const float w_exam = 0.6;

    struct Student {
        Student(const std::string& _name, float _a1, float _a2, float _a3, float _exam) : name(_name), a1(_a1), a2(_a2), a3(_a3), exam(_exam) {}
        std::string name;
        float a1;
        float a2;
        float a3;
        float exam;
    };

    std::vector<Student> students = {
            Student("Rohit Ramlakhan", 80, 35, 70, 71.5),
            Student("Mihai-Alexandru Erbașu", 75, 64, 64, 58.5),
            Student("Nur An Nisa Milyana ", 60, 75, 85, 60.5),
            Student("Ellie Roy", 69, 80, 70, 70.5),
            Student("Vera Stevers", 68, 60, 69, 75),
            Student("Jos Feenstra", 65, 75, 80, 65),
    };

    for (const auto& s : students) {
        float grade = (s.a1 * w_a1 + s.a2 * w_a2 + s.a3 * w_a3) * w_assignments + s.exam * w_exam;
        std::cout << s.name << ": " << grade << std::endl;
    }

    return 0;






    try {
        const std::vector<std::string> files = {
//                resource::directory() + "/data/repair/non_manifold/complex_edges_1.off",
                resource::directory() + "/data/repair/non_manifold/complex_vertices.off",
                resource::directory() + "/data/repair/non_manifold/3_umbrellas.off",
        };

        // Create the viewer.
        Viewer viewer;
        for (const auto& name : files) {
            if (!viewer.add_model(name, true))
                LOG(FATAL) << "Error: failed to load model. Please make sure the file exists and format is correct.";
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
