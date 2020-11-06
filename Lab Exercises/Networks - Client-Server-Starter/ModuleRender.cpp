#include "Networks.h"


#define SAFE_RELEASE(lp) if (lp != nullptr) { lp->Release(); lp = nullptr; }

using namespace DirectX;

// Window handler
extern HWND                     hwnd;

// Direct3D basics
       ID3D11Device            *g_pd3dDevice = NULL;
       ID3D11DeviceContext     *g_pd3dDeviceContext = NULL;
static ID3D11RenderTargetView  *g_mainRenderTargetView = NULL;
static IDXGISwapChain          *g_pSwapChain = NULL;

// NOTE(jesus): Maybe class members
static ID3D11Buffer            *g_pVertexBuffer;
static ID3D10Blob*              g_pVertexShaderBlob = NULL;
static ID3D11VertexShader*      g_pVertexShader = NULL;
static ID3D10Blob*              g_pPixelShaderBlob = NULL;
static ID3D11PixelShader*       g_pPixelShader = NULL;
static ID3D11InputLayout*       g_pInputLayout = NULL;
static ID3D11Buffer*            g_pConstantBuffer = NULL;
static ID3D11RasterizerState*   g_pRasterizerState = NULL;
static ID3D11BlendState*        g_pBlendState = NULL;
static ID3D11DepthStencilState* g_pDepthStencilState = NULL;
static ID3D11SamplerState*      g_pTextureSampler = NULL;

struct CUSTOMVERTEX
{
	FLOAT x, y;
	FLOAT u, v;
};

struct CONSTANT_BUFFER
{
	float ProjectionMatrix[4][4];
	float ViewMatrix[4][4];
	float WorldMatrix[4][4];
	float TintColor[4];
};

// this function loads a file into an Array^
static void LoadShaderFile(std::string File, uint8 *buffer, int *length)
{
	*length = 0;

	// open the file
	std::ifstream VertexFile(File, std::ios::in | std::ios::binary | std::ios::ate);

	// if open was successful
	if (VertexFile.is_open())
	{
		// find the length of the file
		*length = (int)VertexFile.tellg();

		// collect the file data
		VertexFile.seekg(0, std::ios::beg);
		VertexFile.read(reinterpret_cast<char*>(buffer), *length);
		VertexFile.close();
	}
}

