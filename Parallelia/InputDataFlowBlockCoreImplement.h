#ifndef INPUTDATAFLOWBLOCKCOREIMPLEMENT_T
#define INPUTDATAFLOWBLOCKCOREIMPLEMENT_T


#include "IDataFlowBlock.h"
#include "IInputDataFlowBlock.h"
#include "IOutputDataFlowBlock.h"
#include "ReadyEventReceiver.h"

#include "DataFlowBlockState.h"
#include "DataFlowBlockOptions.h"
#include "DebugInfo.h"

#include "concurrent_queue.h"
#include "concurrent_vector.h"
#include "ppltasks.h"


namespace Parallelia
{

	namespace ParalleliaCore
	{
		template<typename T> 
		class InputDataFlowBlockCoreImplement : public IDataFlowBlock, public IInputDataFlowBlock<T>, public IOutputDataFlowBlock<T>
		{
		public:
			InputDataFlowBlockCoreImplement(const DataflowBlockOptions& options);
			virtual ~InputDataFlowBlockCoreImplement();

			size_t Count() const { return m_count; }//m_queue.unsafe_size(); }
			long ProcessedItems() const { return m_processedItems; }
			long ProcessingItems() const { return m_processingItems; }
			size_t NumSleeping() const { return m_consumerqueue.unsafe_size(); }

			void StartDebug();

		private:
			//no copy
			InputDataFlowBlockCoreImplement(const InputDataFlowBlockCoreImplement&);
			InputDataFlowBlockCoreImplement& operator=(const InputDataFlowBlockCoreImplement&);

			//IDataFlowBlock interface
			virtual void DoComplete();
			virtual Concurrency::task_group& DoCompletion();

			//IInputDataFlowBlock interface
			virtual void DoLinkTo(std::shared_ptr<IOutputDataFlowBlock<T> >& outputBlock);

			//IOutputDataFlowBlock<T> interface
			virtual DataFlowPostItemStatus DoTryPostItem(T item);
			virtual void DoRegisterReadyEventReceiver(std::shared_ptr<ParalleliaCore::ReadyEventReceiver > readyEventReveicer){}
			virtual size_t DoCapacityFactor() const { return -1; } 

			void Init();
			void WakeUpTCE();

		private:
			Concurrency::concurrent_queue<T> m_queue;
			Concurrency::concurrent_queue<std::shared_ptr<Concurrency::event> > m_consumerqueue;
			Concurrency::concurrent_vector<std::shared_ptr<IOutputDataFlowBlock<T> > > m_linktovector;
			Concurrency::task_group m_taskgroup;
			DataFlowBlockState m_blockstate;
			DataflowBlockOptions m_options;
			size_t m_count;
			int m_numConsumers;
			int m_numStopperConsumers;
			std::shared_ptr<ReadyEventReceiver> m_eventReceiver;
			long m_processedItems;
			long m_processingItems;
#ifdef DEBUG_PDF_TRACE
			//debug
			Parallelia::Utils::DebugInfo m_debug;
#endif
		};

		template<typename T> 
		InputDataFlowBlockCoreImplement<T>::InputDataFlowBlockCoreImplement(const DataflowBlockOptions& options) : m_options(options)
																												 , m_blockstate(DataFlowBlockStateReady)
																												 , m_count(0)
																												 , m_eventReceiver(new ReadyEventReceiver(std::bind(&InputDataFlowBlockCoreImplement<T>::WakeUpTCE, this)))
																												 , m_numConsumers(0)
																												 , m_numStopperConsumers(0)
																												 , m_processedItems(0L)
																												 , m_processingItems(0L)
		{ Init(); }

