#ifndef NULLBLOCK_T
#define NULLBLOCK_T


#include "DataFlowBlockState.h"
#include "DataflowBlockOptions.h"
//#include "OutputDataFlowBlockCoreImplement.h"
#include "OutputDataFlowBlockCore.h"

namespace Parallelia
{
	template <typename T>
	class NullBlock : public IOutputDataFlowBlock<T>, public IDataFlowBlock
	{
		class NullBlockCompletion : public IDataFlowBlockCompletion
		{
		private:
			virtual void DoWait() {};
		};
	public:
		NullBlock() : m_processedItems(0L)
		{}
		virtual ~NullBlock() {}
		long ProcessedItems() const { return m_coreimpl.ProcessedItems(); }
	private:
		NullBlock(const NullBlock&);
		NullBlock& operator=(const NullBlock&);
		//IDataFlowBlock
		virtual void DoComplete() {}
		virtual IDataFlowBlockCompletion& DoCompletion() { return m_completion; }

		//IOutputDataFlowBlock
		virtual DataFlowPostItemStatus DoTryPostItem(T item) { _InterlockedIncrement(&m_processedItems); return DataFlowPostItemStatus::Accepted; }
		virtual void DoRegisterReadyEventReceiver(std::shared_ptr<ParalleliaCore::ReadyEventReceiver > readyEventReveicer){};
		//it means this block always has 1 empty place for receiving items.
		virtual size_t  DoCapacityFactor() { return 1; }
	private:
		NullBlockCompletion m_completion;
		volatile long m_processedItems;
	};

}

#endif