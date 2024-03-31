#pragma once
#include "OS-ImGui_Base.h"

/****************************************************
* Copyright (C)	: Liv
* @file			: OS-ImGui_External.h
* @author		: Liv
* @email		: 1319923129@qq.com
* @version		: 1.0
* @date			: 2024/3/31 12:54
****************************************************/

namespace OSImGui
{
	class OSImGui_External : public OSImGui_Base
	{
	private:
		// ��������
		WindowType Type = NEW;
	public:
		// ���ӵ���һ��������
		void AttachAnotherWindow(std::string DestWindowName, std::string DestWindowClassName, std::function<void()> CallBack);
	private:
		void MainLoop();
		bool UpdateWindowData();
		bool CreateDevice();
		bool PeekEndMessage();
	};
}