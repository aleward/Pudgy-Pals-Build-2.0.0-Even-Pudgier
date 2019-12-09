#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

// LOOKAT-1.9.5: Raytracing.hlsl
// This is where most of the important GPU code lives.
// The root signatures you will define will be used here.
// The hitgroups you will define will also be used here.

#define HLSL
#include "RaytracingHlslCompat.h"
#include "ProceduralPrimitivesLibrary.hlsli"
#include "RaytracingShaderHelper.hlsli"

//***************************************************************************
//*****------ Shader resources bound via root signatures -------*************
//***************************************************************************

// Scene wide resources.
//  g_* - bound via a global root signature.
//  l_* - bound via a local root signature.
RaytracingAccelerationStructure g_scene : register(t0, space0); // accel structure
RWTexture2D<float4> g_renderTarget : register(u0); // output texture
ConstantBuffer<SceneConstantBuffer> g_sceneCB : register(b0); // camera projections, lights, time elapsed

// Triangle resources
ByteAddressBuffer g_indices : register(t1, space0); // triangle indices
StructuredBuffer<Vertex> g_vertices : register(t2, space0); // triangle positions

// Procedural geometry resources
StructuredBuffer<PrimitiveInstancePerFrameBuffer> g_AABBPrimitiveAttributes : register(t3, space0); // transforms per procedural
ConstantBuffer<PrimitiveConstantBuffer> l_materialCB : register(b1); // material data per procedural
ConstantBuffer<PrimitiveInstanceConstantBuffer> l_aabbCB: register(b2); // other meta-data: type, instance indices

StructuredBuffer<HeadSpineInfoBuffer> g_headSpineBuffer: register(t4, space0);
StructuredBuffer<AppendageInfoBuffer> g_appenBuffer: register(t5, space0);
StructuredBuffer<LimbInfoBuffer> g_limbBuffer: register(t6, space0);
StructuredBuffer<RotationInfoBuffer> g_rotBuffer: register(t7, space0);

Texture2D g_texture: register(t0, space1);

// Global variables
static float headData[HEAD_COUNT];
static float spineLocData[SPINE_LOC_COUNT];
static float spineRadData[SPINE_RAD_COUNT];

static float numAppendages;
static float appenBools[APPEN_COUNT];
static float appenRads[APPEN_COUNT];

static float limbLengths[LIMBLEN_COUNT];
static float jointLocData[JOINT_LOC_COUNT];
static float jointRadData[JOINT_RAD_COUNT];

static float rotations[ROT_COUNT];

//***************************************************************************
//*********************------ Utilities. -------*****************************
//***************************************************************************

// TODO-3.5: Diffuse lighting calculation. This is just a Lambert shading term.
// HINT:	See https://en.wikipedia.org/wiki/Lambertian_reflectance
// Remember to clamp the dot product term!
float CalculateDiffuseCoefficient(in float3 incidentLightRay, in float3 normal)
{
	return abs(dot(normalize(incidentLightRay), normalize(normal)));
}
            
const float3 a = float3(0.4, 0.5, 0.8);
const float3 b = float3(0.2, 0.4, 0.2);
const float3 c = float3(1.0, 1.0, 2.0);
const float3 d = float3(0.25, 0.25, 0.0);

const float3 e = float3(0.2, 0.5, 0.8);
const float3 f = float3(0.2, 0.25, 0.5);
const float3 g = float3(1.0, 1.0, 0.1);
const float3 h = float3(0.0, 0.8, 0.2);
            
float fract(in float x)
{
    return x - floor(x);
}
            
float2 fract(in float2 x)
{
    return x - floor(x);
}

// Return a random direction in a circle
float2 random2(in float2 p)
{
    return normalize(2 * fract(sin(float2(dot(p, float2(127.1, 311.7)), dot(p, float2(269.5, 183.3)))) * 43758.5453) - 1);
}

float3 Gradient(in float t)
{
    return a + b * cos(6.2831 * (c * t + d));
}

float3 Gradient2(in float t)
{
    return e + f * cos(6.2831 * (g * t + h));
}

float surflet(in float2 P, in float2 gridPoint)
{
// Compute falloff function by converting linear distance to a polynomial
    float distX = abs(P.x - gridPoint.x);
    float distY = abs(P.y - gridPoint.y);
    float tX = 1 - 6 * pow(distX, 5.0) + 15 * pow(distX, 4.0) - 10 * pow(distX, 3.0);
    float tY = 1 - 6 * pow(distY, 5.0) + 15 * pow(distY, 4.0) - 10 * pow(distY, 3.0);

// Get the random vector for the grid point
    float2 gradient = random2(gridPoint);
// Get the vector from the grid point to P
    float2 diff = P - gridPoint;
// Get the value of our height field by dotting grid->P with our gradient
    float height = dot(diff, gradient);
// Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * tX * tY;
}

float PerlinNoise(in float2 uv)
{
// Tile the space
    float2 uvXLYL = floor(uv);
    float2 uvXHYL = uvXLYL + float2(1, 0);
    float2 uvXHYH = uvXLYL + float2(1, 1);
    float2 uvXLYH = uvXLYL + float2(0, 1);

    return surflet(uv, uvXLYL) + surflet(uv, uvXHYL) + surflet(uv, uvXHYH) + surflet(uv, uvXLYH);
}

float2 PixelToGrid(in float2 pixel, in float size)
{
    float2 uv = pixel.xy / 1.0f;
// Account for aspect ratio
    uv.x = uv.x * float(1.0f) / float(1.0f);
// Determine number of cells (NxN)
    uv *= size;

    return uv;
}
            
float3 NoiseGenBasic(in float2 loc)
{
	// Basic Perlin noise
	float2 uv = PixelToGrid(loc, 4.0);
	float perlin = PerlinNoise(uv);
    float3 color = float3((perlin + 1) * 0.5, (perlin + 1) * 0.5, (perlin + 1) * 0.5);
	color.r += step(0.98, fract(uv.x)) + step(0.98, fract(uv.y));
    return color;
}
            
float3 NoiseGenSummed(in float2 loc)
{
    float summedNoise = 0.0;
    float amplitude = 0.5;
    for (int i = 2; i <= 32; i *= 2)
    {
        float2 uv = PixelToGrid(loc, float(i));
        uv = float2(cos(3.14159 / 3.0 * i) * uv.x - sin(3.14159 / 3.0 * i) * uv.y, sin(3.14159 / 3.0 * i) * uv.x + cos(3.14159 / 3.0 * i) * uv.y);
        float perlin = abs(PerlinNoise(uv));
        summedNoise += perlin * amplitude;
        amplitude *= 0.5;
    }
    return float3(summedNoise, summedNoise, summedNoise);
}

