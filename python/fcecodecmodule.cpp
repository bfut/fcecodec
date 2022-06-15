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

#ifdef PYMEM_MALLOC
#define malloc PyMem_Malloc
#define realloc PyMem_Realloc
#define free PyMem_Free
#endif  /* PYMEM_MALLOC */

#include "../src/fcelib/fcelib.h"
#include "../src/fcelib/fcelib_types.h"

namespace py = pybind11;


/* classes, structs --------------------------------------------------------- */

class Mesh : public fcelib::FcelibMesh {
  public:
    Mesh() : mesh_(*this) { fcelib::FCELIB_InitMesh(&mesh_); }
    ~Mesh() { fcelib::FCELIB_FreeMesh(&mesh_); }

    // Internal
    fcelib::FcelibMesh *Get_mesh_() { return &mesh_; }

    // Service
    bool MValid() const { return fcelib::FCELIB_ValidateMesh(mesh_); }

    /* Stats */
    void PrintInfo() const { fcelib::FCELIB_PrintMeshInfo(mesh_); }
    void PrintParts(void) const { fcelib::FCELIB_PrintMeshParts(mesh_); }
    void PrintTriags(void) const { fcelib::FCELIB_PrintMeshTriangles(mesh_); }
    void PrintVerts(void) const { fcelib::FCELIB_PrintMeshVertices(mesh_); }
    int MNumParts() const { return mesh_.hdr.NumParts; };
    int MNumTriags() const { return mesh_.hdr.NumTriangles; };
    int MNumVerts() const { return mesh_.hdr.NumVertices; };

    /* i/o */
    void IoDecode(const std::string &buf);
    py::bytes IoEncode_Fce3(const bool center_parts) const;
    py::bytes IoEncode_Fce4(const bool center_parts) const;
    py::bytes IoEncode_Fce4M(const bool center_parts) const;
    void IoExportObj(std::string &objpath, std::string &mtlpath,
                     std::string &texture_name,
                     const int print_damage, const int print_dummies,
                     const int use_part_positions) const;
    int IoGeomDataToNewPart_numpy(py::array_t<int, py::array::c_style | py::array::forcecast> vert_idxs,
                                  py::array_t<float, py::array::c_style | py::array::forcecast> vert_texcoords,
                                  py::array_t<float, py::array::c_style | py::array::forcecast> vert_pos,
                                  py::array_t<float, py::array::c_style | py::array::forcecast> normals);

    /* Mesh / Header */
    int MGetNumArts() const { return mesh_.hdr.NumArts; };
    void MSetNumArts(const int NumArts) { mesh_.hdr.NumArts = NumArts; };
    int MGetUnknown3() const { return mesh_.hdr.Unknown3; };
    void MSetUnknown3(const int Unknown3) { mesh_.hdr.Unknown3 = Unknown3; };
    py::buffer MGetColors_numpy(void) const;
    void MSetColors_numpy(py::array_t<unsigned char, py::array::c_style | py::array::forcecast> arr);
    std::vector<std::string> GetDummyNames() const;
    void SetDummyNames(std::vector<std::string> &arr);
    py::buffer MGetDummyPos_numpy(void) const;
    void MSetDummyPos_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr);

    /* Parts */
    int PNumTriags(const int pid) const;
    int PNumVerts(const int pid) const;

    const std::string PGetName(const int pid) const;
    void PSetName(const int pid, const std::string &s);
    const std::array<float, 3> PGetPos(const int pid) const;
    py::buffer PGetPos_numpy(const int pid) const;
    void PSetPos(const int pid, std::array<float, 3> &arr);
    void PSetPos_numpy(const int pid, py::array_t<float, py::array::c_style | py::array::forcecast> arr);

    /* Triags */
    std::vector<int> PGetTriagsVidx(const int pid) const;
    py::buffer PGetTriagsVidx_numpy(const int pid) const;
    std::vector<int> PGetTriagsFlags(const int pid) const;
    py::buffer PGetTriagsFlags_numpy(const int pid) const;
    void PSetTriagsFlags(const int pid, std::vector<int> &arr);
    void PSetTriagsFlags_numpy(const int pid, py::array_t<int, py::array::c_style | py::array::forcecast> arr);
    py::buffer PGetTriagsTexcoords_numpy(const int pid) const;
    void PSetTriagsTexcoords_numpy(const int pid, py::array_t<float, py::array::c_style | py::array::forcecast> arr);
    std::vector<int> PGetTriagsTexpages(const int pid) const;
    py::buffer PGetTriagsTexpages_numpy(const int pid) const;
    void PSetTriagsTexpages(const int pid, std::vector<int> &arr);
    void PSetTriagsTexpages_numpy(const int pid, py::array_t<int, py::array::c_style | py::array::forcecast> arr);

    /* Verts */
    std::vector<int> MVertsGetMap_idx2order() const;
    py::buffer MVertsGetMap_idx2order_numpy() const;

    std::vector<float> MGetVertsPos() const;
    py::buffer MGetVertsPos_numpy() const;
    void MSetVertsPos(std::vector<float> arr);
    void MSetVertsPos_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr);
    std::vector<float> MGetVertsNorms() const;
    py::buffer MGetVertsNorms_numpy() const;
    void MSetVertsNorms(std::vector<float> arr);
    void MSetVertsNorms_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr);
    std::vector<float> MGetDamgdVertsPos() const;
    py::buffer MGetDamgdVertsPos_numpy() const;
    void MSetDamgdVertsPos(std::vector<float> arr);
    void MSetDamgdVertsPos_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr);
    std::vector<float> MGetDamgdVertsNorms() const;
    py::buffer MGetDamgdVertsNorms_numpy() const;
    void MSetDamgdVertsNorms(std::vector<float> arr);
    void MSetDamgdVertsNorms_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr);
    std::vector<int> MGetVertsAnimation() const;
    py::buffer MGetVertsAnimation_numpy() const;
    void MSetVertsAnimation(std::vector<int> arr);
    void MSetVertsAnimation_numpy(py::array_t<int, py::array::c_style | py::array::forcecast> arr);

    /* Operations */
    int OpAddHelperPart(const std::string &s, std::array<float, 3> &new_center);
    int OpAddHelperPart_numpy(const std::string &s, py::array_t<float, py::array::c_style | py::array::forcecast> new_center);
    bool OpCenterPart(const int pid);
    bool OpSetPartCenter(const int pid, std::array<float, 3> &new_center);
    bool OpSetPartCenter_numpy(const int pid, py::array_t<float, py::array::c_style | py::array::forcecast> new_center);
    int OpCopyPart(const int pid_src);
    int OpInsertPart(Mesh *mesh_src, const int pid_src);
    bool OpDeletePart(const int pid);
    bool OpDeletePartTriags(const int pid, const std::vector<int> &idxs);
    bool OpDelUnrefdVerts() { return fcelib::FCELIB_DeleteUnrefdVerts(&mesh_); }
    int OpMergeParts(const int pid1, const int pid2);
    int OpMovePart(const int pid);

  private:
    fcelib::FcelibMesh& mesh_;
};


