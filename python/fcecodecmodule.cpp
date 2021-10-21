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

//#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <utility>  // std::move
#include <vector>

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


/* classes, structs ----------------------------------------------------------------- */

class Mesh : public fcelib::FcelibMesh {
  public:
    Mesh() : mesh_(*this) { fcelib::FCELIB_InitMesh(&mesh_); printf("Mesh()\n"); }
    ~Mesh() { fcelib::FCELIB_FreeMesh(&mesh_); printf("~Mesh()\n");}
    
    // Internal
    fcelib::FcelibMesh *Get_mesh_() { return &mesh_; }

    // Service
    bool valid() const { return fcelib::FCELIB_ValidateMesh(mesh_); }
    
    /* Stats */
    void info() const { fcelib::FCELIB_PrintMeshInfo(mesh_); }
    void print_parts(void) const { fcelib::FCELIB_PrintMeshParts(mesh_); }
    void print_triags(void) const { fcelib::FCELIB_PrintMeshTriangles(mesh_); }
    void print_verts(void) const { fcelib::FCELIB_PrintMeshVertices(mesh_); }
    int num_parts() const { return mesh_.hdr.NumParts; };
    int num_triags() const { return mesh_.hdr.NumTriangles; };
    int num_verts() const { return mesh_.hdr.NumVertices; };
    
    /* i/o */
    void decode_fce(const std::string &buf);
    py::bytes encode_fce3(const bool center_parts) const;
    py::bytes encode_fce4(const bool center_parts) const;
    py::bytes encode_fce4m(const bool center_parts) const;
    void export_obj(std::string &objpath, std::string &mtlpath,
                    std::string &texture_name,
                    const int print_damage, const int print_dummies) const;
    
    /* Mesh / Header */
    py::buffer get_colors(void) const;
    void set_colors(py::array_t<unsigned char, py::array::c_style | py::array::forcecast> arr);
    std::vector<std::string> get_dummy_names() const;
    void set_dummy_names(std::vector<std::string> &arr);
    py::buffer get_dummy_pos(void) const;
    void set_dummy_pos(py::array_t<float, py::array::c_style | py::array::forcecast> arr);
    
    /* Verts */
    std::vector<int> GetVertsMap_idx2order() const;
    py::buffer GetVertsMap_idx2order_numpy() const;
    
    std::vector<float> GetVertsPos() const;
    void SetVertsPos(std::vector<float> arr);
    py::buffer GetVertsPos_numpy() const;
    void SetVertsPos_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr);
    
    std::vector<float> GetVertsNorms() const;
    void SetVertsNorms(std::vector<float> arr);
    py::buffer GetVertsNorms_numpy() const;
    void SetVertsNorms_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr);
    
    std::vector<float> GetDamgdVertsPos() const;
    void SetDamgdVertsPos(std::vector<float> arr);
    py::buffer GetDamgdVertsPos_numpy() const;
    void SetDamgdVertsPos_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr);
    
    std::vector<float> GetDamgdVertsNorms() const;
    void SetDamgdVertsNorms(std::vector<float> arr);
    py::buffer GetDamgdVertsNorms_numpy() const;
    void SetDamgdVertsNorms_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr);
    
    std::vector<float> GetVertsAnimation() const;
    void SetVertsAnimation(std::vector<float> arr);
    py::buffer GetVertsAnimation_numpy() const;
    void SetVertsAnimation_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr);
    
    /* Parts */
    int PNumTriags(const int pid) const;
    int PNumVerts(const int pid) const;
    
    const std::string GetPartName(const int pid) const;
    void SetPartName(const int pid, const std::string &s);
    const std::array<float, 3> GetPartPos(const int pid) const;
    void SetPartPos(const int pid, std::array<float, 3> &arr);
    
    /* Operations */
    bool center_part(const int pid);
    int InsertPart(Mesh *mesh_src, const int pid_src);
    bool del_part(const int pid);
    bool DelPartTriags(const int pid, const std::vector<int> &idxs);
    bool DelUnrefdVerts() { return fcelib::FCELIB_DeleteUnrefdVerts(&mesh_); }
    int merge_parts(const int pid1, const int pid2);
    int move_up_part(const int pid);

  private:
    fcelib::FcelibMesh& mesh_;
};


/* aux ---------------------------------------------------------------------- */

int FCECODECMODULE_aux_GetPartIndexByOrder(fcelib::FcelibMesh *mesh, const int pid)
{
  int i = 0;

  for (;;)
  {
    if ((pid < 0) || (pid >= mesh->parts_len))
    {
      fprintf(stderr, "aux_GetInternalPartIdxByOrder: part %d not found (len=%d)\n", pid, mesh->parts_len);
      i = -1;
      break;
    }

    for (int count = -1; i < mesh->parts_len; ++i)
    {
      if (mesh->hdr.Parts[i] > -1)
        ++count;
      if (count == pid)
        break;
    }

    if (i == mesh->parts_len)
    {
      fprintf(stderr, "aux_GetInternalPartIdxByOrder: part %d not found\n", pid);
      i = -1;
      break;
    }

    break;
  }

  return i;
}


