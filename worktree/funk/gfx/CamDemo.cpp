#include "CamDemo.h"

#include <common/Window.h>
#include <imgui/Imgui.h>

namespace funk
{

void CamDemo::Init()
{
	const v2 windowDimen = Window::Get()->Sizef();

	m_cam.SetNearFar( v2(1.0f, 1000.0f) );
	m_cam.SetPos( v3( 0.0f, 0.0f, 30.0f ) );
	m_cam.SetLookAt( v3( 0.0f, 5.0f, 0.0f ) );

	m_aspect = windowDimen.x / windowDimen.y;
	m_camHeight = 7.5f;
	m_camRadius = 75.0f;
	m_fov = 60.0f;
	m_camAngleRad = 0.0f;
	m_focusY = 0.0f;
	m_autoSpinCamera = 0;
	m_totalTime = 0.0f;
}

void CamDemo::Begin()
{
	m_cam.Begin();
}

void CamDemo::End()
{
	m_cam.End();
}

void CamDemo::Update( float dt )
{
	m_totalTime += dt;

	float angle = m_camAngleRad;

	if ( m_autoSpinCamera )
	{
		angle = m_totalTime * 0.1f;
	}

	m_cam.SetViewAngle( m_fov, m_aspect );
	m_cam.SetPos( v3( m_camRadius*sinf(angle), m_camHeight, m_camRadius*cosf(angle) ) );
	m_cam.SetLookAt( v3( 0.0f, m_focusY, 0.0f ) );
}

void CamDemo::Gui()
{
	Imgui::Begin("DEMO CAMERA");

	Imgui::SliderFloat( "Camera Y", m_camHeight, 0.0f, 60.0f );
	Imgui::SliderFloat( "Camera Radius", m_camRadius, 10.0f, 300.0f );
	Imgui::SliderFloat( "Camera FOV", m_fov, 10.0f, 120.0f );
	Imgui::SliderFloat( "Camera Angle", m_camAngleRad, -2.0f * 3.14159f, 2.0f * 3.14159f );
	Imgui::SliderFloat( "Camera Focus Y", m_focusY, -100.0f, 100.0f );
	Imgui::CheckBox( "Auto-spin Camera", m_autoSpinCamera );

	Imgui::End();
}
}