bool ModuleRender::init()
{
	/////////////////////////////////////////////////////////////
	// Direct3D initialization
	/////////////////////////////////////////////////////////////

	if (!CreateDeviceD3D(hwnd))
	{
		LOG("ModuleRender::init() - CreateDeviceD3D() failed");
		CleanupDeviceD3D();
		return false;
	}

	// Just to know if threadingCaps.DriverConcurrentCreates == true
	D3D11_FEATURE_DATA_THREADING threadingCaps = {};
	g_pd3dDevice->CheckFeatureSupport(D3D11_FEATURE_THREADING, &threadingCaps, sizeof(threadingCaps));


	/////////////////////////////////////////////////////////////
	// QUAD VERTICES
	/////////////////////////////////////////////////////////////

	CUSTOMVERTEX vertices[] =
	{
		{ -0.5f, -0.5f,    0.f, 0.f, },
		{ -0.5f,  0.5f,    0.f, 1.f, },
		{  0.5f,  0.5f,    1.f, 1.f, },
		{ -0.5f, -0.5f,    0.f, 0.f, },
		{  0.5f,  0.5f,    1.f, 1.f, },
		{  0.5f, -0.5f,    1.f, 0.f, },
	};

	D3D11_BUFFER_DESC desc = {};
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(vertices);
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertices;

	if (g_pd3dDevice->CreateBuffer(&desc, &InitData, &g_pVertexBuffer) < 0) {
		LOG("d3d->CreateBuffer() failed (Vertex Buffer)");
		return false;
	}







	/////////////////////////////////////////////////////////////
	// VERTEX SHADER
	/////////////////////////////////////////////////////////////

	int shaderSourceLength;
	LoadShaderFile("vertex_shader.hlsl", shaderSource, &shaderSourceLength);
	if (shaderSourceLength == 0)
	{
		LOG("ModuleRender::init() failed - couldn't load vertex_shader.hlsl");
		return false;
	}
	ID3DBlob *errorBlob = NULL;

	D3DCompile(shaderSource, shaderSourceLength, NULL, NULL, NULL, "main", "vs_4_0", 0, 0, &g_pVertexShaderBlob, &errorBlob);
	if (errorBlob) {
		LOG("D3DCompile(\"vertex_shader.hlsl\") messages:\n%s", (const char*)errorBlob->GetBufferPointer());
		SAFE_RELEASE(errorBlob);
	}
	if (g_pVertexShaderBlob == NULL) {
		LOG("D3DCompile(\"vertex_shader.hlsl\") failed");
		return false;
	}
	if (g_pd3dDevice->CreateVertexShader((DWORD*)g_pVertexShaderBlob->GetBufferPointer(), g_pVertexShaderBlob->GetBufferSize(), NULL, &g_pVertexShader) != S_OK) {
		LOG("d3d->CreateVertexShader(\"vertex_shader.hlsl\") failed");
		return false;
	}

	// Create the input layout
	D3D11_INPUT_ELEMENT_DESC local_layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (size_t)(&((CUSTOMVERTEX*)0)->x), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (size_t)(&((CUSTOMVERTEX*)0)->u), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	if (g_pd3dDevice->CreateInputLayout(
		local_layout,
		2,
		g_pVertexShaderBlob->GetBufferPointer(),
		g_pVertexShaderBlob->GetBufferSize(),
		&g_pInputLayout) != S_OK) {
		LOG("d3d->CreateInputLayout(\"vertex_shader.hlsl\") failed");
		return false;
	}

	// Create the constant buffer
	{
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = sizeof(CONSTANT_BUFFER);
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		if (g_pd3dDevice->CreateBuffer(&desc, NULL, &g_pConstantBuffer) != S_OK) {
			LOG("d3d->CreateBuffer() failed (CONSTANT_BUFFER)");
			return false;
		}
	}



	/////////////////////////////////////////////////////////////
	// FRAGMENT SHADER
	/////////////////////////////////////////////////////////////

	LoadShaderFile("pixel_shader.hlsl", shaderSource, &shaderSourceLength);
	if (shaderSourceLength == 0)
	{
		LOG("ModuleRender::init() failed - couldn't load pixel_shader.hlsl");
		return false;
	}

	D3DCompile(shaderSource, shaderSourceLength, NULL, NULL, NULL, "main", "ps_4_0", 0, 0, &g_pPixelShaderBlob, &errorBlob);
	if (errorBlob) {
		LOG("D3DCompile(\"pixel_shader.hlsl\") messages:\n%s", (const char*)errorBlob->GetBufferPointer());
		errorBlob->Release();
	}
	if (g_pPixelShaderBlob == NULL) {
		LOG("D3DCompile(\"pixel_shader.hlsl\") failed");
		return false;
	}
	if (g_pd3dDevice->CreatePixelShader((DWORD*)g_pPixelShaderBlob->GetBufferPointer(), g_pPixelShaderBlob->GetBufferSize(), NULL, &g_pPixelShader) != S_OK) {
		LOG("d3d->CreatePixelShader(\"pixel_shader.hlsl\") failed");
		return false;
	}


	/////////////////////////////////////////////////////////////
	// RENDERING STATE
	/////////////////////////////////////////////////////////////

	// Create the blending setup
	{
		D3D11_BLEND_DESC desc = {};
		desc.AlphaToCoverageEnable = false;
		desc.RenderTarget[0].BlendEnable = true;
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		if (g_pd3dDevice->CreateBlendState(&desc, &g_pBlendState) != S_OK) {
			LOG("d3d->CreateBlendState() failed");
			return false;
		}
	}

	// Create the rasterizer state
	{
		D3D11_RASTERIZER_DESC desc = {};
		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_NONE;
		desc.ScissorEnable = false;
		desc.DepthClipEnable = true;
		if (g_pd3dDevice->CreateRasterizerState(&desc, &g_pRasterizerState) != S_OK) {
			LOG("d3d->CreateRasterizerState() failed");
			return false;
		}
	}

	// Create depth-stencil State
	{
		D3D11_DEPTH_STENCIL_DESC desc = {};
		desc.DepthEnable = false;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		desc.StencilEnable = false;
		desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp = desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		desc.BackFace = desc.FrontFace;
		if (g_pd3dDevice->CreateDepthStencilState(&desc, &g_pDepthStencilState) != S_OK) {
			LOG("d3d->CreateDepthStencilState() failed");
			return false;
		}
	}


	/////////////////////////////////////////////////////////////
	// TEXTURE SAMPLER
	/////////////////////////////////////////////////////////////

	// Create texture sampler
	{
		D3D11_SAMPLER_DESC desc = {};
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.MipLODBias = 0.f;
		desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		desc.MinLOD = 0.f;
		desc.MaxLOD = D3D11_FLOAT32_MAX; // NOTE(jesus): A large value ensures using all levels
		g_pd3dDevice->CreateSamplerState(&desc, &g_pTextureSampler);
	}


	/////////////////////////////////////////////////////////////
	// DEFAULT TEXTURES
	/////////////////////////////////////////////////////////////

	unsigned char white[] = { 255, 255, 255, 255 };
	unsigned char black[] = { 0, 0, 0, 255 };
	whitePixel = App->modTextures->loadTexture(white, 1, 1);
	blackPixel = App->modTextures->loadTexture(black, 1, 1);

	if (whitePixel == nullptr || blackPixel == nullptr)
	{
		LOG("ModuleRender::init() - Could not create white and black textures");
		return false;
	}

	return true;
}

