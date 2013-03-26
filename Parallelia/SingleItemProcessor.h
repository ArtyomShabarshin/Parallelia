#ifndef SINGLEITEMPROCESSOR_H
#define SINGLEITEMPROCESSOR_H

#include "ItemProcessor.h"


namespace Parallelia
{

	namespace ParalleliaCore
	{
		//single item processor tries to fijnd only 1 consumer and sends item to it
		template<typename T> 
		class SingleItemProcessor : public ItemProcessor<T>
		{
		public:
			SingleItemProcessor(Parallelia::Utils::InputDataFlowBlockCoreDebugProxy<T>& debug);
			virtual ~SingleItemProcessor();

		private:
			//no copy
			SingleItemProcessor(const SingleItemProcessor&);
			SingleItemProcessor& operator=(const SingleItemProcessor&);

			virtual void DoProcessItem(const T& t, int consumerid, ConsumerQueue& consumers);

			 typename ItemProcessor<T>::LinkIterator FindLinkConsumer(const T& item, int& linkernumber, size_t& maxcapacity);
		};

		template<typename T> 
		SingleItemProcessor<T>::SingleItemProcessor(Parallelia::Utils::InputDataFlowBlockCoreDebugProxy<T>& debug) : ItemProcessor<T>(debug)
		{}

		template<typename T> 
		SingleItemProcessor<T>::~SingleItemProcessor()
		{}

		template<typename T> 
		void SingleItemProcessor<T>::DoProcessItem(const T& t, int consumerid, typename ItemProcessor<T>::ConsumerQueue& consumers)
		{
			bool complete = false;
			while(!complete)
			{
				int linkernumber = -1;
				size_t maxcapacity = 0;
				LinkIterator it = FindLinkConsumer(t, linkernumber, maxcapacity);
				if(it != EndLink())
				{
					complete = (DataFlowPostItemStatus::Accepted == (*it).first->TryPostItem(t));
				}
				if(!complete)//wait
				{
					Debug().NotCompleteSendingItem(consumerid, t, linkernumber);
		
					std::shared_ptr<Concurrency::event> tce(new Concurrency::event());
					consumers.push(tce);

					Debug().GotoSleep(consumerid);

					size_t waitresult = tce->wait();

					Debug().Wokeup(consumerid, waitresult);

				}
				else
				{
					Debug().CompleteSendingItem(consumerid, t, linkernumber, maxcapacity);

					IncrementProcessedItems();
				}
			}
		}

		template<typename T> 
		typename ItemProcessor<T>::LinkIterator SingleItemProcessor<T>::FindLinkConsumer(const T& item, int& linkernumber, size_t& maxcapacity)
		{
			linkernumber = -1;
			maxcapacity = 0;
			LinkIterator minit = BeginLink();
			while(minit != EndLink() && !(*minit).second(item))
			{++minit;}

			if(minit != EndLink())
			{
				linkernumber = 0;
				//if has one or more links
				maxcapacity = (*minit).first->CapacityFactor();
				LinkIterator it = minit;
				++it;
				int index = 0;
				for( ; it != EndLink(); ++it)
				{
					++index;
					if((*it).second(item))
					{
						size_t cf = (*it).first->CapacityFactor();
						if(maxcapacity < cf)
						{
							linkernumber = index;
							maxcapacity = cf;
							minit = it;
						}
					}
								
				}
			}
			return minit;
		}
	}

}

#endif