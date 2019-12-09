#include "stdafx.h"
#include "DXProceduralProject.h"
#include "CompiledShaders\Raytracing.hlsl.h"
#include "Creature.h"

// LOOKAT-2.1: Initialize scene rendering parameters.
// This will fill in most constant and structured buffers (material properties, plane color, camera, lights)
// The member variables altered are m_aabbMaterialCB, m_planeMaterialCB, camera (m_eye, m_right, ...), and m_sceneCB.
void DXProceduralProject::InitializeScene()
{
	auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

	// Setup materials.
	{
		// Function pointer that sets up material properties for a procedural primitive
		auto SetAttributes = [&](
			UINT primitiveIndex,
			const XMFLOAT4& albedo,
			float reflectanceCoef = 0.0f,
			float diffuseCoef = 0.9f,
			float specularCoef = 0.7f,
			float specularPower = 50.0f)
		{
			auto& attributes = m_aabbMaterialCB[primitiveIndex];
			attributes.albedo = albedo;
			attributes.reflectanceCoef = reflectanceCoef;
			attributes.diffuseCoef = diffuseCoef;
			attributes.specularCoef = specularCoef;
			attributes.specularPower = specularPower;
		};

		// Changes plane color
		m_planeMaterialCB = { XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f), 0.7f, 1, 0.4f, 50};

		// Albedos
		XMFLOAT4 yellow = XMFLOAT4(1.0f, 1.0f, 0.5f, 1.0f);
		XMFLOAT4 purple = XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);
		XMFLOAT4 chromium_purple = XMFLOAT4(purple.x * ChromiumReflectance.x, 
											purple.y * ChromiumReflectance.y,
											purple.z * ChromiumReflectance.z,
											1.0f);

		/*UINT offset = 0;
		// Analytic primitives.
		{
			using namespace AnalyticPrimitive;
			SetAttributes(offset + AABB, yellow, 0.3f);
			SetAttributes(offset + Spheres, chromium_purple, 0.8f);
			offset += AnalyticPrimitive::Count;
		}*/

		// Volumetric primitives.
		{
			using namespace VolumetricPrimitive;
			SetAttributes(Metaballs, ChromiumReflectance, 1);
			//offset += VolumetricPrimitive::Count;
		}
	}

	// Setup camera.
	{
		// Initialize the view and projection inverse matrices.
		m_eye = { -10.0f, 10.0f, -25.0f, 1.0f };
		m_at = { 13.0f, 10.0f, 11.0f, 1.0f };
		XMVECTOR right = { 1.0f, 0.0f, 0.0f, 0.0f };

		XMVECTOR direction = XMVector4Normalize(m_at - m_eye);
		m_up = XMVector3Normalize(XMVector3Cross(direction, right));

		// Rotate camera around Y axis.
		XMMATRIX rotate = XMMatrixRotationY(XMConvertToRadians(-90.0f));
		m_eye = XMVector3Transform(m_eye, rotate);
		m_up = XMVector3Transform(m_up, rotate);

		UpdateCameraMatrices();
	}

	// Setup lights.
	{
		// Initialize the lighting parameters.
		XMFLOAT4 lightPosition;
		XMFLOAT4 lightAmbientColor;
		XMFLOAT4 lightDiffuseColor;

		lightPosition = XMFLOAT4(0.0f, 18.0f, -20.0f, 0.0f);
		m_sceneCB->lightPosition = XMLoadFloat4(&lightPosition);

		lightAmbientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
		m_sceneCB->lightAmbientColor = XMLoadFloat4(&lightAmbientColor);

		float d = 0.8f;
		lightDiffuseColor = XMFLOAT4(d, d, d, 1.0f);
		m_sceneCB->lightDiffuseColor = XMLoadFloat4(&lightDiffuseColor);
	}
}

// LOOKAT-2.1: Creates the scene constant buffer.
// See ConstantBuffer in DXR-Structs.h to understand what this means.
void DXProceduralProject::CreateConstantBuffers()
{
	auto device = m_deviceResources->GetD3DDevice();
	auto frameCount = m_deviceResources->GetBackBufferCount();

	m_sceneCB.Create(device, frameCount, L"Scene Constant Buffer");
}

// TODO-2.1: Create the AABB Primitive Attribute Buffer.
// This means you will need to allocate the AABB attributes on the GPU and map them onto the CPU.
//	*	This should be very similar to CreateConstantBuffers(), which allocates 
//		1 constant buffer on the GPU and maps it on the CPU for writing
//	*	As a refresher:	constant buffers are generally for data that doesn't change on the CPU side (e.g a view matrix)
//		structured buffers are for structs that have dynamic data (e.g lights in a scene, or AABBs in this case)
void DXProceduralProject::CreateAABBPrimitiveAttributesBuffers()
{
	auto device = m_deviceResources->GetD3DDevice();
	auto frameCount = m_deviceResources->GetBackBufferCount();

	// second param is num_Elements, the number of aabbs stored in the buffer
	m_aabbPrimitiveAttributeBuffer.Create(device, IntersectionShaderType::TotalPrimitiveCount, frameCount, L"AABB Primitive Attribute Buffer");
}

