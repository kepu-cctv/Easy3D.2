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


#include <easy3d/viewer/renderer.h>
#include <easy3d/viewer/drawable_points.h>
#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/viewer/setting.h>
#include <easy3d/viewer/tessellator.h>
#include <easy3d/viewer/texture.h>
#include <easy3d/core/random.h>
#include <easy3d/util/logging.h>

#include <cassert>


namespace easy3d {

    namespace renderer {


        void update_buffer(PointCloud *model, PointsDrawable *drawable) {
            assert(model);
            assert(drawable);

            // segmentation information has been stored as properties:
            //      - "v:primitive_type"  (one of PLANE, SPHERE, CYLINDER, CONE, TORUS, and UNKNOWN)
            //      - "v:primitive_index" (0, 1, 2...)
            auto primitive_type = model->get_vertex_property<int>("v:primitive_type");
            auto primitive_index = model->get_vertex_property<int>("v:primitive_index");
            if (primitive_type && primitive_index) { // model has segmentation information
                int num = 0;
                for (auto v : model->vertices())
                    num = std::max(num, primitive_index[v]);
                ++num;
                // assign each plane a unique color
                std::vector<vec3> color_table(num);
                for (auto &c : color_table)
                    c = random_color();

                std::vector<vec3> colors;
                for (auto v : model->vertices()) {
                    int idx = primitive_index[v];
                    if (primitive_type[v] == -1)
                        colors.push_back(vec3(0, 0, 0));
                    else
                        colors.push_back(color_table[idx]); // black for unkonwn type
                }
                drawable->update_color_buffer(colors);
                drawable->set_per_vertex_color(true);

                auto points = model->get_vertex_property<vec3>("v:point");
                drawable->update_vertex_buffer(points.vector());
                auto normals = model->get_vertex_property<vec3>("v:normal");
                if (normals)
                    drawable->update_normal_buffer(normals.vector());
            } else {
                auto points = model->get_vertex_property<vec3>("v:point");
                drawable->update_vertex_buffer(points.vector());
                auto normals = model->get_vertex_property<vec3>("v:normal");
                if (normals)
                    drawable->update_normal_buffer(normals.vector());
                auto colors = model->get_vertex_property<vec3>("v:color");
                if (colors) {
                    drawable->update_color_buffer(colors.vector());
                    drawable->set_per_vertex_color(true);
                } else {
                    drawable->set_default_color(setting::point_cloud_points_color);
                    drawable->set_per_vertex_color(false);
                }
            }
        }


        void update_buffer(SurfaceMesh *model, PointsDrawable *drawable) {
            auto points = model->get_vertex_property<vec3>("v:point");
            drawable->update_vertex_buffer(points.vector());
            drawable->set_default_color(setting::surface_mesh_vertices_color);
            drawable->set_per_vertex_color(false);
            drawable->set_point_size(setting::surface_mesh_vertices_point_size);
            drawable->set_impostor_type(PointsDrawable::SPHERE);
        }


