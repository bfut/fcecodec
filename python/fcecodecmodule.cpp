/*
  fcecodecmodule.c - Python module
  fcecodec Copyright (C) 2021 Benjamin Futasz <https://github.com/bfut>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
  BUILD:
  - to cwd
      python setup.py build
  - install
      python setup.py install
 **/

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <algorithm>  // std::swap()
#include <cstdio>
#include <cstring>
#include <vector>  // std::vector

/* FCEC_MODULE_DEBUG -------------------------------------------------------- */
#ifdef FCEC_MODULE_DEBUG
#define malloc PyMem_Malloc
#define realloc PyMem_Realloc
#define free PyMem_Free
#endif
/* FCEC_MODULE_DEBUG -------------------------------------------------------- */

#include "../src/fcelib/fcelib.h"
#include "../src/fcelib/fcelib_types.h"

namespace py = pybind11;


/* aux --------------------------------------------------------------------- */

int FCECODECMODULE_aux_GetIndexByOrder(fcelib::FcelibMesh *mesh, const int idx)
{
  int i = 0;
  for (int count = -1; i < mesh->parts_len; ++i)
  {
    if (mesh->hdr.Parts[i] > -1)
      ++count;
    if (count == idx)
      break;
  }
  
  return i;
}


/* classes, structs ----------------------------------------------------------------- */

class Part {
  public:
    Part() { part_ = NULL; py::print("Part()"); }
    ~Part() { py::print("~Part()"); }

    // Internal
    fcelib::FcelibPart *get_part_() { return part_; }
    void set_part_(fcelib::FcelibPart *p){ part_ = p; }

    void set_ptriags__from_mesh(fcelib::FcelibMesh *m)  // TODO: is this vector useful?
    {
      const int num_triags = part_->PNumTriangles;
      ptriags_.clear();
      ptriags_.resize(num_triags);
      for(int i = 0; i < num_triags; ++i)
      {
        ptriags_.at(i) = m->triangles[part_->PTriangles[i]];  // FIXME: check for NULL
        if (!ptriags_.at(i)) throw std::runtime_error("set_ptriags__from_mesh(): triangle is NULL");
      }
    }

    void set_pverts__from_mesh(fcelib::FcelibMesh *m)  // TODO: is this vector useful?
    {
      const int num_verts = part_->PNumVertices;
      pverts_.clear();
      pverts_.resize(num_verts);
      for(int i = 0; i < num_verts; ++i)
      {
        pverts_.at(i) = m->vertices[part_->PVertices[i]];  // FIXME: check for NULL
        if (!pverts_.at(i)) throw std::runtime_error("set_pverts__from_mesh(): vertice is NULL");
      }
    }
    
    // Stats
    std::size_t num_triags() { return part_->PNumTriangles; };
    std::size_t num_verts() { return part_->PNumVertices; };

    // Part properties
    std::string get_name() const { if (!part_) return NULL; else return std::string(part_->PartName); };
    void set_name(std::string s) { std::strncpy(part_->PartName, s.c_str(), (std::size_t)sizeof(part_->PartName) - 1); };
    
    py::buffer get_pos() 
    {
      py::array_t<float> result = py::array_t<float>({ static_cast<py::ssize_t>(3) }, {  });
      auto buf = result.mutable_unchecked<1>();
      buf(0) = part_->PartPos.x;
      buf(1) = part_->PartPos.y;
      buf(2) = part_->PartPos.z;

      return result;
    };
    void set_pos(py::array_t<float, py::array::c_style | py::array::forcecast> arr)
    {
      py::buffer_info buf = arr.request();
      float *ptr;

      if (buf.ndim != 1)
        throw std::runtime_error("Number of dimensions must be 1");
      if (buf.shape[0] != 3)
        throw std::runtime_error("Shape must be (3, )");

      ptr = static_cast<float *>(buf.ptr);
      part_->PartPos.x = ptr[0];
      part_->PartPos.y = ptr[1];
      part_->PartPos.z = ptr[2];
    };

    // Triangles
    py::buffer get_triags_vidx()
    {
      const py::ssize_t nrows = ptriags_.size();
      py::array_t<int> result = py::array_t<int>({ nrows, static_cast<py::ssize_t>(3) }, {  });
      auto buf = result.mutable_unchecked<2>();
      for (py::ssize_t i = 0; i < nrows; ++i)
      {
        buf(i, 0) = ptriags_.at(i)->vidx[0];
        buf(i, 1) = ptriags_.at(i)->vidx[1];
        buf(i, 2) = ptriags_.at(i)->vidx[2];
      }
      
      return result;
    };
    
