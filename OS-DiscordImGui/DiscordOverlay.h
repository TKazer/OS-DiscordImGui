#pragma once
#include <iostream>
#include <Windows.h>
#include <string>
#include "OS-ImGui.h"

/****************************************************
* Copyright (C)	: Liv
* @file			: DiscordOverlay.h
* @author		: Liv
* @email		: 1319923129@qq.com
* @version		: 1.0
* @date			: 2024/3/31 13:35
****************************************************/

namespace DiscordOverlay
{
	struct DiscordOverlayData
	{
		UINT Magic;
		UINT FrameCount;
		UINT NoClue;
		UINT Width;
		UINT Height;
		BYTE Buffer[1];
	};

	class DiscordMapCommunication
	{
	public:
		bool Init(DWORD DestProcessID);

		void Stop();

		void SendBuffer(ID3D11DeviceContext* pd3dDeviceContext, IDXGISwapChain* pSwapChain, Vec2 Size);
	public:
		DiscordOverlayData* pData;
		HANDLE hMap;
	};

	inline DiscordMapCommunication DiscordCom;
}