float3 NoiseGenAbsolute(in float2 loc)
{
    float2 uv = PixelToGrid(loc, 10.0);
    float perlin = PerlinNoise(uv);
    return float3(1.0, 1.0, 1.0) - float3(abs(perlin), abs(perlin), abs(perlin));
}
           
float3 NoiseGenRecursive1(in float2 loc)
{
    float2 planet = float2(cos(1.0f * 0.01 * 3.14159), sin(1.0f * 0.01 * 3.14159)) * 2 + float2(4.0, 4.0);
    float2 uv = PixelToGrid(loc, 10.0);
    float2 planetDiff = planet - uv;
    float len = length(planetDiff);
    float2 offset = float2(PerlinNoise(uv + 1.0f * 0.01), PerlinNoise(uv + float2(5.2, 1.3)));
    if (len < 1.0)
    {
        offset += planetDiff * (1.0 - len);
    }
    float perlin = PerlinNoise(uv + offset);
    return float3((perlin + 1) * 0.5, (perlin + 1) * 0.5, (perlin + 1) * 0.5);
}
            
float3 NoiseGenRecursive2(in float2 loc)
{
    // Recursive Perlin noise (2 levels)
    float2 uv = PixelToGrid(loc, 10.0);
    float2 offset1 = float2(PerlinNoise(uv + cos(1.0f * 3.14159 * 0.01)), PerlinNoise(uv + float2(5.2, 1.3)));
    float2 offset2 = float2(PerlinNoise(uv + offset1 + float2(1.7, 9.2)), PerlinNoise(uv + sin(1.0f * 3.14159 * 0.01) + offset1 + float2(8.3, 2.8)));
    float perlin = PerlinNoise(uv + offset2);
    float3 baseGradient = Gradient(perlin);
    baseGradient = baseGradient * length(offset1) + float3(perlin, perlin, perlin) * (1.0f - length(offset1));
    return baseGradient;
}           

// TODO-3.5: Phong lighting specular component.
// The equation should be coefficient = (reflectedRay . reverseRayDirection) ^ (specularPower).
// HINT:	Consider using built-in DirectX functions to find the reflected ray. Remember that a reflected ray is reflected
//			with respect to the normal of the hit position.
// Remember to normalize the reflected ray, and to clamp the dot product term 
float4 CalculateSpecularCoefficient(in float3 incidentLightRay, in float3 normal, in float specularPower)
{
    float3 reflectedRay = normalize(reflect(normalize(incidentLightRay), normalize(normal)));
    return pow(abs(dot(normalize(WorldRayDirection()), reflectedRay)), specularPower);
}

// TODO-3.5: Phong lighting model = ambient + diffuse + specular components.
// See https://en.wikipedia.org/wiki/Phong_reflection_model for the full simple equation.
// We have filled in the ambient color for you.
// HINT 1: remember that you can get the world position of the hitpoint using HitWorldPosition() 
//		   from RaytracingShaderHelper.hlsli. The light position is somewhere in g_sceneCB.
// HINT 1: first you need to figure out the diffuse coefficient using CalculateDiffuseCoefficient()
//		   then you need to figure out the specular coefficient using CalculateSpecularCoefficient()
//		   then you need to combine the diffuse, specular, and ambient colors into one color.
// Remember that if the ray is a shadow ray, then the hit position should be very dim. Consider using
// InShadowRadiance from RaytracingHlslCompat.h tp dim down the diffuse color. The specular color should 
// be completely black if the ray is a shadow ray.
float4 CalculatePhongLighting(in float4 albedo, in float3 normal, in bool isInShadow,
	in float diffuseCoef = 1.0, in float specularCoef = 1.0, in float specularPower = 50)
{                
	// Ambient component
	// Fake AO: Darken faces with normal facing downwards/away from the sky a little bit
	float4 ambientColor = g_sceneCB.lightAmbientColor;
	float4 ambientColorMin = g_sceneCB.lightAmbientColor - 0.1;
	float4 ambientColorMax = g_sceneCB.lightAmbientColor;
	float a = 1 - saturate(dot(normal, float3(0, -1, 0)));
	ambientColor = albedo * lerp(ambientColorMin, ambientColorMax, a);

	float3 lightRay = normalize(g_sceneCB.lightPosition.xyz - HitWorldPosition());
    // Diffuse component
	float4 diffColor = g_sceneCB.lightDiffuseColor;
    float4 diffuseColor = albedo * diffColor * diffuseCoef * CalculateDiffuseCoefficient(lightRay, normal);

    // Specular component
    float4 specularColor = float4(1.0f, 1.0f, 1.0f, 1.0f) * specularCoef * CalculateSpecularCoefficient(lightRay, normal, specularPower);

    // Changes if in shadow
    if (isInShadow)
    {
        diffuseColor *= InShadowRadiance;
        specularColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    }

	return ambientColor + diffuseColor + specularColor;
}

//***************************************************************************
//*****------ TraceRay wrappers for radiance and shadow rays. -------********
//***************************************************************************

