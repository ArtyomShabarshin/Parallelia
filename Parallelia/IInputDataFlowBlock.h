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
		typedef IOutputDataFlowBlock<T>*  Linktype;
	public:
		typedef std::function<bool(T)> Predicate;

		virtual ~IInputDataFlowBlock(){}
		void LinkTo(Linktype outputBlock) { DoLinkTo(outputBlock, [=](T item) { return true; });}
		void LinkTo(Linktype outputBlock, Predicate predicate) { DoLinkTo(outputBlock, predicate);}
	protected:
		IInputDataFlowBlock(){}
	private:
		//no copy
		IInputDataFlowBlock(const IInputDataFlowBlock&);
		IInputDataFlowBlock& operator=(const IInputDataFlowBlock&);

		virtual void DoLinkTo(Linktype outputBlock, Predicate predicate) = 0;
	};


}

#endif