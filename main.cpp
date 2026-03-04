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
		DXWindow::Get().SetFullscreen(true);

		while (!DXWindow::Get().ShouldClose())
		{
			// Process Window Messages
			DXWindow::Get().Update();

			// Handle Resizing
			if (DXWindow::Get().ShouldResize())
			{
				DXContext::Get().Flush(DXWindow::GetFrameCount());
				DXWindow::Get().Resize();
			}

			// Begin Drawing
			auto* cmdList = DXContext::Get().InitCommandList();

			// TODO: Draw stuff
			DXWindow::Get().BeginFrame(cmdList);

			DXWindow::Get().EndFrame(cmdList);

			// Finish Drawing and Present
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
