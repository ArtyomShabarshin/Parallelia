

#ifndef BROADCASTBUFFERBLOCK_T
#define BROADCASTBUFFERBLOCK_T

#include <functional>

#include "InputDataFlowBlockCore.h"


namespace Parallelia
{
	template<typename T> 
	class BroadCastBufferBlock : public IInputDataFlowBlock<T>, public IOutputDataFlowBlock<T>, public IDataFlowBlock
	{
		typedef IOutputDataFlowBlock<T>*  Linktype;
	public:
		explicit BroadCastBufferBlock(const DataflowBlockOptions& options);	
		explicit BroadCastBufferBlock();	
		virtual ~BroadCastBufferBlock();
		bool Post(T item);
		size_t Count() { return m_coreimpl.Count(); }
		long ProcessedItems() const { return m_coreimpl.ProcessedItems(); }
		long ProcessingItems() const { return m_coreimpl.ProcessingItems(); }

		void StartDebug() { m_coreimpl.StartDebug(); }

	private:
		//no copy
		BroadCastBufferBlock(const BroadCastBufferBlock&);
		BroadCastBufferBlock& operator=(const BroadCastBufferBlock&);

		//IDataFlowBlock interface
		virtual void DoComplete();
		virtual IDataFlowBlockCompletion& DoCompletion();

		//IInputDataFlowBlock interface
		virtual void DoLinkTo(Linktype outputBlock, IInputDataFlowBlock<T>::Predicate predicate);

		//IOutputDataFlowBlock interface
		virtual DataFlowPostItemStatus DoTryPostItem(T item);
		virtual void DoRegisterReadyEventReceiver(std::shared_ptr<ParalleliaCore::ReadyEventReceiver > readyEventReveicer){};
		virtual size_t  DoCapacityFactor(){ return -1; }

	private:
		ParalleliaCore::InputDataFlowBlockCore<T> m_coreimpl;
	};

	template<typename T> 
	BroadCastBufferBlock<T>::~BroadCastBufferBlock()
	{}


	template<typename T> 
	BroadCastBufferBlock<T>::BroadCastBufferBlock():  m_coreimpl(DataflowBlockOptions::Default())
	{}


	template<typename T> 
	BroadCastBufferBlock<T>::BroadCastBufferBlock(const DataflowBlockOptions& options) : m_coreimpl(options)
	{}


	template<typename T> 
	bool BroadCastBufferBlock<T>::Post(T item)
	{
		return DataFlowPostItemStatus::Accepted == TryPostItem(item);
	}


	template<typename T> 
	void BroadCastBufferBlock<T>::DoLinkTo(Linktype outputBlock, IInputDataFlowBlock<T>::Predicate predicate)
	{
	
		m_coreimpl.LinkTo(outputBlock, predicate);

	}

	template<typename T> 
	IDataFlowBlockCompletion& BroadCastBufferBlock<T>::DoCompletion()
	{
		return m_coreimpl.Completion();
	}

	template<typename T> 
	void BroadCastBufferBlock<T>::DoComplete()
	{
		m_coreimpl.Complete();
	}

	template<typename T> 
	DataFlowPostItemStatus BroadCastBufferBlock<T>::DoTryPostItem(T item)
	{
		return m_coreimpl.TryPostItem(item);
	}



}

#endif