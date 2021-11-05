/*
  fcecodecmodule.c - Python module
  fcecodec Copyright (C) 2021 Benjamin Futasz <https://github.com/bfut>

  You may not redistribute this program without its source code.

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
      python -m pip install --upgrade pip wheel setuptools pybind11
      python -m pip install -e .
 **/

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

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
    Mesh() : mesh_(*this) { fcelib::FCELIB_InitMesh(&mesh_); }
    ~Mesh() { fcelib::FCELIB_FreeMesh(&mesh_); }

    // Internal
    fcelib::FcelibMesh *Get_mesh_() { return &mesh_; }

    // Service
    bool Valid() const { return fcelib::FCELIB_ValidateMesh(mesh_); }

    /* Stats */
    void Info() const { fcelib::FCELIB_PrintMeshInfo(mesh_); }
    void PrintParts(void) const { fcelib::FCELIB_PrintMeshParts(mesh_); }
    void PrintTriags(void) const { fcelib::FCELIB_PrintMeshTriangles(mesh_); }
    void PrintVerts(void) const { fcelib::FCELIB_PrintMeshVertices(mesh_); }
    int MNumArts() const { return mesh_.hdr.NumArts; };
    int num_parts() const { return mesh_.hdr.NumParts; };
    int num_triags() const { return mesh_.hdr.NumTriangles; };
    int num_verts() const { return mesh_.hdr.NumVertices; };

    /* i/o */
    void Decode(const std::string &buf);
    py::bytes Encode_Fce3(const bool center_parts) const;
    py::bytes Encode_Fce4(const bool center_parts) const;
    py::bytes Encode_Fce4M(const bool center_parts) const;
    void ExportObj(std::string &objpath, std::string &mtlpath,
                    std::string &texture_name,
                    const int print_damage, const int print_dummies) const;
    int IoGeomDataToNewPart_numpy(py::array_t<int, py::array::c_style | py::array::forcecast> vert_idxs,
                                  py::array_t<float, py::array::c_style | py::array::forcecast> vert_texcoords,
                                  py::array_t<float, py::array::c_style | py::array::forcecast> vert_pos,
                                  py::array_t<float, py::array::c_style | py::array::forcecast> normals);

    /* Mesh / Header */
    py::buffer MGetColors_numpy(void) const;
    void MSetColors_numpy(py::array_t<unsigned char, py::array::c_style | py::array::forcecast> arr);
    std::vector<std::string> GetDummyNames() const;
    void SetDummyNames(std::vector<std::string> &arr);
    py::buffer MGetDummyPos_numpy(void) const;
    void MSetDummyPos_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr);

    /* Parts */
    int PNumTriags(const int pid) const;
    int PNumVerts(const int pid) const;

    const std::string GetPartName(const int pid) const;
    void SetPartName(const int pid, const std::string &s);
    const std::array<float, 3> GetPartPos(const int pid) const;
    py::buffer GetPartPos_numpy(const int pid) const;
    void SetPartPos(const int pid, std::array<float, 3> &arr);
    void SetPartPos_numpy(const int pid, py::array_t<float, py::array::c_style | py::array::forcecast> arr);

    /* Triags */
    std::vector<int> GetTriagsVidx(const int pid) const;
    py::buffer GetTriagsVidx_numpy(const int pid) const;
    std::vector<int> GetTriagsFlags(const int pid) const;
    py::buffer GetTriagsFlags_numpy(const int pid) const;
    void SetTriagsFlags(const int pid, std::vector<int> &arr);
    void SetTriagsFlags_numpy(const int pid, py::array_t<int, py::array::c_style | py::array::forcecast> arr);
    py::buffer PGetTriagsTexcoords_numpy(const int pid) const;
    void PSetTriagsTexcoords_numpy(const int pid, py::array_t<float, py::array::c_style | py::array::forcecast> arr);
    std::vector<int> GetTriagsTexpages(const int pid) const;
    py::buffer GetTriagsTexpages_numpy(const int pid) const;
    void SetTriagsTexpages(const int pid, std::vector<int> &arr);
    void SetTriagsTexpages_numpy(const int pid, py::array_t<int, py::array::c_style | py::array::forcecast> arr);

    /* Verts */
    std::vector<int> GetVertsMap_idx2order() const;
    py::buffer GetVertsMap_idx2order_numpy() const;

    std::vector<float> GetVertsPos() const;
    py::buffer GetVertsPos_numpy() const;
    void SetVertsPos(std::vector<float> arr);
    void SetVertsPos_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr);
    std::vector<float> GetVertsNorms() const;
    py::buffer GetVertsNorms_numpy() const;
    void SetVertsNorms(std::vector<float> arr);
    void SetVertsNorms_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr);
    std::vector<float> GetDamgdVertsPos() const;
    py::buffer GetDamgdVertsPos_numpy() const;
    void SetDamgdVertsPos(std::vector<float> arr);
    void SetDamgdVertsPos_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr);
    std::vector<float> GetDamgdVertsNorms() const;
    py::buffer GetDamgdVertsNorms_numpy() const;
    void SetDamgdVertsNorms(std::vector<float> arr);
    void SetDamgdVertsNorms_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr);
    std::vector<int> GetVertsAnimation() const;
    py::buffer GetVertsAnimation_numpy() const;
    void SetVertsAnimation(std::vector<int> arr);
    void SetVertsAnimation_numpy(py::array_t<int, py::array::c_style | py::array::forcecast> arr);

    /* Operations */
    bool CenterPart(const int pid);
    int CopyPart(const int pid_src);
    int InsertPart(Mesh *mesh_src, const int pid_src);
    bool DeletePart(const int pid);
    bool DelPartTriags(const int pid, const std::vector<int> &idxs);
    bool DelUnrefdVerts() { return fcelib::FCELIB_DeleteUnrefdVerts(&mesh_); }
    int MergeParts(const int pid1, const int pid2);
    int MovePart(const int pid);

  private:
    fcelib::FcelibMesh& mesh_;
};


/* Mesh:: wrappers ---------------------------------------------------------- */
/* i/o ------------------------------ */

void Mesh::Decode(const std::string &buf)
{
  fcelib::FCELIB_FreeMesh(&mesh_);
  fcelib::FCELIB_InitMesh(&mesh_);
  if (!fcelib::FCELIB_DecodeFce(buf.c_str(), buf.size(), &mesh_))
    throw std::runtime_error("Decode: Cannot parse FCE data");
}

py::bytes Mesh::Encode_Fce3(const bool center_parts) const
{
  const int bufsize_ = fcelib::FCELIB_FCETYPES_Fce3ComputeSize(mesh_.hdr.NumVertices, mesh_.hdr.NumTriangles);
  unsigned char *buf_ = (unsigned char *)malloc(static_cast<std::size_t>(bufsize_) * sizeof(*buf_));
  if (!buf_)
    throw std::runtime_error("Encode_Fce3: Cannot allocate memory");
  if (!fcelib::FCELIB_EncodeFce3(&buf_, bufsize_, &mesh_, static_cast<int>(center_parts)))
    throw std::runtime_error("Encode_Fce3: Cannot encode FCE3");
  py::bytes result = py::bytes((char *)buf_, static_cast<std::size_t>(bufsize_));
  free(buf_);
  return result;
}

py::bytes Mesh::Encode_Fce4(const bool center_parts) const
{
  const int bufsize_ = fcelib::FCELIB_FCETYPES_Fce4ComputeSize(0x00101014, mesh_.hdr.NumVertices, mesh_.hdr.NumTriangles);
  unsigned char *buf_ = (unsigned char *)malloc(static_cast<std::size_t>(bufsize_) * sizeof(*buf_));
  if (!buf_)
    throw std::runtime_error("Encode_Fce4: Cannot allocate memory");
  if (!fcelib::FCELIB_EncodeFce4(&buf_, bufsize_, &mesh_, static_cast<int>(center_parts)))
    throw std::runtime_error("Encode_Fce4: Cannot encode FCE4");
  py::bytes result = py::bytes((char *)buf_, static_cast<std::size_t>(bufsize_));
  free(buf_);
  return result;
}

py::bytes Mesh::Encode_Fce4M(const bool center_parts) const
{
  const int bufsize_ = fcelib::FCELIB_FCETYPES_Fce4ComputeSize(0x00101015, mesh_.hdr.NumVertices, mesh_.hdr.NumTriangles);
  unsigned char *buf_ = (unsigned char *)malloc(static_cast<std::size_t>(bufsize_) * sizeof(*buf_));
  if (!buf_)
    throw std::runtime_error("Encode_Fce4M: Cannot allocate memory");
  if (!fcelib::FCELIB_EncodeFce4M(&buf_, bufsize_, &mesh_, static_cast<int>(center_parts)))
    throw std::runtime_error("Encode_Fce4M: Cannot encode FCE4M");
  py::bytes result = py::bytes((char *)buf_, static_cast<std::size_t>(bufsize_));
  free(buf_);
  return result;
}

