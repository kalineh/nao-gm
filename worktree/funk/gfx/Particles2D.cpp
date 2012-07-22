#include "Particles2d.h"

#include <gl/glew.h>
#include <assert.h>

#include <gm/gmBind.h>
#include <gfx/Renderer.h>
#include <common/Window.h>

namespace funk
{
const int NUM_VERTS_PER_PARTICLE = 4;
const float PI = 3.14159f;

Particles2d::Particles2d( int numVertsMax ) : m_maxNumParticles(numVertsMax), m_cooldownTimer(0.0f)
{
	InitDefaultProperties();
	Init();
}

void Particles2d::Update( float dt )
{
	m_timer.Start();

	// emission
	m_cooldownTimer -= dt;
	if ( m_cooldownTimer <= 0.0f )
	{
		m_cooldownTimer = m_prop.cooldown;
		EmitInternal( m_prop.numParticlesPerEmission );
	}

	UpdateParticles(dt);
	CleanUpAndBuildVerts(0, m_numParticles);

	m_updateTime = m_timer.GetTimeMs();
}

void Particles2d::Draw()
{
	if ( m_numParticles == 0 ) return;
	UpdateVbo();

	// blend start
	glEnable(GL_BLEND);
	glBlendFunc(m_prop.blendSrc, m_prop.blendDest);
	glBlendEquation(m_prop.blendEq);

	// render
	m_vbo->Bind();
	m_vbo->Render(GL_QUADS, 0, m_numParticles*NUM_VERTS_PER_PARTICLE );
	m_vbo->Unbind();

	// blend end
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);
}

void Particles2d::Init()
{
	m_numParticles = 0;
	m_particles.resize( m_maxNumParticles );
	m_verts.resize( m_maxNumParticles * NUM_VERTS_PER_PARTICLE );
	memset( &m_particles[0], 0, sizeof(m_particles[0]) );
	memset( &m_verts[0], 0, sizeof(m_verts[0]) );

	// vertex attribute
	StrongHandle< VertexAttribute > attrib = new VertexAttribute;
	attrib->AddAttrib( VertexAttribute::ATTRIB_VERTEX, VertexAttribute::ATTRIB_FLOAT, 3, sizeof( Particle2DVert ), 0 );
	attrib->AddAttrib( VertexAttribute::ATTRIB_TEXTURE_0, VertexAttribute::ATTRIB_FLOAT, 4, sizeof( Particle2DVert ), offsetof(Particle2DVert, pos) );
	attrib->AddAttrib( VertexAttribute::ATTRIB_COLOR, VertexAttribute::ATTRIB_UNSIGNED_BYTE, 4, sizeof( Particle2DVert ), offsetof(Particle2DVert, color) );

	m_vbo = new Vbo( NULL, m_verts.size(), sizeof(Particle2DVert), attrib, GL_DYNAMIC_DRAW );

	// init uvs for particles
	for ( int i = 0; i < m_maxNumParticles; ++i )
	{
		int vertIndex = i * NUM_VERTS_PER_PARTICLE;
		m_verts[vertIndex+0].uv = v2(-1.0f);
		m_verts[vertIndex+1].uv = v2(1.0f, -1.0f);
		m_verts[vertIndex+2].uv = v2(1.0f);
		m_verts[vertIndex+3].uv = v2(-1.0f, 1.0f);
	}
}

void Particles2d::UpdateVbo()
{
	m_vbo->Bind();
	m_vbo->SubData( (unsigned char*)&m_verts[0], sizeof(Particle2DVert)*m_numParticles*NUM_VERTS_PER_PARTICLE );
	m_vbo->Unbind();
}

void Particles2d::UpdateParticles( float dt )
{
	const v2 accel = m_prop.accel;
	const float rotAccel = m_prop.rotAccel;
	const float rotDampingCoeff = m_prop.rotDampingCoeff;
	const float speedDampingCoeff = m_prop.speedDampingCoeff;	

	// update positions
	for( int i = 0; i < m_numParticles; ++i )
	{
		Particle2D &p = m_particles[i];
		p.lifeSpan += dt;

		// move particle
		p.vel = p.vel*speedDampingCoeff + accel*dt;
		p.rotVel = p.rotVel*rotDampingCoeff+rotAccel*dt;
		p.pos += p.vel*dt;
		p.rot += p.rotVel*dt;
	}
}