        void update_buffer(SurfaceMesh *model, TrianglesDrawable *drawable) {
            // Priority:
            //  1. per-halfedge/vertex texture coordinates
            //  2. per-vertex texture coordinates
            //  3: per-face color
            //  4: per-vertex color
            //  5: uniform color
            auto halfedge_texcoords = model->get_halfedge_property<vec2>("h:texcoord");
            if (halfedge_texcoords) {
                update_buffer(model, drawable, halfedge_texcoords);
                return;
            }

            auto vertex_texcoords = model->get_vertex_property<vec2>("v:texcoord");
            if (vertex_texcoords) {
                update_buffer(model, drawable, vertex_texcoords);
                return;
            }

            auto face_colors = model->get_face_property<vec3>("f:color");
            if (face_colors) {
                update_buffer(model, drawable, face_colors);
                return;
            }

            auto vertex_colors = model->get_vertex_property<vec3>("v:color");
            if (vertex_colors) {
                update_buffer(model, drawable, vertex_colors);
                return;
            }

            // if reached here, we choose a uniform color.

            /**
             * We use the Tessellator to eliminate duplicated vertices. This allows us to take advantage of index buffer
             * to minimize the number of vertices sent to the GPU.
             */
            Tessellator tessellator;

            /**
             * For non-triangular surface meshes, all polygonal faces are internally triangulated to allow a unified
             * rendering APIs. Thus for performance reasons, the selection of polygonal faces is also internally
             * implemented by selecting triangle primitives using program shaders. This allows data uploaded to the GPU
             * for the rendering purpose be shared for selection. Yeah, performance gain!
             */
            auto triangle_range = model->face_property<std::pair<int, int> >("f:triangle_range");
            int count_triangles = 0;

            /**
             * Efficiency in switching between flat and smooth shading.
             * Easy3d always transfer vertex normals to GPU and the normals for flat shading are computed on the fly in
             * the fragment shader:
             *          normal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));
             * Then, by adding a boolean uniform 'smooth_shading' to the fragment shader, client code can easily switch
             * between flat and smooth shading without transferring different data to the GPU.
             */

            auto points = model->get_vertex_property<vec3>("v:point");
            model->update_vertex_normals();
            auto normals = model->get_vertex_property<vec3>("v:normal");

            for (auto face : model->faces()) {
                tessellator.begin_polygon(model->compute_face_normal(face));
                tessellator.set_winding_rule(Tessellator::NONZERO);  // or POSITIVE
                tessellator.begin_contour();
                for (auto h : model->halfedges(face)) {
                    Tessellator::Vertex vertex;
                    auto v = model->to_vertex(h);
                    vertex.append(points[v]);
                    vertex.append(normals[v]);
                    tessellator.add_vertex(vertex);
                }
                tessellator.end_contour();
                tessellator.end_polygon();

                std::size_t num = tessellator.num_triangles_in_last_polygon();
                triangle_range[face] = std::make_pair(count_triangles, count_triangles + num - 1);
                count_triangles += num;
            }

            std::vector<vec3> d_points, d_normals;
            const std::vector<Tessellator::Vertex *> &vts = tessellator.vertices();
            for (auto v :vts) {
                std::size_t offset = 0;
                d_points.emplace_back(v->data() + offset);
                offset += 3;
                d_normals.emplace_back(v->data() + offset);
            }

            const std::vector<unsigned int> &indices = tessellator.indices();

            drawable->update_vertex_buffer(d_points);
            drawable->update_index_buffer(indices);
            drawable->update_normal_buffer(d_normals);

            drawable->set_per_vertex_color(false);
            model->set_color_scheme(drawable, "uniform color");

            DLOG(INFO) << "num of vertices in model/sent to GPU: " << model->vertices_size() << "/" << d_points.size();
        }