/* Mesh:: wrappers ---------------------------------------------------------- */

void Mesh::decode_fce(const std::string &buf) 
{
  fcelib::FCELIB_FreeMesh(&mesh_);
  fcelib::FCELIB_InitMesh(&mesh_);
  if (!fcelib::FCELIB_DecodeFce(buf.c_str(), buf.size(), &mesh_))
    throw std::runtime_error("decode_fce: Cannot parse FCE data");
}
    
py::bytes Mesh::encode_fce3(const bool center_parts) const 
{
  const std::size_t bufsize_ = static_cast<std::size_t>(fcelib::FCELIB_FCETYPES_Fce3ComputeSize(mesh_.hdr.NumVertices, mesh_.hdr.NumTriangles));
  unsigned char *buf_ = (unsigned char *)malloc(bufsize_ * sizeof(*buf_));
  if (!buf_)
    throw std::runtime_error("encode_fce3: Cannot allocate memory");
  
  if (!fcelib::FCELIB_EncodeFce3(&buf_, bufsize_, &mesh_, static_cast<int>(center_parts)))
    throw std::runtime_error("encode_fce3: Cannot encode FCE3");

  py::bytes result = py::bytes((char *)buf_, bufsize_);
  free(buf_);
  return result;
}

py::bytes Mesh::encode_fce4(const bool center_parts) const
{
  py::print("Warning: fcelib::FCELIB_EncodeFce4() not yet implemented");
  
  const std::size_t bufsize_ = fcelib::FCELIB_FCETYPES_Fce4ComputeSize(0x00101014, mesh_.hdr.NumVertices, mesh_.hdr.NumTriangles);
  unsigned char *buf_ = (unsigned char *)malloc(bufsize_ * sizeof(*buf_));
  if (!buf_)
    throw std::runtime_error("encode_fce4: Cannot allocate memory");
  
  if (!fcelib::FCELIB_EncodeFce4(&buf_, bufsize_, &mesh_, static_cast<int>(center_parts)))
    throw std::runtime_error("Cannot encode FCE4");
  
  py::bytes result = py::bytes((char *)buf_, bufsize_);
  free(buf_);
  return result;
}

py::bytes Mesh::encode_fce4m(const bool center_parts) const
{
  py::print("Warning: encode_fce4m: experimental, untested, unsupported");
    
  const std::size_t bufsize_ = fcelib::FCELIB_FCETYPES_Fce4ComputeSize(0x00101015, mesh_.hdr.NumVertices, mesh_.hdr.NumTriangles);
  unsigned char *buf_ = (unsigned char *)malloc(bufsize_ * sizeof(*buf_));
  if (!buf_)
    throw std::runtime_error("encode_fce4m: Cannot allocate memory");
  
  if (!fcelib::FCELIB_EncodeFce4M(&buf_, bufsize_, &mesh_, static_cast<int>(center_parts)))
    throw std::runtime_error("Cannot encode FCE4M");
  
  py::bytes result = py::bytes((char *)buf_, bufsize_);
  free(buf_);
  return result;
}

void Mesh::export_obj(std::string &objpath, std::string &mtlpath,
                      std::string &texture_name,
                      const int print_damage, const int print_dummies) const
{
  if (fcelib::FCELIB_ExportObj(&mesh_, objpath.c_str(), mtlpath.c_str(), 
                                   texture_name.c_str(),
                                   print_damage, print_dummies) == 0)
    throw std::runtime_error("Cannot export OBJ");
}






py::buffer Mesh::get_colors() const
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
  
  return std::move(result);
}
void Mesh::set_colors(py::array_t<unsigned char, py::array::c_style | py::array::forcecast> arr)
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



std::vector<std::string> Mesh::get_dummy_names() const
{
  const int len = mesh_.hdr.NumDummies;
  std::vector<std::string> retv;
  retv.resize(len);
  
  for (int i = 0; i < len; ++i)
    retv.at(i) = std::string(mesh_.hdr.DummyNames + i * 64);
    
  return retv;
}

void Mesh::set_dummy_names(std::vector<std::string> &arr)
{
  std::string *ptr;
  const std::size_t nrows = arr.size();
  
  std::memset(mesh_.hdr.DummyNames, '\0', (std::size_t)(16 * 64) * sizeof(char));
  
  for (std::size_t i = 0; i < nrows && i < 16; ++i)
  {
    ptr = &arr.at(i);
    std::strncpy(mesh_.hdr.DummyNames + i * 64, ptr->c_str(), std::min((std::size_t)63, ptr->size()) * sizeof(char));
  }
          
  mesh_.hdr.NumDummies = static_cast<int>(nrows);
}

py::buffer Mesh::get_dummy_pos() const
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
  return std::move(result);
}

void Mesh::set_dummy_pos(py::array_t<float, py::array::c_style | py::array::forcecast> arr)
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


/* verts ---------------------------- */

