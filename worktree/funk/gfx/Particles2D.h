#ifndef _INCLUDE_PARTICLES_2D_H_
#define _INCLUDE_PARTICLES_2D_H_

#include <gm/gmBindHeader.h>
#include <common/HandledObj.h>
#include <common/Timer.h>
#include <math/v2.h>
#include <math/v3.h>
#include <gfx/Vbo.h>

namespace funk
{
	class Particles2d : public HandledObj<Particles2d>
	{
	public:

		// guess the max ammount of verts you need
		// will resize (slow) if reached max
		Particles2d( int numVertsMax = 512 );
		~Particles2d();

		void Update( float dt );
		void Draw();
		void Emit( int numParticles );
		int NumParticles() { return m_numParticles; }
		int NumParticlesMax() { return m_maxNumParticles; }

		float UpdateTime() const { return m_updateTime; }

		struct Properties 
		{
			// TODO: handle emitting inwards or outwards from circle
			// spawn
			v2 pos;
			v2 posVariance; // box dimensions
			float speed;
			float speedVariance;  // multiplier by [0,1]
			float speedDampingCoeff; // 1.0 = not-damping, 0.0f = not damping
			float angle;
			float angleVariance;  // add by [-PI,PI]
			v2 accel;

			// emission
			float cooldown;
			int numParticlesPerEmission; // if set to ZERO, won't emit

			// lifespan
			float lifeSpan;
			float lifeSpanVariance;  // multiplier by [0,1]

			// rotation
			float rotStart;
			float rotStartVariance; // add by [-PI,PI]
			float rotVel;
			float rotVelVariance;	// add by [-PI, PI]
			float rotAccel;
			float rotDampingCoeff; // 1.0 = not-damping, 0.0f = not damping

			// scale
			v2 scaleStart;
			v2 scaleEnd;
			float scaleStartVariance; // multiplier by [0,1]
			float scaleEndVariance;

			// color
			funk::v3 colorStart;
			funk::v3 colorEnd;
			funk::v3 colorStartVariance; // width of variance
			funk::v3 colorEndVariance; // width of variance
			float alphaStart;
			float alphaEnd;
			float alphaStartVariance;  // multiplier by [0,1]
			float alphaEndVariance;
			
			// blending
			unsigned int blendSrc;
			unsigned int blendDest;
			unsigned int blendEq;
		};

		Properties &Prop() { return m_prop; }

		struct Particle2D
		{
			v2 pos;
			v2 vel;

			v2 scaleStart;
			v2 scaleEnd;

			float rot;
			float rotVel;

			float lifeSpan;
			float lifeSpanMax;

			funk::v3 colorStart;
			funk::v3 colorEnd;

			float alphaStart;
			float alphaEnd;
		};

		Particle2D & Particle(int i) { assert(i<NumParticles()); return m_particles[i]; }

		GM_BIND_TYPEID(Particles2d);

	private:


		struct Particle2DVert
		{
			v2		uv;
			float	rot;
			v2		pos;
			v2		scale;

			unsigned int color;
		};

		std::vector<Particle2D> m_particles;

		// render
		std::vector<Particle2DVert> m_verts;
		funk::StrongHandle<funk::Vbo> m_vbo;

		// particle properties
		int m_maxNumParticles;
		int m_numParticles;
		float m_cooldownTimer;
		Properties m_prop;
		
		// timing
		Timer m_timer;
		float m_updateTime;

		Particle2D &GetFree() { return m_particles[m_numParticles++]; }
		void InitDefaultProperties();
		void Init();
		void UpdateVbo();
		void UpdateParticles(float dt);
		void CleanUpAndBuildVerts( int index, int numElems );
		void ShaderBegin();
		void ShaderEnd();
		void EmitInternal( int numParticles );
	};

	GM_BIND_DECL( Particles2d );
}
#endif