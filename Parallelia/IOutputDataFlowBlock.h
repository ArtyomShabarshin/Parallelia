#ifndef IOUTPUTDATAFLOWBLOCK
#define IOUTPUTDATAFLOWBLOCK

#include "DataFlowPostItemStatus.h"
#include "ReadyEventReceiver.h"
#include <memory>

namespace Parallelia
{
	//interface for posting items
	template<typename T> 
	class IOutputDataFlowBlock
	{
	public:
		virtual ~IOutputDataFlowBlock(){}
		DataFlowPostItemStatus TryPostItem(T item) { return DoTryPostItem(item); }

		void RegisterReadyEventReceiver(std::shared_ptr<ParalleliaCore::ReadyEventReceiver > readyEventReveicer) { DoRegisterReadyEventReceiver(readyEventReveicer); }
		size_t  CapacityFactor() const { return DoCapacityFactor(); }
	protected:
		IOutputDataFlowBlock(){}

	private:
		//no copy
		IOutputDataFlowBlock(const IOutputDataFlowBlock&);
		IOutputDataFlowBlock& operator=(const IOutputDataFlowBlock&);

		virtual DataFlowPostItemStatus DoTryPostItem(T item) = 0;
		virtual void DoRegisterReadyEventReceiver(std::shared_ptr<ParalleliaCore::ReadyEventReceiver > readyEventReveicer) = 0;
		virtual size_t  DoCapacityFactor() const = 0;

	};


}


#endif