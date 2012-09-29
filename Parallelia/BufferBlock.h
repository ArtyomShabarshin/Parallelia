

#ifndef BUFFERBLOCK_T
#define BUFFERBLOCK_T

#include <functional>

#include "InputDataFlowBlockCoreImplement.h"


namespace Parallelia
{
	template<typename T> 
	class BufferBlock : public IInputDataFlowBlock<T>, public IOutputDataFlowBlock<T>, public IDataFlowBlock
	{
	public:
		explicit BufferBlock(const DataflowBlockOptions& options);	
		explicit BufferBlock();	
		virtual ~BufferBlock();
		bool Post(T item);
		size_t Count() { return m_coreimpl->Count(); }
		long ProcessedItems() const { return m_coreimpl->ProcessedItems(); }
		long ProcessingItems() const { return m_coreimpl->ProcessingItems(); }

		void StartDebug() { m_coreimpl->StartDebug(); }
	private:
		//no copy
		BufferBlock(const BufferBlock&);
		BufferBlock& operator=(const BufferBlock&);

		//IDataFlowBlock interface
		virtual void DoComplete();
		virtual Concurrency::task_group& DoCompletion();

		//IInputDataFlowBlock interface
		virtual void DoLinkTo(std::shared_ptr<IOutputDataFlowBlock<T> >& outputBlock);

		//IOutputDataFlowBlock interface
		virtual DataFlowPostItemStatus DoTryPostItem(T item);
		virtual void DoRegisterReadyEventReceiver(std::shared_ptr<ParalleliaCore::ReadyEventReceiver > readyEventReveicer){};
		virtual size_t  DoCapacityFactor() const { return -1; }

	private:
		std::shared_ptr<ParalleliaCore::InputDataFlowBlockCoreImplement<T> > m_coreimpl;
	};

	template<typename T> 
	BufferBlock<T>::~BufferBlock()
	{}


	template<typename T> 
	BufferBlock<T>::BufferBlock():  m_coreimpl(new ParalleliaCore::InputDataFlowBlockCoreImplement<T>(DataflowBlockOptions::Default()))
	{}


	template<typename T> 
	BufferBlock<T>::BufferBlock(const DataflowBlockOptions& options) : m_coreimpl(new ParalleliaCore::InputDataFlowBlockCoreImplement<T>(options))
	{}


	template<typename T> 
	bool BufferBlock<T>::Post(T item)
	{
		return DataFlowPostItemStatus::Accepted == TryPostItem(item);
	}


	//template<typename T> 
	//void BufferBlock<T>::TransmitItem(T item)
	//{
	//	//m_sync.lock_read(); //begin read

	//	Concurrency::concurrent_vector<std::shared_ptr<IOutputDataFlowBlock<T> > >::const_iterator it = m_linktovector.cbegin();
	//	bool complete = false;
	//	for(; it != m_linktovector.cend(); ++it)
	//	{
	//		if(DataFlowPostItemStatus::Accepted == (*it)->TryPostItem(item))
	//		{
	//			complete = true;
	//			break;
	//		}
	//	}
	//	if(!complete)//store it back to queue
	//	{
	//		Post(item);
	//	}

	//	//m_sync.unlock(); //end write
	//}

	template<typename T> 
	void BufferBlock<T>::DoLinkTo(std::shared_ptr<IOutputDataFlowBlock<T> >& outputBlock)
	{
		//m_sync.lock(); //begin write
		m_coreimpl->LinkTo(outputBlock);
		//m_linktovector.push_back(outputBlock);

		//m_sync.unlock(); //end write
	}

	template<typename T> 
	Concurrency::task_group& BufferBlock<T>::DoCompletion()
	{
		return m_coreimpl->Completion();
	}

	template<typename T> 
	void BufferBlock<T>::DoComplete()
	{
		m_coreimpl->Complete();
	}

	template<typename T> 
	DataFlowPostItemStatus BufferBlock<T>::DoTryPostItem(T item)
	{
		return m_coreimpl->TryPostItem(item);
	}



}

#endif