bool ModuleRender::postUpdate()
{
	//float clear_color[] = { 0.45f, 0.55f, 0.60f, 1.00f };
	float clear_color[] = { 0.f, 0.f, 0.f, 1.f };
	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
	g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&clear_color);

	int minOrder = -9999;
	int maxOrder = 5000;
	renderScene(minOrder, maxOrder);

	return true;
}

bool ModuleRender::cleanUp()
{
	SAFE_RELEASE(g_pTextureSampler);
	SAFE_RELEASE(g_pDepthStencilState);
	SAFE_RELEASE(g_pRasterizerState);
	SAFE_RELEASE(g_pBlendState);
	SAFE_RELEASE(g_pPixelShaderBlob);
	SAFE_RELEASE(g_pPixelShader);
	SAFE_RELEASE(g_pConstantBuffer);
	SAFE_RELEASE(g_pInputLayout);
	SAFE_RELEASE(g_pVertexShaderBlob);
	SAFE_RELEASE(g_pVertexShader);
	SAFE_RELEASE(g_pVertexBuffer);
	CleanupDeviceD3D();
	return true;
}

void ModuleRender::resizeBuffers(unsigned int width, unsigned int height)
{
	if (g_pSwapChain != nullptr)
	{
		CleanupRenderTarget();
		g_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
		CreateRenderTarget();
	}
}

void ModuleRender::present()
{
	renderScene(5001, 9999);

	//App->modRender->present();
	g_pSwapChain->Present(1, 0); // Present with vsync
	//g_pSwapChain->Present(0, 0); // Present without vsync
}

static int partition(GameObject **objects, int begin, int end)
{
	GameObject *tmp = nullptr;
	int pivot = objects[end]->order;
	int pi = begin;

	for (int i = begin; i < end; ++i)
	{
		if (objects[i]->order < pivot)
		{
			tmp = objects[i];
			objects[i] = objects[pi];
			objects[pi] = tmp;
			pi++;
		}
	}

	tmp = objects[end];
	objects[end] = objects[pi];
	objects[pi] = tmp;

	return pi;
}

static void quicksort(GameObject **objects, int begin, int end)
{
	if (begin < end)
	{
		int pi = partition(objects, begin, end);
		quicksort(objects, begin, pi - 1);
		quicksort(objects, pi + 1, end);
	}
}

static bool needsSorting(GameObject *objects[MAX_GAME_OBJECTS], int numElems)
{
	for (int i = 0; i < numElems - 1; ++i)
	{
		if (objects[i]->order > objects[i + 1]->order)
		{
			return true;
		}
	}
	return false;
}

static void selectAndSortObjects(GameObject *toSort[MAX_GAME_OBJECTS], GameObject *result[MAX_GAME_OBJECTS], int *numElems)
{
	*numElems = 0;
	for (int i = 0; i < MAX_GAME_OBJECTS; ++i)
	{
		if (toSort[i] != nullptr) // && toSort[i]->scene->enabled)
		{
			result[*numElems] = toSort[i];
			(*numElems)++;
		}
	}

	if (needsSorting(result, *numElems))
	{
		quicksort(result, 0, *numElems - 1);
	}
}

