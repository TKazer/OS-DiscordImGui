#include <iostream>
#include "DiscordOverlay.h"

/****************************************************
* Copyright (C)	: Liv
* @file			: demo.cpp
* @author		: Liv
* @email		: 1319923129@qq.com
* @version		: 1.0
* @date			: 2024/3/31 13:35
****************************************************/

void Draw()
{
	OSImGui::OSImGui::get().Text("Discord overlay", { 0,0 }, ImColor(255, 255, 255, 255), 20);
	OSImGui::OSImGui::get().Rectangle({ 50,50 }, { 50,50 }, ImColor(255, 255, 255, 255), 2, 0);

	ImGui::Begin("Menu", 0, ImGuiWindowFlags_AlwaysAutoResize);
	{
		ImGui::Text("This is a Discord Overlay test.");
	}ImGui::End();
}

int main()
{
	try
	{
		OSImGui::OSImGui::get().AttachAnotherWindow("ShootHouse", "", Draw);
	}
	catch (OSImGui::OSException& e)
	{
		std::cout << e.what() << std::endl;
	}

	system("pause");
	return 0;
}