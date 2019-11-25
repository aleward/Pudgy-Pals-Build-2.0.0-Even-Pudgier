#ifndef MARCHCUBES_HLSL
#define MARCHCUBES_HLSL

// LOOKAT-1.9.5: Raytracing.hlsl
// This is where most of the important GPU code lives.
// The root signatures you will define will be used here.
// The hitgroups you will define will also be used here.

#define HLSL
#include "RaytracingHlslCompat.h"
#include "ProceduralPrimitivesLibrary.hlsli"
#include "RaytracingShaderHelper.hlsli"
#include "SDFShaderHelper.hlsli"

//***************************************************************************
//*****------ Shader resources bound via root signatures -------*************
//***************************************************************************

// Scene wide resources.
//  g_* - bound via a global root signature.
//  l_* - bound via a local root signature.
/*RaytracingAccelerationStructure g_scene : register(t0, space0); // accel structure
RWTexture2D<float4> g_renderTarget : register(u0); // output texture
ConstantBuffer<SceneConstantBuffer> g_sceneCB : register(b0); // camera projections, lights, time elapsed

// Triangle resources
ByteAddressBuffer g_indices : register(t1, space0); // triangle indices
StructuredBuffer<Vertex> g_vertices : register(t2, space0); // triangle positions*/


//***************************************************************************
//********************------ Compute shader -------**************************
//***************************************************************************

// TODO-3.1: Complete the ray generation shader.
// (1) Generate a ray using the function GenerateCameraRay() in RaytracingShaderHelper.hlsli
// (2) Trace a radiance ray using the generated ray to obtain a color
// (3) Write that color to the render target
[shader("compute")]
void MyComputeShader()
{
    /*Ray camRay = GenerateCameraRay(DispatchRaysIndex().xy, g_sceneCB.cameraPosition.xyz, g_sceneCB.projectionToWorld);
    float4 col = TraceRadianceRay(camRay, 0);

	// Write the color to the render target
    g_renderTarget[DispatchRaysIndex().xy] = col;*/
}

#endif // MARCHCUBES_HLSL