/* Via vector index (=global vert idx) map to global vert order. */
std::vector<int> Mesh::GetVertsMap_idx2order() const
{
  const int nrows = mesh_.vertices_len;
  std::vector<int> result = std::vector<int>(static_cast<std::size_t>(nrows), -1);
  int *ptr = result.data();
  fcelib::FcelibPart *part;
  // i - global vert index, j - global vert order
  int i = 0;
  int j = 0;
  for (int k = 0; k < mesh_.parts_len; ++k)
  {
    if (mesh_.hdr.Parts[k] < 0)
      continue;
    part = mesh_.parts[ mesh_.hdr.Parts[k] ];
    
    for (int m = 0; m < part->pvertices_len; ++m, ++i)
    {
      if(part->PVertices[m] < 0)
        continue;
      ptr[i] = j;
      ++j;
    }  // for i
  }  // for k
  
  return result;
}
/* Via vector index (=global vert idx) map to global vert order. */
py::buffer Mesh::GetVertsMap_idx2order_numpy() const
{
  const py::ssize_t nrows = mesh_.vertices_len;
  py::array_t<int> result = py::array_t<int>({ nrows }, {  });
  auto buf = result.mutable_unchecked<>();
  // i - global vert index, j - global vert order
  for (py::ssize_t i = 0, j = 0; i < nrows; ++i)
  {
    if(!mesh_.vertices[i])
    {
      buf(i) = -1;
      continue;
    }
    buf(i) = j;
    ++j;
  }
  return std::move(result);
}

std::vector<float> Mesh::GetVertsPos() const
{
  const std::size_t nrows = mesh_.hdr.NumVertices;
  std::vector<float> result = std::vector<float>(nrows * 3);
  float *ptr = result.data();
  fcelib::FcelibVertex *vert;
  // i - global vert index, j - global vert order
  for (std::size_t i = 0, j = 0; i < nrows; ++i)
  {
    vert = mesh_.vertices[i];
    if(!vert)
      continue;
    ptr[j * 3 + 0] = vert->VertPos.x;
    ptr[j * 3 + 1] = vert->VertPos.y;
    ptr[j * 3 + 2] = vert->VertPos.z;
    ++j;
  }
  return result;
}
void Mesh::SetVertsPos(std::vector<float> arr)
{
  const std::size_t nrows = mesh_.hdr.NumVertices;
  if (arr.size() != nrows * 3)
    throw std::runtime_error("Shape must be (N*3, ) where N = Mesh.num_verts()");
  float *ptr = arr.data();
  fcelib::FcelibVertex *vert;
  // i - global vert index, j - global vert order
  for (std::size_t i = 0, j = 0; i < nrows; ++i)
  {
    vert = mesh_.vertices[i];
    if(!vert)
      continue;
     vert->VertPos.x = ptr[j * 3 + 0];
     vert->VertPos.y = ptr[j * 3 + 1];
     vert->VertPos.z = ptr[j * 3 + 2];
    ++j;
  }
}

py::buffer Mesh::GetVertsPos_numpy() const
{
  const py::ssize_t nrows = static_cast<py::ssize_t>(mesh_.hdr.NumVertices);
  py::array_t<float> result = py::array_t<float>({ nrows * 3 }, {  });
  auto buf = result.mutable_unchecked<>();
  fcelib::FcelibVertex *vert;
  // i - global vert index, j - global vert order
  for (py::ssize_t i = 0, j = 0; i < nrows; ++i)
  {
    vert = mesh_.vertices[i];
    if(!vert)
      continue;
    buf(j * 3 + 0) = vert->VertPos.x;
    buf(j * 3 + 1) = vert->VertPos.y;
    buf(j * 3 + 2) = vert->VertPos.z;
    ++j;
  }
  return std::move(result);
}
void Mesh::SetVertsPos_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr)
{
  const py::ssize_t nrows = static_cast<py::ssize_t>(mesh_.hdr.NumVertices);
  py::buffer_info buf = arr.request();
  if (buf.ndim != 1)
    throw std::runtime_error("Number of dimensions must be 1");
  if (buf.shape[0] != nrows * 3)
    throw std::runtime_error("Shape must be (N*3, ) where N = Mesh.num_verts()");
  float *ptr = static_cast<float *>(buf.ptr);
  fcelib::FcelibVertex *vert;
  // i - global vert index, j - global vert order
  for (py::ssize_t i = 0, j = 0; i < nrows; ++i)
  {
    vert = mesh_.vertices[i];
    if(!vert)
      continue;
     vert->VertPos.x = ptr[j * 3 + 0];
     vert->VertPos.y = ptr[j * 3 + 1];
     vert->VertPos.z = ptr[j * 3 + 2];
    ++j;
  }
}


