#ifndef OUTPUTDATAFLOWBLOCKCORE_T
#define OUTPUTDATAFLOWBLOCKCORE_T

#include "concurrent_queue.h"
#include "ppltasks.h"


#include "IDataFlowBlock.h"
#include "IOutputDataFlowBlock.h"
#include "DebugInfo.h"

#include "DataFlowBlockState.h"
#include "DataFlowBlockOptions.h"
#include "IsVoid.h"
#include "Int2Type.h"
#include "DataFlowBlockCompletion.h"






namespace Parallelia
{

	namespace ParalleliaCore
	{

		//responsibility:
		//receive items, store its in the queue and execute ProcessItem Func on each item
		//item processing has many threads (to control parallel of degree use DataflowBlockOptions::MaxDegreeOfParallelism)
		//after that item stores to linked input
		template<typename I, typename O> 
		class OutputDataFlowBlockCore  : public IDataFlowBlock, public IOutputDataFlowBlock<I>
		{
			typedef std::function<O(I)> F;
		public:
			OutputDataFlowBlockCore(F func, const DataflowBlockOptions& options);
			virtual ~OutputDataFlowBlockCore();

			size_t Count() { return m_count; }//m_queue.unsafe_size(); }
			long ProcessedItems() const { return m_processedItems; }
			long ProcessingItems() const { return m_processingItems; }
			size_t NumSleeping() const { return m_consumerqueue.unsafe_size(); }
			void SetLink(IOutputDataFlowBlock<O>* linkBlock) { m_link = linkBlock; }
			void StartDebug();
		private:
			//no copy
			OutputDataFlowBlockCore(const OutputDataFlowBlockCore&);
			OutputDataFlowBlockCore& operator=(const OutputDataFlowBlockCore&);

			//IDataFlowBlock interface
			virtual void DoComplete();
			virtual IDataFlowBlockCompletion& DoCompletion();

			//IOutputDataFlowBlock<T> interface
			virtual DataFlowPostItemStatus DoTryPostItem(I item);
			virtual void DoRegisterReadyEventReceiver(std::shared_ptr<ParalleliaCore::ReadyEventReceiver > readyEventReveicer) { m_readyEventReceiver = readyEventReveicer; }
			virtual size_t DoCapacityFactor();

			void Init();
			void WaitForProducer(int consumerid);
			void ProceedItem(I& item, int consumerid);
			void ProceedItem(Int2Type<0>& p, I& item, int consumerid);
			void ProceedItem(Int2Type<1>& p, I& item, int consumerid);
			void CompletionWait();
		private:
			F m_func;
			Concurrency::concurrent_queue<I> m_queue;
			Concurrency::concurrent_queue<Concurrency::event* > m_consumerqueue;
			Concurrency::task_group m_taskgroup;
			DataFlowBlockState m_blockstate;
			DataflowBlockOptions m_options;
			volatile size_t m_count;
			std::shared_ptr<ParalleliaCore::ReadyEventReceiver > m_readyEventReceiver;
			volatile DataFlowPostItemStatus m_lastpoststatus;
			long m_processedItems;
			long m_processingItems;
			IOutputDataFlowBlock<O>* m_link;//we don't have to delete this pointer
			IDataFlowBlockCompletion* m_completion;

#ifdef DEBUG_PDF_TRACE
			//debug
			Parallelia::Utils::DebugInfo m_debug;
#endif

		};

		template<typename I, typename O> 
		OutputDataFlowBlockCore<I,O>::OutputDataFlowBlockCore(F func, const DataflowBlockOptions& options) : m_options(options)
																											, m_blockstate(DataFlowBlockStateReady)
																											, m_func(func)
																											, m_count(0)
																											, m_lastpoststatus(Accepted)
																											, m_processedItems(0L)
																											, m_processingItems(0L)
																											, m_completion(new DataFlowBlockCompletion(std::bind(&OutputDataFlowBlockCore<I,O>::CompletionWait, this)))
		{ Init(); }

		template<typename I, typename O> 
		OutputDataFlowBlockCore<I,O>::~OutputDataFlowBlockCore()
		{
			try
			{
				m_completion->Wait();
			}
			catch(...)  //any exceptions will be ignored
			{}
		}

