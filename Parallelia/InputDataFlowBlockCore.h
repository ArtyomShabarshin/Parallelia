#ifndef INPUTDATAFLOWBLOCKCORE_T
#define INPUTDATAFLOWBLOCKCORE_T


#include "IDataFlowBlock.h"
#include "IInputDataFlowBlock.h"
#include "IOutputDataFlowBlock.h"
#include "ReadyEventReceiver.h"
#include "DataFlowBlockCompletion.h"
#include "BroadcastItemProcessor.h"
#include "SingleItemProcessor.h"

#include "DataFlowBlockState.h"
#include "DataFlowBlockOptions.h"
#include "InputDataFlowBlockCoreDebugProxy.h"
#include "AutomaticGuard.h"

#include "concurrent_queue.h"
#include "concurrent_vector.h"
#include "ppltasks.h"

#include <utility>


namespace Parallelia
{

	namespace ParalleliaCore
	{
		template<typename T> 
		class InputDataFlowBlockCore : public IDataFlowBlock, public IInputDataFlowBlock<T>, public IOutputDataFlowBlock<T>
		{
			
			
			
		public:
			InputDataFlowBlockCore(const DataflowBlockOptions& options, bool isbroadcast);
			virtual ~InputDataFlowBlockCore();

			size_t Count() const { return m_count; }//m_queue.unsafe_size(); }
			long ProcessedItems() const { return m_itemprocessor->ProcessedItems(); }
			long ProcessingItems() const { return m_processingItems; }
			size_t NumSleeping() const { return m_consumerqueue.unsafe_size(); }

			void StartDebug();
		private:
			//no copy
			InputDataFlowBlockCore(const InputDataFlowBlockCore&);
			InputDataFlowBlockCore& operator=(const InputDataFlowBlockCore&);

			//IDataFlowBlock interface
			virtual void DoComplete();
			virtual IDataFlowBlockCompletion& DoCompletion();

			//IInputDataFlowBlock interface
			virtual void DoLinkTo(typename ItemProcessor<T>::Linktype outputBlock, typename IInputDataFlowBlock<T>::Predicate predicate);

			//IOutputDataFlowBlock<T> interface
			virtual DataFlowPostItemStatus DoTryPostItem(T item);
			virtual void DoRegisterReadyEventReceiver(std::shared_ptr<ParalleliaCore::ReadyEventReceiver > readyEventReveicer){}
			virtual size_t DoCapacityFactor() { return -1; } 

			void Init(bool isbroadcast);
			void WakeUpTCE();
			void CompletionWait();
			void ProcessItem(const T& item, int consumerid);

		private:
			Concurrency::concurrent_queue<T> m_queue;
			Concurrency::concurrent_queue<std::shared_ptr<Concurrency::event> > m_consumerqueue;
			Concurrency::task_group m_taskgroup;
			DataFlowBlockState m_blockstate;
			DataflowBlockOptions m_options;
			volatile size_t m_count;
			int m_numConsumers;
			int m_numStopperConsumers;
			std::shared_ptr<ReadyEventReceiver> m_eventReceiver;
		
			long m_processingItems;
			DataFlowBlockCompletion<void>* m_completion;
			Concurrency::critical_section m_csection;

			Parallelia::Utils::InputDataFlowBlockCoreDebugProxy<T> m_debug;
			std::shared_ptr<ItemProcessor<T> > m_itemprocessor;
		};

		template<typename T> 
		InputDataFlowBlockCore<T>::InputDataFlowBlockCore(const DataflowBlockOptions& options, bool isbroadcast) : m_options(options)
																												, m_blockstate(DataFlowBlockStateReady)
																												, m_count(0)
																												//, m_eventReceiver(new ReadyEventReceiver(std::bind(&InputDataFlowBlockCore<T>::WakeUpTCE, this)))
																												, m_eventReceiver(new ReadyEventReceiver([=]() { this->WakeUpTCE(); }))
																												, m_numConsumers(0)
																												, m_numStopperConsumers(0)
																												, m_processingItems(0L)
																												//, m_completion(new DataFlowBlockCompletion<void>(std::bind(&InputDataFlowBlockCore<T>::CompletionWait, this)))
																												, m_completion(new DataFlowBlockCompletion<void>([=]() { this->CompletionWait(); }))
																												
																												
		{ Init(isbroadcast); }

		template<typename T> 
		InputDataFlowBlockCore<T>::~InputDataFlowBlockCore()
		{
			try
			{
				m_completion->Wait();
			}
			catch(...)  //any exceptions will be ignored
			{}
		}