void Mesh::ExportObj(std::string &objpath, std::string &mtlpath,
                     std::string &texture_name,
                     const int print_damage, const int print_dummies) const
{
  if (fcelib::FCELIB_ExportObj(&mesh_, objpath.c_str(), mtlpath.c_str(),
                                   texture_name.c_str(),
                                   print_damage, print_dummies) == 0)
    throw std::runtime_error("ExportObj: Cannot export OBJ");
}

int Mesh::IoGeomDataToNewPart_numpy(py::array_t<int, py::array::c_style | py::array::forcecast> vert_idxs,
                                    py::array_t<float, py::array::c_style | py::array::forcecast> vert_texcoords,
                                    py::array_t<float, py::array::c_style | py::array::forcecast> vert_pos,
                                    py::array_t<float, py::array::c_style | py::array::forcecast> normals)
{
  py::buffer_info tbuf = vert_idxs.request();
  py::buffer_info tcbuf = vert_texcoords.request();
  py::buffer_info vbuf = vert_pos.request();
  py::buffer_info vnbuf = normals.request();

  if (tbuf.ndim != 1)
    throw std::runtime_error("IoGeomDataToNewPart_numpy: Number of dimensions must be 1 (vert_idxs)");
  if (tcbuf.ndim != 1)
    throw std::runtime_error("IoGeomDataToNewPart_numpy: Number of dimensions must be 1 (vert_texcoords)");
  if (vbuf.ndim != 1)
    throw std::runtime_error("IoGeomDataToNewPart_numpy: Number of dimensions must be 1 (vert_pos)");
  if (vnbuf.ndim != 1)
    throw std::runtime_error("IoGeomDataToNewPart_numpy: Number of dimensions must be 1 (normals)");

  if (tbuf.shape[0] * 2 != tcbuf.shape[0])
    throw std::runtime_error("IoGeomDataToNewPart_numpy: Must be vert_idxs shape=(N*3, ) and texcoords shape=(N*6, ) for N triangles");
  if (vbuf.shape[0] != vnbuf.shape[0])
    throw std::runtime_error("IoGeomDataToNewPart_numpy: Must be vert_pos shape=(N*3, ) and normals shape=(N*3, ) for N vertices");

  const int new_pid = fcelib::FCELIB_GeomDataToNewPart(&mesh_,
                                                       static_cast<int *>(tbuf.ptr), static_cast<int>(tbuf.shape[0]),
                                                       static_cast<float *>(tcbuf.ptr), static_cast<int>(tcbuf.shape[0]),
                                                       static_cast<float *>(vbuf.ptr), static_cast<int>(vbuf.shape[0]),
                                                       static_cast<float *>(vnbuf.ptr), static_cast<int>(vnbuf.shape[0]));
  if (new_pid < 0)
    throw std::runtime_error("IoGeomDataToNewPart_numpy: failure");
  return new_pid;
}


/* mesh / header -------------------- */

py::buffer Mesh::MGetColors_numpy() const
{
  const py::ssize_t nrows = static_cast<py::ssize_t>(mesh_.hdr.NumColors);
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
void Mesh::MSetColors_numpy(py::array_t<unsigned char, py::array::c_style | py::array::forcecast> arr)
{
  py::buffer_info buf = arr.request();
  unsigned char *ptr;

  if (buf.ndim != 3)
    throw std::runtime_error("MSetColors_numpy: Number of dimensions must be 3");
  if (buf.shape[1] != 4 || buf.shape[2] != 4)
    throw std::runtime_error("MSetColors_numpy: Shape must be (N, 4, 4)");

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


std::vector<std::string> Mesh::GetDummyNames() const
{
  const std::size_t len = static_cast<std::size_t>(mesh_.hdr.NumDummies);
  std::vector<std::string> retv;
  retv.resize(len);

  for (std::size_t i = 0; i < len; ++i)
    retv.at(i) = std::string(mesh_.hdr.DummyNames + i * 64);

  return retv;
}

void Mesh::SetDummyNames(std::vector<std::string> &arr)
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

py::buffer Mesh::MGetDummyPos_numpy() const
{
  const py::ssize_t len = static_cast<py::ssize_t>(mesh_.hdr.NumDummies);
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

void Mesh::MSetDummyPos_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr)
{
  py::buffer_info buf = arr.request();
  float *ptr;

  if (buf.ndim != 1)
    throw std::runtime_error("MSetDummyPos_numpy: Number of dimensions must be 1");
  if (buf.shape[0] % 3 != 0)
    throw std::runtime_error("MSetDummyPos_numpy: Shape must be (N*3, ) for N dummy positions");

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


/* part ----------------------------- */

int Mesh::PNumTriags(const int pid) const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("PNumTriags: failure");
  const int idx = fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid);
  if (idx < 0)
    throw std::out_of_range("PNumTriags: part index (pid) out of range");
  return mesh_.parts[ mesh_.hdr.Parts[idx] ]->PNumTriangles;
}
int Mesh::PNumVerts(const int pid) const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("PNumVerts: failure");
  const int idx = fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid);
  if (idx < 0)
    throw std::out_of_range("PNumVerts: part index (pid) out of range");
  return mesh_.parts[ mesh_.hdr.Parts[idx] ]->PNumVertices;
}

const std::string Mesh::GetPartName(const int pid) const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("GetPartName: failure");
  const int idx = fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid);
  if (idx < 0)
    throw std::out_of_range("GetPartName: part index (pid) out of range");
  return std::string(mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartName);
}
void Mesh::SetPartName(const int pid, const std::string &s)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("SetPartName: failure");
  const int idx = fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid);
  if (idx < 0)
    throw std::out_of_range("SetPartName: part index (pid) out of range");
  std::strncpy(mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartName, s.c_str(),
               sizeof(mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartName) - 1);  // max 63 chars
}

const std::array<float, 3> Mesh::GetPartPos(const int pid) const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("GetPartPos: failure");
  const int idx = fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid);
  if (idx < 0)
    throw std::out_of_range("GetPartPos: part index (pid) out of range");
  std::array<float, 3> result;
  result[0] = mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.x;
  result[1] = mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.y;
  result[2] = mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.z;
  return result;
}
py::buffer Mesh::GetPartPos_numpy(const int pid) const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("GetPartPos_numpy: failure");
  const int idx = fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid);
  if (idx < 0)
    throw std::out_of_range("GetPartPos_numpy: part index (pid) out of range");

  py::array_t<float> result = py::array_t<float>({ 3 }, {  });
  auto buf = result.mutable_unchecked<1>();
  memcpy(&buf(0), &mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.x, sizeof(float));
  memcpy(&buf(1), &mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.y, sizeof(float));
  memcpy(&buf(2), &mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.z, sizeof(float));
  return std::move(result);
}
void Mesh::SetPartPos(const int pid, std::array<float, 3> &arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("SetPartPos: failure");
  const int idx = fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid);
  if (idx < 0)
    throw std::out_of_range("SetPartPos: part index (pid) out of range");

  mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.x = arr[0];
  mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.y = arr[1];
  mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.z = arr[2];
}
void Mesh::SetPartPos_numpy(const int pid, py::array_t<float, py::array::c_style | py::array::forcecast> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("SetPartPos_numpy: failure");
  const int idx = fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid);
  if (idx < 0)
    throw std::out_of_range("SetPartPos_numpy: part index (pid) out of range");

  py::buffer_info buf = arr.request();
  float *ptr;

  if (buf.ndim != 1)
    throw std::runtime_error("SetPartPos_numpy: Number of dimensions must be 1");
  if (buf.shape[0] != 3)
    throw std::runtime_error("SetPartPos_numpy: Shape must be (3, )");
  ptr = static_cast<float *>(buf.ptr);
  memcpy(&mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.x, ptr + 0, sizeof(float));
  memcpy(&mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.y, ptr + 1, sizeof(float));
  memcpy(&mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.z, ptr + 2, sizeof(float));
}


/* triags --------------------------- */

