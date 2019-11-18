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

        float3 fresnelR = FresnelReflectanceSchlick(WorldRayDirection(), triangleNormal, l_materialCB.albedo.xyz);
        reflectedColor = l_materialCB.reflectanceCoef * float4(fresnelR, 1) * reflectionColor;
    }

    // Calculate final color.
    float4 phongColor = CalculatePhongLighting(l_materialCB.albedo, triangleNormal, shadowRayHit, l_materialCB.diffuseCoef, l_materialCB.specularCoef, l_materialCB.specularPower);
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
    
    // Trace a shadow ray to determine if this ray is a shadow ray
    Ray shadowRay = { hitPosition, normalize(g_sceneCB.lightPosition.xyz - hitPosition) };
    bool shadowRayHit = TraceShadowRayAndReportIfHit(shadowRay, rayPayload.recursionDepth);

    // Reflected component ray.
    float4 reflectedColor = float4(0, 0, 0, 0);
    if (l_materialCB.reflectanceCoef > 0.001)
    {
        Ray reflectionRay = { hitPosition, reflect(WorldRayDirection(), attr.normal) };
        float4 reflectionColor = TraceRadianceRay(reflectionRay, rayPayload.recursionDepth);

        float3 fresnelR = FresnelReflectanceSchlick(WorldRayDirection(), attr.normal, l_materialCB.albedo.xyz);
        reflectedColor = l_materialCB.reflectanceCoef * float4(fresnelR, 1) * reflectionColor;
    }

    // Calculate final color with phong lighting
    float4 phongColor = CalculatePhongLighting(l_materialCB.albedo, attr.normal, shadowRayHit, l_materialCB.diffuseCoef, l_materialCB.specularCoef, l_materialCB.specularPower);
    float4 color = (phongColor + reflectedColor);

    float t = 1.0f - min(RayTCurrent() / 500.0f, 1.0f);
    float4 falloffColor = lerp(BackgroundColor, color, t);

    //HeadSpineInfoBuffer headSpineAttr = g_headSpineBuffer[l_aabbCB.instanceIndex];
    //rayPayload.color = float4(headSpineAttr.headData[3], headSpineAttr.headData[3], headSpineAttr.headData[3], 1.0);
    rayPayload.color = falloffColor;
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

float3x3 rotateMatX(float angle) {
    float rad = radians(angle);
    return float3x3(
        float3(1.0, 0.0, 0.0),
        float3(0.0, cos(rad), -sin(rad)),
        float3(0.0, sin(rad), cos(rad))
    );
}

float3x3 rotateMatY(float angle) {
    float rad = radians(angle);
    return float3x3(
        float3(cos(rad), 0.0, sin(rad)),
        float3(0.0, 1.0, 0.0),
        float3(-sin(rad), 0.0, cos(rad))
    );
}

