#ifndef ACTIONBLOCK_T
#define ACTIONBLOCK_T


#include <functional>
#include <memory>
#include "concurrent_queue.h"
#include "ppltasks.h"

#include "DataFlowBlockState.h"
#include "DataflowBlockOptions.h"
//#include "OutputDataFlowBlockCoreImplement.h"
#include "OutputDataFlowBlockCore.h"

namespace Parallelia
{
	template<typename T> 
	class ActionBlock : public IOutputDataFlowBlock<T>, public IDataFlowBlock
	{
		typedef std::function<void(T)> F;
	public:
		explicit ActionBlock(F func, const DataflowBlockOptions& options);	
		explicit ActionBlock(F func);	
		virtual ~ActionBlock();
		bool Post(T item);
		size_t Count() { return m_coreimpl.Count(); }
		long ProcessedItems() const { return m_coreimpl.ProcessedItems(); }
		long ProcessingItems() const { return m_coreimpl.ProcessingItems(); }
		size_t NumSleeping() const { return m_coreimpl.NumSleeping(); }

		void StartDebug() { m_coreimpl.StartDebug(); }
	private:
		ActionBlock(const ActionBlock&);
		ActionBlock& operator=(const ActionBlock&);
		//IDataFlowBlock
		virtual void DoComplete() { m_coreimpl.Complete(); }
		virtual IDataFlowBlockCompletion& DoCompletion() { return m_coreimpl.Completion(); }

		//IOutputDataFlowBlock
		virtual DataFlowPostItemStatus DoTryPostItem(T item);
		virtual void DoRegisterReadyEventReceiver(std::shared_ptr<ParalleliaCore::ReadyEventReceiver > readyEventReveicer);
		virtual size_t  DoCapacityFactor() { return m_coreimpl.CapacityFactor(); }

		ParalleliaCore::OutputDataFlowBlockCore<T, void> m_coreimpl;

		
	};

	template<typename T> 
	ActionBlock<T>::ActionBlock(F func) :  m_coreimpl(func, DataflowBlockOptions::Default())
	{}


	template<typename T> 
	ActionBlock<T>::ActionBlock(F func, const DataflowBlockOptions& options) : m_coreimpl(func, options)
	{}

	template<typename T> 
	ActionBlock<T>::~ActionBlock()
	{ /*delete m_coreimpl;*/ }

	template<typename T> 
	bool ActionBlock<T>::Post(T item)
	{
		return DataFlowPostItemStatus::Accepted == TryPostItem(item);
	}



	template<typename T> 
	DataFlowPostItemStatus ActionBlock<T>::DoTryPostItem(T item)
	{
		return m_coreimpl.TryPostItem(item);
	}

	template<typename T> 
	void ActionBlock<T>::DoRegisterReadyEventReceiver(std::shared_ptr<ParalleliaCore::ReadyEventReceiver > readyEventReveicer)
	{
		m_coreimpl.RegisterReadyEventReceiver(readyEventReveicer);
	}

}


#endif