void Particles2d::EmitInternal( int numParticles )
{
	// if more than max number of particles
	if ( m_numParticles+numParticles > m_maxNumParticles )
	{
		size_t newSize = m_maxNumParticles*2;
		printf("[Warning] Particle2D effect reached max num particles: %d, wanted: %d. Resizing to: %d\n", m_maxNumParticles, m_numParticles+numParticles, newSize );
		m_maxNumParticles = newSize;

		Init();
	}

	for ( int i = 0; i < numParticles; ++i )
	{
		Particle2D &p = GetFree();

		// spawn position
		p.pos = m_prop.pos + v2( m_prop.posVariance.x*randbetween(-0.5f,0.5f), m_prop.posVariance.y*randbetween(-0.5f,0.5f) );

		// velocity
		float speed = m_prop.speed * randbetween(1.0f-m_prop.speedVariance, 1.0f);
		float angle = m_prop.angle + PI*randbetween(-1.0f, 1.0f) * m_prop.angleVariance; 
		v2 direction = v2( cosf(angle), sinf(angle) );
		p.vel = direction * speed;

		// color
		p.colorStart = saturate(m_prop.colorStart + v3( m_prop.colorStartVariance.x*randbetween(-0.5f,0.5f), m_prop.colorStartVariance.y*randbetween(-0.5f,0.5f), m_prop.colorStartVariance.z*randbetween(-0.5f,0.5f) ) );
		p.colorEnd =  saturate(m_prop.colorEnd + v3( m_prop.colorEndVariance.x*randbetween(-0.5f,0.5f), m_prop.colorEndVariance.y*randbetween(-0.5f,0.5f), m_prop.colorEndVariance.z*randbetween(-0.5f,0.5f) ) );
		p.alphaStart = m_prop.alphaStart * randbetween(1.0f-m_prop.alphaStartVariance, 1.0f);
		p.alphaEnd = m_prop.alphaEnd * randbetween(1.0f-m_prop.alphaEndVariance, 1.0f);

		p.lifeSpan = 0.0f;
		p.lifeSpanMax = m_prop.lifeSpan * randbetween(1.0f-m_prop.lifeSpanVariance, 1.0f);

		p.rot = m_prop.rotStart + PI*randbetween(-1.0f, 1.0f) * m_prop.rotStartVariance;
		p.rotVel = m_prop.rotVel + PI*randbetween(-1.0f, 1.0f) * m_prop.rotVelVariance;

		p.scaleStart = m_prop.scaleStart  * randbetween(1.0f-m_prop.scaleStartVariance, 1.0f);
		p.scaleEnd = m_prop.scaleEnd  * randbetween(1.0f-m_prop.scaleEndVariance, 1.0f);
	}
}

void Particles2d::InitDefaultProperties()
{
	Properties & p = Prop();

	p.pos = Window::Get()->Sizef()*0.5f;
	p.posVariance = v2(30.0f);

	p.cooldown = 0.0f;
	p.numParticlesPerEmission = 10;

	p.lifeSpan = 0.5f;
	p.lifeSpanVariance = 0.1f;

	p.accel = 0.0f;
	p.speed = 150.0f;
	p.speedVariance = 0.1f;
	p.speedDampingCoeff = 1.0f;
	p.angle = 0.0f;
	p.angleVariance = 1.0f;

	p.rotStart = 0.0f;
	p.rotStartVariance = 0.1f;
	p.rotVel = 1.0f;
	p.rotVelVariance = 0.1f;
	p.rotDampingCoeff = 1.0f;

	p.scaleStart = v2(20.0f);
	p.scaleEnd = v2(0.0f);
	p.scaleStartVariance = 0.0f;
	p.scaleEndVariance = 0.0f;

	p.colorStart = v3(1.0f, 0.0f, 0.0f);
	p.colorEnd = v3(0.0f, 0.0f, 1.0f);
	p.colorStartVariance = v3(0.0f);
	p.colorEndVariance = v3(0.0f);
	p.alphaStart = 1.0f;
	p.alphaEnd = 0.0f;
	p.alphaStartVariance = 0.0f;
	p.alphaEndVariance = 0.0f;

	p.blendSrc = GL_SRC_ALPHA;
	p.blendDest = GL_ONE_MINUS_SRC_ALPHA;
	p.blendEq = GL_FUNC_ADD;
}

Particles2d::~Particles2d()
{}