float3x3 rotateMatZ(float angle) {
    float rad = radians(angle);
    return float3x3(
        float3(cos(rad), -sin(rad), 0.0),
        float3(sin(rad), cos(rad), 0.0),
        float3(0.0, 0.0, 1.0)
    );
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
float bugHeadSDF(float3 p, float u_Head[HEAD_COUNT]) {
    p = mul(rotateMatY(-90.0), p);
    float base = sphereSDF(p, u_Head[3]);
    float eyes = min(sphereSDF(p + u_Head[3] * float3(0.55, -0.35, -.71), u_Head[3] * .2), sphereSDF(p + u_Head[3] * float3(-0.55, -0.35, -.71), u_Head[3] * .2));
    float mandibleBase = sdCappedCylinder(p + u_Head[3] * float3(0.0, 0.001, -.9), u_Head[3] * float2(1.2, 0.1));
    float mandibles = max(mandibleBase, -sphereSDF(p + u_Head[3] * float3(0.0, 0.0, -0.60), .7 * u_Head[3]));
    mandibles = max(mandibles, -sphereSDF(p + u_Head[3] * float3(0.0, 0.0, -1.70), .7 * u_Head[3]));
    float head = smin(min(base, eyes), mandibles, .05);
    return head;
}

float dinoHeadSDF(float3 p, float u_Head[HEAD_COUNT]) {
    p = mul(rotateMatY(-90.0), p);
    float base = sphereSDF(p, u_Head[3]);
    float topJaw = sphereSDF(p + u_Head[3] * float3(0.0, 0.3, -1.4), u_Head[3] * 1.08);
    topJaw = max(topJaw, -cubeSDF(p + u_Head[3] * float3(0.0, 1.4, -1.4), u_Head[3] * 1.2));
    float bottomJaw = sphereSDF(p + u_Head[3] * float3(0.0, 0.6, -1.0), u_Head[3] * .7);
    bottomJaw = max(bottomJaw, -cubeSDF(mul(rotateMatX(45.0), (p + u_Head[3] * float3(0.0, -.4, -1.7))), u_Head[3] * 1.1));
    float combine = smin(base, topJaw, .04);
    combine = smin(combine, bottomJaw, .08);

    float eyes = min(sphereSDF(p + u_Head[3] * float3(.9, 0.0, 0.0), u_Head[3] * .3), sphereSDF(p + u_Head[3] * float3(-0.9, 0.0, 0.0), u_Head[3] * .3));
    combine = min(combine, eyes);
    float brows = min(udBox(mul(rotateMatX(-20.0), (p + u_Head[3] * float3(.85, -0.35, 0.0))), u_Head[3] * float3(.3, .2, .5)), 
        udBox(mul(rotateMatX(-20.0), (p + u_Head[3] * float3(-0.85, -0.35, 0.0))), u_Head[3] * float3(.3, .2, .5)));
    combine = min(combine, brows);

    float teeth = sdCappedCone(mul(rotateMatX(180.0), (p + u_Head[3] * float3(0.4, 0.7, -1.8))), u_Head[3] * float3(3.0, 1.0, 1.0));
    teeth = min(teeth, sdCappedCone(mul(rotateMatX(180.0), (p + u_Head[3] * float3(-0.4, 0.7, -1.8))), u_Head[3] * float3(3.0, 1.0, 1.0)));
    teeth = min(teeth, sdCappedCone(mul(rotateMatX(180.0), (p + u_Head[3] * float3(-0.4, 0.7, -1.3))), u_Head[3] * float3(2.7, 1.0, 1.0)));
    teeth = min(teeth, sdCappedCone(mul(rotateMatX(180.0), (p + u_Head[3] * float3(0.4, 0.7, -1.3))), u_Head[3] * float3(2.7, 1.0, 1.0)));
    combine = min(combine, teeth);
    return combine;
}

float trollHeadSDF(float3 p, float u_Head[HEAD_COUNT]) {
    p = mul(rotateMatY(-270.0), p);
    float base = sphereSDF(p, u_Head[3]);
    float bottomJaw = sphereSDF(p + u_Head[3] * float3(0.0, 0.3, .62), u_Head[3] * 1.08);
    bottomJaw = max(bottomJaw, -cubeSDF(p + u_Head[3] * float3(0.0, -1.0, .45), u_Head[3] * 1.3));
    float combine = smin(base, bottomJaw, .04);
    float teeth = sdCappedCone(p + u_Head[3] * float3(0.65, -0.7, 1.1), u_Head[3] * float3(4.0, 1.0, 1.0));
    teeth = min(teeth, sdCappedCone(p + u_Head[3] * float3(-0.65, -0.7, 1.1), u_Head[3] * float3(4.0, 1.0, 1.0)));
    teeth = min(teeth, sdCappedCone(p + u_Head[3] * float3(-0.25, -0.2, 1.4), u_Head[3] * float3(3.4, .5, .5)));
    teeth = min(teeth, sdCappedCone(p + u_Head[3] * float3(0.25, -0.2, 1.4), u_Head[3] * float3(3.4, .5, .5)));
    combine = min(combine, teeth);
    float eyes = min(sphereSDF(p + u_Head[3] * float3(.3, -0.5, 0.7), u_Head[3] * .2), sphereSDF(p + u_Head[3] * float3(-.3, -0.5, 0.7), u_Head[3] * .2));
    float monobrow = udBox(mul(rotateMatX(-20.0), (p + u_Head[3] * float3(0.0, -0.7, .65))), u_Head[3] * float3(.6, .2, .2));
    combine = min(min(combine, eyes), monobrow);
    return combine;
}

//~~~~HAND/FEET SDFs~~~~~//

// For now, size is based on head size, but later make it the average joint size
float clawFootSDF(float3 p, float size) {
    size = size * 2.0;
    float base = udRoundBox(p, size * float3(.6, .6, .3), .001);
    float fingees = sdConeSection(mul(rotateMatZ(-20.0), (p + size * float3(0.5, -0.9, 0.3))), size * 1.0, size * .3, size * .05);
    fingees = min(fingees, sdConeSection(mul(rotateMatZ(20.0), (p + size * float3(-0.5, -0.9, 0.3))), size * 1.0, size * .3, size * .05));
    fingees = min(fingees, sdConeSection(p + size * float3(0.0, -1.1, 0.3), size * 1.0, size * .3, size * .05));
    float combine = smin(base, fingees, .13); // final foot
    return combine;
}


float handSDF(float3 p, float size) {
    //float size = u_Head[3] / 1.5;
    //size = 1.0;
    float base = udRoundBox(p, size * float3(.6, .6, .2), .08);
    float fingee1 = sdConeSection(mul(rotateMatZ(-30.0), (p + size * float3(1.1, -0.7, 0.0))), size * 1.0, size * .5, size * .2);
    float fingee2 = sdConeSection((p + size * float3(0.45, -1.9, 0.0)), size * 1.0, size * .5, size * .2);
    float fingee3 = sdConeSection((p + size * float3(-0.45, -1.9, 0.0)), size * 1.0, size * .5, size * .2);
    fingee1 = min(fingee1, fingee2);
    fingee1 = min(fingee1, fingee3);
    float combine = smin(base, fingee1, .09);
    return combine;
}

float appendagesSDF(float3 p, float u_AppenData[APPEN_COUNT], float u_AppenRad[APPEN_COUNT], float u_AppenBools[APPEN_COUNT], XMMATRIX u_AppenRots[APPEN_COUNT]) {
    //AppendageInfoBuffer appenAttr = g_appenBuffer[l_aabbCB.instanceIndex];
    
    float all = MAX_DIST;
    float angle;
    //angle to slightly offset each foot
    float angle1 = -35.0;
    float angle2 = 35.0;

    int armsNow = 0;
    int numAppen = 0;

    for (int i = 0; i < int(u_AppenData[0]); i++) {
        int start = (i * 3) + 1;
        float3 offset = float3(u_AppenData[start], u_AppenData[start + 1], u_AppenData[start + 2]);

        if ((i % 2) == 0) {
            angle = angle1;
        }
        else {
            angle = angle2;
        }
        float foot;

        if (u_AppenBools[numAppen] == 1) {
            armsNow = 1;
        }

        if (armsNow == 0) {
            float3 rotP = mul(rotateMatZ(angle), mul(rotateMatY(90.0), mul(rotateMatZ(90.0), (p + offset))));
            foot = clawFootSDF(rotP, u_AppenRad[numAppen]);
        }
        else {
            float3 q = mul(float4((p + offset), 1.0), transpose(u_AppenRots[numAppen])).xyz;
            foot = handSDF(mul(rotateMatZ(180.0), q), u_AppenRad[numAppen]);
        }

        // //if(u_AppenBools[numAppen] != 0.0) {
        // if((numAppen % 2) == 0) {
        // 	foot = clawFootSDF((p + offset)*rotateMatZ(90.0) * rotateMatY(90.0) * rotateMatZ(angle));		
        // }
        // else {
        // 	foot = handSDF((p + offset) * rotateMatZ(135.0));
        // }
        // else if(u_AppenBools[2] == 0){			
        // 	
        // }

        numAppen = numAppen + 1;

        // if((p + offset).y == 100.0) { //should be 0, but does this work
        // 	foot = handSDF((p + offset) * rotateMatZ(135.0));
        // }
        // else {
        // 	foot = clawFootSDF((p + offset)*rotateMatZ(90.0) * rotateMatY(90.0) * rotateMatZ(angle));
        // }
        all = min(all, foot);
    }

    return all;
}

float armSDF(float3 p, float u_LimbLengths[LIMBLEN_COUNT], float u_JointLoc[JOINT_LOC_COUNT], float u_JointRad[JOINT_RAD_COUNT], XMMATRIX u_Rotations[ROT_COUNT]) {

    int countSegs = 0;

    float allLimbs = MAX_DIST;
    int incr = 0;
    int numLimbs = 0;
    int jointNum = 0;
    for (int i = 0; i < LIMBLEN_COUNT; i++) {
        jointNum += u_LimbLengths[i];
    }

    //this is for each limb
    for (int j = 0; j < (jointNum * 3); j = j + incr) {
        numLimbs++;

        int count = 0;

        // NEED joint number to do the below operations...

        //count is number of joints in this limb
        count = int(u_LimbLengths[numLimbs - 1]);

        float arm = MAX_DIST;
        // all joint positions for a LIM (jointNum * 3)
        for (int i = j; i < (j + (count * 3)); i = i + 3) {
            float3 pTemp = p + float3(u_JointLoc[i], u_JointLoc[i + 1], u_JointLoc[i + 2]);
            arm = min(arm, sphereSDF(pTemp, u_JointRad[i / 3]));

        }

        // for 3 * (jointNum(per limb) - 1), each joint until last one
        float segments = MAX_DIST;

        for (i = j; i < (j + ((count - 1) * 3)); i = i + 3) {
            float3 point0 = float3(u_JointLoc[i], u_JointLoc[i + 1], u_JointLoc[i + 2]);
            float3 point1 = float3(u_JointLoc[i + 3], u_JointLoc[i + 4], u_JointLoc[i + 5]);
            float3 midpoint = float3((point0.x + point1.x) / 2.0, (point0.y + point1.y) / 2.0, (point0.z + point1.z) / 2.0);
            float len = distance(point0, point1);

            float3 dir = point1 - point0; //dir is correct



            float4x4 outMat4 = transpose(u_Rotations[countSegs]);
            countSegs++;

            float3 q = mul(float4((p + midpoint), 1.0), outMat4).xyz;



            float part = sdConeSection(q, len / 2.0, u_JointRad[(i + 3) / 3], u_JointRad[i / 3]);
            segments = min(segments, part);
        }

        float combine = smin(arm, segments, .2); // this is one arm
        allLimbs = min(allLimbs, combine); //merge with all other limbs

        incr = count * 3;

    }

    return allLimbs;
}


float spineSDF(float3 p, float u_SpineLoc[SPINE_LOC_COUNT], float u_SpineRad[SPINE_RAD_COUNT]) {
    float spine = MAX_DIST;
    for (int i = 0; i < SPINE_LOC_COUNT; i += 3) {
        if (u_SpineLoc[i] == 0. && u_SpineLoc[i + 1] == 0. && u_SpineLoc[i + 2] == 0.) continue;
        float3 pTemp = p + float3(u_SpineLoc[i], u_SpineLoc[i + 1], u_SpineLoc[i + 2]);
        spine = smin(spine, sphereSDF(pTemp, u_SpineRad[i / 3]), 0.06);
    }
    return spine;
}

// OVERALL SCENE SDF -- rotates about z-axis (turn-table style)
float sceneSDF(float3 p) {
    HeadSpineInfoBuffer headSpineAttr = g_headSpineBuffer[l_aabbCB.instanceIndex];
    //AppendageInfoBuffer appenAttr = g_appenBuffer[l_aabbCB.instanceIndex];
    //LimbInfoBuffer limbAttr = g_limbBuffer[l_aabbCB.instanceIndex];
    //RotationInfoBuffer rotAttr = g_rotBuffer[l_aabbCB.instanceIndex];

    float u_Head[HEAD_COUNT] = headSpineAttr.headData;

    p += float3(0, 0, 0);
    // p = p * rotateMatY(u_Time) ; // rotates creature
    //return sphereSDF(p, 1.0);
    //return bugHeadSDF(p + float3(u_Head[0], u_Head[1], u_Head[2]), u_Head);
    
    float headSDF = bugHeadSDF(p + float3(u_Head[0], u_Head[1], u_Head[2]), u_Head);
    return smin(spineSDF(p, headSpineAttr.spineLocData, headSpineAttr.spineRadData), headSDF, .15);
    if (u_Head[4] == 0.0) {
        headSDF = bugHeadSDF(p + float3(u_Head[0], u_Head[1], u_Head[2]), u_Head);
    }
    else if (u_Head[4] == 1.0) {
        headSDF = dinoHeadSDF(p + float3(u_Head[0], u_Head[1], u_Head[2]), u_Head);
    }
    else if (u_Head[4] == 2.0) {
        headSDF = trollHeadSDF(p + float3(u_Head[0], u_Head[1], u_Head[2]), u_Head);
    }
    //float dist = smin(spineSDF(p, headSpineAttr.spineLocData, headSpineAttr.spineRadData), headSDF, .1);
    return headSDF;//smin(smin(armSDF(p, limbAttr.limbLengths, limbAttr.jointLocData, limbAttr.jointRadData, rotAttr.rotations),
        //appendagesSDF(p, appenAttr.appenData, appenAttr.appenRads, appenAttr.appenBools, appenAttr.appenRots), .2), dist, .1);

    //return min(handSDF(p+ float3(-1.0,0.0,0.0), u_AppenRad[0]), clawFootSDF(p + float3(1.0,0.0,0.0), u_AppenRad[0]));
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

// TODO-3.4.2: Volumetric primitive intersection shader. In our case, we only have Metaballs to take care of.
// The overall structure of this function is parallel to MyIntersectionShader_AnalyticPrimitive() 
// except you have to call the appropriate intersection test function (see RayVolumetricGeometryIntersectionTest())
[shader("intersection")]
void MyIntersectionShader_VolumetricPrimitive()
{
    Ray localRay = GetRayInAABBPrimitiveLocalSpace();
    VolumetricPrimitive::Enum primitiveType = (VolumetricPrimitive::Enum) l_aabbCB.primitiveType;
    PrimitiveInstancePerFrameBuffer aabbAttribute = g_AABBPrimitiveAttributes[l_aabbCB.instanceIndex];

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