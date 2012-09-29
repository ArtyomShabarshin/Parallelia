#ifndef IINPUTDATAFLOWBLOCK
#define IINPUTDATAFLOWBLOCK

#include "IOutputDataFlowBlock.h"
#include <memory>

namespace Parallelia
{
	//interface for link to operation
	template<typename T>
	class IInputDataFlowBlock
	{
	public:
		virtual ~IInputDataFlowBlock(){}
		void LinkTo(std::shared_ptr<IOutputDataFlowBlock<T> >& outputBlock) { DoLinkTo(outputBlock);}
	protected:
		IInputDataFlowBlock(){}
	private:
		//no copy
		IInputDataFlowBlock(const IInputDataFlowBlock&);
		IInputDataFlowBlock& operator=(const IInputDataFlowBlock&);

		virtual void DoLinkTo(std::shared_ptr<IOutputDataFlowBlock<T> >& outputBlock) = 0;
	};


}

#endif