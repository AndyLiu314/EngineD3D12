#include <iostream>

#include "Support/CustomWin.h"
#include "Support/ComPointer.h"
#include "Support/Window.h"
#include "Support/Shader.h"
#include "DXDebug/DXDebugLayer.h"
#include "D3D/DXContext.h"

int main()
{
	DXDebugLayer::Get().Init(); 

	if (DXContext::Get().Init() && DXWindow::Get().Init())
	{
		// Defining Heaps Properties
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

		// Vertex Data
		struct Vertex
		{
			float x, y;
		};

		Vertex vertices[] =
		{
			{-1.f, -1.f},
			{ 0.f, 1.f},
			{ 1.f, -1.f},
		};

		D3D12_INPUT_ELEMENT_DESC vertexLayout[] =
		{
			{ "Position", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		// Desc for Upload and Vertex Buffer
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

		// Creating Vertex and Upload Buffers
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
		memcpy(uploadBufferAddress, vertices, sizeof(vertices));
		uploadBuffer->Unmap(0, &uploadRange);

		// Copy CPU Resource -> GPU Resource, asynchronous
		auto* cmdList = DXContext::Get().InitCommandList();
		cmdList->CopyBufferRegion(vertexBuffer, 0, uploadBuffer, 0, 1024);
		DXContext::Get().ExecuteCommandList();

		// Shaders
		Shader rootSignatureShader("RootSignature.cso");
		Shader vertexShader("VertexShader.cso");
		Shader pixelShader("PixelShader.cso");

		// Create Root Signature
		ComPointer<ID3D12RootSignature> rootSignature;
		DXContext::Get().GetDevice()->CreateRootSignature(
			0,
			rootSignatureShader.GetBuffer(),
			rootSignatureShader.GetSize(),
			IID_PPV_ARGS(&rootSignature)
		);

		// === Pipeline State ===
		D3D12_GRAPHICS_PIPELINE_STATE_DESC gfxPSODesc{};
		gfxPSODesc.pRootSignature = rootSignature;

		gfxPSODesc.InputLayout.NumElements = _countof(vertexLayout);
		gfxPSODesc.InputLayout.pInputElementDescs = vertexLayout;
		gfxPSODesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
		
		gfxPSODesc.VS.BytecodeLength = vertexShader.GetSize();
		gfxPSODesc.VS.pShaderBytecode = vertexShader.GetBuffer();
		gfxPSODesc.PS.BytecodeLength = pixelShader.GetSize();
		gfxPSODesc.PS.pShaderBytecode = pixelShader.GetBuffer();
		gfxPSODesc.DS.BytecodeLength = 0;
		gfxPSODesc.DS.pShaderBytecode = nullptr;
		gfxPSODesc.HS.BytecodeLength = 0; 
		gfxPSODesc.HS.pShaderBytecode = nullptr;
		gfxPSODesc.GS.BytecodeLength = 0;
		gfxPSODesc.GS.pShaderBytecode = nullptr;

		gfxPSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		gfxPSODesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		gfxPSODesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		gfxPSODesc.RasterizerState.FrontCounterClockwise = FALSE;
		gfxPSODesc.RasterizerState.DepthBias = 0;
		gfxPSODesc.RasterizerState.DepthBiasClamp = .0f;
		gfxPSODesc.RasterizerState.SlopeScaledDepthBias = .0f;
		gfxPSODesc.RasterizerState.DepthClipEnable = FALSE;
		gfxPSODesc.RasterizerState.MultisampleEnable = FALSE;
		gfxPSODesc.RasterizerState.AntialiasedLineEnable = FALSE;
		gfxPSODesc.RasterizerState.ForcedSampleCount = 0;

		gfxPSODesc.StreamOutput.NumEntries = 0;
		gfxPSODesc.StreamOutput.NumStrides = 0;
		gfxPSODesc.StreamOutput.pBufferStrides = nullptr;
		gfxPSODesc.StreamOutput.pSODeclaration = nullptr;
		gfxPSODesc.StreamOutput.RasterizedStream = 0;

		gfxPSODesc.NumRenderTargets = 1;
		gfxPSODesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		gfxPSODesc.DSVFormat = DXGI_FORMAT_UNKNOWN;

		gfxPSODesc.BlendState.AlphaToCoverageEnable = FALSE;
		gfxPSODesc.BlendState.IndependentBlendEnable = FALSE;
		gfxPSODesc.BlendState.RenderTarget[0].BlendEnable = FALSE;
		gfxPSODesc.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
		gfxPSODesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
		gfxPSODesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		gfxPSODesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		gfxPSODesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
		gfxPSODesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		gfxPSODesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		gfxPSODesc.BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
		gfxPSODesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		gfxPSODesc.DepthStencilState.DepthEnable = FALSE;
		gfxPSODesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		gfxPSODesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		gfxPSODesc.DepthStencilState.StencilEnable = FALSE;
		gfxPSODesc.DepthStencilState.StencilReadMask = 0;
		gfxPSODesc.DepthStencilState.StencilWriteMask = 0;
		gfxPSODesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		gfxPSODesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		gfxPSODesc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		gfxPSODesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		gfxPSODesc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		gfxPSODesc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		gfxPSODesc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		gfxPSODesc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;

		gfxPSODesc.SampleMask = 0xFFFFFFFF;
		gfxPSODesc.SampleDesc.Count = 1;
		gfxPSODesc.SampleDesc.Quality = 0;

		gfxPSODesc.NodeMask = 0;
		gfxPSODesc.CachedPSO.CachedBlobSizeInBytes = 0;
		gfxPSODesc.CachedPSO.pCachedBlob = nullptr;
		gfxPSODesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		ComPointer<ID3D12PipelineState> pso;
		DXContext::Get().GetDevice()->CreateGraphicsPipelineState(&gfxPSODesc, IID_PPV_ARGS(&pso));

		// Vertex Buffer View
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
		vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vertexBufferView.SizeInBytes = sizeof(Vertex) * _countof(vertices);
		vertexBufferView.StrideInBytes = sizeof(Vertex);

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

			// == PSO ==
			cmdList->SetPipelineState(pso);
			cmdList->SetGraphicsRootSignature(rootSignature);

			// == IA ==
			cmdList->IASetVertexBuffers(0, 1, &vertexBufferView);
			cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// Draw
			cmdList->DrawInstanced(_countof(vertices), 1, 0, 0);

			DXWindow::Get().EndFrame(cmdList);

			// Finish Drawing and Present
			DXContext::Get().ExecuteCommandList();
			DXWindow::Get().Present();
		}

		// Flushing
		DXContext::Get().Flush(DXWindow::GetFrameCount());

		// Close Resources
		vertexBuffer.Release();
		uploadBuffer.Release();

		DXWindow::Get().Shutdown();
		DXContext::Get().Shutdown();
	}

	DXDebugLayer::Get().Shutdown();
}
