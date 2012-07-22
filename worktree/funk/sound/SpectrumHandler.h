#ifndef _INCLUDE_SPECTRUM_HANDLER_H
#define _INCLUDE_SPECTRUM_HANDLER_H

#include "Sound.h"

#include <common/StrongHandle.h>
#include <gfx/Cam2d.h>

#include <string>
#include <map>


namespace funk
{
class SpectrumHandler
{
public:
	SpectrumHandler();
	~SpectrumHandler();

	void Init( const char* file, int numBandsRead = 128, float percentFreqUsed = 0.25f );
	void Update();
	void Render(); // renders spectrum on screen
	void Gui();
	void Add( const char* title, float defaultVal, float min, float max, int band = -1, float scaleCoeff = 1.0f );

	float Get( const char * title ) const;
	inline StrongHandle<Sound> GetSnd() const { return m_snd; }

private:

	void RenderSpectrum();
	void UpdateSpectrum();
	void UpdateDatum();

	struct Datum
	{
		float	val;
		float	defaultVal;
		float	min;
		float	max;

		int		band;
		float	scaleCoeff;
	};

	std::map< std::string, Datum > m_data;
	typedef std::map< std::string, Datum >::iterator DataIter;
	typedef std::map< std::string, Datum >::const_iterator ConstDataIter;

	StrongHandle<Sound> m_snd;

	// spectrum data
	int		m_numBands;
	int		m_numBandsToWatch;
	float*  m_songSpectrumLeft;
	float*  m_songSpectrumRight;
	float*	m_songSpectrum;
	float*	m_songSpectrumMax;

	// global data
	float m_spectrumLerpSpeed;
	float m_songPercent;
	float m_volume;
	float m_spectrumBarSize;

	Cam2d	m_cam2d;
};
}
#endif