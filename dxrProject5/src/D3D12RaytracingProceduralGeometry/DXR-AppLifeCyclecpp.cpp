#include "stdafx.h"
#include "DXProceduralProject.h"
#include "CompiledShaders\Raytracing.hlsl.h"

using namespace std;
using namespace DX;

// LOOKAT-1.8.0: Very first function called from Win32Application.cpp
void DXProceduralProject::OnInit()
{
	m_deviceResources = std::make_unique<DeviceResources>(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_UNKNOWN,
		FrameCount,
		D3D_FEATURE_LEVEL_11_0,
		// Sample shows handling of use cases with tearing support, which is OS dependent and has been supported since TH2.
		// Since the Fallback Layer requires Fall Creator's update (RS3), we don't need to handle non-tearing cases.
		DeviceResources::c_RequireTearingSupport,
		m_adapterIDoverride
		);
	m_deviceResources->RegisterDeviceNotify(this);
	m_deviceResources->SetWindow(Win32Application::GetHwnd(), m_width, m_height);
	m_deviceResources->InitializeDXGIAdapter();
	EnableDirectXRaytracing(m_deviceResources->GetAdapter());

	m_deviceResources->CreateDeviceResources();
	m_deviceResources->CreateWindowSizeDependentResources();

	InitializeScene();

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

    UpdateCreatureAttributes();
}

// LOOKAT-1.8.0: Update frame-based values (e.g. camera effects, light, animation time)
void DXProceduralProject::OnUpdate()
{
	m_timer.Tick();
	CalculateFrameStats();
	float elapsedTime = static_cast<float>(m_timer.GetElapsedSeconds());
	auto frameIndex = m_deviceResources->GetCurrentFrameIndex();
	auto prevFrameIndex = m_deviceResources->GetPreviousFrameIndex();

	// Rotate the camera around Y axis.
	if (m_animateCamera)
	{
		float secondsToRotateAround = 10.0f;
		float angleToRotateBy = 360.0f * (elapsedTime / secondsToRotateAround);
		XMMATRIX rotate = XMMatrixRotationY(XMConvertToRadians(angleToRotateBy));
		XMVECTOR diff = XMVectorSubtract(m_eye, m_at);
		diff = XMVector3Transform(diff, rotate);
		//m_eye = XMVector3Transform(m_eye, rotate);
		m_up = XMVector3Transform(m_up, rotate);
		//m_at = XMVector3Transform(m_at, rotate);
		m_eye = XMVectorAdd(diff, m_at);
		UpdateCameraMatrices();
	}

	// Rotate the second light around Y axis.
	if (m_animateLight)
	{
		float secondsToRotateAround = 8.0f;
		float angleToRotateBy = -360.0f * (elapsedTime / secondsToRotateAround);
		XMMATRIX rotate = XMMatrixRotationY(XMConvertToRadians(angleToRotateBy));
		const XMVECTOR& prevLightPosition = m_sceneCB->lightPosition;
		m_sceneCB->lightPosition = XMVector3Transform(prevLightPosition, rotate);
	}

	// Transform the procedural geometry.
	if (m_animateGeometry)
	{
		m_animateGeometryTime += elapsedTime;
	}
	UpdateAABBPrimitiveAttributes(m_animateGeometryTime);
	m_sceneCB->elapsedTime = m_animateGeometryTime;
}

// LOOKAT-1.8.0: Render the scene. Note the very important call to DoRaytracing() that you 
// will be filling out
void DXProceduralProject::OnRender()
{
	if (!m_deviceResources->IsWindowVisible())
	{
		return;
	}

	auto device = m_deviceResources->GetD3DDevice();
	auto commandList = m_deviceResources->GetCommandList();

	// Begin frame.
	m_deviceResources->Prepare();
	for (auto& gpuTimer : m_gpuTimers)
	{
		gpuTimer.BeginFrame(commandList);
	}

	// LOOKAT-1.8.0: This is what actually calls DispatchRays() and tells the GPU to work!
	DoRaytracing();
	CopyRaytracingOutputToBackbuffer();

	// End frame.
	for (auto& gpuTimer : m_gpuTimers)
	{
		gpuTimer.EndFrame(commandList);
	}

	m_deviceResources->Present(D3D12_RESOURCE_STATE_PRESENT);
}