        // with a per-face color
        void update_buffer(SurfaceMesh* model, TrianglesDrawable* drawable, SurfaceMesh::FaceProperty<vec3> fcolor) {
            assert(model);
            assert(drawable);
            assert(fcolor);

            /**
             * We use the Tessellator to eliminate duplicated vertices. This allows us to take advantage of index buffer
             * to minimize the number of vertices sent to the GPU.
             */
            Tessellator tessellator;

            /**
             * For non-triangular surface meshes, all polygonal faces are internally triangulated to allow a unified
             * rendering APIs. Thus for performance reasons, the selection of polygonal faces is also internally
             * implemented by selecting triangle primitives using program shaders. This allows data uploaded to the GPU
             * for the rendering purpose be shared for selection. Yeah, performance gain!
             */
            auto triangle_range = model->face_property<std::pair<int, int> >("f:triangle_range");
            int count_triangles = 0;

            /**
             * Efficiency in switching between flat and smooth shading.
             * Easy3d always transfer vertex normals to GPU and the normals for flat shading are computed on the fly in
             * the fragment shader:
             *          normal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));
             * Then, by adding a boolean uniform 'smooth_shading' to the fragment shader, client code can easily switch
             * between flat and smooth shading without transferring different data to the GPU.
             */

            auto points = model->get_vertex_property<vec3>("v:point");
            model->update_vertex_normals();
            auto normals = model->get_vertex_property<vec3>("v:normal");

            for (auto face : model->faces()) {
                tessellator.begin_polygon(model->compute_face_normal(face));
                tessellator.set_winding_rule(Tessellator::NONZERO);  // or POSITIVE
                tessellator.begin_contour();
                const vec3& color = fcolor[face];
                for (auto h : model->halfedges(face)) {
                    Tessellator::Vertex vertex;
                    auto v = model->to_vertex(h);
                    vertex.append(points[v]);
                    vertex.append(normals[v]);
                    vertex.append(color);
                    tessellator.add_vertex(vertex);
                }
                tessellator.end_contour();
                tessellator.end_polygon();

                std::size_t num = tessellator.num_triangles_in_last_polygon();
                triangle_range[face] = std::make_pair(count_triangles, count_triangles + num - 1);
                count_triangles += num;
            }

            std::vector<vec3> d_points, d_normals, d_colors;
            const std::vector<Tessellator::Vertex *> &vts = tessellator.vertices();
            for (auto v :vts) {
                std::size_t offset = 0;
                d_points.emplace_back(v->data() + offset);
                offset += 3;
                d_normals.emplace_back(v->data() + offset);
                offset += 3;
                d_colors.emplace_back(v->data() + offset);
            }

            const std::vector<unsigned int> &d_indices = tessellator.indices();

            drawable->update_vertex_buffer(d_points);
            drawable->update_index_buffer(d_indices);
            drawable->update_normal_buffer(d_normals);
            drawable->update_color_buffer(d_colors);

            drawable->set_per_vertex_color(true);
            model->set_color_scheme(drawable, "f:color");

            DLOG(INFO) << "num of vertices in model/sent to GPU: " << model->vertices_size() << "/" << d_points.size();
        }


        void update_buffer(SurfaceMesh* model, TrianglesDrawable* drawable, SurfaceMesh::FaceProperty<float> fscalar) {
            assert(model);
            assert(drawable);
            assert(fscalar);

            /**
             * We use the Tessellator to eliminate duplicated vertices. This allows us to take advantage of index buffer
             * to minimize the number of vertices sent to the GPU.
             */
            Tessellator tessellator;

            /**
             * For non-triangular surface meshes, all polygonal faces are internally triangulated to allow a unified
             * rendering APIs. Thus for performance reasons, the selection of polygonal faces is also internally
             * implemented by selecting triangle primitives using program shaders. This allows data uploaded to the GPU
             * for the rendering purpose be shared for selection. Yeah, performance gain!
             */
            auto triangle_range = model->face_property<std::pair<int, int> >("f:triangle_range");
            int count_triangles = 0;

            /**
             * Efficiency in switching between flat and smooth shading.
             * Easy3d always transfer vertex normals to GPU and the normals for flat shading are computed on the fly in
             * the fragment shader:
             *          normal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));
             * Then, by adding a boolean uniform 'smooth_shading' to the fragment shader, client code can easily switch
             * between flat and smooth shading without transferring different data to the GPU.
             */

            auto points = model->get_vertex_property<vec3>("v:point");
            model->update_vertex_normals();
            auto normals = model->get_vertex_property<vec3>("v:normal");

            float min_value = FLT_MAX;
            float max_value = -FLT_MAX;
            for (auto f : model->faces()) {
                float value = fscalar[f];
                min_value = std::min(value, min_value);
                max_value = std::max(value, max_value);
            }

            for (auto face : model->faces()) {
                tessellator.begin_polygon(model->compute_face_normal(face));
                tessellator.set_winding_rule(Tessellator::NONZERO);  // or POSITIVE
                tessellator.begin_contour();
                float coord = (fscalar[face] - min_value) / (max_value - min_value);

                for (auto h : model->halfedges(face)) {
                    Tessellator::Vertex vertex;
                    auto v = model->to_vertex(h);
                    vertex.append(points[v]);
                    vertex.append(normals[v]);
                    vertex.append(vec2(coord, 0.5));
                    tessellator.add_vertex(vertex);
                }
                tessellator.end_contour();
                tessellator.end_polygon();

                std::size_t num = tessellator.num_triangles_in_last_polygon();
                triangle_range[face] = std::make_pair(count_triangles, count_triangles + num - 1);
                count_triangles += num;
            }

            std::vector<vec3> d_points, d_normals;
            std::vector<vec2> d_texcoords;
            const std::vector<Tessellator::Vertex *> &vts = tessellator.vertices();
            for (auto v :vts) {
                std::size_t offset = 0;
                d_points.emplace_back(v->data() + offset);
                offset += 3;
                d_normals.emplace_back(v->data() + offset);
                offset += 3;
                d_texcoords.emplace_back(v->data() + offset);
            }

            const std::vector<unsigned int> &d_indices = tessellator.indices();

            drawable->update_vertex_buffer(d_points);
            drawable->update_index_buffer(d_indices);
            drawable->update_normal_buffer(d_normals);
            drawable->update_texcoord_buffer(d_texcoords);

            drawable->set_per_vertex_color(true);
            model->set_color_scheme(drawable, "scalar");

            DLOG(INFO) << "num of vertices in model/sent to GPU: " << model->vertices_size() << "/" << d_points.size();
        }