// LOOKAT-1.9.5: Trace a radiance ray into the scene and returns a shaded color.
// You should read about the TraceRay() function to understand what this does.
float4 TraceRadianceRay(in Ray ray, in UINT currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= MAX_RAY_RECURSION_DEPTH)
    {
        return float4(0, 0, 0, 0);
    }

    // Set the ray's extents.
    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
    rayDesc.Direction = ray.direction;
    // Set TMin to a zero value to avoid aliasing artifacts along contact areas.
    // Note: make sure to enable face culling so as to avoid surface face fighting.
    rayDesc.TMin = 0;
    rayDesc.TMax = 10000;

    RayPayload rayPayload = { float4(0, 0, 0, 0), currentRayRecursionDepth + 1 };

	// TraceRay() is a built-in DXR function. Lookup its documentation to see what it does.
	// To understand how a ray "finds out" what type of object it hit and therefore call the correct shader hitgroup, it indexes into the shader
	// table as follows:
	// hitgroup to choose = ptr to shader table + size of a shader record + (ray contribution + (geometry index * geometry stride) + instance contribution)
	// * `ray contribution` and `geometry stride` are given here.
	// * `ptr to shader table` + `size of a shader record` are given in BuildShaderTables() in DXR-ShaderTable.cpp
	// * `geometry index` is implicitly given when you build your bottom-level AS in BuildGeometryDescsForBottomLevelAS() in DXR-AccelerationStructure.cpp
	// * `instance contribution` is given in BuildBottomLevelASInstanceDescs() in DXR-AccelerationStructure.cpp
	// Essentially, when a ray hits a Geometry in a Bottom-Level AS Instance contained within a Top Level AS, it does the following:
	// (1) Identify which Instance of a BLAS it hit (Triangle or AABB) --> this gives it the `instance contribution`
	// (2) Identify which Geometry *inside* this Instance it hit
	//			(If hit Triangle instance, then Triangle geom!, if hit AABB instance, then might be Sphere, Metaball, Fractal, etc..)
	//			--> this gives it the `geometry index`
	// (3) Identify what type of Ray it is --> this is given right here, say a Radiance ray
	// (4) Combines all of these inputs to index into the correct shader into the hitgroup shader table.
    TraceRay(g_scene,
        RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
        TraceRayParameters::InstanceMask,
        TraceRayParameters::HitGroup::Offset[RayType::Radiance],
        TraceRayParameters::HitGroup::GeometryStride,
        TraceRayParameters::MissShader::Offset[RayType::Radiance],
        rayDesc, rayPayload);

    return rayPayload.color;
}

// TODO-3.2: Trace a shadow ray and return true if it hits any geometry. Very similar to TraceRay() above
// Hint 1: for the TraceRay() flags, make sure you cull back facing triangles, skip any hit shaders, skip closest hit shaders, 
//		   and just accept any geometry you hit
// Hint 2: remember what the ShadowRay payload looks like. See RaytracingHlslCompat.h
bool TraceShadowRayAndReportIfHit(in Ray ray, in UINT currentRayRecursionDepth)
{
	if (currentRayRecursionDepth >= MAX_RAY_RECURSION_DEPTH)
    {
        return false;
    }

    // Set the ray's extents.
    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
    rayDesc.Direction = ray.direction;
    // Set TMin to a zero value to avoid aliasing artifacts along contact areas.
    // Note: make sure to enable face culling so as to avoid surface face fighting.
    rayDesc.TMin = 0;
    rayDesc.TMax = 10000;

    ShadowRayPayload rayPayload = { true };

    TraceRay(g_scene,
        RAY_FLAG_CULL_BACK_FACING_TRIANGLES | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER | RAY_FLAG_FORCE_OPAQUE,
        TraceRayParameters::InstanceMask,
        TraceRayParameters::HitGroup::Offset[RayType::Shadow],
        TraceRayParameters::HitGroup::GeometryStride,
        TraceRayParameters::MissShader::Offset[RayType::Shadow],
        rayDesc, rayPayload);

    return rayPayload.hit;
}

bool TraceAORayAndReportIfHit(in Ray ray, in float maxDist)
{
	// Set the ray's extents.
	RayDesc rayDesc;
	rayDesc.Origin = ray.origin;
	rayDesc.Direction = ray.direction;
	// Set TMin to a zero value to avoid aliasing artifacts along contact areas.
	// Note: make sure to enable face culling so as to avoid surface face fighting.
	rayDesc.TMin = 0;
	rayDesc.TMax = maxDist;

	ShadowRayPayload rayPayload = { true };

	TraceRay(g_scene,
		RAY_FLAG_CULL_BACK_FACING_TRIANGLES | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER | RAY_FLAG_FORCE_OPAQUE,
		TraceRayParameters::InstanceMask,
		TraceRayParameters::HitGroup::Offset[RayType::Shadow],
		TraceRayParameters::HitGroup::GeometryStride,
		TraceRayParameters::MissShader::Offset[RayType::Shadow],
		rayDesc, rayPayload);

	return rayPayload.hit;
}

//***************************************************************************
//********************------ Ray gen shader -------**************************
//***************************************************************************

// TODO-3.1: Complete the ray generation shader.
// (1) Generate a ray using the function GenerateCameraRay() in RaytracingShaderHelper.hlsli
// (2) Trace a radiance ray using the generated ray to obtain a color
// (3) Write that color to the render target
[shader("raygeneration")]
void MyRaygenShader()
{
    Ray camRay = GenerateCameraRay(DispatchRaysIndex().xy, g_sceneCB.cameraPosition.xyz, g_sceneCB.projectionToWorld);
    float4 col = TraceRadianceRay(camRay, 0);

	// Write the color to the render target
    g_renderTarget[DispatchRaysIndex().xy] = col;
}

//***************************************************************************
//******************------ Closest hit shaders -------***********************
//***************************************************************************

// LOOKAT-1.9.5: ClosestHitShader for a triangle.
// This is invoked after:
// (1) a call to TraceRay() and
// (2) the ray hits the closest possible Triangle geometry to it
[shader("closesthit")]
void MyClosestHitShader_Triangle(inout RayPayload rayPayload, in BuiltInTriangleIntersectionAttributes attr)
{
    // Get the base index of the triangle's first 16 bit index.
    uint indexSizeInBytes = 2;
    uint indicesPerTriangle = 3;
    uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes;

	// PrimitiveIndex() is the triangle index within the mesh. For Procedural primitives, this is the index into the AABB array defining the geometry.
    uint baseIndex = PrimitiveIndex() * triangleIndexStride;

    // Load up three 16 bit indices for the triangle.
    const uint3 indices = Load3x16BitIndices(baseIndex, g_indices);

    // Retrieve corresponding vertex normals for the triangle vertices.
    float3 triangleNormal = g_vertices[indices[0]].normal;

	// This is the intersection point on the triangle.
	float3 hitPosition = HitWorldPosition();

    // Trace a ray from the hit position towards the single light source we have. If on our way to the light we hit something, then we have a shadow!
    Ray shadowRay = { hitPosition, normalize(g_sceneCB.lightPosition.xyz - hitPosition) };
    bool shadowRayHit = TraceShadowRayAndReportIfHit(shadowRay, rayPayload.recursionDepth);

    // Reflected component ray.
    float4 reflectedColor = float4(0, 0, 0, 0);
    if (l_materialCB.reflectanceCoef > 0.001 )
    {
        // Trace a reflection ray from the intersection points using Snell's law. The reflect() HLSL built-in function does this for you!
		// See https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-intrinsic-functions
        Ray reflectionRay = { hitPosition, reflect(WorldRayDirection(), triangleNormal) };
        float4 reflectionColor = TraceRadianceRay(reflectionRay, rayPayload.recursionDepth);

        float3 fresnelR = FresnelReflectanceSchlick(WorldRayDirection(), triangleNormal, l_materialCB.albedo0.xyz);
        reflectedColor = l_materialCB.reflectanceCoef * float4(fresnelR, 1) * reflectionColor;
    }

    // Calculate final color.
    float4 phongColor = CalculatePhongLighting(l_materialCB.albedo0, triangleNormal, shadowRayHit, l_materialCB.diffuseCoef, l_materialCB.specularCoef, l_materialCB.specularPower);
    float4 color = (phongColor + reflectedColor);

	// TODO: Apply a visibility falloff.
	// If the ray is very very very far away, tends to sample the background color rather than the color you computed.
	// This is to mimic some form of distance fog where farther objects appear to blend with the background.
	// Hint 1: look at the intrinsic function RayTCurrent() that returns how "far away" your ray is.
	// Hint 2: use the built-in function lerp() to linearly interpolate between the computed color and the Background color.
	//		   When t is big, we want the background color to be more pronounced.
    float t = 1.0f - min(RayTCurrent()/500.0f, 1.0f);
    float4 falloffColor = lerp(BackgroundColor, color, t);

	rayPayload.color = falloffColor;
}

