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


#include <easy3d/algo/text_mesher.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/curve.h>
#include <easy3d/algo/extrusion.h>
#include <easy3d/algo/tessellator.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/string.h>
#include <easy3d/util/progress.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <3rd_party/stb/stb_truetype.h>


namespace easy3d {


#define get_font(x)      (reinterpret_cast<stbtt_fontinfo*>(x))


    TextMesher::TextMesher(const std::string &font_file, int font_size)
            : bezier_steps_(4)
    {
        font_ = new stbtt_fontinfo;
        set_font(font_file, font_size);
    }


    TextMesher::~TextMesher() {
        delete get_font(font_);
    }


    void TextMesher::set_font(const std::string &font_file, int font_size) {
        if (!file_system::is_file(font_file)) {
            LOG(WARNING) << "font file does not exist: " << font_file;
            return;
        }

        if (font_file == font_file_ && font_size == font_size_)
            return;

        ready_ = false;

        auto read_font_file = [](const std::string& font_file, unsigned char **out) -> int {
            FILE *fp = fopen(font_file.c_str(), "rb");
            if (!fp)
                return 0;
            fseek(fp, 0, SEEK_END);
            int size = ftell(fp);
            rewind(fp);
            *out = (unsigned char *) malloc(size + 1);
            fread(*out, size, 1, fp);
            (*out)[size] = 0;
            fclose(fp);
            return size;
        };

        // load font
        unsigned char *ttf;
        if (read_font_file(font_file.c_str(), &ttf) == 0) {
            LOG(ERROR) << "failed loading font file: " << font_file;
            return;
        }

        int font_offset = stbtt_GetFontOffsetForIndex(ttf, 0);
        if (font_offset != 0) {
            LOG(ERROR) << "invalid font file";
            return;
        }

        if (0 == stbtt_InitFont(get_font(font_), ttf, font_offset)) {
            LOG(ERROR) << "init font (building font cache) failed";
            return;
        }

        font_file_ = font_file;
        font_size_ = font_size;
        ready_ = true;
    }


    bool TextMesher::_generate_contours(int codepoint, float& x, float& y, std::vector<Polygon2>& contours) {
        int glyph_index = stbtt_FindGlyphIndex(get_font(font_), codepoint);
        if (glyph_index == 0) {
            LOG(WARNING) << "given font does not support character " << string::to_string({wchar_t(codepoint)});
            return false;
        }

        std::size_t old_num = contours.size();

        stbtt_vertex *vertices = nullptr ;
        const int num_verts = stbtt_GetGlyphShape(get_font(font_), glyph_index, &vertices);

        int contour_begin_index = 0;
        while (contour_begin_index < num_verts) {
            stbtt_vertex *next_contour_begin = std::find_if(vertices + contour_begin_index + 1, vertices + num_verts,
                                                          [](const stbtt_vertex &p) { return p.type == STBTT_vmove; });
            const int next_contour_first_index = std::distance(vertices, next_contour_begin);
            const int current_contour_last_index = next_contour_first_index - 1;

            Polygon2 contour;
            for (int vi = contour_begin_index; vi < current_contour_last_index; ++vi) {
                stbtt_vertex *v1 = vertices + vi;
                stbtt_vertex *v2 = vertices + vi + 1;

                const vec2 p1(v1->x + x, v1->y + y);
                const vec2 p2(v2->x + x, v2->y + y);
                const vec2 pc(v2->cx + x, v2->cy + y);
                const vec2 pc1(v2->cx1 + x, v2->cy1 + y);

                if (v2->type == STBTT_vline) // line
                    contour.push_back(p1);
                else if (v2->type == STBTT_vcurve)  //quadratic Bezier
                    curve::quadratic(p1, pc, p2, contour, bezier_steps_);
                else if (v2->type == STBTT_vcubic)  //cubic Bezier
                    curve::cubic(p1, pc, pc1, p2, contour, bezier_steps_);
                else
                    LOG(ERROR) << "unrecognized contour point type";
            }

#if 0
            // ignore tiny contours (some fonts even have degenerate contours)
            if (contour.area() >= font_size_ * font_size_ * 0.001)
#endif
                contours.push_back(contour);

            contour_begin_index = next_contour_first_index;
        }

        stbtt_FreeShape(get_font(font_), vertices);

        int advanceWidth, leftSideBearing;
        stbtt_GetCodepointHMetrics(get_font(font_), codepoint, &advanceWidth, &leftSideBearing);
        x += advanceWidth - leftSideBearing;

//        int ascent, descent, line_gap;
//        stbtt_GetFontVMetrics(get_font(font_), &ascent, &descent, &line_gap);
//        y += (ascent - descent + line_gap);

        return contours.size() > old_num;
    }