        void update_buffer(SurfaceMesh* model, TrianglesDrawable* drawable, SurfaceMesh::VertexProperty<vec3> vcolor) {
            assert(model);
            assert(drawable);
            assert(vcolor);

            /**
             * We use the Tessellator to eliminate duplicated vertices. This allows us to take advantage of index buffer
             * to minimize the number of vertices sent to the GPU.
             */
            Tessellator tessellator;

            /**
             * For non-triangular surface meshes, all polygonal faces are internally triangulated to allow a unified
             * rendering APIs. Thus for performance reasons, the selection of polygonal faces is also internally
             * implemented by selecting triangle primitives using program shaders. This allows data uploaded to the GPU
             * for the rendering purpose be shared for selection. Yeah, performance gain!
             */
            auto triangle_range = model->face_property<std::pair<int, int> >("f:triangle_range");
            int count_triangles = 0;

            /**
             * Efficiency in switching between flat and smooth shading.
             * Easy3d always transfer vertex normals to GPU and the normals for flat shading are computed on the fly in
             * the fragment shader:
             *          normal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));
             * Then, by adding a boolean uniform 'smooth_shading' to the fragment shader, client code can easily switch
             * between flat and smooth shading without transferring different data to the GPU.
             */
            auto points = model->get_vertex_property<vec3>("v:point");
            model->update_vertex_normals();
            auto normals = model->get_vertex_property<vec3>("v:normal");

            for (auto face : model->faces()) {
                tessellator.begin_polygon(model->compute_face_normal(face));
                tessellator.set_winding_rule(Tessellator::NONZERO);  // or POSITIVE
                tessellator.begin_contour();
                for (auto h : model->halfedges(face)) {
                    Tessellator::Vertex vertex;
                    auto v = model->to_vertex(h);
                    vertex.append(points[v]);
                    vertex.append(normals[v]);
                    vertex.append(vcolor[v]);
                    tessellator.add_vertex(vertex);
                }
                tessellator.end_contour();
                tessellator.end_polygon();

                std::size_t num = tessellator.num_triangles_in_last_polygon();
                triangle_range[face] = std::make_pair(count_triangles, count_triangles + num - 1);
                count_triangles += num;
            }

            std::vector<vec3> d_points, d_normals, d_colors;
            const std::vector<Tessellator::Vertex *> &vts = tessellator.vertices();
            for (auto v :vts) {
                std::size_t offset = 0;
                d_points.emplace_back(v->data() + offset);
                offset += 3;
                d_normals.emplace_back(v->data() + offset);
                offset += 3;
                d_colors.emplace_back(v->data() + offset);
            }

            const std::vector<unsigned int> &d_indices = tessellator.indices();

            drawable->update_vertex_buffer(d_points);
            drawable->update_index_buffer(d_indices);
            drawable->update_normal_buffer(d_normals);
            drawable->update_color_buffer(d_colors);

            drawable->set_per_vertex_color(true);
            model->set_color_scheme(drawable, "v:color");

            DLOG(INFO) << "num of vertices in model/sent to GPU: " << model->vertices_size() << "/" << d_points.size();
        }


