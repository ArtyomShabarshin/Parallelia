#ifndef IDATAFLOWBLOCK
#define IDATAFLOWBLOCK

#include "ppl.h"


namespace Parallelia
{
	//interface for completion operation
	//any block has to have this operation
	class IDataFlowBlock
	{
	public:
		virtual ~IDataFlowBlock(){}
		void Complete() { DoComplete(); }
		Concurrency::task_group& Completion() { return DoCompletion(); }
	protected:
		IDataFlowBlock(){}

	private:
		//no copy
		IDataFlowBlock(const IDataFlowBlock&);
		IDataFlowBlock& operator=(const IDataFlowBlock&);

		virtual void DoComplete() = 0;
		virtual Concurrency::task_group& DoCompletion() = 0;


		
	};


}


#endif