    py::buffer get_triags_flags()
    {
      const py::ssize_t nrows = ptriags_.size();
      py::array_t<int> result = py::array_t<int>({ nrows }, {  });
      auto buf = result.mutable_unchecked<1>();
      for (py::ssize_t i = 0; i < nrows; ++i)
        buf(i) = ptriags_.at(i)->flag;
      return result;
    };
    void set_triags_flags(py::array_t<int, py::array::c_style | py::array::forcecast> arr)
    {
      const py::ssize_t nrows = ptriags_.size();
      py::buffer_info buf = arr.request();
      int *ptr;
        
      if (buf.ndim != 1)
        throw std::runtime_error("Number of dimensions must be 1");
      if (buf.shape[0] != nrows)
        throw std::runtime_error("Shape must be (N, ) where N == Part.num_triags()");
        
      ptr = static_cast<int *>(buf.ptr);
      for (py::ssize_t i = 0; i < nrows; ++i)
        ptriags_.at(i)->flag = ptr[i];
    }

    py::buffer get_triags_texpages() 
    {
      const py::ssize_t nrows = ptriags_.size();
      py::array_t<int> result = py::array_t<int>({ nrows }, {  });
      auto buf = result.mutable_unchecked<1>();
      for (py::ssize_t i = 0; i < nrows; ++i)
        buf(i) = ptriags_.at(i)->tex_page;
      return result;
    };
    void set_triags_texpages(py::array_t<int, py::array::c_style | py::array::forcecast> arr)
    {
      const py::ssize_t nrows = ptriags_.size();
      py::buffer_info buf = arr.request();
      int *ptr;
        
      if (buf.ndim != 1)
        throw std::runtime_error("Number of dimensions must be 1");
      if (buf.shape[0] != nrows)
        throw std::runtime_error("Shape must be (N, ) where N == Part.num_triags()");
        
      ptr = static_cast<int *>(buf.ptr);
      for (py::ssize_t i = 0; i < nrows; ++i)
        ptriags_.at(i)->tex_page = ptr[i];
    }

  private:
    fcelib::FcelibPart *part_;
    std::vector<fcelib::FcelibTriangle *> ptriags_;  // Note: triags contain global vert indexes
    std::vector<fcelib::FcelibVertex *> pverts_;  // this vector: access verts by part order
};


class Mesh {  // TODO: implement valid_ logic?
  public:
    Mesh() : valid_(false) { fcelib::FCELIB_InitMesh(&mesh_); py::print("Mesh()"); }
    ~Mesh() { fcelib::FCELIB_FreeMesh(&mesh_); py::print("~Mesh()"); }

    // Internal
    void free_Mesh() { parts_.clear(); fcelib::FCELIB_FreeMesh(&mesh_); }
    fcelib::FcelibMesh *get_mesh_() { return &mesh_; }
    std::vector<Part> *get_parts_() { return &parts_; }

    // Stats
    void info() const { fcelib::FCELIB_PrintMeshInfo(mesh_); }
    bool valid() {
      valid_ = false;
      for (;;)
      {
        if (mesh_.hdr.NumParts != static_cast<int>(parts_.size()))
        {
          py::print("Mesh.valid(): NumParts != parts_.size()");
          break;
        }
        if (fcelib::FCELIB_ValidateMesh(mesh_))
          valid_ = true;
        break;
      }
      return valid_;
    }

    // i/o
    bool decode_fce(std::string &buf) 
    {
      bool retv;
      std::size_t numparts;
      Part *ptr;

      if (mesh_.freed != 1)
      {
        this->free_Mesh();
        fcelib::FCELIB_InitMesh(&mesh_);
      }

      retv = static_cast<bool>(fcelib::FCELIB_DecodeFce(buf.c_str(), buf.size(), &mesh_));
      if (retv == false)
        throw std::runtime_error("decode_fce: Cannot parse FCE data");

      numparts = mesh_.hdr.NumParts;
      parts_.resize(numparts);
      for (std::size_t i = 0; i < numparts; ++i)
      {
        ptr = &parts_.at(i);
        ptr->set_part_(mesh_.parts[mesh_.hdr.Parts[i]]);
        ptr->set_ptriags__from_mesh(&mesh_);
        ptr->set_pverts__from_mesh(&mesh_);
      }
      ptr = NULL;

      return retv;
    }
    
    bool encode_fce3(std::string &fcepath, bool center_parts)
    {
      if (parts_.size() == 0) return false;
      return fcelib::FCELIB_EncodeFce3(&mesh_, fcepath.c_str(), static_cast<int>(center_parts));
    }
    
    bool encode_fce4(std::string &fcepath, bool center_parts)
    {
      py::print("Warning: fcelib::FCELIB_EncodeFce4() not yet implemented");
      if (parts_.size() == 0) return false;
      return fcelib::FCELIB_EncodeFce4(&mesh_, fcepath.c_str(), static_cast<int>(center_parts));  // TODO: implement fcelib::FCELIB_EncodeFce4()
    }
    
