#include "SpectrumHandler.h"

#include <gl/glew.h>
#include <assert.h>

#include "SoundMngr.h"

#include <math/Util.h>
#include <common/Window.h>
#include <gfx/Renderer.h>
#include <imgui/Imgui.h>

namespace funk
{
SpectrumHandler::SpectrumHandler()
: m_songSpectrumLeft(0), m_songSpectrumRight(0), m_songSpectrum(0), m_songSpectrumMax(0)
{;}

SpectrumHandler::~SpectrumHandler()
{
	if ( m_songSpectrumLeft ) delete m_songSpectrumLeft;
	if ( m_songSpectrumRight ) delete m_songSpectrumRight;
	if ( m_songSpectrum ) delete m_songSpectrum;
	if ( m_songSpectrumMax ) delete m_songSpectrumMax;
}

void SpectrumHandler::Init( const char* file, int numBands, float percentFreqUsed )
{
	assert( numBands > 0 );
	assert( percentFreqUsed > 0 && percentFreqUsed <= 1.0f );

	m_numBands = numBands;
	m_numBandsToWatch = int(percentFreqUsed * numBands);

	m_songSpectrumLeft = new float[m_numBands];
	m_songSpectrumRight = new float[m_numBands];
	m_songSpectrum = new float[m_numBands];
	m_songSpectrumMax = new float[m_numBands];

	m_volume = 1.0f;
	m_spectrumLerpSpeed = 0.15f;
	m_songPercent = 0;
	m_spectrumBarSize = 0.5f;

	memset( m_songSpectrum, 0, m_numBands * sizeof(float) );
	memset( m_songSpectrumMax, 0, m_numBands * sizeof(float) );

	m_snd = SoundMngr::Get()->GetSound(file);

	// camera
	const v2 windowDimen = Window::Get()->Sizef();
	m_cam2d.SetNearFar( v2(0.0f, 100.0f) );
	m_cam2d.SetBounds( v2(-0.5f * windowDimen.x, -0.5f*windowDimen.y), v2( 0.5f * windowDimen.x, 0.5f*windowDimen.y) );
	m_cam2d.SetPos( v3( 0.0f, 0.0f, 10.0f ) );
	m_cam2d.SetLookAt( v3( 0.0f, 0.0f, 0.0f ) );
}

void SpectrumHandler::Add( const char* title, float val, float min, float max, int band /*= -1*/, float scaleCoeff /*= 1.0f */ )
{
	m_data[ title ] = Datum();

	Datum &datum = m_data[title];
	datum.val = datum.defaultVal = val;
	datum.min = min;
	datum.max = max;
	datum.band = band;
	datum.scaleCoeff = scaleCoeff;
}

float SpectrumHandler::Get( const char * title ) const
{
	ConstDataIter it =  m_data.find(title);
	assert ( it != m_data.end() );

	const Datum & d = it->second;
	if ( d.band == -1 ) return d.defaultVal;
	return d.val;
}

void SpectrumHandler::Update()
{
	if ( !m_snd.IsNull() ) 
	{
		m_snd->SetVolume( m_volume );

		float currPos = (float)m_snd->GetCurrPosMillisec() / (float)m_snd->GetTotalLenMillisec();
		m_songPercent = currPos;
	}

	UpdateSpectrum();
	UpdateDatum();
}

void SpectrumHandler::UpdateSpectrum()
{
	m_snd->GetSpectrum( m_songSpectrumLeft, m_numBands );
	m_snd->GetSpectrum( m_songSpectrumRight, m_numBands );

	for ( int i = 0; i < m_numBands; ++i )
	{
		float newSpecData = (m_songSpectrumLeft[i]+m_songSpectrumRight[i])*0.5f;
		if ( newSpecData > m_songSpectrumMax[i] )  m_songSpectrumMax[i] = newSpecData;
		m_songSpectrum[i] = lerp( m_songSpectrum[i], newSpecData, m_spectrumLerpSpeed );	
	}
}

void SpectrumHandler::UpdateDatum()
{
	for ( DataIter it = m_data.begin(); it != m_data.end(); ++it )
	{
		Datum & datum = it->second;
		const std::string & title = it->first;

		if ( datum.band >= 0 )
		{
			float freqCoeff = datum.scaleCoeff * m_songSpectrum[ datum.band ] / m_songSpectrumMax[ datum.band ];
			float targetVal =  datum.min + (datum.max - datum.min ) * freqCoeff;
			datum.val = clamp( targetVal, datum.min, datum.max );
		}
		else if ( datum.band == -1 )
		{
			datum.val = datum.defaultVal;
		}
	}
}

void SpectrumHandler::Render()
{
	m_cam2d.Begin();
	glMatrixMode( GL_MODELVIEW );
	Renderer::Get()->BeginDefaultShader();
	RenderSpectrum();
	Renderer::Get()->EndDefaultShader();
	m_cam2d.End();
}

void SpectrumHandler::RenderSpectrum()
{
	glDisable( GL_TEXTURE_2D );
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA );
	glEnable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );


	const v2 windowDimen = Window::Get()->Sizef();
	const float min_x = -windowDimen.x * 0.5f * m_spectrumBarSize;
	const float max_x = windowDimen.x * 0.5f * m_spectrumBarSize;
	const float min_y = -windowDimen.y * 0.5f * m_spectrumBarSize;
	const float max_y = windowDimen.y * 0.5f * m_spectrumBarSize;
	const float delta_x = (max_x - min_x ) / m_numBandsToWatch;

	// draw spectrum
	glBegin(GL_QUADS);

	// background
	glColor4f( 0, 0, 0, 0.35f );
	glVertex2f(min_x, min_y);
	glVertex2f(min_x, max_y);
	glVertex2f(max_x, max_y);
	glVertex2f(max_x, min_y);

	for ( int i = 0; i < m_numBandsToWatch; ++i )
	{
		const float freqCoeff = m_songSpectrum[i] / m_songSpectrumMax[i];

		glColor3f( 0, 0, 1 );

		for ( DataIter it = m_data.begin(); it != m_data.end(); ++it )
		{
			if ( it->second.band == i )
			{
				glColor3f( 243.0f/255.0f, 105.0f/255.0f, 0 );
			}
		}

		float x0 = min_x + i * delta_x;
		float x1 = x0 + delta_x;

		float y0 = min_y;
		float y1 = min_y + ( max_y - min_y ) * freqCoeff;

		glVertex2f( x0, y0 );
		glVertex2f( x0, y1 );
		glVertex2f( x1, y1 );
		glVertex2f( x1, y0 );
	}
	glEnd();

	// draw surrounding box
	glColor4f( 1, 1, 1, 0.5f );
	glBegin(GL_LINE_LOOP);
	glVertex2f( min_x, min_y );
	glVertex2f( min_x, max_y );
	glVertex2f( max_x, max_y );
	glVertex2f( max_x, min_y );
	glEnd();

	// draw separation lines
	const int kDrawSpectrumLineDelta = 4;
	glBegin( GL_LINES );
	for ( int i = 0; i < m_numBandsToWatch/kDrawSpectrumLineDelta; ++i )
	{
		float x = min_x + i * kDrawSpectrumLineDelta  * delta_x;
		glVertex2f(x, min_y);
		glVertex2f(x, max_y);
	}

	// draw horizontal lines
	const int kNumHorizontalSegments = 4;
	const float deltaHorizontal = 1.0f / kNumHorizontalSegments;
	for ( int i = 0; i <kNumHorizontalSegments; ++i )
	{
		float y = i * deltaHorizontal * ( max_y - min_y ) + min_y;

		glVertex2f( min_x, y );
		glVertex2f( max_x, y );
	}

	glEnd();
}

void SpectrumHandler::Gui()
{
	Imgui::Begin( "Spectrum" );

	Imgui::FillBarFloat( "Song Duration", m_songPercent, 0.0f, 1.0f );
	Imgui::SliderFloat( "Freq Lerp", m_spectrumLerpSpeed, 0.01f, 1.0f );
	Imgui::SliderFloat( "Song Volume", m_volume, 0.0f, 1.0f );
	Imgui::SliderFloat( "Spectrum Size", m_spectrumBarSize, 0.1f, 1.0f );

	// output check element
	for( DataIter it = m_data.begin(); it != m_data.end(); ++it )
	{
		Datum &datum = it->second;
		const char* title = it->first.c_str();

		// insert into GUI
		Imgui::SliderInt(title, (datum.band), -1, m_numBandsToWatch-1 );
		Imgui::SliderFloat(title, (datum.val), datum.min, datum.max );
		Imgui::SliderFloat(title, (datum.scaleCoeff), 0.1f, 20.0f );
	}

	Imgui::End();
}
}