		//on init we start tasks by number of consumers (default MaxDegreeOfParallelism)
		//each task is trying to get element from queue. If it get one, it process it with func.
		//If no, task go to sleep to m_consumerqueue
		template<typename I, typename O> 
		void OutputDataFlowBlockCore<I,O>::Init()
		{
			int numConsumers = m_options.MaxDegreeOfParallelism;
			bool result = false;

			for(int i = 0; i < numConsumers; ++i)
			{
				m_taskgroup.run([&, i]()
				{
					int consumerid = i;
#ifdef DEBUG_PDF_TRACE
					m_debug.Add(std::string("Start consumer ") + std::to_string(consumerid));
#endif
					while(true)
					{
						I item;
						result = m_queue.try_pop(item);
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

							ProceedItem(item, consumerid);

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

							WaitForProducer(consumerid);
						}
					}
				}
				);
			}
		}

		template<typename I, typename O> 
		void OutputDataFlowBlockCore<I,O>::DoComplete()
		{
			m_blockstate = DataFlowBlockStateComplete;
			//run all threads and then they all will die
			//WHAT ABOUT IF IN THAT TIME SOMEONE CALLS TryPostItem(item); ??????
			Concurrency::event* tce;
			while(m_consumerqueue.try_pop(tce))
			{
					tce->set();
			}
		}


		template<typename I, typename O> 
		IDataFlowBlockCompletion& OutputDataFlowBlockCore<I,O>::DoCompletion()
		{
			return *m_completion; 
		}

		template<typename I, typename O> 
		DataFlowPostItemStatus  OutputDataFlowBlockCore<I,O>::DoTryPostItem(I item)
		{
			DataFlowPostItemStatus result = DataFlowPostItemStatus::Accepted;

			//-1 means "minus infinity"
			size_t cap = m_options.Capacity;
			size_t currentCount = _InterlockedCompareExchangeSizeT(&m_count, 0, 0);
			if(currentCount >= cap && -1 !=  cap)
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
				
				Concurrency::event* tce;
				if(m_consumerqueue.try_pop(tce))
				{
#ifdef DEBUG_PDF_TRACE	
					m_debug.Add(std::string("try to wake up consumer (consumer wait queue:") + std::to_string(m_consumerqueue.unsafe_size()) + std::string(")"));
#endif
					tce->set();
				}
			}
			//store last post result to m_lastpoststatus
			_InterlockedExchange((long*)&m_lastpoststatus, result);
			return result;
		}

		//return capacity factor of output block
		template<typename I, typename O> 
		size_t OutputDataFlowBlockCore<I,O>::DoCapacityFactor()
		{
			size_t freeConsumers = m_consumerqueue.unsafe_size();
			size_t currentCount = _InterlockedCompareExchangeSizeT(&m_count, 0, 0);//
			size_t freeSpace = m_options.Capacity - currentCount;
			
			return freeConsumers > freeSpace ? freeConsumers : freeSpace;
		}

		template<typename I, typename O> 
		void OutputDataFlowBlockCore<I,O>::StartDebug()
		{ 
#ifdef DEBUG_PDF_TRACE
			m_debug.StartDebug(); 
#endif
		}

		//we have no any avialable items. go to sleep and wait for producer
		template<typename I, typename O> 
		void OutputDataFlowBlockCore<I,O>::WaitForProducer(int consumerid)
		{
#ifdef DEBUG_PDF_TRACE
			m_debug.Add(std::string("No data. Go to sleep. consumer ") + std::to_string(consumerid));
#endif
			Concurrency::event* tce = new Concurrency::event();
			m_consumerqueue.push(tce);
			size_t result = tce->wait();
			delete tce;

#ifdef DEBUG_PDF_TRACE
			m_debug.Add(std::string("Woke up after no data with result ") 
						+ std::to_string(result)
						+ std::string(". consumer ") + std::to_string(consumerid));
#endif
		}


		//process item and put result to linked consumers
		template<typename I, typename O> 
		void OutputDataFlowBlockCore<I,O>::ProceedItem(I& item, int consumerid)
		{
			ProceedItem(Int2Type<IsVoid<O>::value >(), item, consumerid);
		}

		template<typename I, typename O> 
		void OutputDataFlowBlockCore<I,O>::ProceedItem(Int2Type<0>& p, I& item, int consumerid)
		{
			m_func(item);
#ifdef DEBUG_PDF_TRACE
			m_debug.Add(std::string("Complete processing item. consumer ") + std::to_string(consumerid));
#endif
		}

		template<typename I, typename O> 
		void OutputDataFlowBlockCore<I,O>::ProceedItem(Int2Type<1>& p, I& item, int consumerid)
		{
			O output = m_func(item);
#ifdef DEBUG_PDF_TRACE
			m_debug.Add(std::string("Send item to link consumer ") + std::to_string(consumerid));
#endif
			m_link->TryPostItem(output);
		}

		template<typename I, typename O> 
		void OutputDataFlowBlockCore<I,O>::CompletionWait()
		{
			m_taskgroup.wait();
		}

	}

	
}

#endif