        void update_buffer(SurfaceMesh* model, TrianglesDrawable* drawable, SurfaceMesh::VertexProperty<float> vscalar) {
            assert(model);
            assert(drawable);
            assert(vscalar);

            /**
             * We use the Tessellator to eliminate duplicated vertices. This allows us to take advantage of index buffer
             * to minimize the number of vertices sent to the GPU.
             */
            Tessellator tessellator;

            /**
             * For non-triangular surface meshes, all polygonal faces are internally triangulated to allow a unified
             * rendering APIs. Thus for performance reasons, the selection of polygonal faces is also internally
             * implemented by selecting triangle primitives using program shaders. This allows data uploaded to the GPU
             * for the rendering purpose be shared for selection. Yeah, performance gain!
             */
            auto triangle_range = model->face_property<std::pair<int, int> >("f:triangle_range");
            int count_triangles = 0;

            /**
             * Efficiency in switching between flat and smooth shading.
             * Easy3d always transfer vertex normals to GPU and the normals for flat shading are computed on the fly in
             * the fragment shader:
             *          normal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));
             * Then, by adding a boolean uniform 'smooth_shading' to the fragment shader, client code can easily switch
             * between flat and smooth shading without transferring different data to the GPU.
             */

            auto points = model->get_vertex_property<vec3>("v:point");
            model->update_vertex_normals();
            auto normals = model->get_vertex_property<vec3>("v:normal");

            float min_value = FLT_MAX;
            float max_value = -FLT_MAX;
            for (auto v : model->vertices()) {
                float value = vscalar[v];
                min_value = std::min(value, min_value);
                max_value = std::max(value, max_value);
            }

            for (auto face : model->faces()) {
                tessellator.begin_polygon(model->compute_face_normal(face));
                tessellator.set_winding_rule(Tessellator::NONZERO);  // or POSITIVE
                tessellator.begin_contour();
                for (auto h : model->halfedges(face)) {
                    Tessellator::Vertex vertex;
                    auto v = model->to_vertex(h);
                    vertex.append(points[v]);
                    vertex.append(normals[v]);

                    float coord = (vscalar[v] - min_value) / (max_value - min_value);
                    vertex.append(vec2(coord, 0.5));
                    tessellator.add_vertex(vertex);
                }
                tessellator.end_contour();
                tessellator.end_polygon();

                std::size_t num = tessellator.num_triangles_in_last_polygon();
                triangle_range[face] = std::make_pair(count_triangles, count_triangles + num - 1);
                count_triangles += num;
            }

            std::vector<vec3> d_points, d_normals;
            std::vector<vec2> d_texcoords;
            const std::vector<Tessellator::Vertex *> &vts = tessellator.vertices();
            for (auto v :vts) {
                std::size_t offset = 0;
                d_points.emplace_back(v->data() + offset);
                offset += 3;
                d_normals.emplace_back(v->data() + offset);
                offset += 3;
                d_texcoords.emplace_back(v->data() + offset);
            }

            const std::vector<unsigned int> &d_indices = tessellator.indices();

            drawable->update_vertex_buffer(d_points);
            drawable->update_index_buffer(d_indices);
            drawable->update_normal_buffer(d_normals);
            drawable->update_texcoord_buffer(d_texcoords);

            drawable->set_per_vertex_color(true);
            model->set_color_scheme(drawable, "scalar");

            DLOG(INFO) << "num of vertices in model/sent to GPU: " << model->vertices_size() << "/" << d_points.size();
        }


