
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

#ifndef EASY3D_GUI_PICKER_SURFACE_MESH_H
#define EASY3D_GUI_PICKER_SURFACE_MESH_H

#include <easy3d/gui/picker.h>
#include <easy3d/core/surface_mesh.h>


namespace easy3d {

    class ShaderProgram;

    class SurfaceMeshPicker : public Picker {
    public:
        SurfaceMeshPicker(Camera *cam);

        ~SurfaceMeshPicker();

        //------------------ sensitivity  ---------------------

        int resolution() const { return hit_resolution_; }

        void set_resolution(int r) { hit_resolution_ = r; }

        //------------------ pick elements ---------------------

        /**
         * Pick elements of a surface mesh.
         * @param x The cursor x-coordinate, relative to the left edge of the viewer.
         * @param y	The cursor y-coordinate, relative to the top edge of the viewer.
         * @return The picked element.
         */
        SurfaceMesh::Face pick_face(SurfaceMesh *model, int x, int y);

        SurfaceMesh::Vertex pick_vertex(SurfaceMesh *model, int x, int y);

        SurfaceMesh::Halfedge pick_edge(SurfaceMesh *model, int x, int y);

        /**
         * Pick elements when a face is already picked.
         * @param x The cursor x-coordinate, relative to the left edge of the viewer.
         * @param y	The cursor y-coordinate, relative to the top edge of the viewer.
         * @return The picked element.
         * @attention This method must be called after calling to pick_face(). The result is valid only if the
         *            picked_face is valid.
         */
        // @attention call this version if you already picked the face
        SurfaceMesh::Vertex pick_vertex(SurfaceMesh *model, SurfaceMesh::Face picked_face, int x, int y);

        SurfaceMesh::Halfedge pick_edge(SurfaceMesh *model, SurfaceMesh::Face picked_face, int x, int y);

        //-------------------- query after picking ----------------------

        /**
         * Query the picked face.
         * @return The picked face.
         * @attention This method must be called after calling to one of the above pick element methods. The results is
         *            valid only if a face has been picked.
         */
        SurfaceMesh::Face picked_face() const;

        /**
         * Query the xyz coordinate of the picked position.
         * @return The xyz coordinate of the picked position.
         * @attention This method must be called after calling to one of the above pick element methods. The results is
         *            valid only if a face has been picked.
         */
        vec3 picked_point() const;

    private:
        // selection implemented in GPU (using shader program)
        SurfaceMesh::Face pick_facet_gpu(SurfaceMesh *model, int x, int y);

        // selection implemented in CPU (with OpenMP if supported)
        SurfaceMesh::Face pick_facet_cpu(SurfaceMesh *model, int x, int y);

        Plane3 face_plane(SurfaceMesh *model, SurfaceMesh::Face face) const;

        /**
         * TODO: check if this works also for non-convex faces.
         */
        inline bool do_intersect(SurfaceMesh *model, SurfaceMesh::Face picked_face, const OrientedLine3 &line) const {
            // Uses Plucker coordinates (see OrientedLine)
            Sign face_sign = ZERO;
            for (auto h : model->halfedges(picked_face)) {
                auto s = model->from_vertex(h);
                auto t = model->to_vertex(h);
                const OrientedLine3 edge_line(model->position(t), model->position(s));
                Sign sign = OrientedLine3::side(line, edge_line);
                if (sign != ZERO) {
                    if (face_sign != ZERO && sign != face_sign)
                        return false;
                    face_sign = sign;
                }
            }
            return true;
        }

    private:
        unsigned int hit_resolution_;     // in pixels

        ShaderProgram *program_;

        vec3 picked_point_;
        SurfaceMesh::Face picked_face_;

    };

}


#endif  // EASY3D_GUI_PICKER_SURFACE_MESH_H