// LOOKAT-2.1: Update camera matrices stored in m_sceneCB.
void DXProceduralProject::UpdateCameraMatrices()
{
	auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

	m_sceneCB->cameraPosition = m_eye;
	float fovAngleY = 45.0f;
	XMMATRIX view = XMMatrixLookAtLH(m_eye, m_at, m_up);
	XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngleY), m_aspectRatio, 0.01f, 125.0f);
	XMMATRIX viewProj = view * proj;
	m_sceneCB->projectionToWorld = XMMatrixInverse(nullptr, viewProj);
}

// TODO-2.1: Update the PrimitiveInstancePerFrameBuffer for every AABB stored in m_aabbPrimitiveAttributeBuffer[].
//	*	The localSpaceToBottomLevelAS will transform the AABB from local space (a.k.a model space) to bottom-level AS space
//		which is an intermediate transform state. Think of it as taking a bland T-posing model and making a funny pose with it,
//		say now it is squatting instead.
//	*	Again, this should be very similar to what we do in UpdateCameraMatrices() which updates a ConstantBuffer
//		In this case, we are updating a *slot* in a StructuredBuffer, which is basically the same thing except it holds structs
//		instead of constants.
//	*	You will have to do this for every frame we render, which is easy if you read into how the operator[] works in 
//		the StructuredBuffer class in DXR-Structs.h.
void DXProceduralProject::UpdateAABBPrimitiveAttributes(float animationTime)
{
	auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

	XMMATRIX mIdentity = XMMatrixIdentity();

	// Different scale matrices
	XMMATRIX mScale = XMMatrixScaling(6, 6, 6);
	//XMMATRIX mScale15 = XMMatrixScaling(1.5, 1.5, 1.5);
	//XMMATRIX mScale2 = XMMatrixScaling(2, 2, 2);

	// Rotation matrix that changes over time
	XMMATRIX mRotation = XMMatrixRotationY(XMConvertToRadians(0.0f));

	
	auto SetTransformForAABB = [&](UINT primitiveIndex, XMMATRIX& mScale, XMMATRIX& mRotation)
	{
		XMVECTOR vTranslation =
			0.5f * (XMLoadFloat3(reinterpret_cast<XMFLOAT3*>(&m_aabbs[primitiveIndex].MinX))
				+ XMLoadFloat3(reinterpret_cast<XMFLOAT3*>(&m_aabbs[primitiveIndex].MaxX))); // i.e middle of AABB.
		XMMATRIX mTranslation = XMMatrixTranslationFromVector(vTranslation);

		// TODO-2.1: Fill in this lambda function.
		// It should create a transform matrix that is equal to scale * rotation * translation.
		// This matrix would transform the AABB from local space to bottom level AS space.
		// You can infer what the bottom level AS space to local space transform should be.
		// The intersection shader tests in this project work with local space, but the geometries are provided in bottom level 
		// AS space. So this data will be used to convert back and forth from these spaces.
		XMMATRIX locToBLAS = mScale * mRotation * mTranslation;
		m_aabbPrimitiveAttributeBuffer[primitiveIndex].localSpaceToBottomLevelAS = locToBLAS;
		m_aabbPrimitiveAttributeBuffer[primitiveIndex].bottomLevelASToLocalSpace = XMMatrixInverse(nullptr, locToBLAS);

        /*m_aabbPrimitiveAttributeBuffer[primitiveIndex].ballPositions[0] = XMFLOAT3(-2, -2, -2);
        m_aabbPrimitiveAttributeBuffer[primitiveIndex].ballPositions[1] = XMFLOAT3(1, 1, 1);
        m_aabbPrimitiveAttributeBuffer[primitiveIndex].ballPositions[2] = XMFLOAT3(-1, -1, -1);
        m_aabbPrimitiveAttributeBuffer[primitiveIndex].ballRadii[0] = 1;
        m_aabbPrimitiveAttributeBuffer[primitiveIndex].ballRadii[1] = 2;
        m_aabbPrimitiveAttributeBuffer[primitiveIndex].ballRadii[2] = 1;
        m_aabbPrimitiveAttributeBuffer[primitiveIndex].numBalls = 3;*/
	};

	/*UINT offset = 0;
	// Analytic primitives.
	{
		using namespace AnalyticPrimitive;
		SetTransformForAABB(offset + AABB, mScale15y, mIdentity);
		SetTransformForAABB(offset + Spheres, mScale15, mRotation);
		offset += AnalyticPrimitive::Count;
	}*/

	// Volumetric primitives.
	{
		using namespace VolumetricPrimitive;
		SetTransformForAABB(Metaballs, mScale, mIdentity);
		//offset += VolumetricPrimitive::Count;
	}
}