        void update_buffer(SurfaceMesh* model, TrianglesDrawable* drawable, SurfaceMesh::VertexProperty<vec2> vtexcoords) {
            assert(model);
            assert(drawable);
            assert(vtexcoords);

            /**
             * We use the Tessellator to eliminate duplicated vertices. This allows us to take advantage of index buffer
             * to minimize the number of vertices sent to the GPU.
             */
            Tessellator tessellator;

            /**
             * For non-triangular surface meshes, all polygonal faces are internally triangulated to allow a unified
             * rendering APIs. Thus for performance reasons, the selection of polygonal faces is also internally
             * implemented by selecting triangle primitives using program shaders. This allows data uploaded to the GPU
             * for the rendering purpose be shared for selection. Yeah, performance gain!
             */
            auto triangle_range = model->face_property<std::pair<int, int> >("f:triangle_range");
            int count_triangles = 0;

            /**
             * Efficiency in switching between flat and smooth shading.
             * Easy3d always transfer vertex normals to GPU and the normals for flat shading are computed on the fly in
             * the fragment shader:
             *          normal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));
             * Then, by adding a boolean uniform 'smooth_shading' to the fragment shader, client code can easily switch
             * between flat and smooth shading without transferring different data to the GPU.
             */

            auto points = model->get_vertex_property<vec3>("v:point");
            model->update_vertex_normals();
            auto normals = model->get_vertex_property<vec3>("v:normal");

            for (auto face : model->faces()) {
                tessellator.begin_polygon(model->compute_face_normal(face));
                tessellator.set_winding_rule(Tessellator::NONZERO);  // or POSITIVE
                tessellator.begin_contour();
                for (auto h : model->halfedges(face)) {
                    Tessellator::Vertex vertex;
                    auto v = model->to_vertex(h);
                    vertex.append(points[v]);
                    vertex.append(normals[v]);
                    vertex.append(vtexcoords[v]);
                    tessellator.add_vertex(vertex);
                }
                tessellator.end_contour();
                tessellator.end_polygon();

                std::size_t num = tessellator.num_triangles_in_last_polygon();
                triangle_range[face] = std::make_pair(count_triangles, count_triangles + num - 1);
                count_triangles += num;
            }

            std::vector<vec3> d_points, d_normals;
            std::vector<vec2> d_texcoords;
            const std::vector<Tessellator::Vertex *> &vts = tessellator.vertices();
            for (auto v :vts) {
                std::size_t offset = 0;
                d_points.emplace_back(v->data() + offset);
                offset += 3;
                d_normals.emplace_back(v->data() + offset);
                offset += 3;
                d_texcoords.emplace_back(v->data() + offset);
            }

            const std::vector<unsigned int> &d_indices = tessellator.indices();

            drawable->update_vertex_buffer(d_points);
            drawable->update_index_buffer(d_indices);
            drawable->update_normal_buffer(d_normals);
            drawable->update_texcoord_buffer(d_texcoords);

            drawable->set_per_vertex_color(true);
            model->set_color_scheme(drawable, "v:texcoord");

#if 0 // Model has texture coordinates, should we put a default texture?
            const std::string texture_file = resource::directory() + "/textures/checkerboard_gray.png";
            auto tex = Texture::create(texture_file, GL_REPEAT);
            drawable->set_texture(tex);
            drawable->set_texture_repeat(10);
            drawable->set_use_texture(true);
#endif

            DLOG(INFO) << "num of vertices in model/sent to GPU: " << model->vertices_size() << "/" << d_points.size();
        }