    bool export_obj(std::string &objpath, std::string &mtlpath,
                    std::string &texture_name,
                    int print_damage, int print_dummies)
    {
      return fcelib::FCELIB_ExportObj(&mesh_, objpath.c_str(), mtlpath.c_str(), 
                                      texture_name.c_str(),
                                      print_damage, print_dummies); 
    }

    // Parts
    std::vector<Part> &get_parts() { return parts_; }

    bool center_part(const std::size_t idx)
    {
      if (idx >= parts_.size()) 
        throw std::out_of_range("Mesh.center_part(): part index (idx) out of range");
      if (idx == 0)
        return 0;
      return static_cast<int>(fcelib::FCELIB_CenterPart(&mesh_, idx));
    }

    int copy_part_to_mesh(Mesh *mesh_src, const int idx_src) 
    {
      int retv = -1;
      fcelib::FcelibMesh *mesh_src_ = mesh_src->get_mesh_();
      fcelib::FcelibMesh *new_mesh_;

      if (idx_src < 0 || idx_src == mesh_src_->parts_len)
        throw std::out_of_range("copy_part_to_mesh(mesh_src, part_idx): part index (idx_src) out of range");

      new_mesh_ = this->get_mesh_();

      const int idx_new = fcelib::FCELIB_CopyPartToMesh(new_mesh_, mesh_src_, idx_src);
      if (idx_new < 0)
        throw std::runtime_error("copy_part_to_mesh(mesh_src, part_idx): Cannot copy part");

      std::vector<Part> *new_parts_;
      Part *ptr;
      const int idx_internal = FCECODECMODULE_aux_GetIndexByOrder(new_mesh_, idx_new);
      if (idx_internal < 0)
        throw std::out_of_range("copy_part_to_mesh(mesh_src, part_idx): part index (idx_new) out of range");
      new_parts_ = this->get_parts_();
      new_parts_->resize(new_parts_->size() + 1);
      ptr = &new_parts_->at(new_parts_->size() - 1);
      ptr->set_part_(new_mesh_->parts[new_mesh_->hdr.Parts[idx_internal]]);
      ptr->set_ptriags__from_mesh(new_mesh_);
      ptr->set_pverts__from_mesh(new_mesh_);

      retv = idx_new;
      return retv;
    }

    bool del_part(const std::size_t idx)
    {
      if (idx >= parts_.size()) 
        throw std::out_of_range("Mesh.delete_part(): part index (idx) out of range");

      auto it = parts_.begin();
      std::size_t i = -1;
      while (++i < idx)
        ++it;
      parts_.erase(it);
      
      return fcelib::FCELIB_DeletePart(&mesh_, idx); 
    }

    int merge_parts(std::size_t idx1, std::size_t idx2)
    {
      if (idx1 >= parts_.size()) 
        throw std::out_of_range("Mesh.merge_parts(): part index (idx1) out of range");
      if (idx2 >= parts_.size()) 
        throw std::out_of_range("Mesh.merge_parts(): part index (idx2) out of range");
      
      int idx_new = fcelib::FCELIB_MergePartsToNew(&mesh_, idx1, idx2);
      if (idx_new < 0)
        throw std::runtime_error("Mesh.merge_parts()");

      Part *ptr;
      const int idx_internal = FCECODECMODULE_aux_GetIndexByOrder(&mesh_, idx_new);
      if (idx_internal < 0)
        throw std::out_of_range("Mesh.merge_parts(): part index (idx_new) out of range");

      parts_.resize(parts_.size() + 1);
      ptr = &parts_.at(parts_.size() - 1);
      ptr->set_part_(mesh_.parts[mesh_.hdr.Parts[idx_internal]]);
      ptr->set_ptriags__from_mesh(&mesh_);
      ptr->set_pverts__from_mesh(&mesh_);

      return idx_new;
    }

    int move_up_part(const std::size_t idx)
    {
      if (idx >= parts_.size()) 
        throw std::out_of_range("Mesh.move_up_part(): part index (idx) out of range");
      if (idx == 0)
        return 0;
      std::swap(parts_.at(idx), parts_.at(idx - 1));
      
      return fcelib::FCELIB_MeshMoveUpPart(&mesh_, idx);
    }
    
    Part &part_at(const std::size_t idx) { return parts_.at(idx); }  // TODO: deprecated - remove
    

    // Triags / Verts
    bool del_unrefd_verts() 
    {
      return fcelib::FCELIB_DeleteUnrefdVerts(&mesh_);
    }

    /* Via vector index (=global vert idx) map to global vert order. */
    py::buffer get_verts_map_idx2order()  // TODO: test
    {
      const int nrows = mesh_.vertices_len;
      py::array_t<int> result = py::array_t<int>({ nrows }, {  });
      auto buf = result.mutable_unchecked<>();
      // i - global vert index, j - global vert order
      for (int i = 0, j = 0; i < nrows; ++i)
      {
        if(!mesh_.vertices[i])
        {
          buf(i) = -1;
          continue;
        }
        buf(i) = j;
        ++j;
      }
      return result;
    };
    