// From https://gamedev.stackexchange.com/questions/32681/random-number-hlsl
float2 rand_2_10(in float inX, in float inY) {
	float2 uv = float2(inX, inY);
	float noiseX = (frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453));
	float noiseY = sqrt(1 - noiseX * noiseX);
	return float2(noiseX, noiseY);
}

float3 squareToHemisphereUniform(in float inX, in float inY)
{
	float z = inX;
	float x = cos(2 * 3.1415926 * inY) * sqrt(1 - z * z);
	float y = sin(2 * 3.1415926 * inY) * sqrt(1 - z * z);
	return float3(x, y, z);
}

float3 planar(in float2 loc, in float scale, in float3 pos, in int whichNoise) {
    loc = loc / scale;
    if (whichNoise == 0)
    {
        return NoiseGenBasic(loc);
    }
    else if (whichNoise == 1)
    {
        return NoiseGenAbsolute(loc);
    }
    else if (whichNoise == 2)
    {
        return NoiseGenSummed(loc);
    }
    else
    {
        return NoiseGenRecursive1(loc);
    }
}
            
float3 triplanar(in float3 top, in float3 front, in float3 side, in float3 normal) {
    return float4(lerp(lerp(side, top, abs(normal.y)), front, abs(normal.x)), 1);
}

// TODO: Write the closest hit shader for a procedural geometry.
// You suppose this is called after the ray successfully iterated over all geometries and determined it intersected with some AABB.
// The attributes of the AABB are in attr (basically the normal)
// You need to:
// (0) Realize that you do not need to load in indices or vertices because we're talking procedural geometry here (wohoo!)
// (1) Trace a shadow ray to determine if this ray is a shadow ray.
// (2) Trace a reflectance ray --> compute the reflected color.
// (3) Use the fact that your ray is a shadow ray or not to compute the Phong lighting.
// (4) Combine the reflect color and the phong color into one color.
// (5) Apply visibility falloff to select some interpolation between the computed color or the background color
// (6) Fill the payload color with whatever final color you computed
[shader("closesthit")]
void MyClosestHitShader_AABB(inout RayPayload rayPayload, in ProceduralPrimitiveAttributes attr)
{
	
    // This is the intersection point on the triangle.
    float3 hitPosition = HitWorldPosition();
    float3 normal = attr.normal;
    
    // Trace a shadow ray to determine if this ray is a shadow ray
    Ray shadowRay = { hitPosition, normalize(g_sceneCB.lightPosition.xyz - hitPosition) };
    bool shadowRayHit = TraceShadowRayAndReportIfHit(shadowRay, rayPayload.recursionDepth);

    float3 top = lerp(l_materialCB.albedo0.xyz, l_materialCB.albedo1.xyz, planar(hitPosition.xz, 1.0f, hitPosition, l_materialCB.whichNoise0));
    float3 side = lerp(l_materialCB.albedo2.xyz, l_materialCB.albedo3.xyz, planar(hitPosition.xy, 1.0f, hitPosition, l_materialCB.whichNoise1));
    float3 front = lerp(l_materialCB.albedo2.xyz, l_materialCB.albedo3.xyz, planar(hitPosition.yz, 1.0f, hitPosition, l_materialCB.whichNoise1));
    float4 albedo = float4(triplanar(top, front, side, normal), 1);
	
	// Reflected component ray.
    float4 reflectedColor = float4(0, 0, 0, 0);
    if (l_materialCB.reflectanceCoef > 0.001)
    {
        Ray reflectionRay = { hitPosition, reflect(WorldRayDirection(), attr.normal) };
        float4 reflectionColor = TraceRadianceRay(reflectionRay, rayPayload.recursionDepth);

        float3 fresnelR = FresnelReflectanceSchlick(WorldRayDirection(), attr.normal, albedo.xyz);
        reflectedColor = l_materialCB.reflectanceCoef * float4(fresnelR, 1) * reflectionColor;
    }

    float3 topRough = planar(hitPosition.xz, 1.0f, hitPosition, l_materialCB.whichNoise0);
    float3 sideRough = planar(hitPosition.xy, 1.0f, hitPosition, l_materialCB.whichNoise0);
    float3 frontRough = planar(hitPosition.yz, 1.0f, hitPosition, l_materialCB.whichNoise0);
    // Calculate final color with phong lighting
                float4 phongColor = CalculatePhongLighting(albedo, attr.normal, shadowRayHit, l_materialCB.diffuseCoef, l_materialCB.specularCoef, lerp(l_materialCB.specularPower / 4.0f, l_materialCB.specularPower, triplanar(topRough, frontRough, sideRough, normal).r));
    float4 color = (phongColor + reflectedColor);

    float t = 1.0f - min(RayTCurrent() / 500.0f, 1.0f);
    float4 falloffColor = lerp(BackgroundColor, color, t);

	rayPayload.color = falloffColor;
	

	/*float4 aoColor = float4(1, 1, 1, 1);
	int aoSamples = 20;
	int aoHits = 0;
	float3 n = attr.normal;
	float3 t2 = normalize(cross(n, n + float3(0, 0, 1)));
	float3 b = normalize(cross(n, t2));
	float3x3 worldToTangent = float3x3(b, t2, n);
	for (int i = 0; i < aoSamples; i++) {
		float2 s = rand_2_10(hitPosition.x + i, hitPosition.x + i + 10);
		float3 hemPoint = squareToHemisphereUniform(s.x, s.y);
		hemPoint = mul(hemPoint, worldToTangent);
		hemPoint += hitPosition;
		Ray aoRay = { hitPosition + n * 0.02, normalize(hemPoint - hitPosition) };
		bool aoRayHit = TraceAORayAndReportIfHit(aoRay, 2.f);
		if (aoRayHit) aoHits++;
	}
	aoColor *= 1.0f - (float(aoHits) / float(aoSamples));

	rayPayload.color *= aoColor;*/
}

