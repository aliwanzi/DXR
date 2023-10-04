#include "RasterScene.h"
#include "DXRScene.h"
#include "Utils.h"

int CALLBACK wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	
	ConfigInfo config;
	Utils::Validate(Utils::ParseCommandLine(lpCmdLine, config), L"Error: failed to create window!");

	auto spScene = std::make_shared<DXRScene>(config);

	MSG msg = {};
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		spScene->Update();
		spScene->Render();
	}

	spScene->CleanUp();
	return EXIT_SUCCESS;
}