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
		const char* hello = "Hello World!";

		D3D12_HEAP_PROPERTIES heapPropUpload{};
		heapPropUpload.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapPropUpload.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN; // depends on device, if iGPU: L0, if dedicated GPU L1, in general unknown 
		heapPropUpload.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapPropUpload.CreationNodeMask = 0;
		heapPropUpload.VisibleNodeMask = 0;

		D3D12_HEAP_PROPERTIES heapPropDefault{};
		heapPropDefault.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapPropDefault.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN; // depends on device, if iGPU: L0, if dedicated GPU L1, in general unknown 
		heapPropDefault.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapPropDefault.CreationNodeMask = 0;
		heapPropDefault.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		resourceDesc.Width = 1024;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ComPointer<ID3D12Resource2> uploadBuffer, vertexBuffer;
		DXContext::Get().GetDevice()->CreateCommittedResource(
			&heapPropUpload,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&uploadBuffer)
		);
		DXContext::Get().GetDevice()->CreateCommittedResource(
			&heapPropDefault,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&vertexBuffer)
		);

		// Copy void* -> CPU Resource, synchronous
		void* uploadBufferAddress;
		D3D12_RANGE uploadRange;
		uploadRange.Begin = 0;
		uploadRange.End = 1023;
		uploadBuffer->Map(0, &uploadRange, &uploadBufferAddress);
		memcpy(uploadBufferAddress, hello, strlen(hello) + 1);
		uploadBuffer->Unmap(0, &uploadRange);

		// Copy CPU Resource -> GPU Resource, asynchronous
		auto* cmdList = DXContext::Get().InitCommandList();
		cmdList->CopyBufferRegion(vertexBuffer, 0, uploadBuffer, 0, 1024);
		DXContext::Get().ExecuteCommandList();

		// Draw Loop
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
			cmdList = DXContext::Get().InitCommandList();

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