void ModuleRender::renderScene(int minOrder, int maxOrder)
{
	ID3D11DeviceContext *ctx = g_pd3dDeviceContext;

	// Setup viewport
	D3D11_VIEWPORT vp = {};
	RECT rect;
	::GetClientRect(hwnd, &rect);
	vp.Width = (float)(rect.right - rect.left);
	vp.Height = (float)(rect.bottom - rect.top);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = vp.TopLeftY = 0;
	if (vp.Width <= 0.0f || vp.Height <= 0.0f)
	{
		return;
	}
	ctx->RSSetViewports(1, &vp);

	// Setup shader and vertex buffers
	unsigned int stride = sizeof(CUSTOMVERTEX);
	unsigned int offset = 0;
	ctx->IASetInputLayout(g_pInputLayout);
	ctx->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ctx->VSSetShader(g_pVertexShader, NULL, 0);
	ctx->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	ctx->PSSetShader(g_pPixelShader, NULL, 0);
	ctx->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	ctx->PSSetSamplers(0, 1, &g_pTextureSampler);
	ctx->GSSetShader(NULL, NULL, 0);
	ctx->HSSetShader(NULL, NULL, 0); // In theory we should backup and restore this as well.. very infrequently used..
	ctx->DSSetShader(NULL, NULL, 0); // In theory we should backup and restore this as well.. very infrequently used..
	ctx->CSSetShader(NULL, NULL, 0); // In theory we should backup and restore this as well.. very infrequently used..

	// Setup blend state
	const float blend_factor[4] = { 1.f, 1.f, 1.f, 1.f };
	ctx->OMSetBlendState(g_pBlendState, blend_factor, 0xffffffff);
	ctx->OMSetDepthStencilState(g_pDepthStencilState, 0);
	ctx->RSSetState(g_pRasterizerState);


	// Render game objects
	int numObjects = 0;
	selectAndSortObjects(App->modGameObject->gameObjects, orderedGameObjects, &numObjects);

	for (int i = 0; i < numObjects; ++i)
	{
		GameObject *gameObject = orderedGameObjects[i];

		if (gameObject->order < minOrder || gameObject->order > maxOrder) continue;

		// Setup matrices into our constant buffer
		{
			// Projection matrix
			float L = 0;
			float R = vp.Width;
			float T = 0;
			float B = vp.Height;
			XMMATRIX ProjectionMatrix = ::XMMatrixOrthographicOffCenterRH(L, R, B, T, -1, 1);

			// View matrix
			XMMATRIX ViewMatrix = ::XMMatrixTranslation(R*0.5f, B*0.5f, 0.0f);

			// World matrix
			float texture_width = gameObject->texture ? gameObject->texture->width : 100.0f;
			float texture_height = gameObject->texture ? gameObject->texture->height : 100.0f;
			float sx = gameObject->width == 0.0f ? texture_width : gameObject->width;
			float sy = gameObject->height == 0.0f ? texture_height : gameObject->height;
			float x = gameObject->x;
			float y = gameObject->y;
			float a = gameObject->angle;
			XMMATRIX WorldMatrix =
				::XMMatrixTranslation(0.5f - gameObject->pivot_x,
					0.5f - gameObject->pivot_y,
					0.0f) *
				::XMMatrixScaling(sx, sy, 1.0f) *
				::XMMatrixRotationZ(::XMConvertToRadians(a)) *
				::XMMatrixTranslation(x, y, 0.0f);

			// Copy matrices into the constant buffer
			D3D11_MAPPED_SUBRESOURCE mapped_resource;
			if (ctx->Map(g_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource) != S_OK) {
				LOG("d3d->Map() failed (CONSTANT BUFFER)");
				return;
			}
			CONSTANT_BUFFER* constant_buffer = (CONSTANT_BUFFER*)mapped_resource.pData;
			memcpy(&constant_buffer->ProjectionMatrix, &ProjectionMatrix, sizeof(ProjectionMatrix));
			memcpy(&constant_buffer->ViewMatrix, &ViewMatrix, sizeof(ViewMatrix));
			memcpy(&constant_buffer->WorldMatrix, &WorldMatrix, sizeof(WorldMatrix));
			memcpy(&constant_buffer->TintColor, gameObject->color, sizeof(gameObject->color));
			ctx->Unmap(g_pConstantBuffer, 0);
		}

		// Pass texture to shader
		Texture *texture = gameObject->texture ? gameObject->texture : whitePixel;
		ID3D11ShaderResourceView* texture_srv = (ID3D11ShaderResourceView*)texture->shaderResource;
		ctx->PSSetShaderResources(0, 1, &texture_srv);

		ctx->Draw(6, 0);
	}
}

bool ModuleRender::CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
	{
		LOG("ModuleRender::CreateDeviceD3D() failed");
		return false;
	}

	CreateRenderTarget();

	return true;
}

void ModuleRender::CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void ModuleRender::CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void ModuleRender::CleanupRenderTarget()
{
	if (g_mainRenderTargetView)
	{
		g_mainRenderTargetView->Release();
		g_mainRenderTargetView = NULL;
	}
}