void DXProceduralProject::CreateCreatureBuffers()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto frameCount = m_deviceResources->GetBackBufferCount();

    // second param is num_Elements, the number of aabbs stored in the buffer
    m_headSpineBuffer.Create(device, IntersectionShaderType::TotalPrimitiveCount, frameCount, L"Head Spine Info Buffer");
    m_appenBuffer.Create(device, IntersectionShaderType::TotalPrimitiveCount, frameCount, L"Head Spine Info Buffer");
    m_limbBuffer.Create(device, IntersectionShaderType::TotalPrimitiveCount, frameCount, L"Head Spine Info Buffer");
    m_rotBuffer.Create(device, IntersectionShaderType::TotalPrimitiveCount, frameCount, L"Head Spine Info Buffer");
}

void DXProceduralProject::ResetCreatureAttributes() {
	for (int h = 0; h < HEAD_COUNT; h++)
	{
		m_headSpineBuffer[0].headData[h] = 0;
	}
	for (int sl = 0; sl < SPINE_LOC_COUNT; sl++)
	{
		m_headSpineBuffer[0].spineLocData[sl] = 0;
	}
	for (int sr = 0; sr < SPINE_RAD_COUNT; sr++)
	{
		m_headSpineBuffer[0].spineRadData[sr] = 0;
	}

	m_appenBuffer[0].numAppen = 0;
	for (int a = 0; a < APPEN_COUNT; a++)
	{
		m_appenBuffer[0].appenBools[a] = 0;
		m_appenBuffer[0].appenRads[a] = 0;
	}

	for (int l = 0; l < LIMBLEN_COUNT; l++)
	{
		m_limbBuffer[0].limbLengths[l] = 0;
	}
	for (int jl = 0; jl < JOINT_LOC_COUNT; jl++)
	{
		m_limbBuffer[0].jointLocData[jl] = 0;
	}
	for (int jr = 0; jr < JOINT_RAD_COUNT; jr++)
	{
		m_limbBuffer[0].jointRadData[jr] = 0;
	}

	for (int r = 0; r < ROT_COUNT; r++)
	{
		m_rotBuffer[0].rotations[r] = 0;
	}
}

void DXProceduralProject::UpdateCreatureAttributes()
{
    auto SetCreatureBuffers = [&](UINT primitiveIndex)
    {
        Creature *creature = new Creature();
        creature->generate(0, 2, -1);

        for (int h = 0; h < min(HEAD_COUNT, creature->head->headData.size()); h++)
        {
            m_headSpineBuffer[primitiveIndex].headData[h] = creature->head->headData[h];
        }
        for (int sl = 0; sl < min(SPINE_LOC_COUNT, creature->spineLocations.size()); sl++)
        {
            m_headSpineBuffer[primitiveIndex].spineLocData[sl] = creature->spineLocations[sl];
        }
        for (int sr = 0; sr < min(SPINE_RAD_COUNT, creature->spine->metaBallRadii.size()); sr++)
        {
            m_headSpineBuffer[primitiveIndex].spineRadData[sr] = creature->spine->metaBallRadii[sr];
        }

        m_appenBuffer[primitiveIndex].numAppen = creature->appendages->appendageData[0];
        for (int a = 0; a < min(APPEN_COUNT, creature->appenBools.size()); a++)
        {
            m_appenBuffer[primitiveIndex].appenBools[a] = creature->appenBools[a];
            m_appenBuffer[primitiveIndex].appenRads[a] = creature->appenRads[a];
        }

        for (int l = 0; l < min(LIMBLEN_COUNT, creature->limbLengths.size()); l++)
        {
            m_limbBuffer[primitiveIndex].limbLengths[l] = creature->limbLengths[l];
        }
        for (int jl = 0; jl < min(JOINT_LOC_COUNT, creature->jointLocations.size()); jl++)
        {
            m_limbBuffer[primitiveIndex].jointLocData[jl] = creature->jointLocations[jl];
        }
        for (int jr = 0; jr < min(JOINT_RAD_COUNT, creature->jointRadii.size()); jr++)
        {
            m_limbBuffer[primitiveIndex].jointRadData[jr] = creature->jointRadii[jr];
        }

        for (int r = 0; r < min(ROT_COUNT, creature->jointRots.size()); r++)
        {
            m_rotBuffer[primitiveIndex].rotations[r] = creature->jointRots[r];
        }
    };

    /*UINT offset = 0;
    // Analytic primitives.
    {
        using namespace AnalyticPrimitive;
        SetTransformForAABB(offset + AABB, mScale15y, mIdentity);
        SetTransformForAABB(offset + Spheres, mScale15, mRotation);
        offset += AnalyticPrimitive::Count;
    }*/

    // Volumetric primitives.
    {
        using namespace VolumetricPrimitive;
        SetCreatureBuffers(Metaballs);
        //offset += VolumetricPrimitive::Count;
    }
}
