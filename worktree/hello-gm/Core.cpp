#include "Core.h"

#include <SDL.h>
#include <gl/glew.h>

#include <common/ResourcePath.h>
#include <common/IniReader.h>
#include <common/Debug.h>
#include <common/Timer.h>
#include <common/Input.h>
#include <common/Window.h>
#include <imgui/Imgui.h>
#include <imgui/ImguiManager.h>
#include <sound/SoundMngr.h>
#include <vm/VirtualMachine.h>
#include <gfx/Renderer.h>
#include <gfx/TextureManager.h>
#include <gfx/GpuTimer.h>

namespace funk
{
Core::Core()
	: m_gameRunning(true)
    , m_msGpu(0.0f)
    , m_showAnalyticsGui(false)
{;}

void Core::Init()
{
	Window::CreateInst();

	printf("by Eddie (illogictree.com)\n");

	Renderer::CreateInst();
	TextureManager::CreateInst();
	SoundMngr::CreateInst();
	Input::CreateInst();
	ImguiManager::CreateInst();
	VirtualMachine::CreateInst();

	IniReader reader( RESOURCE_PATH("common/ini/main.ini") );
	m_fps = reader.GetInt( "Window", "FPS" );
	m_showAnalyticsGui = reader.GetInt( "Window", "ShowAnalytics" ) == 1;

	Timer initTimer;
	printf("Initializing...\n");
	BaseEntityGroup::Init();
	printf("initialized in %f secs!\n", initTimer.GetTimeMs() );

	InitGuiGraphs();
}

void Core::HandleArgs( int argc, char ** argv )
{

}

void Core::Deinit()
{
	VirtualMachine::DestroyInst();
	BaseEntityGroup::Deinit();

	ImguiManager::DestroyInst();
	TextureManager::DestroyInst();
	Input::DestroyInst();
	SoundMngr::DestroyInst();
	
	Window::DestroyInst();
}

void Core::Run()
{
	VirtualMachine::Get()->RunMain();

	const float secPerFrame = 1.0f / float( m_fps );
	uint32_t msLimit = uint32_t(secPerFrame * 1000.0f);
	uint32_t msBeg = SDL_GetTicks();

	while (m_gameRunning)
	{
		// Calculate change of time
		uint32_t startMs = SDL_GetTicks();
		uint32_t deltaTicks = startMs - msBeg;
		float dt = float(deltaTicks) / 1000.0f;

		Timer timer;
		{
			HandleInputs();
			Update( dt );
			Render();
			ImguiManager::Get()->CleanUp();
		}
		m_msTotal = timer.GetTimeMs();
		m_fpsFrame = 1000.0f / m_msTotal;

		timer.Start();
		Renderer::Get()->Present();

		// Handle waiting so that frame ends correctly
		uint32_t endMs = SDL_GetTicks();
		uint32_t diffMs = endMs-startMs;
		uint32_t msLeftInFrame = msLimit - diffMs;
		while( msLimit > diffMs && SDL_GetTicks() - endMs < msLeftInFrame ) {;}
		msBeg = startMs;
	}	
}

void Core::HandleInputs()
{
	SDL_EnableUNICODE(1); 
	SDL_Event event = {0};

	if (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
		{
			m_gameRunning = false;
		}
	}

	Input::Get()->Update();
	Input::Get()->HandleEvent(event);

	// do not commit, kalin mode
	if ( Input::Get()->DidKeyJustGoDown("F4") && Input::Get()->IsKeyDown("LALT") )
	//if ( Input::Get()->DidKeyJustGoDown("ESCAPE") )
	{
		m_gameRunning = false;
	}
}

void Core::Update( float dt )
{
	Timer timer;

	VirtualMachine::Get()->Update();
	BaseEntityGroup::Update( dt );
	SoundMngr::Get()->Update();
	Renderer::Get()->Update();

	m_msUpdate = timer.GetTimeMs();
}

void Core::Render()
{
#ifndef FNK_FINAL
	static GpuTimer gpuTimer;
	if ( m_showAnalyticsGui ) 
	{
		gpuTimer.Begin();
	}
#endif

	Timer timer;
	{
		BaseEntityGroup::Render();
		VirtualMachine::Get()->Render();	
	}
	m_msRender = timer.GetTimeMs();
	VirtualMachine::Get()->Gui();

	if ( m_showAnalyticsGui ) 
	{
		GuiStats();

#ifndef FNK_FINAL
		gpuTimer.End();
		m_msGpu = gpuTimer.GetTimeMs();
#endif
	}
}

void Core::GuiStats()
{
	const int padding = 5;
	const v2i windowDimen = Window::Get()->Sizei();
	const float deltaMs = 1000.0f / m_fps;

	Imgui::Begin("Analytics", v2i(padding, windowDimen.y-padding) );
	Imgui::Minimize();
	Imgui::FillBarFloat("Total", m_msUpdate+m_msRender, 0.0f, deltaMs );
	Imgui::FillBarFloat("Update", m_msUpdate, 0.0f, deltaMs );
	Imgui::FillBarFloat("Render", m_msRender, 0.0f, deltaMs );
	Imgui::FillBarFloat("GPU", m_msGpu, 0.0f, deltaMs );
	Imgui::CheckBox("Show Graphs", m_showGraphsGui );
	VirtualMachine::Get()->GuiStats();
	SoundMngr::Get()->GuiStats();
	Imgui::End();	

	if ( m_showGraphsGui ) 
	{
		UpdateGuiGraphs();
		GuiGraphs();
	}
 }

void Core::InitGuiGraphs()
{
	m_showGraphsGui = false;

	const float minVal = 0.0f;
	const float maxVal = 1000.0f / m_fps;
	const int width = 200;
	const int height = 100;
	const int numVals = 128;

	m_lineGraphRender = new LineGraph( minVal, maxVal, v2i(width, height), numVals );
	m_lineGraphUpdate = new LineGraph( minVal, maxVal, v2i(width, height), numVals );
	m_lineGraphTotal = new LineGraph( minVal, maxVal, v2i(width, height), numVals );
	m_lineGraphGpu = new LineGraph( minVal, maxVal, v2i(width, height), numVals );
}

void Core::UpdateGuiGraphs()
{
	m_lineGraphRender->PushVal(m_msRender);
	m_lineGraphUpdate->PushVal(m_msUpdate);
	m_lineGraphTotal->PushVal(m_msUpdate+m_msRender);
	m_lineGraphGpu->PushVal(m_msGpu);
}

void Core::GuiGraphs()
{
	const v2i pos = v2i(300, Window::Get()->Sizei().y - 20 );

	Imgui::Begin("Main Analytics", pos );
	Imgui::Print("Total");
	Imgui::LineGraph(m_lineGraphTotal);
	Imgui::Print("Update");
	Imgui::LineGraph(m_lineGraphUpdate);
	Imgui::Print("Render");
	Imgui::LineGraph(m_lineGraphRender);
	Imgui::Print("GPU");
	Imgui::LineGraph(m_lineGraphGpu);
	Imgui::End();
}
} // namespace funk