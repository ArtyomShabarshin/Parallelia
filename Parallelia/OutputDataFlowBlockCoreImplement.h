#ifndef OUTPUTDATAFLOWBLOCKCOREOMPLEMENT_T
#define OUTPUTDATAFLOWBLOCKCOREOMPLEMENT_T

#include "concurrent_queue.h"
#include "ppltasks.h"


#include "IDataFlowBlock.h"
#include "IOutputDataFlowBlock.h"
#include "DebugInfo.h"

#include "DataFlowBlockState.h"
#include "DataFlowBlockOptions.h"






namespace Parallelia
{

	namespace ParalleliaCore
	{

		//responsibility:
		//receive items, store its in the queue and execute Func on each item
		//item processing has many threads (to control parallel of degree use DataflowBlockOptions::MaxDegreeOfParallelism)
		template<typename T> 
		class OutputDataFlowBlockCoreImplement  : public IDataFlowBlock, public IOutputDataFlowBlock<T>
		{
			typedef std::function<void(T)> F;
		public:
			OutputDataFlowBlockCoreImplement(F func, const DataflowBlockOptions& options);
			virtual ~OutputDataFlowBlockCoreImplement();

			size_t Count() { return m_count; }//m_queue.unsafe_size(); }
			long ProcessedItems() const { return m_processedItems; }
			long ProcessingItems() const { return m_processingItems; }
			size_t NumSleeping() const { return m_consumerqueue.unsafe_size(); }
			void StartDebug();
		private:
			//no copy
			OutputDataFlowBlockCoreImplement(const OutputDataFlowBlockCoreImplement&);
			OutputDataFlowBlockCoreImplement& operator=(const OutputDataFlowBlockCoreImplement&);

			//IDataFlowBlock interface
			virtual void DoComplete();
			virtual Concurrency::task_group& DoCompletion();

			//IOutputDataFlowBlock<T> interface
			virtual DataFlowPostItemStatus DoTryPostItem(T item);
			virtual void DoRegisterReadyEventReceiver(std::shared_ptr<ParalleliaCore::ReadyEventReceiver > readyEventReveicer) { m_readyEventReceiver = readyEventReveicer; }
			virtual size_t DoCapacityFactor() const;

			void Init();
		private:
			F m_func;
			Concurrency::concurrent_queue<T> m_queue;
			Concurrency::concurrent_queue<std::shared_ptr<Concurrency::event > > m_consumerqueue;
			Concurrency::task_group m_taskgroup;
			DataFlowBlockState m_blockstate;
			DataflowBlockOptions m_options;
			size_t m_count;
			int m_numConsumers;
			std::shared_ptr<ParalleliaCore::ReadyEventReceiver > m_readyEventReceiver;
			volatile DataFlowPostItemStatus m_lastpoststatus;
			long m_processedItems;
			long m_processingItems;
#ifdef DEBUG_PDF_TRACE
			//debug
			Parallelia::Utils::DebugInfo m_debug;
#endif

		};

		template<typename T> 
		OutputDataFlowBlockCoreImplement<T>::OutputDataFlowBlockCoreImplement(F func, const DataflowBlockOptions& options) : m_options(options)
																														   , m_blockstate(DataFlowBlockStateReady)
																														   , m_func(func)
																														   , m_count(0)
																														   , m_lastpoststatus(Accepted)
																														   , m_processedItems(0L)
																														   , m_processingItems(0L)
		{ Init(); }

		template<typename T> 
		OutputDataFlowBlockCoreImplement<T>::~OutputDataFlowBlockCoreImplement()
		{
			try
			{
				m_taskgroup.wait();
			}
			catch(...)  //any exceptions will be ignored
			{}
		}