// Destroy the device attached to the application (i.e kill D3D12)
void DXProceduralProject::OnDestroy()
{
	// Let GPU finish before releasing D3D resources.
	m_deviceResources->WaitForGpu();
	OnDeviceLost();
}

// Release all device dependent resources when a device is lost
void DXProceduralProject::OnDeviceLost()
{
	ReleaseWindowSizeDependentResources();
	ReleaseDeviceDependentResources();
}

// Create all device dependent resources when a device is restored
void DXProceduralProject::OnDeviceRestored()
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

void DXProceduralProject::OnMarchCubes() {
	sdf = SDF(m_headSpineBuffer, m_appenBuffer, m_limbBuffer, m_rotBuffer);
	/*setHeadBuffer(m_headSpineBuffer);
	setAppenBuffer(m_appenBuffer);
	setLimbBuffer(m_limbBuffer);
	setLimbBuffer(m_rotBuffer);*/
	March currMarch = March(vec3(1.7, 1.7, 1.8), vec3(0.0, -0.1, -0.2), 10.0, &cases, &sdf);
	currMarch.testVertexSDFs();
	currMarch.testBoxValues();
	currMarch.setTriangles();
	currMarch.callMeshClass();

	BuildMeshFromMarch(currMarch.triIndxVBO, currMarch.triVerts, currMarch.triNorms);

	float num = sdf.sceneSDF(vec3(0.0, 0.0, 0.0));
	float num2 = sdf.sceneSDF(vec3(15.0, 10.0, 0.0));

	vec3 v = mat3::rotateX(sdf.radians(90.0)) * vec3(0.0, 0.0, 1.0);
	OutputDebugStringA(LPCSTR((to_string(num) + "\n").c_str()));//printVec(v).c_str()));
	OutputDebugStringA(LPCSTR((to_string(num2) + "\n").c_str()));
}

// Handles all keyboard inputs. You can add fun camera controls here if you wish to.
void DXProceduralProject::OnKeyDown(UINT8 key)
{
	// Store previous values.
	RaytracingAPI previousRaytracingAPI = m_raytracingAPI;
	bool previousForceComputeFallback = m_forceComputeFallback;

	switch (key)
	{
	case VK_NUMPAD1:
	case '1': // Fallback Layer
		m_forceComputeFallback = false;
		SelectRaytracingAPI(RaytracingAPI::FallbackLayer);
		break;
	case VK_NUMPAD2:
	case '2': // Fallback Layer + force compute path
		m_forceComputeFallback = true;
		SelectRaytracingAPI(RaytracingAPI::FallbackLayer);
		break;
	case VK_NUMPAD3:
	case '3': // DirectX Raytracing
		SelectRaytracingAPI(RaytracingAPI::DirectXRaytracing);
		break;
	case 'C':
		m_animateCamera = !m_animateCamera;
		break;
	case 'M':
		OnMarchCubes();
		ResetCreatureAttributes();
		break;
	case 'G':
		m_animateGeometry = !m_animateGeometry;
		break;
	case 'L':
		m_animateLight = !m_animateLight;
		break;
		break;
	}

	if (m_raytracingAPI != previousRaytracingAPI ||
		m_forceComputeFallback != previousForceComputeFallback)
	{
		// Raytracing API selection changed, recreate everything.
		RecreateD3D();
	}
}

// Handle OnSizeChanged message event
void DXProceduralProject::OnSizeChanged(UINT width, UINT height, bool minimized)
{
	if (!m_deviceResources->WindowSizeChanged(width, height, minimized))
	{
		return;
	}

	UpdateForSizeChange(width, height);

	ReleaseWindowSizeDependentResources();
	CreateWindowSizeDependentResources();
}