		template<typename T> 
		InputDataFlowBlockCoreImplement<T>::~InputDataFlowBlockCoreImplement()
		{
			try
			{
				m_taskgroup.wait();
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
		void InputDataFlowBlockCoreImplement<T>::Init()
		{
			m_numConsumers = m_options.MaxDegreeOfParallelism;
			bool result = false;

			for(int i = 0; i < m_numConsumers; ++i)
			{
				m_taskgroup.run([&, i]()
				{
					Concurrency::critical_section csection;
					int consumerid = i;

#ifdef DEBUG_PDF_TRACE
					m_debug.Add(std::string("Start consumer ") + std::to_string(consumerid));
#endif

					while(true)
					{
						
						T t;
						result = m_queue.try_pop(t);
						if(result)
						{
#ifdef DEBUG_PDF_TRACE
							m_debug.Add(std::string("Start sending item. consumer ") + std::to_string(consumerid));
#endif
							_InterlockedIncrement(&m_processingItems);
							_InterlockedDecrementSizeT(&m_count);

							int linkernumber = -1;
							size_t maxcapacity = 0;
							Concurrency::concurrent_vector<std::shared_ptr<IOutputDataFlowBlock<T> > >::const_iterator minit = m_linktovector.cbegin();
							bool complete = false;
							if(minit != m_linktovector.cend())
							{
								linkernumber = 0;
								//if has one or more links
								maxcapacity = (*minit)->CapacityFactor();
								Concurrency::concurrent_vector<std::shared_ptr<IOutputDataFlowBlock<T> > >::const_iterator it = minit;
								++it;
								int index = 0;
								for( ; it != m_linktovector.cend(); ++it)
								{
									++index;
									size_t cf = (*it)->CapacityFactor();
									if(maxcapacity < cf)
									{
										linkernumber = index;
										maxcapacity = cf;
										minit = it;
									}
								
								}
								complete = DataFlowPostItemStatus::Accepted == (*minit)->TryPostItem(t);
							}
							
							
							if(!complete)//store it back to queue and wait
							{
#ifdef DEBUG_PDF_TRACE
								m_debug.Add(std::string("Not complete sending item to linker ") + std::to_string(linkernumber) + std::string(". consumer ") + std::to_string(consumerid));
#endif
								m_queue.push(t);
								_InterlockedIncrementSizeT(&m_count);

								std::shared_ptr<Concurrency::event> tce(new Concurrency::event());
								m_consumerqueue.push(tce);
#ifdef DEBUG_PDF_TRACE
								m_debug.Add(std::string("Go to sleep. consumer ") + std::to_string(consumerid));
#endif
								tce->wait();
#ifdef DEBUG_PDF_TRACE
								m_debug.Add(std::string("Woke up. consumer ") + std::to_string(consumerid));
#endif
							}
							else
							{
#ifdef DEBUG_PDF_TRACE
								m_debug.Add(std::string("Complete sending item to linker ") + std::to_string(linkernumber) + std::string(".(cap:") + std::to_string(maxcapacity) + std::string(") consumer ") + std::to_string(consumerid));
#endif
								_InterlockedIncrement(&m_processedItems);
							}
							_InterlockedDecrement(&m_processingItems);
						}
						else
						{
							if(DataFlowBlockStateReady != m_blockstate)
							{
								_InterlockedIncrementSizeT(&m_numStopperConsumers);
#ifdef DEBUG_PDF_TRACE
								m_debug.Add(std::string("Finish consumer ") + std::to_string(consumerid));
#endif
								break; //task exit
							}
#ifdef DEBUG_PDF_TRACE
							m_debug.Add(std::string("No data. Go to sleep. consumer ") + std::to_string(consumerid));
#endif
							std::shared_ptr<Concurrency::event> tce(new Concurrency::event());
							m_consumerqueue.push(tce);
							tce->wait();
#ifdef DEBUG_PDF_TRACE
							m_debug.Add(std::string("Woke up after no data. consumer ") + std::to_string(consumerid));
#endif
						}
					}
				}
				);
			}
		}


		template<typename T> 
		void InputDataFlowBlockCoreImplement<T>::DoComplete()
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
		Concurrency::task_group& InputDataFlowBlockCoreImplement<T>::DoCompletion()
		{
			return m_taskgroup; 
		}

		//add item to consumer queue
		//if we have waiter, then we wake up it
		template<typename T> 
		void InputDataFlowBlockCoreImplement<T>::DoLinkTo(std::shared_ptr<IOutputDataFlowBlock<T> >& outputBlock)
		{
#ifdef DEBUG_PDF_TRACE	
			m_debug.Add(std::string("Add link"));
#endif
			m_linktovector.push_back(outputBlock);
			outputBlock->RegisterReadyEventReceiver(m_eventReceiver);
			WakeUpTCE();
		}

		//add item to consumer queue
		//if we have waiter (in m_consumerqueue), then we wake up it
		template<typename T> 
		DataFlowPostItemStatus InputDataFlowBlockCoreImplement<T>::DoTryPostItem(T item)
		{
			DataFlowPostItemStatus result = DataFlowPostItemStatus::Accepted;

			m_queue.push(item);
			_InterlockedIncrementSizeT(&m_count);

#ifdef DEBUG_PDF_TRACE	
			m_debug.Add(std::string("Received item (TryPostItem)"));
#endif
				
			WakeUpTCE();
			return result;
		}

		//try to wake up one of the task completion event from consumer queue
		template<typename T> 
		void InputDataFlowBlockCoreImplement<T>::WakeUpTCE()
		{
			try
			{
				std::shared_ptr<Concurrency::event> tce;
				if(m_consumerqueue.try_pop(tce))
				{
#ifdef DEBUG_PDF_TRACE	
					m_debug.Add(std::string("Trying to wake up some consumer"));
#endif
					tce->set();
				}
			}
			catch(...)
			{
				printf("exception\n");
			}
		}

		template<typename T> 
		void InputDataFlowBlockCoreImplement<T>::StartDebug()
		{ 
#ifdef DEBUG_PDF_TRACE
			m_debug.StartDebug(); 
#endif
			;
		}


	}

}

#endif