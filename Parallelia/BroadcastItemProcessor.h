#ifndef BROADCASTITEMPROCESSOR_H
#define BROADCASTITEMPROCESSOR_H

#include "ItemProcessor.h"


namespace Parallelia
{

	namespace ParalleliaCore
	{
		//broadcast item processr tries to send item to all linked consumers.
		//if consumer can't receive item BroadcastItemProcessor passes this consumer and send it to anothers
		template<typename T> 
		class BroadcastItemProcessor : public ItemProcessor<T>
		{
		public:
			BroadcastItemProcessor(Parallelia::Utils::InputDataFlowBlockCoreDebugProxy<T>& debug);
			virtual ~BroadcastItemProcessor();
		private:
			//no copy
			BroadcastItemProcessor(const BroadcastItemProcessor&);
			BroadcastItemProcessor& operator=(const BroadcastItemProcessor&);

			virtual void DoProcessItem(const T& t, int consumerid, ConsumerQueue& consumers);
		};

		template<typename T> 
		BroadcastItemProcessor<T>::BroadcastItemProcessor(Parallelia::Utils::InputDataFlowBlockCoreDebugProxy<T>& debug) : ItemProcessor<T>(debug)
		{}

		template<typename T> 
		BroadcastItemProcessor<T>::~BroadcastItemProcessor()
		{}

		template<typename T> 
		void BroadcastItemProcessor<T>::DoProcessItem(const T& t, int consumerid, typename ItemProcessor<T>::ConsumerQueue& consumers)
		{

		}
	}

}

#endif