//***************************************************************************
//**********************------ Miss shaders -------**************************
//***************************************************************************

// TODO-3.3: Complete the Radiance ray miss shader. What color should you output if you hit no geometry?
// Make sure you edit the rayPayload so your color gets passed down to other shaders.
// NOTE: whether we missed a Triangle or a Procedural Geometry does not matter. The miss output should be the same!
[shader("miss")]
void MyMissShader(inout RayPayload rayPayload)
{
    rayPayload.color = BackgroundColor;
}

// TODO-3.3: Complete the Shadow ray miss shader. Is this ray a shadow ray if it hit nothing?
[shader("miss")]
void MyMissShader_ShadowRay(inout ShadowRayPayload rayPayload)
{
    rayPayload.hit = false;
}

//***************************************************************************
//*****************------ Intersection shaders-------************************
//***************************************************************************

// Get ray in AABB's local space.
Ray GetRayInAABBPrimitiveLocalSpace()
{
    PrimitiveInstancePerFrameBuffer attr = g_AABBPrimitiveAttributes[l_aabbCB.instanceIndex];

    // Retrieve a ray origin position and direction in bottom level AS space 
    // and transform them into the AABB primitive's local space.
    Ray ray;
    ray.origin = mul(float4(ObjectRayOrigin(), 1), attr.bottomLevelASToLocalSpace).xyz;
    ray.direction = mul(ObjectRayDirection(), (float3x3) attr.bottomLevelASToLocalSpace);
    return ray;
}

// LOOKAT-3.4.1: Analytic Primitive intersection shader.
// Analytic primitives are spheres and boxes in this project.
[shader("intersection")]
void MyIntersectionShader_AnalyticPrimitive()
{
    Ray localRay = GetRayInAABBPrimitiveLocalSpace();
    AnalyticPrimitive::Enum primitiveType = (AnalyticPrimitive::Enum) l_aabbCB.primitiveType;

	// The point of the intersection shader is to:
	// (1) find out what is the t at which the ray hits the procedural
	// (2) pass on some attributes used by the closest hit shader to do some shading (e.g: normal vector)
    float thit;
    ProceduralPrimitiveAttributes attr;
    if (RayAnalyticGeometryIntersectionTest(localRay, primitiveType, thit, attr))
    {
        PrimitiveInstancePerFrameBuffer aabbAttribute = g_AABBPrimitiveAttributes[l_aabbCB.instanceIndex];

		// Make sure the normals are stored in BLAS space and not the local space
        attr.normal = mul(attr.normal, (float3x3) aabbAttribute.localSpaceToBottomLevelAS);
        attr.normal = normalize(mul((float3x3) ObjectToWorld3x4(), attr.normal));

		// thit is invariant to the space transformation
        ReportHit(thit, /*hitKind*/ 0, attr);
    }
}

float3 rotateX(float3 p, float angle) {
	float rad = radians(angle);
	float co = cos(rad);
	float si = sin(rad);
	float2x2 mat = float2x2(co, si, -si, co);
	p.yz = mul(p.yz, mat);
	return p;
}

float3 rotateY(float3 p, float angle) {
	float rad = radians(angle);
	float co = cos(rad);
	float si = sin(rad);
	float2x2 mat = float2x2(co, -si, si, co);
	p.xz = mul(p.xz, mat);
	return p;
}

float3 rotateZ(float3 p, float angle) {
	float rad = radians(angle);
	float co = cos(rad);
	float si = sin(rad);
	float2x2 mat = float2x2(co, si, -si, co);
	p.xy = mul(p.xy, mat);
	return p;
}

float3 rotateInverseAxisAngle(float angle, float x, float y, float z, float3 p)
{
    float c, s;
    sincos(angle, s, c);

    float t = 1 - c;

    float3x3 m = float3x3(
        t * x * x + c,      t * x * y - s * z,  t * x * z + s * y,
        t * x * y + s * z,  t * y * y + c,      t * y * z - s * x,
        t * x * z - s * y,  t * y * z + s * x,  t * z * z + c);

    return mul(transpose(m), p);
}

float3x3 scaleMat(float amt) {
    return float3x3(
        float3(amt, 0.0, 0.0),
        float3(0.0, amt, 0.0),
        float3(0.0, 0.0, amt)
    );
}

// polynomial smooth min
float smin(float a, float b, float k) {
    float h = clamp(0.5 + 0.5*(b - a) / k, 0.0, 1.0);
    return lerp(b, a, h) - k * h*(1.0 - h);
}

float sminExp(float a, float b, float k) {
    float res = exp(-k * a) + exp(-k * b);
    return -log(res) / k;
}

float sminPow(float a, float b, float k) {
    a = pow(a, k); b = pow(b, k);
    return pow((a*b) / (a + b), 1.0 / k);
}

float sphereSDF(float3 p, float r) {
    return length(p) - r;
}

float cubeSDF(float3 p, float r) {
    float3 d = abs(p) - float3(r, r, r);
    float insideDistance = min(max(d.x, max(d.y, d.z)), 0.0);
    float outsideDistance = length(max(d, 0.0));
    return insideDistance + outsideDistance;
}

float sdCappedCylinder(float3 p, float2 h) {
    float2 d = abs(float2(length(p.xz), p.y)) - h;
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}

float udBox(float3 p, float3 b) {
    return length(max(abs(p) - b, 0.0));
}

float sdCone(float3 p, float2 c) {
    // c must be normalized
    float q = length(p.xy);
    return dot(c, float2(q, p.z));
}