    bool TextMesher::_generate_contours(const std::wstring &text, float x, float y,
                                        std::vector< std::vector<Polygon2> > &results, bool collision_free) {
        if (!ready_)
            return false;

        if (collision_free) {
            std::vector<Polygon2> all_contours;
            for (int i = 0; i < text.size(); ++i) {
                //std::cout << i << ": " << string::to_string({text[i]}) << ", int value: " << int(text[i]) << std::endl;
                std::vector<Polygon2> contours;
                const int codepoint = static_cast<int>(text[i]);
                if (_generate_contours(codepoint, x, y, contours)) {
                    if (collision_free) {
                        // resolve intersections and determine interior/exterior for each char.
                        csg::tessellate(contours, Tessellator::WINDING_ODD);
                        all_contours.insert(all_contours.end(), contours.begin(), contours.end());
                    }
                }
            }
            // compute the union of all characters.
            csg::tessellate(all_contours, Tessellator::WINDING_NONZERO); // the union of the neighboring chars
            results.push_back(all_contours);
        }
        else {
            for (int i = 0; i < text.size(); ++i) {
                //std::cout << i << ": " << string::to_string({text[i]}) << ", int value: " << int(text[i]) << std::endl;
                std::vector<Polygon2> contours;
                if (_generate_contours(text[i], x, y, contours)) {
                    // resolve intersections and determine interior/exterior for each char.
                    csg::tessellate(contours, Tessellator::WINDING_ODD);
                    results.push_back(contours);
                }
            }
        }

        return !results.empty();
    }


    bool TextMesher::generate(const std::string &text, float x, float y, std::vector< std::vector<Polygon2> > &contours, bool collision_free) {
        const std::wstring the_text = string::to_wstring(text);
        return _generate_contours(the_text, x, y, contours, collision_free);
    }


    bool TextMesher::generate(SurfaceMesh *mesh, const std::string &text, float x, float y, float height, bool collision_free) {
        if (!mesh)
            return false;

        // The std::string class handles bytes independently of the encoding used. If used to handle sequences of
        // multi-byte or variable-length characters (such as UTF-8), all members of this class (such as length or size),
        // as well as its iterators, will still operate in terms of bytes (not actual encoded characters).
        // So I convert it to a muilti-byte character string
        const std::wstring the_text = string::to_wstring(text);
        return _generate(mesh, the_text, x, y, height, collision_free);
    }


    bool TextMesher::_generate(SurfaceMesh *mesh, const std::wstring &text, float x, float y, float height, bool collision_free) {
        if (!ready_)
            return false;

        std::vector< std::vector<Polygon2> > contours;
        if (!_generate_contours(text, x, y, contours, collision_free)) {
            LOG(WARNING) << "no contour generated from the text using the specified font";
            return false;
        }

        ProgressLogger progress(contours.size());
        for (const auto& contour : contours) {
            extrude(mesh, contour, height);
            progress.next();
        }

        return mesh->n_faces() > 0;
    }


    SurfaceMesh *TextMesher::generate(const std::string &text, float x, float y, float extrude, bool collision_free) {
        if (!ready_)
            return nullptr;

        SurfaceMesh *mesh = new SurfaceMesh;
        if (generate(mesh, text, x, y, extrude, collision_free))
            return mesh;
        else {
            delete mesh;
            return nullptr;
        }
    }

}