void Particles2d::CleanUpAndBuildVerts( int index, int numElems )
{
	// check expired & build verts
	for( int i = index; i < index+numElems; ++i )
	{
		Particle2D &p = m_particles[i];

		// check expired
		if ( p.lifeSpan > p.lifeSpanMax)
		{
			p = m_particles[m_numParticles-1];
			--m_numParticles;
		}

		// create verts
		int vertOffset = i * NUM_VERTS_PER_PARTICLE;

		// lerp vals
		const float lerpVal = p.lifeSpan/p.lifeSpanMax;
		v2 pos = p.pos;
		v2 scale = lerp( p.scaleStart, p.scaleEnd, lerpVal);
		float rot = p.rot;
		v3 color = lerp( p.colorStart, p.colorEnd, lerpVal);
		float alpha = saturate( lerp( p.alphaStart, p.alphaEnd, lerpVal ) );
		unsigned colorUint = MakeColor(color.x, color.y, color.z, alpha );

		for ( int j = 0; j < 4; ++j )
		{
			int vertIndex = vertOffset+j;

			Particle2DVert & v = m_verts[vertIndex];
			v.color = colorUint;
			v.pos = pos;
			v.rot = rot;
			v.scale = scale;
		}
	}
}

void Particles2d::Emit( int numParticles )
{
	int startIndex = m_numParticles;
	EmitInternal(numParticles);
	CleanUpAndBuildVerts(startIndex, numParticles);
}