std::vector<int> Mesh::GetTriagsVidx(const int pid) const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("GetTriagsVidx: failure");
  if (pid < 0 || pid >= mesh_.hdr.NumParts)
    throw std::range_error("GetTriagsVidx: pid");
  fcelib::FcelibPart *part = mesh_.parts[ mesh_.hdr.Parts[fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid)] ];

  const std::size_t nrows = static_cast<std::size_t>(part->PNumTriangles);
  std::vector<int> result = std::vector<int>(nrows * 3);
  int *ptr = result.data();
  fcelib::FcelibTriangle *triag;

  // i - global triag index, j - global triag order
  std::size_t j = 0;
  for (int i = 0; i < part->ptriangles_len && j < nrows; ++i)
  {
    if (part->PTriangles[i] < 0)
      continue;
    triag = mesh_.triangles[ part->PTriangles[i] ];
    ptr[j * 3 + 0] = triag->vidx[0];
    ptr[j * 3 + 1] = triag->vidx[1];
    ptr[j * 3 + 2] = triag->vidx[2];
    ++j;
  }

  return result;
}
py::buffer Mesh::GetTriagsVidx_numpy(const int pid) const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("GetTriagsVidx_numpy: failure");
  if (pid < 0 || pid >= mesh_.hdr.NumParts)
    throw std::range_error("GetTriagsVidx_numpy: pid");

  fcelib::FcelibPart *part = mesh_.parts[ mesh_.hdr.Parts[fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid)] ];

  const py::ssize_t nrows = static_cast<py::ssize_t>(part->PNumTriangles);
  py::array_t<int> result = py::array_t<int>({ nrows * 3 }, {  });
  auto buf = result.mutable_unchecked<>();
  fcelib::FcelibTriangle *triag;

  // i - global triag index, j - global triag order
  py::ssize_t j = 0;
  for (int i = 0; i < part->ptriangles_len && j < nrows; ++i)
  {
    if (part->PTriangles[i] < 0)
      continue;
    triag = mesh_.triangles[ part->PTriangles[i] ];
    buf(j * 3 + 0) = triag->vidx[0];
    buf(j * 3 + 1) = triag->vidx[1];
    buf(j * 3 + 2) = triag->vidx[2];
    ++j;
  }

  return std::move(result);
}


std::vector<int> Mesh::GetTriagsFlags(const int pid) const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("GetTriagsFlags: failure");
  if (pid < 0 || pid >= mesh_.hdr.NumParts)
    throw std::range_error("GetTriagsFlags: pid");
  fcelib::FcelibPart *part = mesh_.parts[ mesh_.hdr.Parts[fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid)] ];

  const std::size_t nrows = static_cast<std::size_t>(part->PNumTriangles);
  std::vector<int> result = std::vector<int>(nrows);
  int *ptr = result.data();
  // i - global triag index, j - global triag order
  std::size_t j = 0;
  for (int i = 0; i < part->ptriangles_len && j < nrows; ++i)
  {
    if (part->PTriangles[i] < 0)
      continue;
    ptr[j] = mesh_.triangles[ part->PTriangles[i] ]->flag;
    ++j;
  }
  return result;
}
py::buffer Mesh::GetTriagsFlags_numpy(const int pid) const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("GetTriagsFlags_numpy: failure");
  if (pid < 0 || pid >= mesh_.hdr.NumParts)
    throw std::range_error("GetTriagsFlags_numpy: pid");
  fcelib::FcelibPart *part = mesh_.parts[ mesh_.hdr.Parts[fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid)] ];

  const py::ssize_t nrows = static_cast<py::ssize_t>(part->PNumTriangles);
  py::array_t<int> result = py::array_t<int>({ nrows }, {  });
  auto buf = result.mutable_unchecked<>();
  // i - global triag index, j - global triag order
  py::ssize_t j = 0;
  for (int i = 0; i < part->ptriangles_len && j < nrows; ++i)
  {
    if (part->PTriangles[i] < 0)
      continue;
    buf(j) = mesh_.triangles[ part->PTriangles[i] ]->flag;
    ++j;
  }
  return std::move(result);
}
void Mesh::SetTriagsFlags(const int pid, std::vector<int> &arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("SetTriagsFlags: failure");
  if (pid < 0 || pid >= mesh_.hdr.NumParts)
    throw std::range_error("SetTriagsFlags: pid");
  fcelib::FcelibPart *part = mesh_.parts[ mesh_.hdr.Parts[fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid)] ];

  const int nrows = part->PNumTriangles;
  if (arr.size() != static_cast<std::size_t>(nrows))
    throw std::runtime_error("Shape must be (N, ) for N triangles");

  int *ptr = arr.data();
  // i - global triag index, j - global triag order
  int j = 0;
  for (int i = 0; i < part->ptriangles_len && j < nrows; ++i)
  {
    if (part->PTriangles[i] < 0)
      continue;
    mesh_.triangles[ part->PTriangles[i] ]->flag = ptr[j];
    ++j;
  }
}
void Mesh::SetTriagsFlags_numpy(const int pid, py::array_t<int, py::array::c_style | py::array::forcecast> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("SetTriagsFlags_numpy: failure");
  if (pid < 0 || pid >= mesh_.hdr.NumParts)
    throw std::range_error("SetTriagsFlags_numpy: pid");
  fcelib::FcelibPart *part = mesh_.parts[ mesh_.hdr.Parts[fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid)] ];

  const py::ssize_t nrows = static_cast<py::ssize_t>(part->PNumTriangles);
  py::buffer_info buf = arr.request();
  if (buf.ndim != 1)
    throw std::runtime_error("Number of dimensions must be 1");
  if (buf.shape[0] != nrows)
    throw std::runtime_error("Shape must be (N, ) for N triangles");

  int *ptr = static_cast<int *>(buf.ptr);
  // i - global triag index, j - global triag order
  py::ssize_t j = 0;
  for (int i = 0; i < part->ptriangles_len && j < nrows; ++i)
  {
    if (part->PTriangles[i] < 0)
      continue;
    mesh_.triangles[ part->PTriangles[i] ]->flag = ptr[j];
    ++j;
  }
}


py::buffer Mesh::PGetTriagsTexcoords_numpy(const int pid) const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("PGetTriagsTexcoords_numpy: failure");
  if (pid < 0 || pid >= mesh_.hdr.NumParts)
    throw std::range_error("PGetTriagsTexcoords_numpy: pid");

  fcelib::FcelibPart *part = mesh_.parts[ mesh_.hdr.Parts[fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid)] ];

  const py::ssize_t nrows = static_cast<py::ssize_t>(part->PNumTriangles);
  py::array_t<float> result = py::array_t<float>({ nrows * 6 }, {  });
  auto buf = result.mutable_unchecked<>();
  fcelib::FcelibTriangle *triag;

  // i - global triag index, j - global triag order
  py::ssize_t j = 0;
  for (int i = 0; i < part->ptriangles_len && j < nrows; ++i)
  {
    if (part->PTriangles[i] < 0)
      continue;
    triag = mesh_.triangles[ part->PTriangles[i] ];
    memcpy(&buf(j * 6 + 0), &triag->U, sizeof(triag->U));
    memcpy(&buf(j * 6 + 3), &triag->V, sizeof(triag->U));
    // memcpy(&buf(j * 6 + 0), &triag->U[0], sizeof(triag->U[0]));
    // memcpy(&buf(j * 6 + 1), &triag->U[1], sizeof(triag->U[1]));
    // memcpy(&buf(j * 6 + 2), &triag->U[2], sizeof(triag->U[2]));
    // memcpy(&buf(j * 6 + 3), &triag->V[0], sizeof(triag->V[0]));
    // memcpy(&buf(j * 6 + 4), &triag->V[1], sizeof(triag->V[1]));
    // memcpy(&buf(j * 6 + 5), &triag->V[2], sizeof(triag->V[2]));
    ++j;
  }

  return std::move(result);
}
void Mesh::PSetTriagsTexcoords_numpy(const int pid, py::array_t<float, py::array::c_style | py::array::forcecast> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("PSetTriagsTexcoords_numpy: failure");
  if (pid < 0 || pid >= mesh_.hdr.NumParts)
    throw std::range_error("PSetTriagsTexcoords_numpy: pid");
  fcelib::FcelibPart *part = mesh_.parts[ mesh_.hdr.Parts[fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid)] ];

  const py::ssize_t nrows = static_cast<py::ssize_t>(part->PNumTriangles);
  py::buffer_info buf = arr.request();
  if (buf.ndim != 1)
    throw std::runtime_error("Number of dimensions must be 1");
  if (buf.shape[0] != nrows * 6)
    throw std::runtime_error("Shape must be (N*6, ) for N triangles");
  fcelib::FcelibTriangle *triag;

  float *ptr = static_cast<float *>(buf.ptr);
  // i - global triag index, j - global triag order
  py::ssize_t j = 0;
  for (int i = 0; i < part->ptriangles_len && j < nrows; ++i)
  {
    if (part->PTriangles[i] < 0)
      continue;
    triag = mesh_.triangles[ part->PTriangles[i] ];
    memcpy(triag->U, ptr + j * 6 + 0, sizeof(triag->U));
    memcpy(triag->V, ptr + j * 6 + 3, sizeof(triag->V));
    // mesh_.triangles[ part->PTriangles[i] ]->flag = ptr[j];
    ++j;
  }
}


