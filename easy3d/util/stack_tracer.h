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

#ifndef EASY3D_UTIL_STACK_TRACER_H
#define EASY3D_UTIL_STACK_TRACER_H

#include <string>


namespace easy3d {


    /**
     * \brief Tracks and dumps the stack frames when program fails.
     * \class StackTracer easy3d/util/stack_tracer.h
     *
     * Track failure example:
     *      \code
     *          #include <easy3d/util/stack_tracer.h>
     *          int main() {
     *              easy3d::StackTracer dh;
     *              int a = 1;
     *              a /= 0;
     *              return 0;
     *          }
     *      \endcode
     *
     * Read recent stack activities example:
     *      \code
     *          std::cout << StackTracer::backtrace();
     *      \endcode
     */
    class StackTracer {
    public:
        StackTracer();
        ~StackTracer();

        /// This function produces a stack backtrace with demangled function & method names.
        /// \param skip The number of the most recent frames to skip.
        /// \param amount The number of frames to retrieve. Default value is 32.
        ///     You can pass -1 to retrieve all available traces.
        static std::string back_trace(int skip = 1, int amount = 32);

        /// The function to be called when a signal was triggered
        static void signal_handler(int sig, void* info, void* secret);
    };

}


#endif  // EASY3D_UTIL_STACK_TRACER_H