float sdCappedCone(in float3 p, in float3 c) {
    float2 q = float2(length(p.xz), p.y);
    float2 v = float2(c.z*c.y / c.x, -c.z);
    float2 w = v - q;
    float2 vv = float2(dot(v, v), v.x*v.x);
    float2 qv = float2(dot(v, w), v.x*w.x);
    float2 d = max(qv, 0.0)*qv / vv;
    return sqrt(dot(w, w) - max(d.x, d.y)) * sign(max(q.y*v.x - q.x*v.y, w.y));
}

float udRoundBox(float3 p, float3 b, float r) {
    return length(max(abs(p) - b, 0.0)) - r;
}

float sdCapsule(float3 p, float3 a, float3 b, float r) {
    float3 pa = p - a, ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * h) - r;
}

// a better capped cone function (like what)
float sdConeSection(in float3 p, in float h, in float r1, in float r2) {
    float d1 = -p.y - h;
    float q = p.y - h;
    float si = 0.5*(r1 - r2) / h;
    float d2 = max(sqrt(dot(p.xz, p.xz)*(1.0 - si * si)) + q * si - r2, q);
    return length(max(float2(d1, d2), 0.0)) + min(max(d1, d2), 0.);
}

//~~~~~HEAD SDFs~~~~~///
float bugHeadSDF(float3 p) {
	p = p + float3(headData[0], headData[1], headData[2]);
	p = rotateY(p, -90.0);
    float base = sphereSDF(p, headData[3]);
    float eyes = min(sphereSDF(p + headData[3] * float3(0.55, -0.35, -.71), headData[3] * .2), sphereSDF(p + headData[3] * float3(-0.55, -0.35, -.71), headData[3] * .2));
    float mandibleBase = sdCappedCylinder(p + headData[3] * float3(0.0, 0.001, -.9), headData[3] * float2(1.2, 0.1));
    float mandibles = max(mandibleBase, -sphereSDF(p + headData[3] * float3(0.0, 0.0, -0.60), .7 * headData[3]));
    mandibles = max(mandibles, -sphereSDF(p + headData[3] * float3(0.0, 0.0, -1.70), .7 * headData[3]));
    float head = smin(min(base, eyes), mandibles, .05);
    return head;
}

float dinoHeadSDF(float3 p) {
	p = p + float3(headData[0], headData[1], headData[2]);
	p = rotateY(p, -90.0);
    float base = sphereSDF(p, headData[3]);
    float topJaw = sphereSDF(p + headData[3] * float3(0.0, 0.3, -1.4), headData[3] * 1.08);
    topJaw = max(topJaw, -cubeSDF(p + headData[3] * float3(0.0, 1.4, -1.4), headData[3] * 1.2));
    float bottomJaw = sphereSDF(p + headData[3] * float3(0.0, 0.6, -1.0), headData[3] * .7);
    bottomJaw = max(bottomJaw, -cubeSDF(rotateX((p + headData[3] * float3(0.0, -.4, -1.7)), 45.0), headData[3] * 1.1));
    float combine = smin(base, topJaw, .04);
    combine = smin(combine, bottomJaw, .08);

    float eyes = min(sphereSDF(p + headData[3] * float3(.9, 0.0, 0.0), headData[3] * .3), sphereSDF(p + headData[3] * float3(-0.9, 0.0, 0.0), headData[3] * .3));
    combine = min(combine, eyes);
    float brows = min(udBox(rotateX((p + headData[3] * float3(.85, -0.35, 0.0)), -20.0), headData[3] * float3(.3, .2, .5)),
        udBox(rotateX((p + headData[3] * float3(-0.85, -0.35, 0.0)), -20.0), headData[3] * float3(.3, .2, .5)));
    combine = min(combine, brows);

    float teeth = sdCappedCone(rotateX((p + headData[3] * float3(0.4, 0.7, -1.8)), 180.0), headData[3] * float3(3.0, 1.0, 1.0));
    teeth = min(teeth, sdCappedCone(rotateX((p + headData[3] * float3(-0.4, 0.7, -1.8)), 180.0), headData[3] * float3(3.0, 1.0, 1.0)));
    teeth = min(teeth, sdCappedCone(rotateX((p + headData[3] * float3(-0.4, 0.7, -1.3)), 180.0), headData[3] * float3(2.7, 1.0, 1.0)));
    teeth = min(teeth, sdCappedCone(rotateX((p + headData[3] * float3(0.4, 0.7, -1.3)), 180.0), headData[3] * float3(2.7, 1.0, 1.0)));
    combine = min(combine, teeth);
    return combine;
}

float trollHeadSDF(float3 p) {
	p = p + float3(headData[0], headData[1], headData[2]);
	p = rotateY(p, -270.0);
    float base = sphereSDF(p, headData[3]);
    float bottomJaw = sphereSDF(p + headData[3] * float3(0.0, 0.3, .62), headData[3] * 1.08);
    bottomJaw = max(bottomJaw, -cubeSDF(p + headData[3] * float3(0.0, -1.0, .45), headData[3] * 1.3));
    float combine = smin(base, bottomJaw, .04);
    float teeth = sdCappedCone(p + headData[3] * float3(0.65, -0.7, 1.1), headData[3] * float3(4.0, 1.0, 1.0));
    teeth = min(teeth, sdCappedCone(p + headData[3] * float3(-0.65, -0.7, 1.1), headData[3] * float3(4.0, 1.0, 1.0)));
    teeth = min(teeth, sdCappedCone(p + headData[3] * float3(-0.25, -0.2, 1.4), headData[3] * float3(3.4, .5, .5)));
    teeth = min(teeth, sdCappedCone(p + headData[3] * float3(0.25, -0.2, 1.4), headData[3] * float3(3.4, .5, .5)));
    combine = min(combine, teeth);
    float eyes = min(sphereSDF(p + headData[3] * float3(.3, -0.5, 0.7), headData[3] * .2), sphereSDF(p + headData[3] * float3(-.3, -0.5, 0.7), headData[3] * .2));
	float monobrow = udBox(rotateX((p + headData[3] * float3(0.0, -0.7, .65)), -20.0), headData[3] * float3(.6, .2, .2));
    combine = min(min(combine, eyes), monobrow);
    return combine;
}

//~~~~HAND/FEET SDFs~~~~~//