GM_REG_NAMESPACE(Particles2d)
{
	GM_MEMFUNC_CONSTRUCTOR(Particles2d)
	{
		GM_INT_PARAM( numParticles, 0, 512 );
		GM_PUSH_USER_HANDLED( Particles2d, new Particles2d(numParticles) );
		return GM_OK;
	}

	GM_GEN_MEMFUNC_VOID_VOID(Particles2d, Draw)
	GM_GEN_MEMFUNC_VOID_FLOAT(Particles2d, Update)
	GM_GEN_MEMFUNC_VOID_INT(Particles2d, Emit)
	GM_GEN_MEMFUNC_INT_VOID(Particles2d, NumParticles)
	GM_GEN_MEMFUNC_INT_VOID(Particles2d, NumParticlesMax)
	GM_GEN_MEMFUNC_FLOAT_VOID(Particles2d, UpdateTime)

	GM_SETDOT_FUNC_BEGIN(Particles2d)
		GM_SETDOT_PARAM_VEC2("pos", Prop().pos)
		GM_SETDOT_PARAM_VEC2("posVariance", Prop().posVariance)
		GM_SETDOT_PARAM_FLOAT("speed", Prop().speed)
		GM_SETDOT_PARAM_FLOAT("speedVariance", Prop().speedVariance)
		GM_SETDOT_PARAM_FLOAT("speedDampingCoeff", Prop().speedDampingCoeff)
		GM_SETDOT_PARAM_FLOAT("angle", Prop().angle)
		GM_SETDOT_PARAM_FLOAT("angleVariance", Prop().angleVariance)
		GM_SETDOT_PARAM_VEC2("accel", Prop().accel)
		GM_SETDOT_PARAM_FLOAT("cooldown", Prop().cooldown)
		GM_SETDOT_PARAM_INT("numParticlesPerEmission", Prop().numParticlesPerEmission)
		GM_SETDOT_PARAM_FLOAT("lifeSpan", Prop().lifeSpan)
		GM_SETDOT_PARAM_FLOAT("lifeSpanVariance", Prop().lifeSpanVariance)
		GM_SETDOT_PARAM_FLOAT("rotStart", Prop().rotStart)
		GM_SETDOT_PARAM_FLOAT("rotStartVariance", Prop().rotStartVariance)
		GM_SETDOT_PARAM_FLOAT("rotVel", Prop().rotVel)
		GM_SETDOT_PARAM_FLOAT("rotVelVariance", Prop().rotVelVariance)
		GM_SETDOT_PARAM_FLOAT("rotAccel", Prop().rotAccel)
		GM_SETDOT_PARAM_FLOAT("rotDampingCoeff", Prop().rotDampingCoeff)
		GM_SETDOT_PARAM_VEC2("scaleStart", Prop().scaleStart)
		GM_SETDOT_PARAM_VEC2("scaleEnd", Prop().scaleEnd)
		GM_SETDOT_PARAM_FLOAT("scaleStartVariance", Prop().scaleStartVariance)
		GM_SETDOT_PARAM_FLOAT("scaleEndVariance", Prop().scaleEndVariance)
		GM_SETDOT_PARAM_VEC3("colorStart", Prop().colorStart)
		GM_SETDOT_PARAM_VEC3("colorEnd", Prop().colorEnd)
		GM_SETDOT_PARAM_VEC3("colorStartVariance", Prop().colorStartVariance)
		GM_SETDOT_PARAM_VEC3("colorEndVariance", Prop().colorEndVariance)
		GM_SETDOT_PARAM_FLOAT("alphaStart", Prop().alphaStart)
		GM_SETDOT_PARAM_FLOAT("alphaEnd", Prop().alphaEnd)
		GM_SETDOT_PARAM_FLOAT("alphaStartVariance", Prop().alphaStartVariance)
		GM_SETDOT_PARAM_FLOAT("alphaEndVariance", Prop().alphaEndVariance)
		GM_SETDOT_PARAM_INT("blendSrc", Prop().blendSrc)
		GM_SETDOT_PARAM_INT("blendDest", Prop().blendDest)
		GM_SETDOT_PARAM_INT("blendEq", Prop().blendEq)
	GM_SETDOT_FUNC_END()

	GM_GETDOT_FUNC_BEGIN(Particles2d)
		GM_GETDOT_PARAM_VEC2("pos", Prop().pos)
		GM_GETDOT_PARAM_VEC2("posVariance", Prop().posVariance)
		GM_GETDOT_PARAM_FLOAT("speed", Prop().speed)
		GM_GETDOT_PARAM_FLOAT("speedVariance", Prop().speedVariance)
		GM_GETDOT_PARAM_FLOAT("speedDampingCoeff", Prop().speedDampingCoeff)
		GM_GETDOT_PARAM_FLOAT("angle", Prop().angle)
		GM_GETDOT_PARAM_FLOAT("angleVariance", Prop().angleVariance)
		GM_GETDOT_PARAM_VEC2("accel", Prop().accel)
		GM_GETDOT_PARAM_FLOAT("cooldown", Prop().cooldown)
		GM_GETDOT_PARAM_INT("numParticlesPerEmission", Prop().numParticlesPerEmission)
		GM_GETDOT_PARAM_FLOAT("lifeSpan", Prop().lifeSpan)
		GM_GETDOT_PARAM_FLOAT("lifeSpanVariance", Prop().lifeSpanVariance)
		GM_GETDOT_PARAM_FLOAT("rotStart", Prop().rotStart)
		GM_GETDOT_PARAM_FLOAT("rotStartVariance", Prop().rotStartVariance)
		GM_GETDOT_PARAM_FLOAT("rotVel", Prop().rotVel)
		GM_GETDOT_PARAM_FLOAT("rotVelVariance", Prop().rotVelVariance)
		GM_GETDOT_PARAM_FLOAT("rotAccel", Prop().rotAccel)
		GM_GETDOT_PARAM_FLOAT("rotDampingCoeff", Prop().rotDampingCoeff)
		GM_GETDOT_PARAM_VEC2("scaleStart", Prop().scaleStart)
		GM_GETDOT_PARAM_VEC2("scaleEnd", Prop().scaleEnd)
		GM_GETDOT_PARAM_FLOAT("scaleStartVariance", Prop().scaleStartVariance)
		GM_GETDOT_PARAM_FLOAT("scaleEndVariance", Prop().scaleEndVariance)
		GM_GETDOT_PARAM_VEC3("colorStart", Prop().colorStart)
		GM_GETDOT_PARAM_VEC3("colorEnd", Prop().colorEnd)
		GM_GETDOT_PARAM_VEC3("colorStartVariance", Prop().colorStartVariance)
		GM_GETDOT_PARAM_VEC3("colorEndVariance", Prop().colorEndVariance)
		GM_GETDOT_PARAM_FLOAT("alphaStart", Prop().alphaStart)
		GM_GETDOT_PARAM_FLOAT("alphaEnd", Prop().alphaEnd)
		GM_GETDOT_PARAM_FLOAT("alphaStartVariance", Prop().alphaStartVariance)
		GM_GETDOT_PARAM_FLOAT("alphaEndVariance", Prop().alphaEndVariance)
		GM_GETDOT_PARAM_INT("blendSrc", Prop().blendSrc)
		GM_GETDOT_PARAM_INT("blendDest", Prop().blendDest)
		GM_GETDOT_PARAM_INT("blendEq", Prop().blendEq)
	GM_GETDOT_FUNC_END()
}

GM_REG_MEM_BEGIN(Particles2d)
GM_REG_SETDOT_FUNC(Particles2d)
GM_REG_GETDOT_FUNC(Particles2d)
GM_REG_MEMFUNC( Particles2d, Draw )
GM_REG_MEMFUNC( Particles2d, Update )
GM_REG_MEMFUNC( Particles2d, Emit )
GM_REG_MEMFUNC( Particles2d, NumParticles )
GM_REG_MEMFUNC( Particles2d, NumParticlesMax )
GM_REG_MEMFUNC( Particles2d, UpdateTime )

GM_REG_HANDLED_DESTRUCTORS(Particles2d)
GM_REG_MEM_END()
GM_BIND_DEFINE(Particles2d)

}