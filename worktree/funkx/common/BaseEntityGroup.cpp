#include "BaseEntityGroup.h"

namespace funk
{
typedef std::vector< StrongHandle<BaseEntity> >::iterator ChildIt;
typedef std::vector< StrongHandle<BaseEntity> >::reverse_iterator ReverseChildIt;
typedef std::vector< StrongHandle<BaseEntity> >::const_iterator ConstChildIt;

void BaseEntityGroup::Init()
{
	for ( ChildIt it = m_children.begin(); it != m_children.end(); ++it )
	{
		(*it)->Init();
	}
}

void BaseEntityGroup::Deinit()
{
	for ( ReverseChildIt it = m_children.rbegin(); it < m_children.rend(); ++it )
	{
		(*it)->Deinit();
	}
}

void BaseEntityGroup::Update( float dt )
{
	for ( ChildIt it = m_children.begin(); it != m_children.end(); ++it )
	{
		(*it)->Update( dt );
	}
}

void BaseEntityGroup::Render()
{
	for ( ChildIt it = m_children.begin(); it != m_children.end(); ++it )
	{
		(*it)->Render();
	}
}
}