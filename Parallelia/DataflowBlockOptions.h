#ifndef DATAFLOWBLOCKOPTIONS
#define DATAFLOWBLOCKOPTIONS

#include "ppl.h"

namespace Parallelia
{

	struct DataflowBlockOptions
	{
		//Gets/Sets the maximum number of items that may be processed by the block concurrently.
		int MaxDegreeOfParallelism;
		//Gets/Sets the capacity of the queue
		size_t Capacity;

		DataflowBlockOptions() : MaxDegreeOfParallelism(1)
								,Capacity(-1)
		{}

		static DataflowBlockOptions Default()
		{
			DataflowBlockOptions options;
			options.MaxDegreeOfParallelism = Concurrency::CurrentScheduler::Get()->GetNumberOfVirtualProcessors();
			options.Capacity = -1;
			return options;
		}
	};




}


#endif