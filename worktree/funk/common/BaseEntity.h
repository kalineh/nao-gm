#ifndef _INCLUDE_BASEENTITY_H_
#define _INCLUDE_BASEENTITY_H_

#include <common/HandledObj.h>

namespace funk
{
	class BaseEntity: public HandledObj<BaseEntity>
	{
	public:
		virtual void Init() {;}
		virtual void Deinit() {;}
		virtual void Update( float dt ) {;}
		virtual void Render() {;}

		virtual ~BaseEntity() {;}

	private:
	};
}

#endif