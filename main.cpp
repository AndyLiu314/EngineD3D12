#include <iostream>

#include "Support/CustomWin.h"
#include "Support/ComPointer.h"
#include "DXDebug/DXDebugLayer.h"
#include "D3D/DXContext.h"

int main()
{
	DXDebugLayer::Get().Init(); 

	if (DXContext::Get().Init())
	{
		DXContext::Get().Shutdown();
	}

	DXDebugLayer::Get().Shutdown();
}
