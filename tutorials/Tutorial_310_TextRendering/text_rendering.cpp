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

#include "text_rendering.h"

#include <easy3d/viewer/opengl_text.h>
#include <easy3d/util/file_system.h>
#include <easy3d/fileio/resources.h>
#include <easy3d/core/random.h>

#include <3rd_party/glfw/include/GLFW/glfw3.h>	// for the KEYs


using namespace easy3d;


TextRendering::TextRendering(const std::string &title)
        : Viewer(title)
        , font_size_delta_(0)
{
    set_background_color(vec3(1, 1, 1));
}


std::string TextRendering::usage() const {
    return ("----------------- Text Rendering usage ----------------- \n"
            "Press '+/-' to increase/decrease font size\n"
            "Press 'up/down' to increase/decrease character spacing\n"
            "Press key 'space' to enable/disable kerning\n"
            "----------------------------------------------------------- \n");
}



void TextRendering::init() {
    Viewer::init();

    texter_ = new OpenGLText(dpi_scaling());
    
    std::vector<std::string> files;
    file_system::get_directory_entries(resource::directory() + "/fonts/", files, false);
    for (const auto& file : files) {
        if (file_system::extension(file) == "ttf") {
            texter_->add_font(resource::directory() + "/fonts/" + file);
            colors_.push_back(random_color());
        }
    }

    const auto& names = texter_->font_names();
    std::cout << "available fonts: " << std::endl;
    for (std::size_t i =0; i< names.size(); ++i)
        std::cout << "\tfont " << i << ": " << names[i] << std::endl;
}


void TextRendering::cleanup() {
    Viewer::cleanup();
    delete texter_;
}


void TextRendering::draw() const {
    Viewer::draw();

#if 0
    texter_->draw_multi_line("This example shows how to\nshows how to\nrender text in Easy3D. It is quite simple. It is quite simple.",
                             5 * dpi_scaling(), 5 * dpi_scaling(), 40 + font_size_delta_, OpenGLText::Align::ALIGN_CENTER, width());
#else
    texter_->draw("--- This example shows how to render text in Easy3D ---", 50 * dpi_scaling(), 50 * dpi_scaling(), 40 + font_size_delta_, 0);

    const float font_size = 35.0f + font_size_delta_;
    float x = 50.0f;
    float y = 120.0f;

    const int num_fonts = texter_->num_fonts();
    const float font_height = texter_->font_height(font_size);

    float next_x = 0.0f;
    for (int i = 0; i < num_fonts; ++i) {
        if (i % 2 == 0) {
            next_x = texter_->draw("Easy3D makes 3D easy! ", x * dpi_scaling(), y * dpi_scaling(), font_size, i, colors_[i]);
        }
        else {
            texter_->draw("I Love Easy3D!", next_x * dpi_scaling(), y * dpi_scaling(), font_size, i, colors_[i]);
            y += font_height * 1.5;
        }
    }
#endif
}


bool TextRendering::key_press_event(int key, int modifiers) {
    if (key == GLFW_KEY_SPACE) {
        const bool kerning = texter_->kerning();
        texter_->set_kerning(!kerning);
        update();
        return true;
    }

    else if (key == GLFW_KEY_MINUS) {
        font_size_delta_ = std::max(font_size_delta_ - 1.0f, -10.0f);
        update();
        return true;
    }
    else if (key == GLFW_KEY_EQUAL) {
        font_size_delta_ = std::min(font_size_delta_ + 1.0f, 250.0f);
        update();
        return true;
    }

    else if (key == GLFW_KEY_DOWN) {
        const float spacing = texter_->character_spacing();
        texter_->set_character_spacing(std::max(spacing - 0.5f, 0.0f));
        update();
        return true;
    }
    else if (key == GLFW_KEY_UP) {
        const float spacing = texter_->character_spacing();
        texter_->set_character_spacing(std::min(spacing + 0.5f, 50.0f));
        update();
        return true;
    }
    else
        return Viewer::key_press_event(key, modifiers);
}