    py::buffer get_verts_pos()
    {
      const int nrows = mesh_.hdr.NumVertices;
      py::array_t<float> result = py::array_t<float>({ nrows * 3 }, {  });
      auto buf = result.mutable_unchecked<>();
      fcelib::FcelibVertex *vert;
      // i - global vert index, j - global vert order
      for (int i = 0, j = 0; i < nrows; ++i)
      {
        vert = mesh_.vertices[i];
        if(!vert)
          continue;

        buf(j * 3 + 0) = vert->VertPos.x;
        buf(j * 3 + 1) = vert->VertPos.y;
        buf(j * 3 + 2) = vert->VertPos.z;
        ++j;
      }
      return result;
    };
    py::buffer get_verts_norms()
    {
      const int nrows = mesh_.hdr.NumVertices;
      py::array_t<float> result = py::array_t<float>({ nrows * 3 }, {  });
      auto buf = result.mutable_unchecked<>();
      fcelib::FcelibVertex *vert;
      // i - global vert index, j - global vert order
      for (int i = 0, j = 0; i < nrows; ++i)
      {
        vert = mesh_.vertices[i];
        if(!vert)
          continue;

        buf(j * 3 + 0) = vert->NormPos.x;
        buf(j * 3 + 1) = vert->NormPos.y;
        buf(j * 3 + 2) = vert->NormPos.z;
        ++j;
      }
      return result;
    };
    py::buffer get_dmgd_verts_pos()
    {
      const int nrows = mesh_.hdr.NumVertices;
      py::array_t<float> result = py::array_t<float>({ nrows * 3 }, {  });
      auto buf = result.mutable_unchecked<>();
      fcelib::FcelibVertex *vert;
      // i - global vert index, j - global vert order
      for (int i = 0, j = 0; i < nrows; ++i)
      {
        vert = mesh_.vertices[i];
        if(!vert)
          continue;

        buf(j * 3 + 0) = vert->DamgdVertPos.x;
        buf(j * 3 + 1) = vert->DamgdVertPos.y;
        buf(j * 3 + 2) = vert->DamgdVertPos.z;
        ++j;
      }
      return result;
    };
    py::buffer get_dmgd_verts_norms()
    {
      const int nrows = mesh_.hdr.NumVertices;
      py::array_t<float> result = py::array_t<float>({ nrows * 3 }, {  });
      auto buf = result.mutable_unchecked<>();
      fcelib::FcelibVertex *vert;
      // i - global vert index, j - global vert order
      for (int i = 0, j = 0; i < nrows; ++i)
      {
        vert = mesh_.vertices[i];
        if(!vert)
          continue;

        buf(j * 3 + 0) = vert->DamgdNormPos.x;
        buf(j * 3 + 1) = vert->DamgdNormPos.y;
        buf(j * 3 + 2) = vert->DamgdNormPos.z;
        ++j;
      }
      return result;
    };
    void set_dmgd_verts_norms(py::array_t<int, py::array::c_style | py::array::forcecast> arr)
    {
      const int nrows = mesh_.hdr.NumVertices;
      py::buffer_info buf = arr.request();
      int *ptr;
      fcelib::FcelibVertex *vert;
      if (buf.ndim != 1)
        throw std::runtime_error("Number of dimensions must be 1");
      if (buf.shape[0] != nrows * 3)
        throw std::runtime_error("Shape must be (N*3, ) where N = Mesh.num_verts()");
      ptr = static_cast<int *>(buf.ptr);
      // i - global vert index, j - global vert order
      for (int i = 0, j = 0; i < nrows; ++i)
      {
        vert = mesh_.vertices[i];
        if(!vert)
          continue;
         vert->DamgdNormPos.x = ptr[j * 3 + 0];
         vert->DamgdNormPos.y = ptr[j * 3 + 1];
         vert->DamgdNormPos.z = ptr[j * 3 + 2];
        ++j;
      }
    };
    