std::vector<float> Mesh::GetVertsNorms() const
{
  const std::size_t nrows = mesh_.hdr.NumVertices;
  std::vector<float> result = std::vector<float>(nrows * 3);
  float *ptr = result.data();
  fcelib::FcelibVertex *vert;
  // i - global vert index, j - global vert order
  for (std::size_t i = 0, j = 0; i < nrows; ++i)
  {
    vert = mesh_.vertices[i];
    if(!vert)
      continue;
    ptr[j * 3 + 0] = vert->NormPos.x;
    ptr[j * 3 + 1] = vert->NormPos.y;
    ptr[j * 3 + 2] = vert->NormPos.z;
    ++j;
  }
  return result;
}
void Mesh::SetVertsNorms(std::vector<float> arr)
{
  const std::size_t nrows = mesh_.hdr.NumVertices;
  if (arr.size() != nrows * 3)
    throw std::runtime_error("Shape must be (N*3, ) where N = Mesh.num_verts()");
  float *ptr = arr.data();
  fcelib::FcelibVertex *vert;
  // i - global vert index, j - global vert order
  for (std::size_t i = 0, j = 0; i < nrows; ++i)
  {
    vert = mesh_.vertices[i];
    if(!vert)
      continue;
     vert->NormPos.x = ptr[j * 3 + 0];
     vert->NormPos.y = ptr[j * 3 + 1];
     vert->NormPos.z = ptr[j * 3 + 2];
    ++j;
  }
}

py::buffer Mesh::GetVertsNorms_numpy() const
{
  const py::ssize_t nrows = static_cast<py::ssize_t>(mesh_.hdr.NumVertices);
  py::array_t<float> result = py::array_t<float>({ nrows * 3 }, {  });
  auto buf = result.mutable_unchecked<>();
  fcelib::FcelibVertex *vert;
  // i - global vert index, j - global vert order
  for (py::ssize_t i = 0, j = 0; i < nrows; ++i)
  {
    vert = mesh_.vertices[i];
    if(!vert)
      continue;
    buf(j * 3 + 0) = vert->NormPos.x;
    buf(j * 3 + 1) = vert->NormPos.y;
    buf(j * 3 + 2) = vert->NormPos.z;
    ++j;
  }
  return std::move(result);
}
void Mesh::SetVertsNorms_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr)
{
  const py::ssize_t nrows = static_cast<py::ssize_t>(mesh_.hdr.NumVertices);
  py::buffer_info buf = arr.request();
  if (buf.ndim != 1)
    throw std::runtime_error("Number of dimensions must be 1");
  if (buf.shape[0] != nrows * 3)
    throw std::runtime_error("Shape must be (N*3, ) where N = Mesh.num_verts()");
  float *ptr = static_cast<float *>(buf.ptr);
  fcelib::FcelibVertex *vert;
  // i - global vert index, j - global vert order
  for (py::ssize_t i = 0, j = 0; i < nrows; ++i)
  {
    vert = mesh_.vertices[i];
    if(!vert)
      continue;
     vert->NormPos.x = ptr[j * 3 + 0];
     vert->NormPos.y = ptr[j * 3 + 1];
     vert->NormPos.z = ptr[j * 3 + 2];
    ++j;
  }
}


std::vector<float> Mesh::GetDamgdVertsPos() const
{
  const std::size_t nrows = mesh_.hdr.NumVertices;
  std::vector<float> result = std::vector<float>(nrows * 3);
  float *ptr = result.data();
  fcelib::FcelibVertex *vert;
  // i - global vert index, j - global vert order
  for (std::size_t i = 0, j = 0; i < nrows; ++i)
  {
    vert = mesh_.vertices[i];
    if(!vert)
      continue;
    ptr[j * 3 + 0] = vert->DamgdVertPos.x;
    ptr[j * 3 + 1] = vert->DamgdVertPos.y;
    ptr[j * 3 + 2] = vert->DamgdVertPos.z;
    ++j;
  }
  return result;
}
void Mesh::SetDamgdVertsPos(std::vector<float> arr)
{
  const std::size_t nrows = mesh_.hdr.NumVertices;
  if (arr.size() != nrows * 3)
    throw std::runtime_error("Shape must be (N*3, ) where N = Mesh.num_verts()");
  float *ptr = arr.data();
  fcelib::FcelibVertex *vert;
  // i - global vert index, j - global vert order
  for (std::size_t i = 0, j = 0; i < nrows; ++i)
  {
    vert = mesh_.vertices[i];
    if(!vert)
      continue;
     vert->DamgdVertPos.x = ptr[j * 3 + 0];
     vert->DamgdVertPos.y = ptr[j * 3 + 1];
     vert->DamgdVertPos.z = ptr[j * 3 + 2];
    ++j;
  }
}