std::vector<int> Mesh::GetTriagsTexpages(const int pid) const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("GetTriagsTexpages: failure");
  if (pid < 0 || pid >= mesh_.hdr.NumParts)
    throw std::range_error("GetTriagsTexpages: pid");
  fcelib::FcelibPart *part = mesh_.parts[ mesh_.hdr.Parts[fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid)] ];

  const std::size_t nrows = static_cast<std::size_t>(part->PNumTriangles);
  std::vector<int> result = std::vector<int>(nrows);
  int *ptr = result.data();
  // i - global triag index, j - global triag order
  std::size_t j = 0;
  for (int i = 0; i < part->ptriangles_len && j < nrows; ++i)
  {
    if (part->PTriangles[i] < 0)
      continue;
    ptr[j] = mesh_.triangles[ part->PTriangles[i] ]->tex_page;
    ++j;
  }
  return result;
}
py::buffer Mesh::GetTriagsTexpages_numpy(const int pid) const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("GetTriagsTexpages_numpy: failure");
  if (pid < 0 || pid >= mesh_.hdr.NumParts)
    throw std::range_error("GetTriagsTexpages_numpy: pid");
  fcelib::FcelibPart *part = mesh_.parts[ mesh_.hdr.Parts[fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid)] ];

  const py::ssize_t nrows = static_cast<py::ssize_t>(part->PNumTriangles);
  py::array_t<int> result = py::array_t<int>({ nrows }, {  });
  auto buf = result.mutable_unchecked<>();
  // i - global triag index, j - global triag order
  py::ssize_t j = 0;
  for (int i = 0; i < part->ptriangles_len && j < nrows; ++i)
  {
    if (part->PTriangles[i] < 0)
      continue;
    buf(j) = mesh_.triangles[ part->PTriangles[i] ]->tex_page;
    ++j;
  }
  return std::move(result);
}
void Mesh::SetTriagsTexpages(const int pid, std::vector<int> &arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("SetTriagsTexpages: failure");
  if (pid < 0 || pid >= mesh_.hdr.NumParts)
    throw std::range_error("SetTriagsTexpages: pid");
  fcelib::FcelibPart *part = mesh_.parts[ mesh_.hdr.Parts[fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid)] ];

  const int nrows = part->PNumTriangles;
  if (arr.size() != static_cast<std::size_t>(nrows))
    throw std::runtime_error("Shape must be (N, ) for N triangles");

  int *ptr = arr.data();
  // i - global triag index, j - global triag order
  int j = 0;
  for (int i = 0; i < part->ptriangles_len && j < nrows; ++i)
  {
    if (part->PTriangles[i] < 0)
      continue;
    mesh_.triangles[ part->PTriangles[i] ]->tex_page = ptr[j];
    ++j;
  }
}
void Mesh::SetTriagsTexpages_numpy(const int pid, py::array_t<int, py::array::c_style | py::array::forcecast> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("SetTriagsTexpages_numpy: failure");
  if (pid < 0 || pid >= mesh_.hdr.NumParts)
    throw std::range_error("SetTriagsTexpages_numpy: pid");
  fcelib::FcelibPart *part = mesh_.parts[ mesh_.hdr.Parts[fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid)] ];

  const py::ssize_t nrows = static_cast<py::ssize_t>(part->PNumTriangles);
  py::buffer_info buf = arr.request();
  if (buf.ndim != 1)
    throw std::runtime_error("Number of dimensions must be 1");
  if (buf.shape[0] != nrows)
    throw std::runtime_error("Shape must be (N, ) for N triangles");

  int *ptr = static_cast<int *>(buf.ptr);
  // i - global triag index, j - global triag order
  py::ssize_t j = 0;
  for (int i = 0; i < part->ptriangles_len && j < nrows; ++i)
  {
    if (part->PTriangles[i] < 0)
      continue;
    mesh_.triangles[ part->PTriangles[i] ]->tex_page = ptr[j];
    ++j;
  }
}


/* verts ---------------------------- */

/* Via vector index (=global vert idx) map to global vert order. */
std::vector<int> Mesh::GetVertsMap_idx2order() const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("GetVertsMap_idx2order: failure");
  std::vector<int> result = std::vector<int>(static_cast<std::size_t>(mesh_.vertices_len), -1);
  int *ptr = result.data();
  fcelib::FcelibPart *part;
  /* part->PVertices[m] - global vert index, j - global vert order */
  memset(ptr, -1, static_cast<std::size_t>(mesh_.vertices_len) * sizeof(int));
  int j = 0;
  for (int k = 0; k < mesh_.parts_len; ++k)
  {
    if (mesh_.hdr.Parts[k] < 0)
      continue;
    part = mesh_.parts[ mesh_.hdr.Parts[k] ];

    for (int m = 0; m < part->pvertices_len; ++m)
    {
      if(part->PVertices[m] < 0)
        continue;
      ptr[ part->PVertices[m] ] = j;
      ++j;
    }  // for m
  }  // for k

  return result;
}
/* Via vector index (=global vert idx) map to global vert order. */
py::buffer Mesh::GetVertsMap_idx2order_numpy() const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("GetVertsMap_idx2order_numpy: failure");
  py::array_t<int> result = py::array_t<int>({ static_cast<py::ssize_t>(mesh_.vertices_len) }, {  });
  auto buf = result.request();
  int *ptr = static_cast<int *>(buf.ptr);
  fcelib::FcelibPart *part;
  /* part->PVertices[m] - global vert index, j - global vert order */
  memset(ptr, -1, static_cast<std::size_t>(mesh_.vertices_len) * sizeof(int));
  int j = 0;
  for (int k = 0; k < mesh_.parts_len; ++k)
  {
    if (mesh_.hdr.Parts[k] < 0)
      continue;
    part = mesh_.parts[ mesh_.hdr.Parts[k] ];

    for (int m = 0; m < part->pvertices_len; ++m)
    {
      if(part->PVertices[m] < 0)
        continue;
      ptr[ part->PVertices[m] ] = j;
      ++j;
    }  // for m
  }  // for k

  return std::move(result);
}