		//on init we start tasks by number of consumers (default MaxDegreeOfParallelism)
		//each task is trying to get element from queue. If it get one, it process it with func.
		//If no, task go to sleep to m_consumerqueue
		template<typename T> 
		void OutputDataFlowBlockCoreImplement<T>::Init()
		{
			m_numConsumers = m_options.MaxDegreeOfParallelism;
			bool result = false;

			for(int i = 0; i < m_numConsumers; ++i)
			{
				m_taskgroup.run([&, i]()
				{
					
#ifdef DEBUG_PDF_TRACE
					int consumerid = i;
					m_debug.Add(std::string("Start consumer ") + std::to_string(consumerid));
#endif
					while(true)
					{
						T t;
						result = m_queue.try_pop(t);
						if(result)
						{
#ifdef DEBUG_PDF_TRACE
							m_debug.Add(std::string("Start processing item. consumer ") + std::to_string(consumerid));
#endif

							_InterlockedIncrement(&m_processingItems);
							//if we had overflow of queue we have to send event that we can rceive a new item
							if(_InterlockedCompareExchangeSizeT(&m_lastpoststatus, DataFlowPostItemStatus::Accepted, DataFlowPostItemStatus::Decline) == DataFlowPostItemStatus::Decline)
							{
								if(m_readyEventReceiver.get())
								{
#ifdef DEBUG_PDF_TRACE
									m_debug.Add(std::string("Send ready event to producer. consumer ") + std::to_string(consumerid));
#endif

									m_readyEventReceiver->Invoke();
								}
	
							}
							_InterlockedDecrementSizeT(&m_count);
							m_func(t);
							_InterlockedIncrement(&m_processedItems);
							_InterlockedDecrement(&m_processingItems);

#ifdef DEBUG_PDF_TRACE
							m_debug.Add(std::string("Complete processing item. consumer ") + std::to_string(consumerid));
#endif
						}
						else
						{
							
							if(DataFlowBlockStateReady != m_blockstate)
							{
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
							size_t result = tce->wait();

#ifdef DEBUG_PDF_TRACE
							m_debug.Add(std::string("Woke up after no data with result ") 
									 + std::to_string(result)
									 + std::string(". consumer ") + std::to_string(consumerid));
#endif
						}
					}
				}
				);
			}
		}

		template<typename T> 
		void OutputDataFlowBlockCoreImplement<T>::DoComplete()
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
		Concurrency::task_group& OutputDataFlowBlockCoreImplement<T>::DoCompletion()
		{
			return m_taskgroup; 
		}

		template<typename T> 
		DataFlowPostItemStatus  OutputDataFlowBlockCoreImplement<T>::DoTryPostItem(T item)
		{
			DataFlowPostItemStatus result = DataFlowPostItemStatus::Accepted;

			//-1 means "minus infinity"
			if(m_count >= m_options.Capacity && -1 !=  m_options.Capacity)
			{
#ifdef DEBUG_PDF_TRACE				
				m_debug.Add(std::string("decline trypostitem (overflow)"));
#endif
				result = DataFlowPostItemStatus::Decline;
			}
			else
			{
#ifdef DEBUG_PDF_TRACE		
				m_debug.Add(std::string("received item from producer"));
#endif

				m_queue.push(item);
				_InterlockedIncrementSizeT(&m_count);
				
				std::shared_ptr<Concurrency::event> tce;
				if(m_consumerqueue.try_pop(tce))
				{
#ifdef DEBUG_PDF_TRACE	
					m_debug.Add(std::string("try to wake up consumer (consumer wait queue:") + std::to_string(m_consumerqueue.unsafe_size()) + std::string(")"));
#endif
					tce->set();
				}
			}
			//store last post result to m_lastpoststatus
			m_lastpoststatus =  result;
			return result;
		}

		//return capacity factor of output block
		template<typename T> 
		size_t OutputDataFlowBlockCoreImplement<T>::DoCapacityFactor() const
		{
			size_t freeConsumers = m_consumerqueue.unsafe_size();
			size_t freeSpace = m_options.Capacity - m_count;
			
			return freeConsumers > freeSpace ? freeConsumers : freeSpace;
		}

		template<typename T> 
		void OutputDataFlowBlockCoreImplement<T>::StartDebug()
		{ 
#ifdef DEBUG_PDF_TRACE
			m_debug.StartDebug(); 
#endif
		}
	}

	
}

#endif