py::buffer Mesh::GetDamgdVertsPos_numpy() const
{
  const py::ssize_t nrows = static_cast<py::ssize_t>(mesh_.hdr.NumVertices);
  py::array_t<float> result = py::array_t<float>({ nrows * 3 }, {  });
  auto buf = result.mutable_unchecked<>();
  fcelib::FcelibVertex *vert;
  // i - global vert index, j - global vert order
  for (py::ssize_t i = 0, j = 0; i < nrows; ++i)
  {
    vert = mesh_.vertices[i];
    if(!vert)
      continue;
    buf(j * 3 + 0) = vert->DamgdVertPos.x;
    buf(j * 3 + 1) = vert->DamgdVertPos.y;
    buf(j * 3 + 2) = vert->DamgdVertPos.z;
    ++j;
  }
  return std::move(result);
}
void Mesh::SetDamgdVertsPos_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr)
{
  const py::ssize_t nrows = static_cast<py::ssize_t>(mesh_.hdr.NumVertices);
  py::buffer_info buf = arr.request();
  if (buf.ndim != 1)
    throw std::runtime_error("Number of dimensions must be 1");
  if (buf.shape[0] != nrows * 3)
    throw std::runtime_error("Shape must be (N*3, ) where N = Mesh.num_verts()");
  float *ptr = static_cast<float *>(buf.ptr);
  fcelib::FcelibVertex *vert;
  // i - global vert index, j - global vert order
  for (py::ssize_t i = 0, j = 0; i < nrows; ++i)
  {
    vert = mesh_.vertices[i];
    if(!vert)
      continue;
     vert->DamgdVertPos.x = ptr[j * 3 + 0];
     vert->DamgdVertPos.y = ptr[j * 3 + 1];
     vert->DamgdVertPos.z = ptr[j * 3 + 2];
    ++j;
  }
}


std::vector<float> Mesh::GetDamgdVertsNorms() const
{
  const std::size_t nrows = mesh_.hdr.NumVertices;
  std::vector<float> result = std::vector<float>(nrows * 3);
  float *ptr = result.data();
  fcelib::FcelibVertex *vert;
  // i - global vert index, j - global vert order
  for (std::size_t i = 0, j = 0; i < nrows; ++i)
  {
    vert = mesh_.vertices[i];
    if(!vert)
      continue;
    ptr[j * 3 + 0] = vert->DamgdNormPos.x;
    ptr[j * 3 + 1] = vert->DamgdNormPos.y;
    ptr[j * 3 + 2] = vert->DamgdNormPos.z;
    ++j;
  }
  return result;
}
void Mesh::SetDamgdVertsNorms(std::vector<float> arr)
{
  const std::size_t nrows = mesh_.hdr.NumVertices;
  if (arr.size() != nrows * 3)
    throw std::runtime_error("Shape must be (N*3, ) where N = Mesh.num_verts()");
  float *ptr = arr.data();
  fcelib::FcelibVertex *vert;
  // i - global vert index, j - global vert order
  for (std::size_t i = 0, j = 0; i < nrows; ++i)
  {
    vert = mesh_.vertices[i];
    if(!vert)
      continue;
     vert->DamgdNormPos.x = ptr[j * 3 + 0];
     vert->DamgdNormPos.y = ptr[j * 3 + 1];
     vert->DamgdNormPos.z = ptr[j * 3 + 2];
    ++j;
  }
}

py::buffer Mesh::GetDamgdVertsNorms_numpy() const
{
  const py::ssize_t nrows = static_cast<py::ssize_t>(mesh_.hdr.NumVertices);
  py::array_t<float> result = py::array_t<float>({ nrows * 3 }, {  });
  auto buf = result.mutable_unchecked<>();
  fcelib::FcelibVertex *vert;
  // i - global vert index, j - global vert order
  for (py::ssize_t i = 0, j = 0; i < nrows; ++i)
  {
    vert = mesh_.vertices[i];
    if(!vert)
      continue;
    buf(j * 3 + 0) = vert->DamgdNormPos.x;
    buf(j * 3 + 1) = vert->DamgdNormPos.y;
    buf(j * 3 + 2) = vert->DamgdNormPos.z;
    ++j;
  }
  return std::move(result);
}
void Mesh::SetDamgdVertsNorms_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr)
{
  const py::ssize_t nrows = static_cast<py::ssize_t>(mesh_.hdr.NumVertices);
  py::buffer_info buf = arr.request();
  if (buf.ndim != 1)
    throw std::runtime_error("Number of dimensions must be 1");
  if (buf.shape[0] != nrows * 3)
    throw std::runtime_error("Shape must be (N*3, ) where N = Mesh.num_verts()");
  float *ptr = static_cast<float *>(buf.ptr);
  fcelib::FcelibVertex *vert;
  // i - global vert index, j - global vert order
  for (py::ssize_t i = 0, j = 0; i < nrows; ++i)
  {
    vert = mesh_.vertices[i];
    if(!vert)
      continue;
     vert->DamgdNormPos.x = ptr[j * 3 + 0];
     vert->DamgdNormPos.y = ptr[j * 3 + 1];
     vert->DamgdNormPos.z = ptr[j * 3 + 2];
    ++j;
  }
}