    py::buffer get_verts_anim()
    {
      const int nrows = mesh_.hdr.NumVertices;
      py::array_t<int> result = py::array_t<int>({ nrows }, {  });
      auto buf = result.mutable_unchecked<>();
      fcelib::FcelibVertex *vert;
      // i - global vert index, j - global vert order
      for (int i = 0, j = 0; i < nrows; ++i)
      {
        vert = mesh_.vertices[i];
        if(!vert)
          continue;

        buf(j) = vert->Animation;
        ++j;
      }
      return result;
    };
    void set_verts_anim(py::array_t<int, py::array::c_style | py::array::forcecast> arr)
    {
      const int nrows = mesh_.hdr.NumVertices;
      py::buffer_info buf = arr.request();
      int *ptr;
      fcelib::FcelibVertex *vert;
      if (buf.ndim != 1)
        throw std::runtime_error("Number of dimensions must be 1");
      if (buf.shape[0] != nrows)
        throw std::runtime_error("Shape must be (N, ) where N = Mesh.num_verts()");
      ptr = static_cast<int *>(buf.ptr);
      // i - global vert index, j - global vert order
      for (int i = 0, j = 0; i < nrows; ++i)
      {
        vert = mesh_.vertices[i];
        if(!vert)
          continue;
         vert->Animation = ptr[j];
        ++j;
      }
    };
    
#if 0    
    py::buffer get_verts_anim()  // re-adapt for Mesh class
    {
      const py::ssize_t nrows = pverts_.size();
      py::array_t<int> result = py::array_t<int>({ nrows }, {  });
      auto buf = result.mutable_unchecked<1>();
      for (py::ssize_t i = 0; i < nrows; ++i)
        buf(i) = pverts_.at(i)->Animation;

      return result;
    };
    void set_verts_anim(py::array_t<int, py::array::c_style | py::array::forcecast> arr)
    {
      const py::ssize_t nrows = pverts_.size();
      py::buffer_info buf = arr.request();
      int *ptr;
        
      if (buf.ndim != 1)
        throw std::runtime_error("Number of dimensions must be 1");
      if (buf.shape[0] != nrows)
        throw std::runtime_error("Shape must be (N, ) where N = Part.num_verts()");
        
      ptr = static_cast<int *>(buf.ptr);
      for (py::ssize_t i = 0; i < nrows; ++i)
        pverts_.at(i)->Animation = ptr[i];
    };
#endif
    
    // Header
    py::buffer get_colors()
    {
      const py::ssize_t nrows = mesh_.hdr.NumColors;
      py::array_t<unsigned char> result = py::array_t<unsigned char>({ nrows, static_cast<py::ssize_t>(4), static_cast<py::ssize_t>(4) }, {  });
      auto buf = result.mutable_unchecked<3>();
      
      for (py::ssize_t i = 0; i < nrows; ++i)
      {
        buf(i, 0, 0) = mesh_.hdr.PriColors[i].hue;
        buf(i, 0, 1) = mesh_.hdr.PriColors[i].saturation;
        buf(i, 0, 2) = mesh_.hdr.PriColors[i].brightness;
        buf(i, 0, 3) = mesh_.hdr.PriColors[i].transparency;

        buf(i, 1, 0) = mesh_.hdr.IntColors[i].hue;
        buf(i, 1, 1) = mesh_.hdr.IntColors[i].saturation;
        buf(i, 1, 2) = mesh_.hdr.IntColors[i].brightness;
        buf(i, 1, 3) = mesh_.hdr.IntColors[i].transparency;

        buf(i, 2, 0) = mesh_.hdr.SecColors[i].hue;
        buf(i, 2, 1) = mesh_.hdr.SecColors[i].saturation;
        buf(i, 2, 2) = mesh_.hdr.SecColors[i].brightness;
        buf(i, 2, 3) = mesh_.hdr.SecColors[i].transparency;
        
        buf(i, 3, 0) = mesh_.hdr.DriColors[i].hue;
        buf(i, 3, 1) = mesh_.hdr.DriColors[i].saturation;
        buf(i, 3, 2) = mesh_.hdr.DriColors[i].brightness;
        buf(i, 3, 3) = mesh_.hdr.DriColors[i].transparency;
      }
      
      return result;
    }
    