/* Mesh:: wrappers ---------------------------------------------------------- */
/* i/o ------------------------------ */

void Mesh::IoDecode(const std::string &buf)
{
  fcelib::FCELIB_FreeMesh(&mesh_);
  fcelib::FCELIB_InitMesh(&mesh_);
  if (!fcelib::FCELIB_DecodeFce(buf.c_str(), buf.size(), &mesh_))
    throw std::runtime_error("IoDecode: Cannot parse FCE data");
}

py::bytes Mesh::IoEncode_Fce3(const bool center_parts) const
{
  const int bufsize_ = fcelib::FCELIB_FCETYPES_Fce3ComputeSize(mesh_.hdr.NumVertices, mesh_.hdr.NumTriangles);
  unsigned char *buf_ = (unsigned char *)malloc(static_cast<std::size_t>(bufsize_) * sizeof(*buf_));
  if (!buf_)
    throw std::runtime_error("IoEncode_Fce3: Cannot allocate memory");
  if (!fcelib::FCELIB_EncodeFce3(&buf_, bufsize_, &mesh_, static_cast<int>(center_parts)))
    throw std::runtime_error("IoEncode_Fce3: Cannot encode FCE3");
  py::bytes result = py::bytes((char *)buf_, static_cast<std::size_t>(bufsize_));
  free(buf_);
  return result;
}

py::bytes Mesh::IoEncode_Fce4(const bool center_parts) const
{
  const int bufsize_ = fcelib::FCELIB_FCETYPES_Fce4ComputeSize(0x00101014, mesh_.hdr.NumVertices, mesh_.hdr.NumTriangles);
  unsigned char *buf_ = (unsigned char *)malloc(static_cast<std::size_t>(bufsize_) * sizeof(*buf_));
  if (!buf_)
    throw std::runtime_error("IoEncode_Fce4: Cannot allocate memory");
  if (!fcelib::FCELIB_EncodeFce4(&buf_, bufsize_, &mesh_, static_cast<int>(center_parts)))
    throw std::runtime_error("IoEncode_Fce4: Cannot encode FCE4");
  py::bytes result = py::bytes((char *)buf_, static_cast<std::size_t>(bufsize_));
  free(buf_);
  return result;
}

py::bytes Mesh::IoEncode_Fce4M(const bool center_parts) const
{
  const int bufsize_ = fcelib::FCELIB_FCETYPES_Fce4ComputeSize(0x00101015, mesh_.hdr.NumVertices, mesh_.hdr.NumTriangles);
  unsigned char *buf_ = (unsigned char *)malloc(static_cast<std::size_t>(bufsize_) * sizeof(*buf_));
  if (!buf_)
    throw std::runtime_error("IoEncode_Fce4M: Cannot allocate memory");
  if (!fcelib::FCELIB_EncodeFce4M(&buf_, bufsize_, &mesh_, static_cast<int>(center_parts)))
    throw std::runtime_error("IoEncode_Fce4M: Cannot encode FCE4M");
  py::bytes result = py::bytes((char *)buf_, static_cast<std::size_t>(bufsize_));
  free(buf_);
  return result;
}