std::vector<float> Mesh::GetVertsPos() const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("GetVertsPos: failure");
  std::vector<float> result = std::vector<float>(static_cast<std::size_t>(mesh_.hdr.NumVertices) * 3);
  float *ptr = result.data();
  fcelib::FcelibPart *part;
  fcelib::FcelibVertex *vert;
  // i - part vert index, j - global vert order
  int j = 0;
  for (int k = 0; k < mesh_.parts_len; ++k)
  {
    if (mesh_.hdr.Parts[k] < 0)
      continue;
    part = mesh_.parts[ mesh_.hdr.Parts[k] ];

    for (int i = 0; i < part->pvertices_len; ++i)
    {
      if(part->PVertices[i] < 0)
        continue;
      vert = mesh_.vertices[ part->PVertices[i] ];
      ptr[j * 3 + 0] = vert->VertPos.x;
      ptr[j * 3 + 1] = vert->VertPos.y;
      ptr[j * 3 + 2] = vert->VertPos.z;
      ++j;
    }  // for i
  }  // for k

  return result;
}
py::buffer Mesh::GetVertsPos_numpy() const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("GetVertsPos_numpy: failure");
  py::array_t<float> result = py::array_t<float>({ static_cast<py::ssize_t>(mesh_.hdr.NumVertices) * 3 }, {  });
  auto buf = result.mutable_unchecked<>();
  fcelib::FcelibPart *part;
  fcelib::FcelibVertex *vert;
  // i - part vert index, j - global vert order
  int j = 0;
  for (int k = 0; k < mesh_.parts_len; ++k)
  {
    if (mesh_.hdr.Parts[k] < 0)
      continue;
    part = mesh_.parts[ mesh_.hdr.Parts[k] ];

    for (int i = 0; i < part->pvertices_len; ++i)
    {
      if(part->PVertices[i] < 0)
        continue;
      vert = mesh_.vertices[ part->PVertices[i] ];
      buf(j * 3 + 0) = vert->VertPos.x;
      buf(j * 3 + 1) = vert->VertPos.y;
      buf(j * 3 + 2) = vert->VertPos.z;
      ++j;
    }  // for i
  }  // for k

  return std::move(result);
}
void Mesh::SetVertsPos(std::vector<float> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("SetVertsPos: failure");
  const std::size_t nrows = static_cast<std::size_t>(mesh_.hdr.NumVertices);
  if (arr.size() != nrows * 3)
    throw std::runtime_error("Shape must be (N*3, ) where N = Mesh.num_verts()");
  float *ptr = arr.data();
  fcelib::FcelibPart *part;
  fcelib::FcelibVertex *vert;
  // i - part vert index, j - global vert order
  int j = 0;
  for (int k = 0; k < mesh_.parts_len; ++k)
  {
    if (mesh_.hdr.Parts[k] < 0)
      continue;
    part = mesh_.parts[ mesh_.hdr.Parts[k] ];

    for (int i = 0; i < part->pvertices_len; ++i)
    {
      if(part->PVertices[i] < 0)
        continue;
      vert = mesh_.vertices[ part->PVertices[i] ];
      vert->VertPos.x = ptr[j * 3 + 0];
      vert->VertPos.y = ptr[j * 3 + 1];
      vert->VertPos.z = ptr[j * 3 + 2];
      ++j;
    }  // for i
  }  // for k
}
void Mesh::SetVertsPos_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("SetVertsPos_numpy: failure");
  const py::ssize_t nrows = static_cast<py::ssize_t>(mesh_.hdr.NumVertices);
  py::buffer_info buf = arr.request();
  if (buf.ndim != 1)
    throw std::runtime_error("Number of dimensions must be 1");
  if (buf.shape[0] != nrows * 3)
    throw std::runtime_error("Shape must be (N*3, ) where N = Mesh.num_verts()");
  float *ptr = static_cast<float *>(buf.ptr);
  fcelib::FcelibPart *part;
  fcelib::FcelibVertex *vert;
  // i - part vert index, j - global vert order
  int j = 0;
  for (int k = 0; k < mesh_.parts_len; ++k)
  {
    if (mesh_.hdr.Parts[k] < 0)
      continue;
    part = mesh_.parts[ mesh_.hdr.Parts[k] ];

    for (int i = 0; i < part->pvertices_len; ++i)
    {
      if(part->PVertices[i] < 0)
        continue;
      vert = mesh_.vertices[ part->PVertices[i] ];
      vert->VertPos.x = ptr[j * 3 + 0];
      vert->VertPos.y = ptr[j * 3 + 1];
      vert->VertPos.z = ptr[j * 3 + 2];
      ++j;
    }  // for i
  }  // for k
}


std::vector<float> Mesh::GetVertsNorms() const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("GetVertsNorms: failure");
  std::vector<float> result = std::vector<float>(static_cast<std::size_t>(mesh_.hdr.NumVertices) * 3);
  float *ptr = result.data();
  fcelib::FcelibPart *part;
  fcelib::FcelibVertex *vert;
  // i - part vert index, j - global vert order
  int j = 0;
  for (int k = 0; k < mesh_.parts_len; ++k)
  {
    if (mesh_.hdr.Parts[k] < 0)
      continue;
    part = mesh_.parts[ mesh_.hdr.Parts[k] ];

    for (int i = 0; i < part->pvertices_len; ++i)
    {
      if(part->PVertices[i] < 0)
        continue;
      vert = mesh_.vertices[ part->PVertices[i] ];
      ptr[j * 3 + 0] = vert->NormPos.x;
      ptr[j * 3 + 1] = vert->NormPos.y;
      ptr[j * 3 + 2] = vert->NormPos.z;
      ++j;
    }  // for i
  }  // for k

  return result;
}
py::buffer Mesh::GetVertsNorms_numpy() const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("GetVertsNorms_numpy: failure");
  py::array_t<float> result = py::array_t<float>({ static_cast<py::ssize_t>(mesh_.hdr.NumVertices) * 3 }, {  });
  auto buf = result.mutable_unchecked<>();
  fcelib::FcelibPart *part;
  fcelib::FcelibVertex *vert;
  // i - part vert index, j - global vert order
  int j = 0;
  for (int k = 0; k < mesh_.parts_len; ++k)
  {
    if (mesh_.hdr.Parts[k] < 0)
      continue;
    part = mesh_.parts[ mesh_.hdr.Parts[k] ];

    for (int i = 0; i < part->pvertices_len; ++i)
    {
      if(part->PVertices[i] < 0)
        continue;
      vert = mesh_.vertices[ part->PVertices[i] ];
      buf(j * 3 + 0) = vert->NormPos.x;
      buf(j * 3 + 1) = vert->NormPos.y;
      buf(j * 3 + 2) = vert->NormPos.z;
      ++j;
    }  // for i
  }  // for k

  return std::move(result);
}
void Mesh::SetVertsNorms(std::vector<float> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("SetVertsNorms: failure");
  const std::size_t nrows = static_cast<std::size_t>(mesh_.hdr.NumVertices);
  if (arr.size() != nrows * 3)
    throw std::runtime_error("Shape must be (N*3, ) where N = Mesh.num_verts()");
  float *ptr = arr.data();
  fcelib::FcelibPart *part;
  fcelib::FcelibVertex *vert;
  // i - part vert index, j - global vert order
  int j = 0;
  for (int k = 0; k < mesh_.parts_len; ++k)
  {
    if (mesh_.hdr.Parts[k] < 0)
      continue;
    part = mesh_.parts[ mesh_.hdr.Parts[k] ];

    for (int i = 0; i < part->pvertices_len; ++i)
    {
      if(part->PVertices[i] < 0)
        continue;
      vert = mesh_.vertices[ part->PVertices[i] ];
      vert->NormPos.x = ptr[j * 3 + 0];
      vert->NormPos.y = ptr[j * 3 + 1];
      vert->NormPos.z = ptr[j * 3 + 2];
      ++j;
    }  // for i
  }  // for k
}
void Mesh::SetVertsNorms_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("SetVertsNorms_numpy: failure");
  const py::ssize_t nrows = static_cast<py::ssize_t>(mesh_.hdr.NumVertices);
  py::buffer_info buf = arr.request();
  if (buf.ndim != 1)
    throw std::runtime_error("Number of dimensions must be 1");
  if (buf.shape[0] != nrows * 3)
    throw std::runtime_error("Shape must be (N*3, ) where N = Mesh.num_verts()");
  float *ptr = static_cast<float *>(buf.ptr);
  fcelib::FcelibPart *part;
  fcelib::FcelibVertex *vert;
  // i - part vert index, j - global vert order
  int j = 0;
  for (int k = 0; k < mesh_.parts_len; ++k)
  {
    if (mesh_.hdr.Parts[k] < 0)
      continue;
    part = mesh_.parts[ mesh_.hdr.Parts[k] ];

    for (int i = 0; i < part->pvertices_len; ++i)
    {
      if(part->PVertices[i] < 0)
        continue;
      vert = mesh_.vertices[ part->PVertices[i] ];
      vert->NormPos.x = ptr[j * 3 + 0];
      vert->NormPos.y = ptr[j * 3 + 1];
      vert->NormPos.z = ptr[j * 3 + 2];
      ++j;
    }  // for i
  }  // for k
}


