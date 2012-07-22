#ifndef _INCLUDE_BASEENTITYGROUP_H_
#define _INCLUDE_BASEENTITYGROUP_H_

#include <common/StrongHandle.h>
#include "BaseEntity.h"

#include <vector>

namespace funk
{
	class BaseEntityGroup : public BaseEntity
	{
	public:

		virtual void Init();
		virtual void Deinit();
		virtual void Update( float dt );
		virtual void Render();

		void AddChild( StrongHandle<BaseEntity> child ) { m_children.push_back(child); }

	protected:
		std::vector< StrongHandle<BaseEntity> > m_children;
	};
}

#endif