    void set_colors(py::array_t<unsigned char, py::array::c_style | py::array::forcecast> arr)
    {
      py::buffer_info buf = arr.request();
      unsigned char *ptr;
        
      if (buf.ndim != 3)
        throw std::runtime_error("Number of dimensions must be 3");
      if (buf.shape[1] != 4 || buf.shape[2] != 4)
        throw std::runtime_error("Shape must be (N, 4, 4)");
        
      const py::ssize_t nrows = buf.shape[0];
      ptr = static_cast<unsigned char *>(buf.ptr);
      
      for (py::ssize_t i = 0; i < nrows && i < 16; ++i)
      {
        mesh_.hdr.PriColors[i].hue          = ptr[i * 16 + 0 * 4 + 0];
        mesh_.hdr.PriColors[i].saturation   = ptr[i * 16 + 0 * 4 + 1];
        mesh_.hdr.PriColors[i].brightness   = ptr[i * 16 + 0 * 4 + 2];
        mesh_.hdr.PriColors[i].transparency = ptr[i * 16 + 0 * 4 + 3];
        
        mesh_.hdr.IntColors[i].hue          = ptr[i * 16 + 1 * 4 + 0];
        mesh_.hdr.IntColors[i].saturation   = ptr[i * 16 + 1 * 4 + 1];
        mesh_.hdr.IntColors[i].brightness   = ptr[i * 16 + 1 * 4 + 2];
        mesh_.hdr.IntColors[i].transparency = ptr[i * 16 + 1 * 4 + 3];
        
        mesh_.hdr.SecColors[i].hue          = ptr[i * 16 + 2 * 4 + 0];
        mesh_.hdr.SecColors[i].saturation   = ptr[i * 16 + 2 * 4 + 1];
        mesh_.hdr.SecColors[i].brightness   = ptr[i * 16 + 2 * 4 + 2];
        mesh_.hdr.SecColors[i].transparency = ptr[i * 16 + 2 * 4 + 3];
        
        mesh_.hdr.DriColors[i].hue          = ptr[i * 16 + 3 * 4 + 0];
        mesh_.hdr.DriColors[i].saturation   = ptr[i * 16 + 3 * 4 + 1];
        mesh_.hdr.DriColors[i].brightness   = ptr[i * 16 + 3 * 4 + 2];
        mesh_.hdr.DriColors[i].transparency = ptr[i * 16 + 3 * 4 + 3];
      }
      
      for (py::ssize_t i = nrows; i < 16; ++i)
      {
        mesh_.hdr.PriColors[i].hue          = 0;
        mesh_.hdr.PriColors[i].saturation   = 0;
        mesh_.hdr.PriColors[i].brightness   = 0;
        mesh_.hdr.PriColors[i].transparency = 0;
        
        mesh_.hdr.IntColors[i].hue          = 0;
        mesh_.hdr.IntColors[i].saturation   = 0;
        mesh_.hdr.IntColors[i].brightness   = 0;
        mesh_.hdr.IntColors[i].transparency = 0;
        
        mesh_.hdr.SecColors[i].hue          = 0;
        mesh_.hdr.SecColors[i].saturation   = 0;
        mesh_.hdr.SecColors[i].brightness   = 0;
        mesh_.hdr.SecColors[i].transparency = 0;
        
        mesh_.hdr.DriColors[i].hue          = 0;
        mesh_.hdr.DriColors[i].saturation   = 0;
        mesh_.hdr.DriColors[i].brightness   = 0;
        mesh_.hdr.DriColors[i].transparency = 0;
      }
      
      mesh_.hdr.NumColors    = static_cast<int>(nrows);
      mesh_.hdr.NumSecColors = static_cast<int>(nrows);
    }
    
    std::vector<std::string> get_dummy_names()
    {
      const int len = mesh_.hdr.NumDummies;
      std::vector<std::string> retv;
      retv.resize(len);
      
      for (int i = 0; i < len; ++i)
        retv.at(i) = std::string(mesh_.hdr.DummyNames + i * 64);
        
      return retv;
    }
    
    void set_dummy_names(std::vector<std::string> &arr)
    {
      std::string *ptr;
      const std::size_t nrows = arr.size();
      
      std::memset(mesh_.hdr.DummyNames, '\0', (std::size_t)(16 * 64) * sizeof(char));
      
      for (std::size_t i = 0; i < nrows && i < 16; ++i)
      {
        ptr = &arr.at(i);
        std::cout << ptr->c_str() << "%  ";
        std::strncpy(mesh_.hdr.DummyNames + i * 64, ptr->c_str(), std::min((std::size_t)63, ptr->size()) * sizeof(char));
      }
              
      mesh_.hdr.NumDummies = static_cast<int>(nrows);
    }
    
    py::buffer get_dummy_pos()
    {
      const py::ssize_t len = mesh_.hdr.NumDummies;
      py::array_t<float> result = py::array_t<float>({ len * 3 }, {  });
      auto buf = result.mutable_unchecked<1>();
      
      for (py::ssize_t i = 0; i < len; ++i)
      {
        buf(i * 3 + 0) = mesh_.hdr.Dummies[i].x;
        buf(i * 3 + 1) = mesh_.hdr.Dummies[i].y;
        buf(i * 3 + 2) = mesh_.hdr.Dummies[i].z;
      }
      
      return result;
    }
      
    void set_dummy_pos(py::array_t<float, py::array::c_style | py::array::forcecast> arr)
    {
      py::buffer_info buf = arr.request();
      float *ptr;
        
      if (buf.ndim != 1)
        throw std::runtime_error("set_dummy_pos(): Number of dimensions must be 1");
      
      const py::ssize_t nrows = py::ssize_t(buf.shape[0] / 3);
      ptr = static_cast<float *>(buf.ptr);
      for (py::ssize_t i = 0; i < nrows && i < 16; ++i)
      {
        mesh_.hdr.Dummies[i].x = ptr[i * 3 + 0];
        mesh_.hdr.Dummies[i].y = ptr[i * 3 + 1];
        mesh_.hdr.Dummies[i].z = ptr[i * 3 + 2];
      }
      for (py::ssize_t i = nrows; i < 16; ++i)
      {
        mesh_.hdr.Dummies[i].x = 0;
        mesh_.hdr.Dummies[i].y = 0;
        mesh_.hdr.Dummies[i].z = 0;
      }
              
      mesh_.hdr.NumDummies = static_cast<int>(nrows);
    }
      