// For now, size is based on head size, but later make it the average joint size
float clawFootSDF(float3 p, float size) {
    size = size * 2.0;
    float base = udRoundBox(p, size * float3(.6, .6, .3), .001);
	float fingees = sdConeSection(rotateZ(p + size * float3(0.5, -0.9, 0.3), -20.0), size, size * .3, size * .05);
    fingees = min(fingees, sdConeSection(rotateZ(p + size * float3(-0.5, -0.9, 0.3), 20.0), size, size * .3, size * .05));
    fingees = min(fingees, sdConeSection(p + size * float3(0.0, -1.1, 0.3), size, size * .3, size * .05));
    float combine = smin(base, fingees, .13); // final foot
    return combine;
}


float handSDF(float3 p, float size) {
    //float size = headData[3] / 1.5;
    //size = 1.0;
    float base = udRoundBox(p, size * float3(.6, .6, .2), .08);
    float fingee1 = sdConeSection(rotateZ(p + size * float3(1.1, -0.7, 0.0), -30.0), size, size * .5, size * .2);
    float fingee2 = sdConeSection((p + size * float3(0.45, -1.9, 0.0)), size, size * .5, size * .2);
    float fingee3 = sdConeSection((p + size * float3(-0.45, -1.9, 0.0)), size, size * .5, size * .2);
    fingee1 = min(fingee1, fingee2);
    fingee1 = min(fingee1, fingee3);
    float combine = smin(base, fingee1, .09);
    return combine;
}

float appendagesSDF(float3 p) {
	float all = MAX_DIST;
	float angle = 35.0;

	int armsNow = 0;
	int numAppen = 0;

	int startPos = 0;
	int startRot = 0;
	for (int i = 0; i < numAppendages; i++) {
		int thisPos = startPos + (3 * ((limbLengths[i] - 1)));
		float3 offset = float3(jointLocData[thisPos], jointLocData[thisPos + 1], jointLocData[thisPos + 2]);

		if ((i % 2) == 0) {
			angle *= -1.0;
		}
		float foot;

		if (appenBools[numAppen] == 1) {
			armsNow = 1;
		}

		int thisRot = startRot + (4 * ((limbLengths[i] - 1)));
		if (armsNow == 0) {
			float3 rotP = rotateZ((p + offset), 90.0);
			rotP = rotateY(rotP, 90.0);
			rotP = rotateZ(rotP, angle);
			foot = clawFootSDF(rotP, appenRads[numAppen]);
		}
		else {
			float3 q = rotateInverseAxisAngle(rotations[thisRot], rotations[thisRot + 1], rotations[thisRot + 2], rotations[thisRot + 3],
				p + offset);
			foot = handSDF(rotateZ(q, 180.0), appenRads[numAppen]);
		}

		numAppen = numAppen + 1;
		startPos = thisPos + 3;
		startRot = thisRot + 4;

		all = min(all, foot);
	}

	return all;
}

float armSDF(float3 p) {

	int countSegs = 0;

	float allLimbs = MAX_DIST;
	int incr = 0;
	int numLimbs = 0;
	int jointNum = 0;
	for (int i = 0; i < LIMBLEN_COUNT; i++) {
		jointNum += limbLengths[i];
	}

	//this is for each limb
	for (int j = 0; j < (jointNum * 3); j = j + incr) {
		numLimbs++;

		int count = 0;

		// NEED joint number to do the below operations...

		//count is number of joints in this limb
		count = int(limbLengths[numLimbs - 1]);

		float arm = MAX_DIST;
		// all joint positions for a LIM (jointNum * 3)
		int endJoint = (j + (count * 3));
		for (int i = j; i < endJoint; i = i + 3) {
			float3 pTemp = p + float3(jointLocData[i], jointLocData[i + 1], jointLocData[i + 2]);
			arm = min(arm, sphereSDF(pTemp, jointRadData[i / 3]));
		}

		// for 3 * (jointNum(per limb) - 1), each joint until last one
		float segments = MAX_DIST;

		endJoint = (j + ((count - 1) * 3));
		for (i = j; i < endJoint; i = i + 3) {
			float3 point0 = float3(jointLocData[i], jointLocData[i + 1], jointLocData[i + 2]);
			float3 point1 = float3(jointLocData[i + 3], jointLocData[i + 4], jointLocData[i + 5]);
			float3 midpoint = float3((point0.x + point1.x) / 2.0, (point0.y + point1.y) / 2.0, (point0.z + point1.z) / 2.0);
			float len = distance(point0, point1);

			float3 dir = point1 - point0; //dir is correct

			int r = (i / 3) * 4;
			float3 q = rotateInverseAxisAngle(rotations[r], rotations[r + 1], rotations[r + 2], rotations[r + 3],
				p + midpoint);

			float part = sdConeSection(q, len / 2.0, jointRadData[(i + 3) / 3], jointRadData[i / 3]);
			segments = min(segments, part);
			countSegs++;
		}

		float combine = smin(arm, segments, .2); // this is one arm
		allLimbs = min(allLimbs, combine); //merge with all other limbs

		incr = count * 3;
	}

	return allLimbs;
}


float spineSDF(float3 p) {

	float spine = MAX_DIST;
	for (int i = 0; i < SPINE_LOC_COUNT; i += 3) {
		if (spineLocData[i] == 0. && spineLocData[i + 1] == 0. && spineLocData[i + 2] == 0.) continue;
		float3 pTemp = p + float3(spineLocData[i], spineLocData[i + 1], spineLocData[i + 2]);
		spine = smin(spine, sphereSDF(pTemp, spineRadData[i / 3]), 0.06);
	}
	return spine;
}

// OVERALL SCENE SDF -- rotates about z-axis (turn-table style)
float sceneSDF(float3 p) {
	float headSDF = 0;
	int headType = headData[4];
	if (headType == 0) {
		headSDF = bugHeadSDF(p);
	}
	else if (headType == 1) {
		headSDF = dinoHeadSDF(p);
	}
	else if (headType == 2) {
		headSDF = trollHeadSDF(p);
	}
	float spine = spineSDF(p);
	float headSpine = smin(spine, headSDF, .1);
	float limbs = armSDF(p);
	float appendages = appendagesSDF(p);
<<<<<<< HEAD
	return headSpine;//smin(smin(limbs, appendages, .2), headSpine, .1);
=======
	return smin(smin(limbs, appendages, .2), headSpine, .1);
>>>>>>> 25563dc25b54e05cb73d027ef8fb273cf9dcbbaa
}