std::vector<int> Mesh::GetVertsAnimation() const
{
  const std::size_t nrows = mesh_.hdr.NumVertices;
  std::vector<int> result = std::vector<int>(nrows);
  int *ptr = result.data();
  fcelib::FcelibVertex *vert;
  // i - global vert index, j - global vert order
  for (std::size_t i = 0, j = 0; i < nrows; ++i)
  {
    vert = mesh_.vertices[i];
    if(!vert)
      continue;
    ptr[j] = vert->Animation;
    ++j;
  }
  return result;
}
void Mesh::SetVertsAnimation(std::vector<int> arr)
{
  const std::size_t nrows = mesh_.hdr.NumVertices;
  if (arr.size() != nrows)
    throw std::runtime_error("Shape must be (N, ) where N = Mesh.num_verts()");
  int *ptr = arr.data();
  fcelib::FcelibVertex *vert;
  // i - global vert index, j - global vert order
  for (std::size_t i = 0, j = 0; i < nrows; ++i)
  {
    vert = mesh_.vertices[i];
    if(!vert)
      continue;
     vert->Animation = ptr[j];
    ++j;
  }
}

py::buffer Mesh::GetVertsAnimation_numpy() const
{
  const py::ssize_t nrows = static_cast<py::ssize_t>(mesh_.hdr.NumVertices);
  py::array_t<int> result = py::array_t<int>({ nrows }, {  });
  auto buf = result.mutable_unchecked<>();
  fcelib::FcelibVertex *vert;
  // i - global vert index, j - global vert order
  for (py::ssize_t i = 0, j = 0; i < nrows; ++i)
  {
    vert = mesh_.vertices[i];
    if(!vert)
      continue;
    buf(j) = vert->Animation;
    ++j;
  }
  return std::move(result);
}
void Mesh::SetVertsAnimation_numpy(py::array_t<int, py::array::c_style | py::array::forcecast> arr)
{
  const py::ssize_t nrows = static_cast<py::ssize_t>(mesh_.hdr.NumVertices);
  py::buffer_info buf = arr.request();
  if (buf.ndim != 1)
    throw std::runtime_error("Number of dimensions must be 1");
  if (buf.shape[0] != nrows)
    throw std::runtime_error("Shape must be (N*3, ) where N = Mesh.num_verts()");
  int *ptr = static_cast<int *>(buf.ptr);
  fcelib::FcelibVertex *vert;
  // i - global vert index, j - global vert order
  for (py::ssize_t i = 0, j = 0; i < nrows; ++i)
  {
    vert = mesh_.vertices[i];
    if(!vert)
      continue;
     vert->Animation = ptr[j];
    ++j;
  }
}


/* part ----------------------------- */

int Mesh::PNumTriags(const int pid) const
{
  const int idx = FCECODECMODULE_aux_GetPartIndexByOrder(&mesh_, pid);
  if (idx < 0) 
    throw std::out_of_range("Mesh.PNumTriags(): part index (pid) out of range");
  return mesh_.parts[ mesh_.hdr.Parts[idx] ]->PNumTriangles;
}
int Mesh::PNumVerts(const int pid) const
{
  const int idx = FCECODECMODULE_aux_GetPartIndexByOrder(&mesh_, pid);
  if (idx < 0) 
    throw std::out_of_range("Mesh.PNumVerts(): part index (pid) out of range");
  return mesh_.parts[ mesh_.hdr.Parts[idx] ]->PNumVertices;
}


const std::string Mesh::GetPartName(const int pid) const
{
  const int idx = FCECODECMODULE_aux_GetPartIndexByOrder(&mesh_, pid);
  if (idx < 0) 
    throw std::out_of_range("Mesh.get_partname(): part index (pid) out of range");
  return std::string(mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartName);
}
void Mesh::SetPartName(const int pid, const std::string &s) 
{
  const int idx = FCECODECMODULE_aux_GetPartIndexByOrder(&mesh_, pid);
  if (idx < 0)
    throw std::out_of_range("Mesh.get_partname(): part index (pid) out of range");
  std::strncpy(mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartName, s.c_str(), 
               sizeof(mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartName) - 1);  // max 63 chars
}

const std::array<float, 3> Mesh::GetPartPos(const int pid) const
{
  const int idx = FCECODECMODULE_aux_GetPartIndexByOrder(&mesh_, pid);
  if (idx < 0) 
    throw std::out_of_range("Mesh.GetPartPos(): part index (pid) out of range");
  std::array<float, 3> result;
  result[0] = mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.x;
  result[1] = mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.y;
  result[2] = mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.z;
  return result;
}
void Mesh::SetPartPos(const int pid, std::array<float, 3> &arr) 
{
  const int idx = FCECODECMODULE_aux_GetPartIndexByOrder(&mesh_, pid);
  if (idx < 0)
    throw std::out_of_range("Mesh.SetPartPos(): part index (pid) out of range");
  mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.x = arr[0];
  mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.y = arr[1];
  mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.z = arr[2];
}


/* mesh: operations ----------------- */

bool Mesh::center_part(const int pid)
{
  if (pid > mesh_.hdr.NumParts || pid < 0) 
    throw std::out_of_range("Mesh.center_part(): part index (pid) out of range");
  return fcelib::FCELIB_CenterPart(&mesh_, pid);
}