void Mesh::IoExportObj(std::string &objpath, std::string &mtlpath,
                       std::string &texture_name,
                       const int print_damage, const int print_dummies,
                       const int use_part_positions) const
{
  if (fcelib::FCELIB_ExportObj(&mesh_, objpath.c_str(), mtlpath.c_str(),
                               texture_name.c_str(),
                               print_damage, print_dummies,
                               use_part_positions) == 0)
    throw std::runtime_error("IoExportObj: Cannot export OBJ");
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

const std::string Mesh::PGetName(const int pid) const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("PGetName: failure");
  const int idx = fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid);
  if (idx < 0)
    throw std::out_of_range("PGetName: part index (pid) out of range");
  return std::string(mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartName);
}
void Mesh::PSetName(const int pid, const std::string &s)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("PSetName: failure");
  const int idx = fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid);
  if (idx < 0)
    throw std::out_of_range("PSetName: part index (pid) out of range");
  std::strncpy(mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartName, s.c_str(),
               sizeof(mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartName) - 1);  // max 63 chars
}

const std::array<float, 3> Mesh::PGetPos(const int pid) const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("PGetPos: failure");
  const int idx = fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid);
  if (idx < 0)
    throw std::out_of_range("PGetPos: part index (pid) out of range");
  std::array<float, 3> result;
  result[0] = mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.x;
  result[1] = mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.y;
  result[2] = mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.z;
  return result;
}
py::buffer Mesh::PGetPos_numpy(const int pid) const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("PGetPos_numpy: failure");
  const int idx = fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid);
  if (idx < 0)
    throw std::out_of_range("PGetPos_numpy: part index (pid) out of range");

  py::array_t<float> result = py::array_t<float>({ 3 }, {  });
  auto buf = result.mutable_unchecked<1>();
  memcpy(&buf(0), &mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.x, sizeof(float));
  memcpy(&buf(1), &mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.y, sizeof(float));
  memcpy(&buf(2), &mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.z, sizeof(float));
  return std::move(result);
}
void Mesh::PSetPos(const int pid, std::array<float, 3> &arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("PSetPos: failure");
  const int idx = fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid);
  if (idx < 0)
    throw std::out_of_range("PSetPos: part index (pid) out of range");

  mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.x = arr[0];
  mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.y = arr[1];
  mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.z = arr[2];
}
void Mesh::PSetPos_numpy(const int pid, py::array_t<float, py::array::c_style | py::array::forcecast> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("PSetPos_numpy: failure");
  const int idx = fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid);
  if (idx < 0)
    throw std::out_of_range("PSetPos_numpy: part index (pid) out of range");

  py::buffer_info buf = arr.request();
  float *ptr;

  if (buf.ndim != 1)
    throw std::runtime_error("PSetPos_numpy: Number of dimensions must be 1");
  if (buf.shape[0] != 3)
    throw std::runtime_error("PSetPos_numpy: Shape must be (3, )");
  ptr = static_cast<float *>(buf.ptr);
  memcpy(&mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.x, ptr + 0, sizeof(float));
  memcpy(&mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.y, ptr + 1, sizeof(float));
  memcpy(&mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartPos.z, ptr + 2, sizeof(float));
}


/* triags --------------------------- */

std::vector<int> Mesh::PGetTriagsVidx(const int pid) const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("PGetTriagsVidx: failure");
  if (pid < 0 || pid >= mesh_.hdr.NumParts)
    throw std::range_error("PGetTriagsVidx: pid");
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
py::buffer Mesh::PGetTriagsVidx_numpy(const int pid) const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("PGetTriagsVidx_numpy: failure");
  if (pid < 0 || pid >= mesh_.hdr.NumParts)
    throw std::range_error("PGetTriagsVidx_numpy: pid");

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


std::vector<int> Mesh::PGetTriagsFlags(const int pid) const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("PGetTriagsFlags: failure");
  if (pid < 0 || pid >= mesh_.hdr.NumParts)
    throw std::range_error("PGetTriagsFlags: pid");
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
py::buffer Mesh::PGetTriagsFlags_numpy(const int pid) const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("PGetTriagsFlags_numpy: failure");
  if (pid < 0 || pid >= mesh_.hdr.NumParts)
    throw std::range_error("PGetTriagsFlags_numpy: pid");
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
void Mesh::PSetTriagsFlags(const int pid, std::vector<int> &arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("PSetTriagsFlags: failure");
  if (pid < 0 || pid >= mesh_.hdr.NumParts)
    throw std::range_error("PSetTriagsFlags: pid");
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
void Mesh::PSetTriagsFlags_numpy(const int pid, py::array_t<int, py::array::c_style | py::array::forcecast> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("PSetTriagsFlags_numpy: failure");
  if (pid < 0 || pid >= mesh_.hdr.NumParts)
    throw std::range_error("PSetTriagsFlags_numpy: pid");
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
    ++j;
  }
}


