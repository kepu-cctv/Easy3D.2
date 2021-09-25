/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D: software for processing and rendering
*   meshes and point clouds.
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


/** ----------------------------------------------------------
 *
 * the code is adapted from Surface_mesh with modifications.
 *		- Surface_mesh (version 1.1)
 * The original code is available at
 * https://opensource.cit-ec.de/projects/surface_mesh
 *
 * Surface_mesh is a halfedge-based mesh data structure for
 * representing and processing 2-manifold polygonal surface
 * meshes. It is implemented in C++ and designed with an
 * emphasis on simplicity and efficiency.
 *
 *----------------------------------------------------------*/


//== INCLUDES =================================================================

#include <easy3d/IO.h>

#include <cstdio>

#include <cstring>


//== NAMESPACES ===============================================================


namespace easy3d {


//== IMPLEMENTATION ===========================================================


bool read_obj(SurfaceMesh& mesh, const std::string& filename)
{
    char   s[200];
    float  x, y, z;
    std::vector<SurfaceMesh::Vertex>  vertices;
    std::vector<vec2> all_tex_coords;   //individual texture coordinates
    std::vector<int> halfedge_tex_idx; //texture coordinates sorted for halfedges
    SurfaceMesh::HalfedgeProperty <vec2> tex_coords = mesh.halfedge_property<vec2>("h:texcoord");
    bool with_tex_coord=false;

    // clear mesh
    mesh.clear();


    // open file (in ASCII mode)
    FILE* in = fopen(filename.c_str(), "r");
    if (!in) return false;


    // clear line once
    memset(&s, 0, 200);


    // parse line by line (currently only supports vertex positions & faces
    while(in && !feof(in) && fgets(s, 200, in))
    {
        // comment
        if (s[0] == '#' || isspace(s[0])) continue;

        // vertex
        else if (strncmp(s, "v ", 2) == 0)
        {
            if (sscanf(s, "v %f %f %f", &x, &y, &z))
            {
                mesh.add_vertex(vec3(x,y,z));
            }
        }
        // normal
        else if (strncmp(s, "vn ", 3) == 0)
        {
          if (sscanf(s, "vn %f %f %f", &x, &y, &z))
          {
            // problematic as it can be either a vertex property when interpolated
            // or a halfedge property for hard edges
          }
        }

        // texture coordinate
        else if (strncmp(s, "vt ", 3) == 0)
        {
          if (sscanf(s, "vt %f %f", &x, &y))
          {
            all_tex_coords.push_back(vec2(x,y));
          }
        }

        // face
        else if (strncmp(s, "f ", 2) == 0)
        {
          int component(0), nV(0);
          bool endOfVertex(false);
          char *p0, *p1(s+1);

          vertices.clear();
          halfedge_tex_idx.clear();

          // skip white-spaces
          while (*p1==' ') ++p1;

          while (p1)
          {
            p0 = p1;

            // overwrite next separator

            // skip '/', '\n', ' ', '\0', '\r' <-- don't forget Windows
            while (*p1!='/' && *p1!='\r' && *p1!='\n' && *p1!=' ' && *p1!='\0') ++p1;

            // detect end of vertex
            if (*p1 != '/')
            {
              endOfVertex = true;
            }

            // replace separator by '\0'
            if (*p1 != '\0')
            {
              *p1 = '\0';
              p1++; // point to next token
            }

            // detect end of line and break
            if (*p1 == '\0' || *p1 == '\n')
            {
              p1 = 0;
            }

            // read next vertex component
            if (*p0 != '\0')
            {
              switch (component)
              {
                case 0: // vertex
                {
                  vertices.push_back( SurfaceMesh::Vertex(atoi(p0) - 1) );
                  break;
                }
                case 1: // texture coord
                {
                  int idx = atoi(p0)-1;
                  halfedge_tex_idx.push_back(idx);
                  with_tex_coord=true;
                  break;
                }
                case 2: // normal
                  break;
              }
            }

            ++component;

            if (endOfVertex)
            {
              component = 0;
              nV++;
              endOfVertex = false;
            }
          }

          SurfaceMesh::Face f=mesh.add_face(vertices);


          // add texture coordinates
          if(with_tex_coord)
          {
              SurfaceMesh::HalfedgeAroundFaceCirculator h_fit = mesh.halfedges(f);
              SurfaceMesh::HalfedgeAroundFaceCirculator h_end = h_fit;
              unsigned v_idx =0;
              do
              {
                  tex_coords[*h_fit]=all_tex_coords.at(halfedge_tex_idx.at(v_idx));
                  ++v_idx;
                  ++h_fit;
              }
              while(h_fit!=h_end);
          }
        }
        // clear line
        memset(&s, 0, 200);
    }

    fclose(in);
    return mesh.n_faces() > 0;
}


//-----------------------------------------------------------------------------


bool write_obj(const SurfaceMesh& mesh, const std::string& filename)
{
    FILE* out = fopen(filename.c_str(), "w");
    if (!out)
        return false;

    // comment
    fprintf(out, "# OBJ export from SurfaceMesh\n");

    //vertices
    SurfaceMesh::VertexProperty<vec3> points = mesh.get_vertex_property<vec3>("v:point");
    for (SurfaceMesh::VertexIterator vit=mesh.vertices_begin(); vit!=mesh.vertices_end(); ++vit)
    {
        const vec3& p = points[*vit];
        fprintf(out, "v %.10f %.10f %.10f\n", p[0], p[1], p[2]);
    }

    //normals
    SurfaceMesh::VertexProperty<vec3> normals = mesh.get_vertex_property<vec3>("v:normal");
    if(normals)
    {
        for (SurfaceMesh::VertexIterator vit=mesh.vertices_begin(); vit!=mesh.vertices_end(); ++vit)
        {
            const vec3& p = normals[*vit];
            fprintf(out, "vn %.10f %.10f %.10f\n", p[0], p[1], p[2]);
        }
    }

    //optionally texture coordinates
    // do we have them?
    std::vector<std::string> h_props= mesh.halfedge_properties();
    bool with_tex_coord = false;
    std::vector<std::string>::iterator h_prop_end = h_props.end();
    std::vector<std::string>::iterator h_prop_start= h_props.begin();
    while(h_prop_start!=h_prop_end)
    {
        if(0==(*h_prop_start).compare("h:texcoord"))
        {
            with_tex_coord=true;
        }
        ++h_prop_start;
    }

    //if so then add
    if(with_tex_coord)
    {
        SurfaceMesh::HalfedgeProperty<vec2> tex_coord = mesh.get_halfedge_property<vec2>("h:texcoord");
        for (SurfaceMesh::HalfedgeIterator hit=mesh.halfedges_begin(); hit!=mesh.halfedges_end(); ++hit)
        {
            const vec2& pt = tex_coord[*hit];
            fprintf(out, "vt %.10f %.10f %.10f\n", pt[0], pt[1], pt[2]);
        }
    }

    //faces
    for (SurfaceMesh::FaceIterator fit=mesh.faces_begin(); fit!=mesh.faces_end(); ++fit)
    {
        fprintf(out, "f");
        SurfaceMesh::VertexAroundFaceCirculator fvit=mesh.vertices(*fit), fvend=fvit;
        SurfaceMesh::HalfedgeAroundFaceCirculator fhit=mesh.halfedges(*fit);
        do
        {
            if(with_tex_coord)
            {
                // write vertex index, tex_coord index and normal index
                fprintf(out, " %d/%d/%d", (*fvit).idx()+1, (*fhit).idx()+1, (*fvit).idx()+1);
                ++fhit;
            }
            else
            {
                // write vertex index and normal index
                fprintf(out, " %d//%d", (*fvit).idx()+1, (*fvit).idx()+1);
            }
        }
        while (++fvit != fvend);
        fprintf(out, "\n");
    }

    fclose(out);
    return true;
}


//=============================================================================
} // namespace easy3d
//=============================================================================