        void update_buffer(SurfaceMesh* model, TrianglesDrawable* drawable, SurfaceMesh::HalfedgeProperty<vec2> htexcoords) {
            assert(model);
            assert(drawable);
            assert(htexcoords);

            /**
             * We use the Tessellator to eliminate duplicated vertices. This allows us to take advantage of index buffer
             * to minimize the number of vertices sent to the GPU.
             */
            Tessellator tessellator;

            /**
             * For non-triangular surface meshes, all polygonal faces are internally triangulated to allow a unified
             * rendering APIs. Thus for performance reasons, the selection of polygonal faces is also internally
             * implemented by selecting triangle primitives using program shaders. This allows data uploaded to the GPU
             * for the rendering purpose be shared for selection. Yeah, performance gain!
             */
            auto triangle_range = model->face_property<std::pair<int, int> >("f:triangle_range");
            int count_triangles = 0;

            /**
             * Efficiency in switching between flat and smooth shading.
             * Easy3d always transfer vertex normals to GPU and the normals for flat shading are computed on the fly in
             * the fragment shader:
             *          normal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));
             * Then, by adding a boolean uniform 'smooth_shading' to the fragment shader, client code can easily switch
             * between flat and smooth shading without transferring different data to the GPU.
             */

            auto points = model->get_vertex_property<vec3>("v:point");
            model->update_vertex_normals();
            auto normals = model->get_vertex_property<vec3>("v:normal");

            for (auto face : model->faces()) {
                tessellator.begin_polygon(model->compute_face_normal(face));
                tessellator.set_winding_rule(Tessellator::NONZERO);  // or POSITIVE
                tessellator.begin_contour();
                for (auto h : model->halfedges(face)) {
                    Tessellator::Vertex vertex;
                    auto v = model->to_vertex(h);
                    vertex.append(points[v]);
                    vertex.append(normals[v]);
                    vertex.append(htexcoords[h]);
                    tessellator.add_vertex(vertex);
                }
                tessellator.end_contour();
                tessellator.end_polygon();

                std::size_t num = tessellator.num_triangles_in_last_polygon();
                triangle_range[face] = std::make_pair(count_triangles, count_triangles + num - 1);
                count_triangles += num;
            }

            std::vector<vec3> d_points, d_normals;
            std::vector<vec2> d_texcoords;
            const std::vector<Tessellator::Vertex *> &vts = tessellator.vertices();
            for (auto v :vts) {
                std::size_t offset = 0;
                d_points.emplace_back(v->data() + offset);
                offset += 3;
                d_normals.emplace_back(v->data() + offset);
                offset += 3;
                d_texcoords.emplace_back(v->data() + offset);
            }

            const std::vector<unsigned int> &d_indices = tessellator.indices();

            drawable->update_vertex_buffer(d_points);
            drawable->update_index_buffer(d_indices);
            drawable->update_normal_buffer(d_normals);
            drawable->update_texcoord_buffer(d_texcoords);

            drawable->set_per_vertex_color(true);
            model->set_color_scheme(drawable, "h:texcoord");

#if 0 // Model has texture coordinates, should we put a default texture?
            const std::string texture_file = resource::directory() + "/textures/checkerboard_gray.png";
            auto tex = Texture::create(texture_file, GL_REPEAT);
            drawable->set_texture(tex);
            drawable->set_texture_repeat(10);
            drawable->set_use_texture(true);
#endif

            DLOG(INFO) << "num of vertices in model/sent to GPU: " << model->vertices_size() << "/" << d_points.size();
        }


        void update_buffer(SurfaceMesh *model, LinesDrawable *drawable) {
            std::vector<unsigned int> indices;
            for (auto e : model->edges()) {
                SurfaceMesh::Vertex s = model->vertex(e, 0);
                SurfaceMesh::Vertex t = model->vertex(e, 1);
                indices.push_back(s.idx());
                indices.push_back(t.idx());
            }
            auto points = model->get_vertex_property<vec3>("v:point");
            drawable->update_vertex_buffer(points.vector());
            drawable->update_index_buffer(indices);
            drawable->set_default_color(setting::surface_mesh_edges_color);
            drawable->set_per_vertex_color(false);
            drawable->set_line_width(setting::surface_mesh_edges_line_width);
        }


        void update_buffer(Graph *model, PointsDrawable *drawable) {
            auto points = model->get_vertex_property<vec3>("v:point");
            drawable->update_vertex_buffer(points.vector());
            drawable->set_per_vertex_color(false);
            drawable->set_default_color(vec3(1.0f, 0.0f, 0.0f));
            drawable->set_point_size(15.0f);
            drawable->set_impostor_type(PointsDrawable::SPHERE);
        }


        void update_buffer(Graph *model, LinesDrawable *drawable) {
            auto points = model->get_vertex_property<vec3>("v:point");
            drawable->update_vertex_buffer(points.vector());

            std::vector<unsigned int> indices;
            for (auto e : model->edges()) {
                unsigned int s = model->from_vertex(e).idx();
                indices.push_back(s);
                unsigned int t = model->to_vertex(e).idx();
                indices.push_back(t);
            }
            drawable->update_index_buffer(indices);

            drawable->set_per_vertex_color(false);
            drawable->set_default_color(vec3(1.0f, 0.67f, 0.5f));
            drawable->set_line_width(3.0f);
            drawable->set_impostor_type(LinesDrawable::CYLINDER);
        }

    }

}
