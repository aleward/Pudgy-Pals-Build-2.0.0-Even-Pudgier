#pragma once

#include "DirectXRaytracingHelper.h"
#include "RayTracingHlslCompat.h"

class D3D12RaytracingSimpleLighting;

namespace ModelLoading {

// Holds the buffer that contains the texture data
struct Texture {
  int id;
  std::string name{};
  UINT sampler_offset = 0;

  bool was_loaded_from_gltf = false;

  D3DBuffer texBuffer;
  ID3D12Resource *textureBufferUploadHeap;
  D3D12_RESOURCE_DESC textureDesc;
};

// Holds a pointer to each type of texture
/*struct TextureBundle {
  Texture *albedoTex;
  Texture *normalTex;
};*/

/*struct MaterialResource
{
  Material material;
  int id;
  std::string name{};

  bool was_loaded_from_gltf = false;

  D3DBuffer d3d12_material_resource;
};*/

struct InfoResource
{
  ModelInfo info;
  D3DBuffer d3d12_resource;
};

// Holds the vertex and index buffer (triangulated) for a loaded model
struct Model {
  int id;
  std::string name{};

  D3DBuffer indices;
  D3DBuffer vertices;

  //ImGUI stuff
  std::vector<Vertex> vertices_vec;
  std::vector<Index> indices_vec;
  //line vertex buffer is on
  int vertex_line = 0;
  int indices_line = 0;

  D3D12_RAYTRACING_GEOMETRY_DESC& GetGeomDesc();
  D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC &GetBottomLevelBuildDesc();

  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO &
  GetPreBuild(bool is_fallback,
              ComPtr<ID3D12RaytracingFallbackDevice> m_fallbackDevice,
              ComPtr<ID3D12Device5> m_dxrDevice);

  ComPtr<ID3D12Resource> GetBottomLevelScratchAS(
      bool is_fallback, ComPtr<ID3D12Device> device,
      ComPtr<ID3D12RaytracingFallbackDevice> m_fallbackDevice,
      ComPtr<ID3D12Device5> m_dxrDevice);

  ComPtr<ID3D12Resource>
  GetBottomAS(bool is_fallback, ComPtr<ID3D12Device> device,
              ComPtr<ID3D12RaytracingFallbackDevice> m_fallbackDevice,
              ComPtr<ID3D12Device5> m_dxrDevice);

  WRAPPED_GPU_POINTER
  GetFallBackWrappedPoint(
      D3D12RaytracingSimpleLighting *programState, bool is_fallback,
      ComPtr<ID3D12RaytracingFallbackDevice> m_fallbackDevice,
      ComPtr<ID3D12Device5> m_dxrDevice, UINT numBufferElements);

  void FinalizeAS();

  bool is_gpu_ptr_allocated = false;
  WRAPPED_GPU_POINTER gpuPtr{};
  D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc;
  bool is_m_bottomLevelAccelerationStructure_allocated = false;
  ComPtr<ID3D12Resource> m_bottomLevelAccelerationStructure;
  bool is_scratchResource_allocated = false;
  ComPtr<ID3D12Resource> scratchResource;

  bool bottom_level_build_desc_allocated = false;
  D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottom_level_build_desc{};
  bool bottom_level_prebuild_info_allocated = false;
  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottom_level_prebuild_info;

  int verticesCount = 0;
  int indicesCount = 0;
};

// Holds pointers to a model, textures, and a material
class SceneObject {
public:
  FLOAT *getTransform3x4();

  int id;
  std::string name{};

  Model *model = nullptr;
  /*TextureBundle textures{};
  MaterialResource *material = nullptr;*/
  InfoResource info_resource{};

  XMFLOAT3 translation; // parsed transform values
  XMFLOAT3 rotation;
  XMFLOAT3 scale;

  bool transformBuilt = false;

private:
  FLOAT transform[3][4]; // instance desc transform
};

struct Camera {
  float fov;

  XMVECTOR eye;
  XMVECTOR lookat;
  XMVECTOR up;
  int maxDepth;
};
} // namespace ModelLoading