    // Stats
    int num_parts() { return mesh_.hdr.NumParts; };
    int num_triags() { return mesh_.hdr.NumTriangles; };
    int num_verts() { return mesh_.hdr.NumVertices; };
    
    void print_parts() { fcelib::FCELIB_PrintMeshParts(mesh_); }
    void print_triags() { fcelib::FCELIB_PrintMeshTriangles(mesh_); }
    void print_verts() { fcelib::FCELIB_PrintMeshVertices(mesh_); }
    
  private:
    bool valid_;
    
    fcelib::FcelibMesh mesh_;
    std::vector<Part> parts_;
};


/* wrappers ---------------------------------------------------------------- */

/* fcecodec. */
void FCECODECMODULE_PrintFceInfo(const std::string &fcepath)
{
  unsigned char *fce_buf = NULL;
  int fce_size;

  printf("File = %s\n", fcepath.c_str());

  for (;;)
  {
    if (!fcelib::FCELIB_GetFileBufRead(&fce_buf, &fce_size, fcepath.c_str()))
    {
      std::fprintf(stderr, "Cannot read file '%s'\n", fcepath.c_str());
      break;
    }

    fcelib::FCELIB_PrintFceInfo(fce_size, fce_buf);

    if (!fcelib::FCELIB_FreeFileBuf((void *)fce_buf, fce_size))  /* always free file buffer */
    {
      std::fprintf(stderr, "Cannot free buffer\n");
    }
    // fce_buf = NULL;

    break;
  }
}

/* fcecodec. */
int FCECODECMODULE_ValidateFce(std::string buf)
{
  return fcelib::FCELIB_ValidateFce(buf.c_str(), buf.size());
}


/* ------------------------------------------------------------------------- */

