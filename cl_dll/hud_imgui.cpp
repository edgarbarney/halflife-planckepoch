// imgui main file
// Base is made by Admer

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "pm_shared.h"
#include "SDL2/SDL.h"
#include <gl/GL.h>

#include "imgui.h"
#include "backends/imgui_impl_opengl2.h"
#include "backends/imgui_impl_sdl.h"

SDL_Window* mainWindow;
SDL_GLContext mainContext;

void CClientImgui::InitExtension()
{
	mainWindow = SDL_GetWindowFromID( 1 );
	//mainContext = SDL_GL_CreateContext(mainWindow);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplOpenGL2_Init();
	ImGui_ImplSDL2_InitForOpenGL(mainWindow, ImGui::GetCurrentContext());
	io.DisplaySize.x = ScreenWidth;
	io.DisplaySize.y = ScreenHeight;
}

void CClientImgui::DrawImgui()
{
	static float angle = 0;
	angle += M_PI * 2.0 / 180;

	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplSDL2_NewFrame(mainWindow);
	ImGui::NewFrame();

	ImGui::SetNextWindowSize(ImVec2(200 + cos(angle) * 50, 200 + sin(angle) * 50));

	ImGui::Begin("Test");

	ImGui::Text("uwu");
	ImGui::Text("Nyaaaa!!!");
	ImGui::Button("OwO");

	ImGui::End();

	//glViewport( 0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y );

	ImGui::Render();
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}