//~~~~~~~~~~~~~~~~~~~~ACTUAL RAY MARCHING STUFF~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
float march(float3 rayOrigin, float3 direction) {
    float dist = MIN_DIST;
    for (int i = 0; i < MAX_STEPS; i++) {
        float3 pos = rayOrigin + dist * direction;
        float dt = sceneSDF(pos);
        if (dt < EPSILON) {
            return dist;
        }
        dist += dt;
        if (dist >= MAX_DIST) {
            return MAX_DIST;
        }
    }
    return MAX_DIST;
}

float3 normal(float3 p) {
    float gradX = sceneSDF(float3(p.x + EPSILON, p.y, p.z)) - sceneSDF(float3(p.x - EPSILON, p.y, p.z));
    float gradY = sceneSDF(float3(p.x, p.y + EPSILON, p.z)) - sceneSDF(float3(p.x, p.y - EPSILON, p.z));
    float gradZ = sceneSDF(float3(p.x, p.y, p.z + EPSILON)) - sceneSDF(float3(p.x, p.y, p.z - EPSILON));
    return normalize(float3(gradX, gradY, gradZ));
}

/*float3 triplanar(sampler2D tex, float scale, float3 norm, float3 pos) {
    float3 xy = texture(tex, pos.xy * scale).rgb;
    float3 xz = texture(tex, pos.xz * scale).rgb;
    float3 yz = texture(tex, pos.yz * scale).rgb;
    return mix(mix(xz, yz, norm.x), xy, norm.z);
}

void main() {
    if (distance >= MAX_DIST - 2.0 * EPSILON) {
        // Colors background with a gradient!
        float4 col1 = float4(0.0, 0.0, 1.0, 1.0);
        float4 col2 = float4(0.0, 1.0, 0.0, 1.0);
        out_Col = float4(col1 * (gl_FragCoord.y / u_Resolution.y)) + float4(col2 * (2.4 - (gl_FragCoord.y / u_Resolution.y)));
        //out_Col = texture(tex_Color, gl_FragCoord.xy/u_Resolution);
        return;
    }

    // Colors geometry with Lambert Shader
    float3 p = eye + distance * dir;
    float3 norm = normalize(normal(p));
    float3 lightVec = float3(17.0, 40.0, 50.0) - p;
    float alpha = dot(norm, float3(0, 1, 0));
    if (alpha < 0.) alpha = 0.;
    float3 texture1 = triplanar(tex_Color1, 1., norm, p);
    texture1 = mix(u_Color1, u_Color2, texture1.r);
    float3 texture2 = triplanar(tex_Color2, 1., norm, p);
    texture2 = mix(u_Color3, u_Color4, texture2.r);
    float3 color = mix(texture1, texture2, alpha);
    float diffuseTerm = dot(norm, normalize(lightVec));
    diffuseTerm = clamp(diffuseTerm, 0.0, 1.0);
    float ambientTerm = 0.2;
    float lightIntensity = diffuseTerm + ambientTerm;

    out_Col = float4(color * lightIntensity, 1.0);
}*/

void fillGlobals()
{
	HeadSpineInfoBuffer headSpineAttr = g_headSpineBuffer[l_aabbCB.instanceIndex];
	
	for (int h = 0; h < HEAD_COUNT; h++)
	{
		headData[h] = headSpineAttr.headData[h];
	}
	for (int sl = 0; sl < SPINE_LOC_COUNT; sl++)
	{
		spineLocData[sl] = headSpineAttr.spineLocData[sl];
	}
	for (int sr = 0; sr < SPINE_RAD_COUNT; sr++)
	{
		spineRadData[sr] = headSpineAttr.spineRadData[sr];
	}
	
	AppendageInfoBuffer appenAttr = g_appenBuffer[l_aabbCB.instanceIndex];
	
	numAppendages = appenAttr.numAppen;
	for (int a = 0; a < APPEN_COUNT; a++)
	{
		appenBools[a] = appenAttr.appenBools[a];
		appenRads[a] = appenAttr.appenRads[a];
	}
	
	LimbInfoBuffer limbAttr = g_limbBuffer[l_aabbCB.instanceIndex];
	
	for (int l = 0; l < LIMBLEN_COUNT; l++)
	{
		limbLengths[l] = limbAttr.limbLengths[l];
	}
	for (int jl = 0; jl < JOINT_LOC_COUNT; jl++)
	{
		jointLocData[jl] = limbAttr.jointLocData[jl];
	}
	for (int jr = 0; jr < JOINT_RAD_COUNT; jr++)
	{
		jointRadData[jr] = limbAttr.jointRadData[jr];
	}
	
	RotationInfoBuffer rotAttr = g_rotBuffer[l_aabbCB.instanceIndex];

	for (int r = 0; r < ROT_COUNT; r++)
	{
		rotations[r] = rotAttr.rotations[r];
	}
}

// TODO-3.4.2: Volumetric primitive intersection shader. In our case, we only have Metaballs to take care of.
// The overall structure of this function is parallel to MyIntersectionShader_AnalyticPrimitive() 
// except you have to call the appropriate intersection test function (see RayVolumetricGeometryIntersectionTest())
[shader("intersection")]
void MyIntersectionShader_VolumetricPrimitive()
{
    Ray localRay = GetRayInAABBPrimitiveLocalSpace();
    VolumetricPrimitive::Enum primitiveType = (VolumetricPrimitive::Enum) l_aabbCB.primitiveType;
    PrimitiveInstancePerFrameBuffer aabbAttribute = g_AABBPrimitiveAttributes[l_aabbCB.instanceIndex];

	fillGlobals();

    /*float3 positions[N_METABALLS];
    float radii[N_METABALLS];
    for (int i = 0; i < N_METABALLS; i++)
    {
        positions[i] = (float3) aabbAttribute.ballPositions[i];
        radii[i] = aabbAttribute.ballRadii[i];
    }*/

    float thit;
    ProceduralPrimitiveAttributes attr;
    float distance = march(localRay.origin, localRay.direction);
    if (distance < MAX_DIST - 2.0 * EPSILON)
    {
        float3 p = localRay.origin + distance * localRay.direction;
        thit = distance;

        // Make sure the normals are stored in BLAS space and not the local space
        attr.normal = normalize(normal(p));
        attr.normal = mul(attr.normal, (float3x3) aabbAttribute.localSpaceToBottomLevelAS);
        attr.normal = normalize(mul((float3x3) ObjectToWorld3x4(), attr.normal));

        // thit is invariant to the space transformation
        ReportHit(thit, 0, attr);
    }
}
#endif // RAYTRACING_HLSL