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

#ifndef EASY3D_HASH_H
#define EASY3D_HASH_H

#include <functional>


namespace easy3d
{

#if 0
    /**
     * The hash combine function copied from boost. I have an example to fail this function:
     *      std::vector<float> a = {16, 0, 0};
     *      std::vector<float> b = {4, 12, 0}; // 15588749483758.
     *      std::cout << "a: " << hash_range(a.begin(), a.end()) << std::endl;
     *      std::cout << "b: " << hash_range(b.begin(), b.end()) << std::endl;
     */
    template<class T>
    inline void hash_combine(std::size_t &seed, T const& value) {
        std::hash<T> hasher;
        seed ^= hasher(value) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    }
#else
    // code from CityHash
    // https://github.com/google/cityhash/blob/master/src/city.h
    template<class T>
    inline void hash_combine(std::size_t &seed, T const& value) {
        std::hash<T> hasher;
        std::size_t a = (hasher(value) ^ seed) * 0x9ddfea08eb382d69ULL;
        a ^= (a >> 47);
        std::size_t b = (seed ^ a) * 0x9ddfea08eb382d69ULL;
        b ^= (b >> 47);
        seed = b * 0x9ddfea08eb382d69ULL;
    }
#endif

    template <typename FT>
    std::size_t hash(const Vec<2, FT>& value) {
        std::size_t seed = 0;
        hash_combine(seed, value.x);
        hash_combine(seed, value.y);
        return seed;
    }

    template <typename FT>
    std::size_t hash(const Vec<3, FT>& value) {
        std::size_t seed = 0;
        hash_combine(seed, value.x);
        hash_combine(seed, value.y);
        hash_combine(seed, value.z);
        return seed;
    }


    template <int DIM, typename FT> inline
    std::size_t hash(const Vec<DIM, FT>& value) {
        std::size_t seed = 0;
        for (std::size_t i=0; i<DIM; ++i)
            hash_combine(seed, value[i]);
        return seed;
    }


    template<class It>
    inline std::size_t hash_range(It first, It last) {
        std::size_t seed = 0;
        for (; first != last; ++first) {
            hash_combine(seed, *first);
        }
        return seed;
    }

    template<class It>
    inline void hash_range(std::size_t &seed, It first, It last) {
        for (; first != last; ++first) {
            hash_combine(seed, *first);
        }
    }

} // namespace easy3d

#endif  // EASY3D_HASH_H