#ifndef DATAFLOWBLOCKCOREOMPLEMENT_T
#define DATAFLOWBLOCKCOREOMPLEMENT_T

#include "IDataFlowBlock.h"
#include "DataFlowBlockState.h"
#include "DataFlowBlockOptions.h"

#include "concurrent_queue.h"
#include "ppltasks.h"


namespace Parallelia
{

	namespace ParalleliaCore
	{
		//base class for all blocks
	//	//it has comletion operation, posting operation, control of parallelism
	//	template<typename T> 
	//	class DataFlowBlockCoreImplement : public IDataFlowBlock
	//	{
	//		typedef std::function<void(T)> F;
	//	public:
	//		DataFlowBlockCoreImplement();
	//		virtual ~DataFlowBlockCoreImplement();
	//	private:
	//		//no copy
	//		DataFlowBlockCoreImplement(const DataFlowBlockCoreImplement&);
	//		DataFlowBlockCoreImplement& operator=(const DataFlowBlockCoreImplement&);

	//		//IDataFlowBlock interface
	//		virtual void DoComplete();
	//		virtual Concurrency::task_group& DoCompletion();
	//	};

	//	template<typename T> 
	//	DataFlowBlockCoreImplement<T>::DataFlowBlockCoreImplement() 
	//	{}

	//	template<typename T> 
	//	DataFlowBlockCoreImplement<T>::~DataFlowBlockCoreImplement()
	//	{
	//		try
	//		{
	//			m_taskgroup.wait();
	//		}
	//		catch(...)  //any exceptions will be ignored
	//		{}
	//	}


	//	template<typename T> 
	//	void DataFlowBlockCoreImplement<T>::DoComplete()
	//	{
	//		m_blockstate = DataFlowBlockStateComplete;
	//		//run all threads and then they all will die
	//		//WHAT ABOUT IF IN THAT TIME SOMEONE CALLS TryPostItem(item); ??????
	//		Concurrency::task_completion_event<void> tce;
	//		while(m_consumerqueue.try_pop(tce))
	//		{
	//				tce.set();
	//		}
	//	}


	//	template<typename T> 
	//	Concurrency::task_group& DataFlowBlockCoreImplement<T>::DoCompletion()
	//	{
	//		return m_taskgroup; 
	//	}


	//	
	//	template<typename T> 
	//	DataFlowPostItemStatus DataFlowBlockCoreImplement<T>::DoTryPostItem(T item)
	//	{
	//		DataFlowPostItemStatus result = DataFlowPostItemStatus::Accepted;

	//		//-1 means "minus infinity"
	//		if(m_count >= m_options.Capacity && -1 !=  m_options.Capacity)
	//		{
	//			result = DataFlowPostItemStatus::Decline;
	//		}
	//		else
	//		{
	//			m_queue.push(item);
	//			_InterlockedIncrementSizeT(&m_count);
	//			
	//			Concurrency::task_completion_event<void> tce;
	//			if(m_consumerqueue.try_pop(tce))
	//			{
	//				tce.set();
	//			}
	//		}
	//		return result;
	//	}

	//	template <class T>
	//	size_t DataFlowBlockCoreImplement<T>::Count()
	//	{
	//		size_t count =  m_queue.unsafe_size();
	//		return count;
	//	}


	}

}


#endif