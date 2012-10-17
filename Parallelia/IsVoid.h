#ifndef ISVOID_H
#define ISVOID_H


namespace Parallelia
{

	namespace ParalleliaCore
	{
		template<class O>
		struct IsVoid
		{
			enum { value = 1 };	
		};

		template<>
		struct IsVoid<void>
		{
			enum { value = 0 };	
		};
	}

}


#endif