std::vector<float> Mesh::GetDamgdVertsPos() const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("GetDamgdVertsPos: failure");
  std::vector<float> result = std::vector<float>(static_cast<std::size_t>(mesh_.hdr.NumVertices) * 3);
  float *ptr = result.data();
  fcelib::FcelibPart *part;
  fcelib::FcelibVertex *vert;
  // i - part vert index, j - global vert order
  int j = 0;
  for (int k = 0; k < mesh_.parts_len; ++k)
  {
    if (mesh_.hdr.Parts[k] < 0)
      continue;
    part = mesh_.parts[ mesh_.hdr.Parts[k] ];

    for (int i = 0; i < part->pvertices_len; ++i)
    {
      if(part->PVertices[i] < 0)
        continue;
      vert = mesh_.vertices[ part->PVertices[i] ];
      ptr[j * 3 + 0] = vert->DamgdVertPos.x;
      ptr[j * 3 + 1] = vert->DamgdVertPos.y;
      ptr[j * 3 + 2] = vert->DamgdVertPos.z;
      ++j;
    }  // for i
  }  // for k

  return result;
}
py::buffer Mesh::GetDamgdVertsPos_numpy() const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("GetDamgdVertsPos_numpy: failure");
  py::array_t<float> result = py::array_t<float>({ static_cast<py::ssize_t>(mesh_.hdr.NumVertices) * 3 }, {  });
  auto buf = result.mutable_unchecked<>();
  fcelib::FcelibPart *part;
  fcelib::FcelibVertex *vert;
  // i - part vert index, j - global vert order
  int j = 0;
  for (int k = 0; k < mesh_.parts_len; ++k)
  {
    if (mesh_.hdr.Parts[k] < 0)
      continue;
    part = mesh_.parts[ mesh_.hdr.Parts[k] ];

    for (int i = 0; i < part->pvertices_len; ++i)
    {
      if(part->PVertices[i] < 0)
        continue;
      vert = mesh_.vertices[ part->PVertices[i] ];
      buf(j * 3 + 0) = vert->DamgdVertPos.x;
      buf(j * 3 + 1) = vert->DamgdVertPos.y;
      buf(j * 3 + 2) = vert->DamgdVertPos.z;
      ++j;
    }  // for i
  }  // for k

  return std::move(result);
}
void Mesh::SetDamgdVertsPos(std::vector<float> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("SetDamgdVertsPos: failure");
  const std::size_t nrows = static_cast<std::size_t>(mesh_.hdr.NumVertices);
  if (arr.size() != nrows * 3)
    throw std::runtime_error("Shape must be (N*3, ) where N = Mesh.num_verts()");
  float *ptr = arr.data();
  fcelib::FcelibPart *part;
  fcelib::FcelibVertex *vert;
  // i - part vert index, j - global vert order
  int j = 0;
  for (int k = 0; k < mesh_.parts_len; ++k)
  {
    if (mesh_.hdr.Parts[k] < 0)
      continue;
    part = mesh_.parts[ mesh_.hdr.Parts[k] ];

    for (int i = 0; i < part->pvertices_len; ++i)
    {
      if(part->PVertices[i] < 0)
        continue;
      vert = mesh_.vertices[ part->PVertices[i] ];
      vert->DamgdVertPos.x = ptr[j * 3 + 0];
      vert->DamgdVertPos.y = ptr[j * 3 + 1];
      vert->DamgdVertPos.z = ptr[j * 3 + 2];
      ++j;
    }  // for i
  }  // for k
}
void Mesh::SetDamgdVertsPos_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("SetDamgdVertsPos_numpy: failure");
  const py::ssize_t nrows = static_cast<py::ssize_t>(mesh_.hdr.NumVertices);
  py::buffer_info buf = arr.request();
  if (buf.ndim != 1)
    throw std::runtime_error("Number of dimensions must be 1");
  if (buf.shape[0] != nrows * 3)
    throw std::runtime_error("Shape must be (N*3, ) where N = Mesh.num_verts()");
  float *ptr = static_cast<float *>(buf.ptr);
  fcelib::FcelibPart *part;
  fcelib::FcelibVertex *vert;
  // i - part vert index, j - global vert order
  int j = 0;
  for (int k = 0; k < mesh_.parts_len; ++k)
  {
    if (mesh_.hdr.Parts[k] < 0)
      continue;
    part = mesh_.parts[ mesh_.hdr.Parts[k] ];

    for (int i = 0; i < part->pvertices_len; ++i)
    {
      if(part->PVertices[i] < 0)
        continue;
      vert = mesh_.vertices[ part->PVertices[i] ];
      vert->DamgdVertPos.x = ptr[j * 3 + 0];
      vert->DamgdVertPos.y = ptr[j * 3 + 1];
      vert->DamgdVertPos.z = ptr[j * 3 + 2];
      ++j;
    }  // for i
  }  // for k
}


std::vector<float> Mesh::GetDamgdVertsNorms() const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("GetDamgdVertsNorms: failure");
  std::vector<float> result = std::vector<float>(static_cast<std::size_t>(mesh_.hdr.NumVertices) * 3);
  float *ptr = result.data();
  fcelib::FcelibPart *part;
  fcelib::FcelibVertex *vert;
  // i - part vert index, j - global vert order
  int j = 0;
  for (int k = 0; k < mesh_.parts_len; ++k)
  {
    if (mesh_.hdr.Parts[k] < 0)
      continue;
    part = mesh_.parts[ mesh_.hdr.Parts[k] ];

    for (int i = 0; i < part->pvertices_len; ++i)
    {
      if(part->PVertices[i] < 0)
        continue;
      vert = mesh_.vertices[ part->PVertices[i] ];
      ptr[j * 3 + 0] = vert->DamgdNormPos.x;
      ptr[j * 3 + 1] = vert->DamgdNormPos.y;
      ptr[j * 3 + 2] = vert->DamgdNormPos.z;
      ++j;
    }  // for i
  }  // for k

  return result;
}
py::buffer Mesh::GetDamgdVertsNorms_numpy() const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("GetDamgdVertsNorms_numpy: failure");
  py::array_t<float> result = py::array_t<float>({ static_cast<py::ssize_t>(mesh_.hdr.NumVertices) * 3 }, {  });
  auto buf = result.mutable_unchecked<>();
  fcelib::FcelibPart *part;
  fcelib::FcelibVertex *vert;
  // i - part vert index, j - global vert order
  int j = 0;
  for (int k = 0; k < mesh_.parts_len; ++k)
  {
    if (mesh_.hdr.Parts[k] < 0)
      continue;
    part = mesh_.parts[ mesh_.hdr.Parts[k] ];

    for (int i = 0; i < part->pvertices_len; ++i)
    {
      if(part->PVertices[i] < 0)
        continue;
      vert = mesh_.vertices[ part->PVertices[i] ];
      buf(j * 3 + 0) = vert->DamgdNormPos.x;
      buf(j * 3 + 1) = vert->DamgdNormPos.y;
      buf(j * 3 + 2) = vert->DamgdNormPos.z;
      ++j;
    }  // for i
  }  // for k

  return std::move(result);
}
void Mesh::SetDamgdVertsNorms(std::vector<float> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("SetDamgdVertsNorms: failure");
  const std::size_t nrows = static_cast<std::size_t>(mesh_.hdr.NumVertices);
  if (arr.size() != nrows * 3)
    throw std::runtime_error("Shape must be (N*3, ) where N = Mesh.num_verts()");
  float *ptr = arr.data();
  fcelib::FcelibPart *part;
  fcelib::FcelibVertex *vert;
  // i - part vert index, j - global vert order
  int j = 0;
  for (int k = 0; k < mesh_.parts_len; ++k)
  {
    if (mesh_.hdr.Parts[k] < 0)
      continue;
    part = mesh_.parts[ mesh_.hdr.Parts[k] ];

    for (int i = 0; i < part->pvertices_len; ++i)
    {
      if(part->PVertices[i] < 0)
        continue;
      vert = mesh_.vertices[ part->PVertices[i] ];
      vert->DamgdNormPos.x = ptr[j * 3 + 0];
      vert->DamgdNormPos.y = ptr[j * 3 + 1];
      vert->DamgdNormPos.z = ptr[j * 3 + 2];
      ++j;
    }  // for i
  }  // for k
}
void Mesh::SetDamgdVertsNorms_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("SetDamgdVertsNorms_numpy: failure");
  const py::ssize_t nrows = static_cast<py::ssize_t>(mesh_.hdr.NumVertices);
  py::buffer_info buf = arr.request();
  if (buf.ndim != 1)
    throw std::runtime_error("Number of dimensions must be 1");
  if (buf.shape[0] != nrows * 3)
    throw std::runtime_error("Shape must be (N*3, ) where N = Mesh.num_verts()");
  float *ptr = static_cast<float *>(buf.ptr);
  fcelib::FcelibPart *part;
  fcelib::FcelibVertex *vert;
  // i - part vert index, j - global vert order
  int j = 0;
  for (int k = 0; k < mesh_.parts_len; ++k)
  {
    if (mesh_.hdr.Parts[k] < 0)
      continue;
    part = mesh_.parts[ mesh_.hdr.Parts[k] ];

    for (int i = 0; i < part->pvertices_len; ++i)
    {
      if(part->PVertices[i] < 0)
        continue;
      vert = mesh_.vertices[ part->PVertices[i] ];
      vert->DamgdNormPos.x = ptr[j * 3 + 0];
      vert->DamgdNormPos.y = ptr[j * 3 + 1];
      vert->DamgdNormPos.z = ptr[j * 3 + 2];
      ++j;
    }  // for i
  }  // for k
}