int Mesh::InsertPart(Mesh *mesh_src, const int pid_src)
{
  fcelib::FcelibMesh *mesh_src_ = mesh_src->Get_mesh_();
  if (pid_src > mesh_src_->hdr.NumParts || pid_src < 0)
    throw std::out_of_range("InsertPart(): part index (pid_src) out of range");
  const int pid_new = fcelib::FCELIB_CopyPartToMesh(&this->mesh_, mesh_src_, pid_src);
  if (pid_new < 0)
    throw std::runtime_error("InsertPart(): Cannot copy part");
  return pid_new;
}

bool Mesh::del_part(const int pid)
{
  if (pid > mesh_.hdr.NumParts || pid < 0)
    throw std::out_of_range("Mesh.del_part(): part index (pid) out of range");
  return fcelib::FCELIB_DeletePart(&mesh_, pid); 
}

bool Mesh::DelPartTriags(const int pid, const std::vector<int> &idxs)
{
  if (pid > mesh_.hdr.NumParts || pid < 0)
    throw std::out_of_range("Mesh.DelPartTriags(): part index (pid) out of range");
  return fcelib::FCELIB_DeletePartTriags(&mesh_, pid, idxs.data(), static_cast<int>(idxs.size()));
}

int Mesh::merge_parts(const int pid1, const int pid2)
{
  if (pid1 > mesh_.hdr.NumParts || pid1 < 0)
    throw std::out_of_range("Mesh.merge_parts(): part index (pid1) out of range");
  if (pid2 > mesh_.hdr.NumParts || pid2 < 0)
    throw std::out_of_range("Mesh.merge_parts(): part index (pid2) out of range");
  const int pid_new = fcelib::FCELIB_MergePartsToNew(&mesh_, pid1, pid2);
  if (pid_new < 0)
    throw std::runtime_error("Mesh.merge_parts()");
  return pid_new;
}

int Mesh::move_up_part(const int pid)
{
  if (pid > mesh_.hdr.NumParts || pid < 0)
    throw std::out_of_range("Mesh.move_up_part(): part index (pid) out of range");
  return fcelib::FCELIB_MeshMoveUpPart(&mesh_, pid);
}


/* wrappers ---------------------------------------------------------------- */

/* fcecodec. */
void FCECODECMODULE_PrintFceInfo(const std::string &buf)
{
  const std::size_t size = buf.size();
  if (size < 0x1F04) 
    throw std::runtime_error("PrintFceInfo: Invalid buffer size (expects >= 0x1F04)");
  fcelib::FCELIB_PrintFceInfo(size, std::move(buf.c_str()));
}

/* fcecodec. */
int FCECODECMODULE_ValidateFce(const std::string &buf)
{
  return fcelib::FCELIB_ValidateFce(buf.c_str(), buf.size());
}


/* ------------------------------------------------------------------------- */