std::vector<int> Mesh::PGetTriagsTexpages(const int pid) const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("PGetTriagsTexpages: failure");
  if (pid < 0 || pid >= mesh_.hdr.NumParts)
    throw std::range_error("PGetTriagsTexpages: pid");
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
py::buffer Mesh::PGetTriagsTexpages_numpy(const int pid) const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("PGetTriagsTexpages_numpy: failure");
  if (pid < 0 || pid >= mesh_.hdr.NumParts)
    throw std::range_error("PGetTriagsTexpages_numpy: pid");
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
void Mesh::PSetTriagsTexpages(const int pid, std::vector<int> &arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("PSetTriagsTexpages: failure");
  if (pid < 0 || pid >= mesh_.hdr.NumParts)
    throw std::range_error("PSetTriagsTexpages: pid");
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
void Mesh::PSetTriagsTexpages_numpy(const int pid, py::array_t<int, py::array::c_style | py::array::forcecast> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("PSetTriagsTexpages_numpy: failure");
  if (pid < 0 || pid >= mesh_.hdr.NumParts)
    throw std::range_error("PSetTriagsTexpages_numpy: pid");
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
std::vector<int> Mesh::MVertsGetMap_idx2order() const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("MVertsGetMap_idx2order: failure");
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
py::buffer Mesh::MVertsGetMap_idx2order_numpy() const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("MVertsGetMap_idx2order_numpy: failure");
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


std::vector<float> Mesh::MGetVertsPos() const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("MGetVertsPos: failure");
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
py::buffer Mesh::MGetVertsPos_numpy() const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("MGetVertsPos_numpy: failure");
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
void Mesh::MSetVertsPos(std::vector<float> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("MSetVertsPos: failure");
  const std::size_t nrows = static_cast<std::size_t>(mesh_.hdr.NumVertices);
  if (arr.size() != nrows * 3)
    throw std::runtime_error("Shape must be (N*3, ) where N = Mesh.MNumVerts()");
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
void Mesh::MSetVertsPos_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("MSetVertsPos_numpy: failure");
  const py::ssize_t nrows = static_cast<py::ssize_t>(mesh_.hdr.NumVertices);
  py::buffer_info buf = arr.request();
  if (buf.ndim != 1)
    throw std::runtime_error("Number of dimensions must be 1");
  if (buf.shape[0] != nrows * 3)
    throw std::runtime_error("Shape must be (N*3, ) where N = Mesh.MNumVerts()");
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


std::vector<float> Mesh::MGetVertsNorms() const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("MGetVertsNorms: failure");
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
py::buffer Mesh::MGetVertsNorms_numpy() const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("MGetVertsNorms_numpy: failure");
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
void Mesh::MSetVertsNorms(std::vector<float> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("MSetVertsNorms: failure");
  const std::size_t nrows = static_cast<std::size_t>(mesh_.hdr.NumVertices);
  if (arr.size() != nrows * 3)
    throw std::runtime_error("Shape must be (N*3, ) where N = Mesh.MNumVerts()");
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
void Mesh::MSetVertsNorms_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("MSetVertsNorms_numpy: failure");
  const py::ssize_t nrows = static_cast<py::ssize_t>(mesh_.hdr.NumVertices);
  py::buffer_info buf = arr.request();
  if (buf.ndim != 1)
    throw std::runtime_error("Number of dimensions must be 1");
  if (buf.shape[0] != nrows * 3)
    throw std::runtime_error("Shape must be (N*3, ) where N = Mesh.MNumVerts()");
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


std::vector<float> Mesh::MGetDamgdVertsPos() const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("MGetDamgdVertsPos: failure");
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
py::buffer Mesh::MGetDamgdVertsPos_numpy() const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("MGetDamgdVertsPos_numpy: failure");
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
void Mesh::MSetDamgdVertsPos(std::vector<float> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("MSetDamgdVertsPos: failure");
  const std::size_t nrows = static_cast<std::size_t>(mesh_.hdr.NumVertices);
  if (arr.size() != nrows * 3)
    throw std::runtime_error("Shape must be (N*3, ) where N = Mesh.MNumVerts()");
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
void Mesh::MSetDamgdVertsPos_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("MSetDamgdVertsPos_numpy: failure");
  const py::ssize_t nrows = static_cast<py::ssize_t>(mesh_.hdr.NumVertices);
  py::buffer_info buf = arr.request();
  if (buf.ndim != 1)
    throw std::runtime_error("Number of dimensions must be 1");
  if (buf.shape[0] != nrows * 3)
    throw std::runtime_error("Shape must be (N*3, ) where N = Mesh.MNumVerts()");
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


std::vector<float> Mesh::MGetDamgdVertsNorms() const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("MGetDamgdVertsNorms: failure");
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
py::buffer Mesh::MGetDamgdVertsNorms_numpy() const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("MGetDamgdVertsNorms_numpy: failure");
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
void Mesh::MSetDamgdVertsNorms(std::vector<float> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("MSetDamgdVertsNorms: failure");
  const std::size_t nrows = static_cast<std::size_t>(mesh_.hdr.NumVertices);
  if (arr.size() != nrows * 3)
    throw std::runtime_error("Shape must be (N*3, ) where N = Mesh.MNumVerts()");
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
void Mesh::MSetDamgdVertsNorms_numpy(py::array_t<float, py::array::c_style | py::array::forcecast> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("MSetDamgdVertsNorms_numpy: failure");
  const py::ssize_t nrows = static_cast<py::ssize_t>(mesh_.hdr.NumVertices);
  py::buffer_info buf = arr.request();
  if (buf.ndim != 1)
    throw std::runtime_error("Number of dimensions must be 1");
  if (buf.shape[0] != nrows * 3)
    throw std::runtime_error("Shape must be (N*3, ) where N = Mesh.MNumVerts()");
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


std::vector<int> Mesh::MGetVertsAnimation() const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("MGetVertsAnimation: failure");
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
py::buffer Mesh::MGetVertsAnimation_numpy() const
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("MGetVertsAnimation_numpy: failure");
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
void Mesh::MSetVertsAnimation(std::vector<int> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("MSetVertsAnimation: failure");
  const std::size_t nrows = static_cast<std::size_t>(mesh_.hdr.NumVertices);
  if (arr.size() != nrows)
    throw std::runtime_error("Shape must be (N, ) where N = Mesh.MNumVerts()");
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
void Mesh::MSetVertsAnimation_numpy(py::array_t<int, py::array::c_style | py::array::forcecast> arr)
{
  if (!fcelib::FCELIB_ValidateMesh(mesh_))
    std::runtime_error("MSetVertsAnimation_numpy: failure");
  const py::ssize_t nrows = static_cast<py::ssize_t>(mesh_.hdr.NumVertices);
  py::buffer_info buf = arr.request();
  if (buf.ndim != 1)
    throw std::runtime_error("Number of dimensions must be 1");
  if (buf.shape[0] != nrows)
    throw std::runtime_error("Shape must be (N, ) where N = Mesh.MNumVerts()");
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

int Mesh::OpAddHelperPart(const std::string &s, std::array<float, 3> &new_center)
{
  const int pid_new = FCELIB_AddHelperPart(&mesh_);
  if (pid_new < 0)
    throw std::runtime_error("OpAddHelperPart: Cannot add helper part");
  Mesh::PSetPos(pid_new, new_center);
  const int idx = fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid_new);
  if (idx < 0)
    throw std::out_of_range("OpAddHelperPart: part index (pid) out of range");
  std::strncpy(mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartName, s.c_str(),
               sizeof(mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartName) - 1);  // max 63 chars
  return pid_new;
}

int Mesh::OpAddHelperPart_numpy(const std::string &s, py::array_t<float, py::array::c_style | py::array::forcecast> new_center)
{
  const int pid_new = FCELIB_AddHelperPart(&mesh_);
  if (pid_new < 0)
    throw std::runtime_error("OpAddHelperPart: Cannot add helper part");
  Mesh::PSetPos_numpy(pid_new, new_center);
  const int idx = fcelib::FCELIB_GetInternalPartIdxByOrder(&mesh_, pid_new);
  if (idx < 0)
    throw std::out_of_range("OpAddHelperPart: part index (pid) out of range");
  std::strncpy(mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartName, s.c_str(),
               sizeof(mesh_.parts[ mesh_.hdr.Parts[idx] ]->PartName) - 1);  // max 63 chars
  return pid_new;
}

bool Mesh::OpCenterPart(const int pid)
{
  if (pid > mesh_.hdr.NumParts || pid < 0)
    throw std::out_of_range("OpCenterPart: part index (pid) out of range");
  return fcelib::FCELIB_CenterPart(&mesh_, pid);
}

bool Mesh::OpSetPartCenter(const int pid, std::array<float, 3> &new_center)
{
  if (pid > mesh_.hdr.NumParts || pid < 0)
    throw std::out_of_range("OpSetPartCenter: part index (pid) out of range");
  return fcelib::FCELIB_SetPartCenter(&mesh_, pid, new_center.data());
}

bool Mesh::OpSetPartCenter_numpy(const int pid, py::array_t<float, py::array::c_style | py::array::forcecast> new_center)
{
  if (pid > mesh_.hdr.NumParts || pid < 0)
    throw std::out_of_range("OpSetPartCenter: part index (pid) out of range");
  py::buffer_info buf = new_center.request();
  if (buf.ndim != 1)
    throw std::runtime_error("OpSetPartCenter: Number of dimensions must be 1");
  if (buf.shape[0] != 3)
    throw std::runtime_error("OpSetPartCenter: Shape must be (3, )");
  return fcelib::FCELIB_SetPartCenter(&mesh_, pid, static_cast<float *>(buf.ptr));
}

int Mesh::OpCopyPart(const int pid_src)
{
  if (pid_src > this->mesh_.hdr.NumParts || pid_src < 0)
    throw std::out_of_range("OpCopyPart: part index (pid_src) out of range");
  const int pid_new = fcelib::FCELIB_CopyPartToMesh(&this->mesh_, &this->mesh_, pid_src);
  if (pid_new < 0)
    throw std::runtime_error("OpCopyPart: Cannot copy part");
  return pid_new;
}

int Mesh::OpInsertPart(Mesh *mesh_src, const int pid_src)
{
  fcelib::FcelibMesh *mesh_src_ = mesh_src->Get_mesh_();
  if (pid_src > mesh_src_->hdr.NumParts || pid_src < 0)
    throw std::out_of_range("OpInsertPart: part index (pid_src) out of range");
  const int pid_new = fcelib::FCELIB_CopyPartToMesh(&this->mesh_, mesh_src_, pid_src);
  if (pid_new < 0)
    throw std::runtime_error("OpInsertPart: Cannot copy part");
  return pid_new;
}

bool Mesh::OpDeletePart(const int pid)
{
  if (pid > mesh_.hdr.NumParts || pid < 0)
    throw std::out_of_range("OpDeletePart: part index (pid) out of range");
  return fcelib::FCELIB_DeletePart(&mesh_, pid);
}

bool Mesh::OpDeletePartTriags(const int pid, const std::vector<int> &idxs)
{
  if (pid > mesh_.hdr.NumParts || pid < 0)
    throw std::out_of_range("OpDeletePartTriags: part index (pid) out of range");
  return fcelib::FCELIB_DeletePartTriags(&mesh_, pid, idxs.data(), static_cast<int>(idxs.size()));
}

int Mesh::OpMergeParts(const int pid1, const int pid2)
{
  if (pid1 > mesh_.hdr.NumParts || pid1 < 0)
    throw std::out_of_range("OpMergeParts: part index (pid1) out of range");
  if (pid2 > mesh_.hdr.NumParts || pid2 < 0)
    throw std::out_of_range("OpMergeParts: part index (pid2) out of range");
  const int pid_new = fcelib::FCELIB_MergePartsToNew(&mesh_, pid1, pid2);
  if (pid_new < 0)
    throw std::runtime_error("OpMergeParts");
  return pid_new;
}

int Mesh::OpMovePart(const int pid)
{
  if (pid > mesh_.hdr.NumParts || pid < 0)
    throw std::out_of_range("OpMovePart: part index (pid) out of range");
  return fcelib::FCELIB_MeshMoveUpPart(&mesh_, pid);
}


/* wrappers ----------------------------------------------------------------- */

/* fcecodec. */
int FCECODECMODULE_GetFceVersion(const std::string &buf)
{
  return fcelib::FCELIB_GetFceVersion(buf.c_str(), buf.size());
}

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


/* -------------------------------------------------------------------------- */

PYBIND11_MODULE(fcecodec, fcecodec_module)
{
  fcecodec_module.doc() = "FCE decoder/encoder";

  fcecodec_module.def("GetFceVersion", &FCECODECMODULE_GetFceVersion, py::arg("buf"), R"pbdoc( Returns 3 (FCE3), 4 (FCE4), 5 (FCE4M), negative (invalid) )pbdoc");
  fcecodec_module.def("PrintFceInfo", &FCECODECMODULE_PrintFceInfo, py::arg("buf"));
  fcecodec_module.def("ValidateFce", &FCECODECMODULE_ValidateFce, py::arg("buf"), R"pbdoc( Returns 1 for valid FCE data, 0 otherwise. )pbdoc");

  py::class_<Mesh>(fcecodec_module, "Mesh", py::buffer_protocol())
    .def(py::init<>())

    .def("MValid", &Mesh::MValid)

    .def("PrintInfo", &Mesh::PrintInfo)
    .def("PrintParts", &Mesh::PrintParts)
    .def("PrintTriags", &Mesh::PrintTriags)
    .def("PrintVerts", &Mesh::PrintVerts)
    .def_property_readonly("MNumParts", &Mesh::MNumParts)
    .def_property_readonly("MNumTriags", &Mesh::MNumTriags)
    .def_property_readonly("MNumVerts", &Mesh::MNumVerts)

    .def("IoDecode", &Mesh::IoDecode)
    .def("IoEncode_Fce3", &Mesh::IoEncode_Fce3, py::arg("center_parts") = true)
    .def("IoEncode_Fce4", &Mesh::IoEncode_Fce4, py::arg("center_parts") = true)
    .def("IoEncode_Fce4M", &Mesh::IoEncode_Fce4M, py::arg("center_parts") = true)
    .def("IoExportObj", &Mesh::IoExportObj, py::arg("objpath"), py::arg("mtlpath"), py::arg("texname"), py::arg("print_damage") = 0, py::arg("print_dummies") = 0, py::arg("use_part_positions") = 1)
    .def("IoGeomDataToNewPart", &Mesh::IoGeomDataToNewPart_numpy,
      py::arg("vert_idxs"), py::arg("vert_texcoords"), py::arg("vert_pos"), py::arg("normals"),
      R"pbdoc( vert_idxs: 012..., vert_texcoords: uuuvvv... , vert_pos: xyzxyzxyz..., normals: xyzxyzxyz... )pbdoc")
    .def("IoGeomDataToNewPart_numpy", &Mesh::IoGeomDataToNewPart_numpy,
      py::arg("vert_idxs"), py::arg("vert_texcoords"), py::arg("vert_pos"), py::arg("normals"),
      R"pbdoc( vert_idxs: 012..., vert_texcoords: uuuvvv... , vert_pos: xyzxyzxyz..., normals: xyzxyzxyz... )pbdoc")

    .def_property("MNumArts", &Mesh::MGetNumArts, &Mesh::MSetNumArts, R"pbdoc( Usually equal to 1. Larger values enable multi-texture access for cop#.fce )pbdoc")
    .def_property("MUnknown3", &Mesh::MGetUnknown3, &Mesh::MSetUnknown3, R"pbdoc( Unknown purpose in FCE4M. Only exists in FCE4M. )pbdoc")
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

    .def("PGetName", &Mesh::PGetName, py::arg("pid"))
    .def("PSetName", &Mesh::PSetName, py::arg("pid"), py::arg("name"))
    .def("PGetPos", &Mesh::PGetPos, py::arg("pid"))
    .def("PGetPos_numpy", &Mesh::PGetPos_numpy, py::arg("pid"))
    .def("PSetPos", &Mesh::PSetPos, py::arg("pid"), py::arg("pos"))
    .def("PSetPos_numpy", &Mesh::PSetPos_numpy, py::arg("pid"), py::arg("pos"))

    .def("PGetTriagsVidx", &Mesh::PGetTriagsVidx, py::arg("pid"), R"pbdoc( Returns (N*3, ) array of global vert indexes for N triangles. )pbdoc")
    .def("PGetTriagsVidx_numpy", &Mesh::PGetTriagsVidx_numpy, py::arg("pid"), R"pbdoc( Returns (N*3, ) numpy array of global vert indexes for N triangles. )pbdoc")
    /* PSetTriagsVidx() is not in scope */
    .def("PGetTriagsFlags", &Mesh::PGetTriagsFlags, py::arg("pid"))
    .def("PGetTriagsFlags_numpy", &Mesh::PGetTriagsFlags_numpy, py::arg("pid"))
    .def("PSetTriagsFlags", &Mesh::PSetTriagsFlags, py::arg("pid"), py::arg("arr"), R"pbdoc( Expects (N, ) array for N triangles )pbdoc")
    .def("PSetTriagsFlags_numpy", &Mesh::PSetTriagsFlags_numpy, py::arg("pid"), py::arg("arr"), R"pbdoc( Expects (N, ) numpy array for N triangles )pbdoc")
    .def("PGetTriagsTexcoords_numpy", &Mesh::PGetTriagsTexcoords_numpy, py::arg("pid"), R"pbdoc( uuuvvv..., Returns (N*6, ) numpy array for N triangles. )pbdoc")
    .def("PSetTriagsTexcoords_numpy", &Mesh::PSetTriagsTexcoords_numpy, py::arg("pid"), py::arg("arr"), R"pbdoc( arr: uuuvvv..., Expects (N*6, ) numpy array for N triangles. )pbdoc")
    .def("PGetTriagsTexpages", &Mesh::PGetTriagsTexpages, py::arg("pid"))
    .def("PGetTriagsTexpages_numpy", &Mesh::PGetTriagsTexpages_numpy, py::arg("pid"))
    .def("PSetTriagsTexpages", &Mesh::PSetTriagsTexpages, py::arg("pid"), py::arg("arr"), R"pbdoc( Expects (N, ) array for N triangles )pbdoc")
    .def("PSetTriagsTexpages_numpy", &Mesh::PSetTriagsTexpages_numpy, py::arg("pid"), py::arg("arr"), R"pbdoc( Expects (N, ) numpy array for N triangles )pbdoc")

    .def_property_readonly("MVertsGetMap_idx2order", &Mesh::MVertsGetMap_idx2order, R"pbdoc( Maps from global vert indexes (contained in triangles) to global vertex order. )pbdoc")
    .def_property_readonly("MVertsGetMap_idx2order_numpy", &Mesh::MVertsGetMap_idx2order_numpy, R"pbdoc( Maps from global vert indexes (contained in triangles) to global vertex order. )pbdoc")
    .def_property("MVertsPos", &Mesh::MGetVertsPos, &Mesh::MSetVertsPos, R"pbdoc( Local vertice positions. Returns (N*3, ) array for N vertices. )pbdoc")
    .def_property("MVertsPos_numpy", &Mesh::MGetVertsPos_numpy, &Mesh::MSetVertsPos_numpy, R"pbdoc( Local vertice positions. Returns (N*3, ) numpy array for N vertices. )pbdoc")
    .def_property("MVertsNorms", &Mesh::MGetVertsNorms, &Mesh::MSetVertsNorms, R"pbdoc( Returns (N*3, ) array for N vertices. )pbdoc")
    .def_property("MVertsNorms_numpy", &Mesh::MGetVertsNorms_numpy, &Mesh::MSetVertsNorms_numpy, R"pbdoc( Returns (N*3, ) numpy array for N vertices. )pbdoc")
    .def_property("MVertsDamgdPos", &Mesh::MGetDamgdVertsPos, &Mesh::MSetDamgdVertsPos, R"pbdoc( Local vertice positions. Returns (N*3, ) array for N vertices. )pbdoc")
    .def_property("MVertsDamgdPos_numpy", &Mesh::MGetDamgdVertsPos_numpy, &Mesh::MSetDamgdVertsPos_numpy, R"pbdoc( Local vertice positions. Returns (N*3, ) numpy array for N vertices. )pbdoc")
    .def_property("MVertsDamgdNorms", &Mesh::MGetDamgdVertsNorms, &Mesh::MSetDamgdVertsNorms, R"pbdoc( Returns (N*3, ) array for N vertices. )pbdoc")
    .def_property("MVertsDamgdNorms_numpy", &Mesh::MGetDamgdVertsNorms_numpy, &Mesh::MSetDamgdVertsNorms_numpy, R"pbdoc( Returns (N*3, ) numpy array for N vertices. )pbdoc")
    .def_property("MVertsAnimation", &Mesh::MGetVertsAnimation, &Mesh::MSetVertsAnimation, R"pbdoc( Returns (N, ) array for N vertices. )pbdoc")
    .def_property("MVertsAnimation_numpy", &Mesh::MGetVertsAnimation_numpy, &Mesh::MSetVertsAnimation_numpy, R"pbdoc( Returns (N, ) numpy array for N vertices. )pbdoc")

    .def("OpAddHelperPart", &Mesh::OpAddHelperPart, py::arg("name"), py::arg("new_center") = std::array<float, 3>({0.0f, 0.0f, 0.0f}), R"pbdoc( Add diamond-shaped part at coordinate origin or at optionally given position. )pbdoc")
    .def("OpAddHelperPart_numpy", &Mesh::OpAddHelperPart_numpy, py::arg("name"), py::arg("new_center") = std::array<float, 3>({0.0f, 0.0f, 0.0f}), R"pbdoc( Add diamond-shaped part at coordinate origin or at optionally given position. )pbdoc")
    .def("OpCenterPart", &Mesh::OpCenterPart, py::arg("pid"), R"pbdoc( Center specified part to local centroid. Does not move part w.r.t. to global coordinates. )pbdoc")
    .def("OpSetPartCenter", &Mesh::OpSetPartCenter, py::arg("pid"), py::arg("new_center"), R"pbdoc( Center specified part to given position. Does not move part w.r.t. to global coordinates. )pbdoc")
    .def("OpSetPartCenter_numpy", &Mesh::OpSetPartCenter_numpy, py::arg("pid"), py::arg("new_center"), R"pbdoc( Center specified part to given position. Does not move part w.r.t. to global coordinates. )pbdoc")
    .def("OpCopyPart", &Mesh::OpCopyPart, py::arg("pid_src"), R"pbdoc( Copy specified part. Returns new part index. )pbdoc")
    .def("OpInsertPart", &Mesh::OpInsertPart, py::arg("mesh_src"), py::arg("pid_src"), R"pbdoc( Insert (copy) specified part from mesh_src. Returns new part index. )pbdoc")
    .def("OpDeletePart", &Mesh::OpDeletePart, py::arg("pid"))
    .def("OpDeletePartTriags", &Mesh::OpDeletePartTriags, py::arg("pid"), py::arg("idxs"))
    .def("OpDelUnrefdVerts", &Mesh::OpDelUnrefdVerts, R"pbdoc( Delete all vertices that are not referenced by any triangle. )pbdoc")
    .def("OpMergeParts", &Mesh::OpMergeParts, py::arg("pid1"), py::arg("pid2"), R"pbdoc( Returns new part index. )pbdoc")
    .def("OpMovePart", &Mesh::OpMovePart, py::arg("pid"), R"pbdoc( Move up specified part towards order 0. Returns new part index. )pbdoc")
    ;
}