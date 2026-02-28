#include <iostream>

#include "Support/CustomWin.h"
#include "Support/ComPointer.h"
#include "Support/Window.h"
#include "DXDebug/DXDebugLayer.h"
#include "D3D/DXContext.h"

int main()
{
	DXDebugLayer::Get().Init(); 

	if (DXContext::Get().Init() && DXWindow::Get().Init())
	{
		while (!DXWindow::Get().ShouldClose())
		{
			DXWindow::Get().Update();

			auto* cmdList = DXContext::Get().InitCommandList();
			DXContext::Get().ExecuteCommandList();

			DXWindow::Get().Present();
		}

		// Flushing
		DXContext::Get().Flush(DXWindow::GetFrameCount());

		DXWindow::Get().Shutdown();
		DXContext::Get().Shutdown();
	}

	DXDebugLayer::Get().Shutdown();
}
