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

#ifndef EASY3D_DRAWABLE_TRIANGLES_H
#define EASY3D_DRAWABLE_TRIANGLES_H

#include <easy3d/viewer/drawable.h>


namespace easy3d {


    // The drawable for rendering a set of triangles, e.g., the surface of a triangular mesh.
    // NOTE: it surports triangles only. To visualize general polygons, the vertex coordinates
    //       and properties (e.g., color, normal) should be provided as consequtive triplets
    //       in an array to be transfered to GPU. See update_vertex_buffer().
    class TrianglesDrawable : public Drawable {
	public:
        TrianglesDrawable(const std::string& name = "")
            : Drawable(name)
            , smooth_shading_(false)
            , opacity_(0.6f)
        {
			default_color_ = vec3(0.4f, 0.8f, 0.8f);
		}
        DrawableType type() const override;

        bool smooth_shading() const { return smooth_shading_; }
        void set_smooth_shading(bool b) { smooth_shading_ = b; }

        /**
         * @brief Query the opacity of the drawable, in the range [0.0, 1.0].
         * @return The opacity of the drawable.
         */
        float opacity() const { return opacity_; }

        /**
         * @brief Set the opacity of the drawable.
         * @param opacity The new opacity value, in the range [0.0, 1.0].
         */
        void set_opacity(float opacity) { opacity_ = opacity; }

		// The selection of a polygonal face is internally implemented by selecting triangle
		// primitives using shaders. So I need a way to map back to the original polygons.
		// \param indices: indices[i] are the triangle indices of the i'th face.
		void set_triangle_indices(const std::vector< std::vector<unsigned int> >& indices);
		const std::vector< std::vector<unsigned int> >& triangle_indices() { return indices_; }

		// a face (i.e., polygon) is internally rendered as multiple triangles
		void get_highlighted_trangles_range(int& tri_min, int& tri_max) const;

		// set/query if a facet is selected.
		// NOTE: a face is selected if all its vertices are selected.
		inline void set_selected(std::size_t face_idx, bool b);
		inline bool is_selected(std::size_t face_idx) const;
        int num_selected() const;

        // Rendering.
        virtual void draw(const Camera* camera, bool with_storage_buffer = false) const override;

    protected:

        // without texture
        void _draw_triangles(const Camera* camera, bool with_storage_buffer) const;

        // textured
        void _draw_triangles_with_texture(const Camera* camera, bool with_storage_buffer) const;

	private:
		std::vector< std::vector<unsigned int> > indices_;

        bool    smooth_shading_;
        float   opacity_;
	};

}


#endif  // EASY3D_DRAWABLE_TRIANGLES_H
