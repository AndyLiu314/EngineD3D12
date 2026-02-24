#include <iostream>

#include "Support/CustomWin.h"
#include "Support/ComPointer.h"
#include "DXDebug/DebugLayer.h"

int main()
{
	DXDebugLayer::Get().Init(); 

	DXDebugLayer::Get().Shutdown();
}