PYBIND11_MODULE(fcecodec, fcecodec_module)
{
  fcecodec_module.doc() = "FCE decoder/encoder";
  
  fcecodec_module.def("print_fce_info", &FCECODECMODULE_PrintFceInfo, py::arg("fcepath"));
  fcecodec_module.def("fce_valid", &FCECODECMODULE_ValidateFce, py::arg("buf"));
  
  py::class_<Part>(fcecodec_module, "Part", py::buffer_protocol())
    .def(py::init<>())

    .def("num_triags", &Part::num_triags)
    .def("num_verts", &Part::num_verts)

    .def("get_name", &Part::get_name)
    .def("set_name", &Part::set_name, py::arg("name"))
    .def("get_pos", &Part::get_pos)
    .def("set_pos", &Part::set_pos, py::arg("position"))

//    .def("del_triags", &Part::get_triags_vidx, py::arg("idxs"), R"pbdoc( Deletes triangles given in index list. )pbdoc")
    .def("get_triags_vidx", &Part::get_triags_vidx, 
      R"pbdoc( Returns (N, 3) array of global vert indexes for N triangles. )pbdoc")
    /* set_triags_vidx() will not be implemented */
    .def("get_triags_flags", &Part::get_triags_flags)
    .def("set_triags_flags", &Part::set_triags_flags, py::arg("flags"), 
      R"pbdoc( Expects (N, ) array where N = Part.num_triags() )pbdoc")
    .def("get_triags_texpages", &Part::get_triags_texpages)
    .def("set_triags_texpages", &Part::set_triags_texpages, py::arg("texpages"), 
      R"pbdoc( Expects (N, ) array where N = Part.num_triags() )pbdoc")
    ;

  py::class_<Mesh>(fcecodec_module, "Mesh", py::buffer_protocol())
    .def(py::init<>())
    
    .def(py::init([](Mesh *mesh_src, const int idx) {
        fcelib::FcelibMesh *mesh_ = mesh_src->get_mesh_();
        Mesh *new_mesh;
        fcelib::FcelibMesh *new_mesh_;

        if (idx == mesh_->parts_len)
          throw std::out_of_range("part index (idx) out of range");

        new_mesh = new Mesh();
        new_mesh_ = new_mesh->get_mesh_();
        fcelib::FCELIB_InitMesh(new_mesh_);

        const int idx_new = fcelib::FCELIB_CopyPartToMesh(new_mesh_, mesh_, idx);
        if (idx_new < 0)
          throw std::runtime_error("fcecodec(mesh, part_idx):");

        std::vector<Part> *new_parts_;
        Part *ptr;
        const int idx_internal = FCECODECMODULE_aux_GetIndexByOrder(new_mesh_, idx_new);
        if (idx_internal < 0)
          throw std::out_of_range("fcecodec(mesh, part_idx): part index (idx_new) out of range");
        new_parts_ = new_mesh->get_parts_();
        new_parts_->resize(new_parts_->size() + 1);
        ptr = &new_parts_->at(new_parts_->size() - 1);
        ptr->set_part_(new_mesh_->parts[new_mesh_->hdr.Parts[idx_internal]]);
        ptr->set_ptriags__from_mesh(new_mesh_);
        ptr->set_pverts__from_mesh(new_mesh_);

        return new_mesh; 
      }),
      py::keep_alive<1, 2>(),
      py::arg("mesh_src"), py::arg("part_idx"),
      R"pbdoc( Copy specified part from mesh_src to new mesh. )pbdoc")

    .def("decode_fce", &Mesh::decode_fce, py::arg("buf"))
    .def("encode_fce3", &Mesh::encode_fce3, py::arg("fcepath"), py::arg("center_parts") = true)  // TODO: implement fcelib function that takes buffer instead of using fopen(), i.e., make Python responsible for file operations
    .def("encode_fce4", &Mesh::encode_fce4, py::arg("fcepath"), py::arg("center_parts") = true)
    .def("export_obj", &Mesh::export_obj, py::arg("objpath"), py::arg("mtlpath"), py::arg("texname"), py::arg("print_damage") = 0, py::arg("print_dummies") = 0)  // TODO: test

    .def("get_parts", &Mesh::get_parts, py::keep_alive<1, 0>())
    .def("center_part", &Mesh::center_part, py::arg("idx"), R"pbdoc( Center specified part pos to local centroid. )pbdoc")
    .def("copy_part", &Mesh::copy_part_to_mesh,
      py::arg("mesh_src"), py::arg("part_idx"),
      R"pbdoc( Copy specified part from mesh_src. Returns new part index. )pbdoc")
    .def("del_part", &Mesh::del_part, py::arg("idx"))
    .def("merge_parts", &Mesh::merge_parts, py::arg("idx1"), py::arg("idx2"), R"pbdoc( Returns new part index. )pbdoc")
    .def("move_part", &Mesh::move_up_part, py::arg("idx"), 
      R"pbdoc( Move up specified part towards order 0. Returns new part index. )pbdoc")
    .def("part_at", &Mesh::part_at, py::arg("idx"), py::keep_alive<1, 0>(), R"pbdoc( DEPRECATED )pbdoc")  // TODO: deprecated - remove

    .def("del_unrefd_verts", &Mesh::del_unrefd_verts, R"pbdoc( Delete vertices that are not referenced by any triangle. )pbdoc")  // TODO: test
    .def("get_verts_map_idx2order", &Mesh::get_verts_map_idx2order, R"pbdoc( Triangles contain global vert indexes. Via those, this vector maps to global vertex order. )pbdoc")
    .def("get_verts_pos", &Mesh::get_verts_pos, R"pbdoc( Local vertice positions. Returns (3*N, ) array for N vertices. )pbdoc")
    .def("get_verts_norms", &Mesh::get_verts_norms, R"pbdoc( Vertice normals. Returns (3*N, ) array for N vertices. )pbdoc")
    .def("get_dmgd_verts_pos", &Mesh::get_dmgd_verts_pos, R"pbdoc( Damage model local vertice positions. Returns (3*N, ) array for N vertices. )pbdoc")
    .def("get_dmgd_verts_norms", &Mesh::get_dmgd_verts_norms, R"pbdoc( Damage model vertice normals. Returns (3*N, ) array for N vertices. )pbdoc")
    .def("get_verts_anim", &Mesh::get_verts_anim, R"pbdoc( Vertice animation flag. Returns (N, ) array for N vertices. )pbdoc")
    .def("set_verts_anim", &Mesh::set_verts_anim, py::arg("data"), R"pbdoc( Expects (N, ) array where N = Part.num_verts() )pbdoc")

    .def("get_colors", &Mesh::get_colors)
    .def("set_colors", &Mesh::set_colors, py::arg("integers"), R"pbdoc( Expects shape=(N, 4, 4) )pbdoc")
    .def("get_dummy_names", &Mesh::get_dummy_names)
    .def("set_dummy_names", &Mesh::set_dummy_names, py::arg("strings"))
    .def("get_dummy_pos", &Mesh::get_dummy_pos)
    .def("set_dummy_pos", &Mesh::set_dummy_pos, py::arg("floats"), R"pbdoc( Expects shape (N*3, ) for N dummies )pbdoc")  // TODO: test setting additional

    .def("num_parts", &Mesh::num_parts)
    .def("num_triags", &Mesh::num_triags)
    .def("num_verts", &Mesh::num_verts)
    .def("info", &Mesh::info, R"pbdoc( Print stats to console. )pbdoc")
    .def("valid", &Mesh::valid)  // FIXME: test internal part indexes occuring multiple times
    .def("print_parts", &Mesh::print_parts)
    .def("print_triags", &Mesh::print_triags)
    .def("print_verts", &Mesh::print_verts)
    ;
}
