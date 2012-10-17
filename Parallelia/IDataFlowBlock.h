#ifndef IDATAFLOWBLOCK
#define IDATAFLOWBLOCK

#include "IDataFlowBlockCompletion.h"
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
		IDataFlowBlockCompletion& Completion() { return DoCompletion(); }
	protected:
		IDataFlowBlock(){}

	private:
		//no copy
		IDataFlowBlock(const IDataFlowBlock&);
		IDataFlowBlock& operator=(const IDataFlowBlock&);

		virtual void DoComplete() = 0;
		virtual IDataFlowBlockCompletion& DoCompletion() = 0;


		
	};


}


#endif