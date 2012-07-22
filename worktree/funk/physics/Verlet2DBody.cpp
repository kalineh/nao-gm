#include "Verlet2DBody.h"

#include <gl/glew.h>
#include <math/v3.h>
#include <math.h>
#include <gfx/Renderer.h>

namespace funk
{

Verlet2DBody::Verlet2DBody() : m_numIter(2)
{;}

void Verlet2DBody::Update( float dt )
{
	for ( int iter = 0; iter < m_numIter; ++iter )
	{
		for ( size_t i = 0; i < m_constraints.size(); ++i )
		{
			Verlet2DConstraint & c = m_constraints[i];
			Verlet2DNode & startNode = m_nodes[ c.start ];
			Verlet2DNode & endNode = m_nodes[ c.end ];
			const float restLen = c.restLen;
			const float restAngle = c.restAngleRel;

			// endpts
			v2 &v0 = startNode.pos;
			v2 &v1 = endNode.pos;

			// parentangle constraint
			float parentAngle = 0.0f;
			if ( c.parent != -1 )
			{
				const Verlet2DConstraint & parentConstraint = m_constraints[c.parent];
				const v2 parentStart = m_nodes[parentConstraint.start].pos;
				const v2 parentEnd = m_nodes[parentConstraint.end].pos;
				const v2 parentBranchVec = parentEnd - parentStart;
				parentAngle = atan2f( parentBranchVec.y, parentBranchVec.x );
			}	

			// goal angle
			float goalAngle = parentAngle + restAngle;
			v2 goalPos = v0+v2( cosf(goalAngle), sinf(goalAngle) ) * restLen;
			v1 = lerp( v1, goalPos, c.angleStiffness );

			
			v2 delta = v1 - v0;
			float deltaLen = length(delta);
			float diff = (deltaLen - restLen)/deltaLen;

			v0 += delta * 0.5f * diff * (1.0f-startNode.rigidity);
			v1 -= delta * 0.5f * diff * (1.0f-endNode.rigidity);
		}
	}
}

void Verlet2DBody::DrawDebug()
{
	Renderer *rndr = Renderer::Get();
	rndr->BeginDefaultShader();
	
	// lines
	glColor3f(1.0f, 0.0f, 0.0f );
	glBegin(GL_LINES);
	for( size_t i = 0; i < m_constraints.size(); ++i )
	{
		v2 v0 = m_nodes[ m_constraints[i].start ].pos;
		v2 v1 = m_nodes[ m_constraints[i].end ].pos;
		glVertex2f( v0.x, v0.y );
		glVertex2f( v1.x, v1.y );
	}
	glEnd();

	glPointSize(5.0f);
	glColor3f(1.0f, 0.0f, 1.0f );
	glBegin(GL_POINTS);
	for( size_t i = 0; i < m_nodes.size(); ++i )
	{
		v2 pos = m_nodes[i].pos;
		glVertex2f(pos.x, pos.y);
	}
	glEnd();

	rndr->EndDefaultShader();
}

int Verlet2DBody::AddNode( funk::v2 pos, float rigidity )
{
	m_nodes.push_back( Verlet2DNode(pos, rigidity) );
	return (int)m_nodes.size()-1;
}

int Verlet2DBody::AddConstraint( int startNodeIndex, int endNodeIndex, int parentIndex )
{
	assert( startNodeIndex >= 0 && startNodeIndex < (int)m_nodes.size() );
	assert( endNodeIndex >= 0 && endNodeIndex < (int)m_nodes.size() );

	m_constraints.push_back( Verlet2DConstraint(startNodeIndex, endNodeIndex, parentIndex) );
	return (int)m_constraints.size()-1;
}

void Verlet2DBody::BakeConstraints()
{
	for ( size_t i = 0; i < m_constraints.size(); ++i )
	{
		Verlet2DConstraint &c = m_constraints[i];

		const Verlet2DNode & nodeStart = m_nodes[ c.start ];
		const Verlet2DNode & nodeEnd = m_nodes[ c.end ];
		const v2 & v0 = nodeStart.pos;
		const v2 & v1 = nodeEnd.pos;
		const v2 dir = v1 - v0;
		const v2 dirNorm = normalize(dir);

		c.restLen = length(v1-v0);

		// calc relative angle relative to parent
		if ( c.parent != -1 )
		{
			const Verlet2DConstraint & parentConstraint = m_constraints[c.parent];
			const v2 parentStart = m_nodes[parentConstraint.start].pos;
			const v2 parentEnd = m_nodes[parentConstraint.end].pos;
			const v2 parentBranchVec = normalize(parentEnd-parentStart);

			float dotProd = dot(parentBranchVec, normalize(dir));
			float relAngle = 0.0f;

			if ( dotProd <= 0.999f)
			{
				relAngle = acosf(dotProd);
				relAngle *= cross( v3(parentBranchVec.x, parentBranchVec.y, 0.0f ), v3(dirNorm.x, dirNorm.y, 0.0f)).z;
			}
			
			c.restAngleRel = relAngle;
		}
		else
		{
			c.restAngleRel = atan2f( dirNorm.y, dirNorm.x );
		}
	}
}

}