/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work,
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan.
*           Easy3D: an easy-to-use, efficient, and open-source C++
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "transparency.h"
#include <easy3d/viewer/model.h>
#include <easy3d/viewer/drawable.h>


// This example shows how to renders a surface mesh with transparency effect 
// using the following techniques 
//		  - Average Color Blending
//		  - Dual Depth Peeling

int main(int /*argc*/, char** /*argv*/) {
    // Create the viewer.
    TutorialTransparency viewer("Tutorial_405_Transparency");

    const std::string file = "../../Easy3D-Mapple/data/hand.poly";
    easy3d::Model* model = viewer.open(file);
    if (model) {
        auto drawable = model->triangles_drawable("surface");
        drawable->set_default_color(easy3d::vec3(0.6f, 0.6f, 1.0f));
    }
    else
        std::cerr << "Error: failed load mesh. Please make sure the file exists and format is correct." << std::endl;

	// Run the viewer
    viewer.run();

    return EXIT_SUCCESS;
}