std::vector<int> Mesh::GetVertsAnimation() const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("GetVertsAnimation: failure");
  std::vector<int> result = std::vector<int>(static_cast<std::size_t>(mesh_.hdr.NumVertices));
  int *ptr = result.data();
  fcelib::FcelibPart *part;
  fcelib::FcelibVertex *vert;
  // i - part vert index, j - global vert order
  int j = 0;
  for (int k = 0; k < mesh_.parts_len; ++k)
  {
    if (mesh_.hdr.Parts[k] < 0)
      continue;
    part = mesh_.parts[ mesh_.hdr.Parts[k] ];

    for (int i = 0; i < part->pvertices_len; ++i)
    {
      if(part->PVertices[i] < 0)
        continue;
      vert = mesh_.vertices[ part->PVertices[i] ];
      ptr[j] = vert->Animation;
      ++j;
    }  // for i
  }  // for k

  return result;
}
py::buffer Mesh::GetVertsAnimation_numpy() const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("GetVertsAnimation_numpy: failure");
  py::array_t<int> result = py::array_t<int>({ static_cast<py::ssize_t>(mesh_.hdr.NumVertices) }, {  });
  auto buf = result.mutable_unchecked<>();
  fcelib::FcelibPart *part;
  fcelib::FcelibVertex *vert;
  // i - part vert index, j - global vert order
  int j = 0;
  for (int k = 0; k < mesh_.parts_len; ++k)
  {
    if (mesh_.hdr.Parts[k] < 0)
      continue;
    part = mesh_.parts[ mesh_.hdr.Parts[k] ];

    for (int i = 0; i < part->pvertices_len; ++i)
    {
      if(part->PVertices[i] < 0)
        continue;
      vert = mesh_.vertices[ part->PVertices[i] ];
      buf(j) = vert->Animation;
      ++j;
    }  // for i
  }  // for k

  return std::move(result);
}
void Mesh::SetVertsAnimation(std::vector<int> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("SetVertsAnimation: failure");
  const std::size_t nrows = static_cast<std::size_t>(mesh_.hdr.NumVertices);
  if (arr.size() != nrows)
    throw std::runtime_error("Shape must be (N, ) where N = Mesh.num_verts()");
  int *ptr = arr.data();
  fcelib::FcelibPart *part;
  fcelib::FcelibVertex *vert;
  // i - part vert index, j - global vert order
  int j = 0;
  for (int k = 0; k < mesh_.parts_len; ++k)
  {
    if (mesh_.hdr.Parts[k] < 0)
      continue;
    part = mesh_.parts[ mesh_.hdr.Parts[k] ];

    for (int i = 0; i < part->pvertices_len; ++i)
    {
      if(part->PVertices[i] < 0)
        continue;
      vert = mesh_.vertices[ part->PVertices[i] ];
      vert->Animation = ptr[j];
      ++j;
    }  // for i
  }  // for k
}
void Mesh::SetVertsAnimation_numpy(py::array_t<int, py::array::c_style | py::array::forcecast> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("SetVertsAnimation_numpy: failure");
  const py::ssize_t nrows = static_cast<py::ssize_t>(mesh_.hdr.NumVertices);
  py::buffer_info buf = arr.request();
  if (buf.ndim != 1)
    throw std::runtime_error("Number of dimensions must be 1");
  if (buf.shape[0] != nrows)
    throw std::runtime_error("Shape must be (N, ) where N = Mesh.num_verts()");
  int *ptr = static_cast<int *>(buf.ptr);
  fcelib::FcelibPart *part;
  fcelib::FcelibVertex *vert;
  // i - part vert index, j - global vert order
  int j = 0;
  for (int k = 0; k < mesh_.parts_len; ++k)
  {
    if (mesh_.hdr.Parts[k] < 0)
      continue;
    part = mesh_.parts[ mesh_.hdr.Parts[k] ];

    for (int i = 0; i < part->pvertices_len; ++i)
    {
      if(part->PVertices[i] < 0)
        continue;
      vert = mesh_.vertices[ part->PVertices[i] ];
      vert->Animation = ptr[j];
      ++j;
    }  // for i
  }  // for k
}


/* mesh: operations ----------------- */

bool Mesh::CenterPart(const int pid)
{
  if (pid > mesh_.hdr.NumParts || pid < 0)
    throw std::out_of_range("CenterPart: part index (pid) out of range");
  return fcelib::FCELIB_CenterPart(&mesh_, pid);
}

int Mesh::CopyPart(const int pid_src)
{
  if (pid_src > this->mesh_.hdr.NumParts || pid_src < 0)
    throw std::out_of_range("CopyPart: part index (pid_src) out of range");
  const int pid_new = fcelib::FCELIB_CopyPartToMesh(&this->mesh_, &this->mesh_, pid_src);
  if (pid_new < 0)
    throw std::runtime_error("CopyPart: Cannot copy part");
  return pid_new;
}

int Mesh::InsertPart(Mesh *mesh_src, const int pid_src)
{
  fcelib::FcelibMesh *mesh_src_ = mesh_src->Get_mesh_();
  if (pid_src > mesh_src_->hdr.NumParts || pid_src < 0)
    throw std::out_of_range("InsertPart: part index (pid_src) out of range");
  const int pid_new = fcelib::FCELIB_CopyPartToMesh(&this->mesh_, mesh_src_, pid_src);
  if (pid_new < 0)
    throw std::runtime_error("InsertPart: Cannot copy part");
  return pid_new;
}

bool Mesh::DeletePart(const int pid)
{
  if (pid > mesh_.hdr.NumParts || pid < 0)
    throw std::out_of_range("DeletePart: part index (pid) out of range");
  return fcelib::FCELIB_DeletePart(&mesh_, pid);
}

bool Mesh::DelPartTriags(const int pid, const std::vector<int> &idxs)
{
  if (pid > mesh_.hdr.NumParts || pid < 0)
    throw std::out_of_range("DelPartTriags: part index (pid) out of range");
  return fcelib::FCELIB_DeletePartTriags(&mesh_, pid, idxs.data(), static_cast<int>(idxs.size()));
}

int Mesh::MergeParts(const int pid1, const int pid2)
{
  if (pid1 > mesh_.hdr.NumParts || pid1 < 0)
    throw std::out_of_range("MergeParts: part index (pid1) out of range");
  if (pid2 > mesh_.hdr.NumParts || pid2 < 0)
    throw std::out_of_range("MergeParts: part index (pid2) out of range");
  const int pid_new = fcelib::FCELIB_MergePartsToNew(&mesh_, pid1, pid2);
  if (pid_new < 0)
    throw std::runtime_error("MergeParts");
  return pid_new;
}

