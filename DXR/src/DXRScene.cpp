#include "DXRScene.h"
#include "Window.h"

DXRScene::DXRScene(ConfigInfo& config)
{
	// Create a new window
	HRESULT hr = Window::Create(config.width, config.height, config.instance, window, L"RasterScene");
	Utils::Validate(hr, L"Error: failed to create window!");

	d3d.width = config.width;
	d3d.height = config.height;
	d3d.vsync = config.vsync;

	// Load a model
	Utils::LoadModel(config.model, model, material);

	// Initialize the shader compiler
	D3DShaders::Init_Shader_Compiler(shaderCompiler);

	// Initialize D3D12
	D3D12::Create_Device(d3d);
	D3D12::Create_Command_Queue(d3d);
	D3D12::Create_Command_Allocator(d3d);
	D3D12::Create_Fence(d3d);
	D3D12::Create_SwapChain(d3d, window);
	D3D12::Create_CommandList(d3d);
	D3D12::Reset_CommandList(d3d);

	// Create common resources
	D3DResources::Create_Descriptor_RTVHeaps(d3d, resources);
	D3DResources::Create_BackBuffer_RTV(d3d, resources);
	D3DResources::Create_Vertex_Buffer(d3d, resources, model);
	D3DResources::Create_Index_Buffer(d3d, resources, model);
	D3DResources::Create_Texture(d3d, resources, material);
	D3DResources::Create_View_CB(d3d, resources);
	D3DResources::Create_Material_CB(d3d, resources, material);

	// Create DXR specific resources
	DXR::Create_DXR_Output(d3d, resources);
	DXR::Create_Bottom_Level_AS(d3d, dxr, resources, model);
	DXR::Create_Top_Level_AS(d3d, dxr, resources);
	
	DXR::Create_RayGen_Program(d3d, dxr, shaderCompiler);
	DXR::Create_Closest_Hit_Program(d3d, dxr, shaderCompiler);
	DXR::Create_Miss_Program(d3d, dxr, shaderCompiler);

	DXR::Create_Descriptor_Heaps(d3d, dxr, resources, model);
	DXR::Create_Pipeline_State_Object(d3d, dxr);
	DXR::Create_Shader_Table(d3d, dxr, resources);

	D3D12::Submit_CmdList(d3d);
	D3D12::WaitForGPU(d3d);
	D3D12::Reset_CommandList(d3d);
}

void DXRScene::Update()
{
	D3DResources::Update_View_CB(d3d, resources);
}

void DXRScene::Render()
{
	DXR::Build_Command_List(d3d, dxr, resources);
	D3D12::Present(d3d);
	D3D12::MoveToNextFrame(d3d);
	D3D12::Reset_CommandList(d3d);
}

void DXRScene::CleanUp()
{
	D3D12::WaitForGPU(d3d);
	CloseHandle(d3d.fenceEvent);

	DXR::Destroy(dxr);
	D3DResources::Destroy(resources);
	D3DShaders::Destroy(shaderCompiler);
	D3D12::Destroy(d3d);

	DestroyWindow(window);
}
