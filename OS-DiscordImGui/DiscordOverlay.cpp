#include "DiscordOverlay.h"

/****************************************************
* Copyright (C)	: Liv
* @file			: DiscordOverlay.cpp
* @author		: Liv
* @email		: 1319923129@qq.com
* @version		: 1.0
* @date			: 2024/3/31 13:35
****************************************************/

bool DiscordOverlay::DiscordMapCommunication::Init(DWORD DestProcessID)
{
    std::string MapFileName = "DiscordOverlay_Framebuffer_Memory_" + std::to_string(DestProcessID);

    this->hMap = OpenFileMappingA(FILE_MAP_ALL_ACCESS, false, MapFileName.c_str());

    if (!this->hMap)
        return false;

    this->pData = static_cast<DiscordOverlay::DiscordOverlayData*>(MapViewOfFile(this->hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0));

    return this->pData;
}

void DiscordOverlay::DiscordMapCommunication::Stop()
{
    UnmapViewOfFile(this->hMap);

    this->pData = nullptr;

    CloseHandle(this->hMap);

    this->hMap = NULL;
}

void DiscordOverlay::DiscordMapCommunication::SendBuffer(ID3D11DeviceContext* pd3dDeviceContext,IDXGISwapChain* pSwapChain, Vec2 Size)
{
    if (!this->hMap)
        return;

    if (!pd3dDeviceContext || !pSwapChain)
        return;

	ID3D11Device* pDevice;
    ID3D11DeviceContext* pContext;

    pSwapChain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&pDevice));

    pDevice->GetImmediateContext(&pContext);

    ID3D11Texture2D* pBackBuffer;
    pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));

    D3D11_TEXTURE2D_DESC TextureDesc;
    pBackBuffer->GetDesc(&TextureDesc);

    TextureDesc.BindFlags = 0;
    TextureDesc.Usage = D3D11_USAGE_STAGING;
    TextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    ID3D11Texture2D* pBackBufferStaging;
    pDevice->CreateTexture2D(&TextureDesc, nullptr, &pBackBufferStaging);
    pContext->CopyResource(pBackBufferStaging, pBackBuffer);

    D3D11_MAPPED_SUBRESOURCE Resource;
    pd3dDeviceContext->Map(pBackBufferStaging, NULL, D3D11_MAP_READ, NULL, &Resource);

    this->pData->Width = static_cast<UINT>(Size.x);
    this->pData->Height = static_cast<UINT>(Size.y);

    memcpy_s(this->pData->Buffer, Size.y * Resource.RowPitch, Resource.pData, Size.y * Resource.RowPitch);

    this->pData->FrameCount++;

    pd3dDeviceContext->Unmap(pBackBufferStaging, 0);

    pBackBufferStaging->Release();

    pBackBuffer->Release();	
}