int Mesh::MovePart(const int pid)
{
  if (pid > mesh_.hdr.NumParts || pid < 0)
    throw std::out_of_range("MovePart: part index (pid) out of range");
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

  fcecodec_module.def("PrintFceInfo", &FCECODECMODULE_PrintFceInfo, py::arg("buf"));
  fcecodec_module.def("ValidateFce", &FCECODECMODULE_ValidateFce, py::arg("buf"));

  py::class_<Mesh>(fcecodec_module, "Mesh", py::buffer_protocol())
    .def(py::init<>())

    .def("MValid", &Mesh::Valid)

    .def("PrintInfo", &Mesh::Info)
    .def("PrintParts", &Mesh::PrintParts)
    .def("PrintTriags", &Mesh::PrintTriags)
    .def("PrintVerts", &Mesh::PrintVerts)
    .def_property_readonly("MNumArts", &Mesh::MNumArts)
    .def_property_readonly("MNumParts", &Mesh::num_parts)
    .def_property_readonly("MNumTriags", &Mesh::num_triags)
    .def_property_readonly("MNumVerts", &Mesh::num_verts)

    .def("IoDecode", &Mesh::Decode)
    .def("IoEncode_Fce3", &Mesh::Encode_Fce3, py::arg("center_parts") = true)
    .def("IoEncode_Fce4", &Mesh::Encode_Fce4, py::arg("center_parts") = true)
    .def("IoEncode_Fce4M", &Mesh::Encode_Fce4M, py::arg("center_parts") = true)
    .def("IoExportObj", &Mesh::ExportObj, py::arg("objpath"), py::arg("mtlpath"), py::arg("texname"), py::arg("print_damage") = 0, py::arg("print_dummies") = 0)  // TODO: test
    .def("IoGeomDataToNewPart", &Mesh::IoGeomDataToNewPart_numpy,
      py::arg("vert_idxs"), py::arg("vert_texcoords"), py::arg("vert_pos"), py::arg("normals"),
      R"pbdoc( vert_idxs: 012..., vert_texcoords: uuuvvv... , vert_pos: xyzxyzxyz..., normals: xyzxyzxyz... )pbdoc")
    .def("IoGeomDataToNewPart_numpy", &Mesh::IoGeomDataToNewPart_numpy,
      py::arg("vert_idxs"), py::arg("vert_texcoords"), py::arg("vert_pos"), py::arg("normals"),
      R"pbdoc( vert_idxs: 012..., vert_texcoords: uuuvvv... , vert_pos: xyzxyzxyz..., normals: xyzxyzxyz... )pbdoc")

    .def("MGetColors", &Mesh::MGetColors_numpy)
    .def("MGetColors_numpy", &Mesh::MGetColors_numpy)
    .def("MSetColors", &Mesh::MSetColors_numpy, py::arg("colors"), R"pbdoc( Expects shape=(N, 4, 4) )pbdoc")
    .def("MSetColors_numpy", &Mesh::MSetColors_numpy, py::arg("colors"), R"pbdoc( Expects shape=(N, 4, 4) )pbdoc")
    .def("MGetDummyNames", &Mesh::GetDummyNames)
    .def("MSetDummyNames", &Mesh::SetDummyNames, py::arg("names"))
    .def("MGetDummyPos", &Mesh::MGetDummyPos_numpy)
    .def("MGetDummyPos_numpy", &Mesh::MGetDummyPos_numpy)
    .def("MSetDummyPos", &Mesh::MSetDummyPos_numpy, py::arg("positions"), R"pbdoc( Expects shape (N*3, ) for N dummies )pbdoc")
    .def("MSetDummyPos_numpy", &Mesh::MSetDummyPos_numpy, py::arg("positions"), R"pbdoc( Expects shape (N*3, ) for N dummies )pbdoc")

    .def("PNumTriags", &Mesh::PNumTriags, py::arg("pid"))
    .def("PNumVerts", &Mesh::PNumVerts, py::arg("pid"))

    .def("PGetName", &Mesh::GetPartName, py::arg("pid"))
    .def("PSetName", &Mesh::SetPartName, py::arg("pid"), py::arg("name"))
    .def("PGetPos", &Mesh::GetPartPos, py::arg("pid"))
    .def("PGetPos_numpy", &Mesh::GetPartPos_numpy, py::arg("pid"))
    .def("PSetPos", &Mesh::SetPartPos, py::arg("pid"), py::arg("pos"))
    .def("PSetPos_numpy", &Mesh::SetPartPos_numpy, py::arg("pid"), py::arg("pos"))

    .def("PGetTriagsVidx", &Mesh::GetTriagsVidx, py::arg("pid"), R"pbdoc( Returns (N*3, ) array of global vert indexes for N triangles. )pbdoc")
    .def("PGetTriagsVidx_numpy", &Mesh::GetTriagsVidx_numpy, py::arg("pid"), R"pbdoc( Returns (N*3, ) numpy array of global vert indexes for N triangles. )pbdoc")
    /* PSetTriagsVidx() is not in scope */
    .def("PGetTriagsFlags", &Mesh::GetTriagsFlags, py::arg("pid"))
    .def("PGetTriagsFlags_numpy", &Mesh::GetTriagsFlags_numpy, py::arg("pid"))
    .def("PSetTriagsFlags", &Mesh::SetTriagsFlags, py::arg("pid"), py::arg("arr"), R"pbdoc( Expects (N, ) array for N triangles )pbdoc")
    .def("PSetTriagsFlags_numpy", &Mesh::SetTriagsFlags_numpy, py::arg("pid"), py::arg("arr"), R"pbdoc( Expects (N, ) numpy array for N triangles )pbdoc")
    .def("PGetTriagsTexcoords_numpy", &Mesh::PGetTriagsTexcoords_numpy, py::arg("pid"), R"pbdoc( uuuvvv..., Returns (N*6, ) numpy array for N triangles. )pbdoc")
    .def("PSetTriagsTexcoords_numpy", &Mesh::PSetTriagsTexcoords_numpy, py::arg("pid"), py::arg("arr"), R"pbdoc( arr: uuuvvv..., Expects (N*6, ) numpy array for N triangles. )pbdoc")
    .def("PGetTriagsTexpages", &Mesh::GetTriagsTexpages, py::arg("pid"))
    .def("PGetTriagsTexpages_numpy", &Mesh::GetTriagsTexpages_numpy, py::arg("pid"))
    .def("PSetTriagsTexpages", &Mesh::SetTriagsTexpages, py::arg("pid"), py::arg("arr"), R"pbdoc( Expects (N, ) array for N triangles )pbdoc")
    .def("PSetTriagsTexpages_numpy", &Mesh::SetTriagsTexpages_numpy, py::arg("pid"), py::arg("arr"), R"pbdoc( Expects (N, ) numpy array for N triangles )pbdoc")

    .def_property_readonly("MVertsGetMap_idx2order", &Mesh::GetVertsMap_idx2order, R"pbdoc( Maps from global vert indexes (contained in triangles) to global vertex order. )pbdoc")
    .def_property_readonly("MVertsGetMap_idx2order_numpy", &Mesh::GetVertsMap_idx2order_numpy, R"pbdoc( Maps from global vert indexes (contained in triangles) to global vertex order. )pbdoc")
    .def_property("MVertsPos", &Mesh::GetVertsPos, &Mesh::SetVertsPos, R"pbdoc( Local vertice positions. Returns (N*3, ) array for N vertices. )pbdoc")
    .def_property("MVertsPos_numpy", &Mesh::GetVertsPos_numpy, &Mesh::SetVertsPos_numpy, R"pbdoc( Local vertice positions. Returns (N*3, ) numpy array for N vertices. )pbdoc")
    .def_property("MVertsNorms", &Mesh::GetVertsNorms, &Mesh::SetVertsNorms, R"pbdoc( Returns (N*3, ) array for N vertices. )pbdoc")
    .def_property("MVertsNorms_numpy", &Mesh::GetVertsNorms_numpy, &Mesh::SetVertsNorms_numpy, R"pbdoc( Returns (N*3, ) numpy array for N vertices. )pbdoc")
    .def_property("MVertsDamgdPos", &Mesh::GetDamgdVertsPos, &Mesh::SetDamgdVertsPos, R"pbdoc( Local vertice positions. Returns (N*3, ) array for N vertices. )pbdoc")
    .def_property("MVertsDamgdPos_numpy", &Mesh::GetDamgdVertsPos_numpy, &Mesh::SetDamgdVertsPos_numpy, R"pbdoc( Local vertice positions. Returns (N*3, ) numpy array for N vertices. )pbdoc")
    .def_property("MVertsDamgdNorms", &Mesh::GetDamgdVertsNorms, &Mesh::SetDamgdVertsNorms, R"pbdoc( Returns (N*3, ) array for N vertices. )pbdoc")
    .def_property("MVertsDamgdNorms_numpy", &Mesh::GetDamgdVertsNorms_numpy, &Mesh::SetDamgdVertsNorms_numpy, R"pbdoc( Returns (N*3, ) numpy array for N vertices. )pbdoc")
    .def_property("MVertsAnimation", &Mesh::GetVertsAnimation, &Mesh::SetVertsAnimation, R"pbdoc( Returns (N, ) array for N vertices. )pbdoc")
    .def_property("MVertsAnimation_numpy", &Mesh::GetVertsAnimation_numpy, &Mesh::SetVertsAnimation_numpy, R"pbdoc( Returns (N, ) numpy array for N vertices. )pbdoc")

    .def("OpCenterPart", &Mesh::CenterPart, py::arg("pid"), R"pbdoc( Center specified part vertices positions to local centroid. )pbdoc")
    .def("OpCopyPart", &Mesh::CopyPart, py::arg("pid_src"), R"pbdoc( Copy specified part. Returns new part index. )pbdoc")
    .def("OpInsertPart", &Mesh::InsertPart, py::arg("mesh_src"), py::arg("pid_src"), R"pbdoc( Insert (copy) specified part from mesh_src. Returns new part index. )pbdoc")
    .def("OpDeletePart", &Mesh::DeletePart, py::arg("pid"))
    .def("OpDelPartTriags", &Mesh::DelPartTriags, py::arg("pid"), py::arg("idxs"))
    .def("OpDelUnrefdVerts", &Mesh::DelUnrefdVerts, R"pbdoc( Delete all vertices that are not referenced by any triangle. )pbdoc")
    .def("OpMergeParts", &Mesh::MergeParts, py::arg("pid1"), py::arg("pid2"), R"pbdoc( Returns new part index. )pbdoc")
    .def("OpMovePart", &Mesh::MovePart, py::arg("pid"), R"pbdoc( Move up specified part towards order 0. Returns new part index. )pbdoc")
    ;
}