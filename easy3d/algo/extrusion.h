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


#ifndef EASY3D_ALGO_EXTRUSION_H
#define EASY3D_ALGO_EXTRUSION_H

#include <string>

#include <easy3d/core/types.h>


namespace easy3d {

    class SurfaceMesh;

    /**
     * Extrude a 3D surface mesh from a set of simple contours.
     * @param contours The input contours. They must be simple, i.e.,
     *  - free of intersections,
     *  - CCW contours defining the outer boundary and CW contours defining holes.
     * @param height The height (in the Z direction) of the extruded 3D model.
     * @return The extruded surface mesh model. NULL on failure.
     */
    SurfaceMesh *extrude(const std::vector<Polygon2> &simple_contours, float height);

    /**
     * Extrude a 3D surface mesh from a set of simple contours.
     * @param contours The input contours. They must be simple, i.e.,
     *  - free of intersections,
     *  - CCW contours defining the outer boundary and CW contours defining holes.
     * @param height The height (in the Z direction) of the extruded 3D model.
     * @return True on success and false on failure.
     */
    bool extrude(SurfaceMesh *mesh, const std::vector<Polygon2> &simple_contours, float height);

}


#endif  // EASY3D_ALGO_EXTRUSION_H
