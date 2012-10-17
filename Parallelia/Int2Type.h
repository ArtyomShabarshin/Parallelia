#ifndef INT2TYPE_H
#define INT2TYPE_H






namespace Parallelia
{

	namespace ParalleliaCore
	{
		template <int V>
		struct Int2Type
		{
		   enum {type = V};
		};
	}

}


#endif