PYBIND11_MODULE(fcecodec, fcecodec_module)
{
  fcecodec_module.doc() = "FCE decoder/encoder";
  
  fcecodec_module.def("print_fce_info", &FCECODECMODULE_PrintFceInfo, py::arg("buf"));
  fcecodec_module.def("fce_valid", &FCECODECMODULE_ValidateFce, py::arg("buf"));

  py::class_<Mesh>(fcecodec_module, "Mesh", py::buffer_protocol())
    .def(py::init<>())
    
    .def("valid", &Mesh::valid)
    
    .def("info", &Mesh::info)
    .def("print_parts", &Mesh::print_parts)
    .def("print_triags", &Mesh::print_triags)
    .def("print_verts", &Mesh::print_verts)
    .def_property_readonly("num_parts", &Mesh::num_parts)
    .def_property_readonly("num_triags", &Mesh::num_triags)
    .def_property_readonly("num_verts", &Mesh::num_verts)

    .def("decode", &Mesh::decode_fce)
    /* .def("encode_fce3", &Mesh::encode_fce3_fopen, py::arg("fcepath"), py::arg("center_parts") = true) */
    .def("encode_fce3", &Mesh::encode_fce3, py::arg("center_parts") = true)
    .def("encode_fce4", &Mesh::encode_fce4, py::arg("center_parts") = true)  // TODO: implement
    .def("encode_fce4m", &Mesh::encode_fce4m, py::arg("center_parts") = true)
    .def("export_obj", &Mesh::export_obj, py::arg("objpath"), py::arg("mtlpath"), py::arg("texname"), py::arg("print_damage") = 0, py::arg("print_dummies") = 0)  // TODO: test

    .def("get_colors", &Mesh::get_colors)
    .def("set_colors", &Mesh::set_colors, py::arg("integers"), R"pbdoc( Expects shape=(N, 4, 4) )pbdoc")
    .def("get_dummy_names", &Mesh::get_dummy_names)
    .def("set_dummy_names", &Mesh::set_dummy_names, py::arg("strings"))
    .def("get_dummy_pos", &Mesh::get_dummy_pos)
    .def("set_dummy_pos", &Mesh::set_dummy_pos, py::arg("floats"), R"pbdoc( Expects shape (N*3, ) for N dummies )pbdoc")

    .def("GetVertsMap_idx2order", &Mesh::GetVertsMap_idx2order, R"pbdoc( Maps from global vert indexes (contained in triangles) to global vertex order. )pbdoc")
    .def("GetVertsMap_idx2order_numpy", &Mesh::GetVertsMap_idx2order_numpy, R"pbdoc( Maps from global vert indexes (contained in triangles) to global vertex order. )pbdoc")
    .def_property("VertsPos", &Mesh::GetVertsPos, &Mesh::SetVertsPos, R"pbdoc( Local vertice positions. Returns (N*3, ) array for N vertices. )pbdoc")
    .def_property("VertsPos_numpy", &Mesh::GetVertsPos_numpy, &Mesh::SetVertsPos_numpy, R"pbdoc( Local vertice positions. Returns (N*3, ) numpy array for N vertices. )pbdoc")
    .def_property("VertsNorms", &Mesh::GetVertsNorms, &Mesh::SetVertsNorms, R"pbdoc( Returns (N*3, ) array for N vertices. )pbdoc")
    .def_property("VertsNorms_numpy", &Mesh::GetVertsNorms_numpy, &Mesh::SetVertsNorms_numpy, R"pbdoc( Returns (N*3, ) numpy array for N vertices. )pbdoc")
    .def_property("DamgdVertsPos", &Mesh::GetDamgdVertsPos, &Mesh::SetDamgdVertsPos, R"pbdoc( Local vertice positions. Returns (N*3, ) array for N vertices. )pbdoc")
    .def_property("DamgdVertsPos_numpy", &Mesh::GetDamgdVertsPos_numpy, &Mesh::SetDamgdVertsPos_numpy, R"pbdoc( Local vertice positions. Returns (N*3, ) numpy array for N vertices. )pbdoc")
    .def_property("DamgdVertsNorms", &Mesh::GetDamgdVertsNorms, &Mesh::SetDamgdVertsNorms, R"pbdoc( Returns (N*3, ) array for N vertices. )pbdoc")
    .def_property("DamgdVertsNorms_numpy", &Mesh::GetDamgdVertsNorms_numpy, &Mesh::SetDamgdVertsNorms_numpy, R"pbdoc( Returns (N*3, ) numpy array for N vertices. )pbdoc")
    .def_property("VertsAnimation", &Mesh::GetVertsAnimation, &Mesh::SetVertsAnimation, R"pbdoc( Returns (N, ) array for N vertices. )pbdoc")
    .def_property("VertsAnimation_numpy", &Mesh::GetVertsAnimation_numpy, &Mesh::SetVertsAnimation_numpy, R"pbdoc( Returns (N, ) numpy array for N vertices. )pbdoc")

    .def("PNumTriags", &Mesh::PNumTriags)
    .def("PNumVerts", &Mesh::PNumVerts)

    .def("GetPartName", &Mesh::GetPartName)
    .def("SetPartName", &Mesh::SetPartName)
    .def("GetPartPos", &Mesh::GetPartPos)
    .def("SetPartPos", &Mesh::SetPartPos)

# if 1 && 0
    .def("get_triags_vidx", &Part::get_triags_vidx, 
      R"pbdoc( Returns (N, 3) array of global vert indexes for N triangles. )pbdoc")
    /* set_triags_vidx() is not in scope */
    .def("get_triags_flags", &Part::get_triags_flags)
    .def("set_triags_flags", &Part::set_triags_flags, py::arg("flags"), R"pbdoc( Expects (N, ) array where N = Part.num_triags() )pbdoc")
    .def("get_triags_texpages", &Part::get_triags_texpages)
    .def("set_triags_texpages", &Part::set_triags_texpages, py::arg("texpages"), R"pbdoc( Expects (N, ) array where N = Part.num_triags() )pbdoc")
#endif

    .def("center_part", &Mesh::center_part, py::arg("pid"), R"pbdoc( Center specified part pos to local centroid. )pbdoc")
    .def("InsertPart", &Mesh::InsertPart, py::arg("mesh_src"), py::arg("part_idx"), R"pbdoc( Insert (copy) specified part from mesh_src. Returns new part index. )pbdoc")
    .def("del_part", &Mesh::del_part, py::arg("pid"))
    .def("DelPartTriags", &Mesh::DelPartTriags, py::arg("pid"), py::arg("idxs"))
    .def("DelUnrefdVerts", &Mesh::DelUnrefdVerts, R"pbdoc( Delete vertices that are not referenced by any triangle. )pbdoc")
    .def("merge_parts", &Mesh::merge_parts, py::arg("pid1"), py::arg("pid2"), R"pbdoc( Returns new part index. )pbdoc")
    .def("move_part", &Mesh::move_up_part, py::arg("pid"), R"pbdoc( Move up specified part towards order 0. Returns new part index. )pbdoc")
    ;
}
