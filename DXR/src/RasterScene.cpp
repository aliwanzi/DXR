#include "RasterScene.h"
#include "Utils.h"
#include "Window.h"

RasterScene::RasterScene(ConfigInfo& config)
{
	// Create a new window
	HRESULT hr = Window::Create(config.width, config.height, config.instance, window, L"RasterScene");
	Utils::Validate(hr, L"Error: failed to create window!");

	d3d.width = config.width;
	d3d.height = config.height;
	d3d.vsync = config.vsync;

	// Load a model
	//Utils::LoadModel(config.model, model, material);

	// Initialize D3D12
	D3D12::Create_Device(d3d);
	D3D12::Create_Command_Queue(d3d);
	D3D12::Create_Command_Allocator(d3d);
	D3D12::Create_Fence(d3d);
	D3D12::Create_SwapChain(d3d, window);

	// Create common resources
	D3DResources::Create_Descriptor_RTVHeaps(d3d, resources);
	D3DResources::Create_Descriptor_DSVHeaps(d3d, resources);
	D3DResources::Create_Descriptor_CBVHeaps(d3d, resources);
	D3DResources::Create_BackBuffer_RTV(d3d, resources);
	D3DResources::Create_Vertex_Buffer(d3d, resources, model);
	D3DResources::Create_Index_Buffer(d3d, resources, model);
	D3DResources::Create_MVP_CB(d3d, resources);

	// Create Raster specific resources
	Raster::Resize(d3d, raster);
	Raster::Create_Raster_Program(d3d, raster);
	Raster::Create_Pipeline_State(d3d, raster);
}

RasterScene::~RasterScene()
{
	
}

void RasterScene::Update()
{
	D3DResources::Update_MVP_CB(d3d, resources);
}

void RasterScene::Render()
{
	D3D12::Reset_CommandList(d3d);
	Raster::Build_Command_List(d3d, raster, resources);
	D3D12::Submit_CmdList(d3d);
	D3D12::Present(d3d);
	D3D12::MoveToNextFrame(d3d);
}

void RasterScene::CleanUp()
{
}