		//создаем потоки.
		//если есть что-то в очереди, то пытаемся это отправить в линк
		//		если не смогли отправить в линк (не принимает), то сохраняем обратно в очередь, спим
		//      иначе элемент считается переданным
		//иначе спим
		template<typename T> 
		void InputDataFlowBlockCore<T>::Init(bool isbroadcast)
		{
			ItemProcessor<T>* itemprocessor = 0;
			if(isbroadcast)
			{
				itemprocessor = new BroadcastItemProcessor<T>(m_debug);
			}
			else
			{
				itemprocessor = new SingleItemProcessor<T>(m_debug);
			}  
			m_itemprocessor = std::shared_ptr<ItemProcessor<T> >(itemprocessor);

			m_numConsumers = m_options.MaxDegreeOfParallelism;
			bool result = false;

			for(int i = 0; i < m_numConsumers; ++i)
			{
				m_taskgroup.run([&, i]()
				{
					try
					{
						Concurrency::critical_section csection;
						int consumerid = i;

						m_debug.StartConsumer(consumerid);

						while(true)
						{
						
							T t;
							result = m_queue.try_pop(t);
							if(result)
							{
								
								m_debug.StartSendingItem(consumerid, t);
								//we have to enter to lock section (FIFO requirements)
								while(!m_csection.try_lock_for(4000) )//spinlock timeout
								{
									m_debug.NotCompleteSendingItemSpinLock(consumerid, t);
		
									std::shared_ptr<Concurrency::event> tce(new Concurrency::event());
									m_consumerqueue.push(tce);

									m_debug.GotoSleep(consumerid);

									size_t waitresult = tce->wait();

									m_debug.Wokeup(consumerid, waitresult);
								}
								try
								{
									_InterlockedIncrement(&m_processingItems);
									_InterlockedDecrementSizeT(&m_count);

									m_itemprocessor->ProcessItem(t, consumerid, m_consumerqueue);

									_InterlockedDecrement(&m_processingItems);
									m_csection.unlock();
								}
								catch(...)
								{
									m_csection.unlock();
								}

								

							}
							else
							{
								if(DataFlowBlockStateReady != m_blockstate)
								{
									_InterlockedIncrementSizeT(&m_numStopperConsumers);

									m_debug.FinishConsumer(consumerid);

									break; //task exit
								}

								m_debug.NoDataGotoSleep(consumerid);

								std::shared_ptr<Concurrency::event> tce(new Concurrency::event());
								m_consumerqueue.push(tce);
								size_t waitresult = tce->wait();

								m_debug.NoDataWokeup(consumerid, waitresult);
							}
							
						}
					}
					catch(...)
					{
						int g = 0;
					}
				}
				);
			}
		}


		template<typename T> 
		void InputDataFlowBlockCore<T>::DoComplete()
		{
			m_blockstate = DataFlowBlockStateComplete;
			//run all threads and then they all will die
			//WHAT ABOUT IF IN THAT TIME SOMEONE CALLS TryPostItem(item); ??????
			std::shared_ptr<Concurrency::event> tce;
			while(m_consumerqueue.try_pop(tce))
			{
					tce->set();
			}
		}


		template<typename T> 
		IDataFlowBlockCompletion& InputDataFlowBlockCore<T>::DoCompletion()
		{
			return *(IDataFlowBlockCompletion*)m_completion; 
		}

		//add item to consumer queue
		//if we have waiter, then we wake up it
		template<typename T> 
		void InputDataFlowBlockCore<T>::DoLinkTo(typename ItemProcessor<T>::Linktype outputBlock, typename IInputDataFlowBlock<T>::Predicate predicate)
		{
			m_debug.AddLink();
			m_itemprocessor->AddLink(outputBlock, predicate);
			outputBlock->RegisterReadyEventReceiver(m_eventReceiver);
			WakeUpTCE();
		}

		//add item to consumer queue
		//if we have waiter (in m_consumerqueue), then we wake up it
		template<typename T> 
		DataFlowPostItemStatus InputDataFlowBlockCore<T>::DoTryPostItem(T item)
		{
			DataFlowPostItemStatus result = DataFlowPostItemStatus::Accepted;

			m_queue.push(item);
			_InterlockedIncrementSizeT(&m_count);

			m_debug.ReceivedItem(item);
				
			WakeUpTCE();
			return result;
		}

		//try to wake up one of the task completion event from consumer queue
		template<typename T> 
		void InputDataFlowBlockCore<T>::WakeUpTCE()
		{
			try
			{
				std::shared_ptr<Concurrency::event> tce;
				if(m_consumerqueue.try_pop(tce))
				{
					m_debug.TryToWakeupConsumer();
					tce->set();
				}
			}
			catch(...)
			{
				//printf("exception\n");
			}
		}

		template<typename T> 
		void InputDataFlowBlockCore<T>::StartDebug()
		{ 
			m_debug.StartDebug(); 
		}

		template<typename T> 
		void InputDataFlowBlockCore<T>::CompletionWait()
		{
			m_taskgroup.wait();
		}



	}

}

#endif