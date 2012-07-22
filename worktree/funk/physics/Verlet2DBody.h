#ifndef _INCLUDE_VERLET_2D_H
#define _INCLUDE_VERLET_2D_H

#include <vector>
#include <math/v2.h>

namespace funk
{

	class Verlet2DBody
	{
	public:

		struct Verlet2DConstraint
		{
			// node index
			int start;
			int end;

			// parent, helps with angle
			int parent;

			// rest properties
			float restLen;
			float restAngleRel; // relative to parent
			float angleStiffness;

			Verlet2DConstraint( int _start, int _end, int _parent )
				:	start(_start), end(_end), parent(_parent),
					restLen(1.0f), restAngleRel(0.0f), angleStiffness(0.1f)
			{;}
		};

		struct Verlet2DNode
		{
			funk::v2 pos;
			float rigidity;

			Verlet2DNode( funk::v2 _pos, float _rigidity ) : pos(_pos), rigidity(_rigidity) {;}
		};

		Verlet2DBody();

		void Update( float dt );
		void DrawDebug();
		void SetNumIterations( int iter ) { m_numIter = iter; }

		// creation (returns index)
		int AddNode( funk::v2 pos, float rigidity = 0.0f );
		int AddConstraint( int startNodeIndex, int endNodeIndex, int parentIndex = -1 );
		void BakeConstraints();

		// get prop
		size_t NumNodes() const  { return m_nodes.size(); }
		size_t NumConstraints() const  { return m_constraints.size(); }
		Verlet2DNode &Node( int index ) { return m_nodes[index]; }
		Verlet2DConstraint &Constraint( int index ) { return m_constraints[index]; }

	private:

		int m_numIter;
		std::vector<Verlet2DNode> m_nodes;
		std::vector<Verlet2DConstraint> m_constraints;
	};
}
#endif