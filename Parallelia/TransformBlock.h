#ifndef TRANSFORMBLOCK_T
#define TRANSFORMBLOCK_T


#include <functional>
#include <memory>
#include "concurrent_queue.h"
#include "concurrent_vector.h"
#include "ppltasks.h"

#include "BufferBlock.h"
#include "OutputDataFlowTransformBlockCoreImplement.h"


namespace Parallelia
{
	//How it works.
	//When client post the item I , it stores into input queue (m_input)
	template<typename I, typename O> 
	class TransformBlock : public IDataFlowBlock, public IInputDataFlowBlock<O>, public IOutputDataFlowBlock<I>
	{
		typedef std::function<O(I)> F;
	public:
		explicit TransformBlock(F func, const DataflowBlockOptions& options);	
		explicit TransformBlock(F func);	
		virtual ~TransformBlock();
		bool Post(I item);
		size_t InputCount() { return m_input->Count(); }
		size_t OutputCount() { return m_output->Count(); }
		void StartDebug();
	private:
		TransformBlock(const TransformBlock&);
		TransformBlock& operator=(const TransformBlock&);
		//IDataFlowBlock
		virtual void DoComplete();
		virtual Concurrency::task_group& DoCompletion();

		//IOutputDataFlowBlock
		virtual DataFlowPostItemStatus DoTryPostItem(I item);
		virtual void DoRegisterReadyEventReceiver(std::shared_ptr<ParalleliaCore::ReadyEventReceiver > readyEventReveicer) { m_input->RegisterReadyEventReceiver(readyEventReveicer); }
		virtual size_t DoCapacityFactor() const { return m_input->CapacityFactor(); }

		//IInputDataFlowBlock
		virtual void DoLinkTo(std::shared_ptr<IOutputDataFlowBlock<O> >& outputBlock);

		void Transform(I item);
	private:
		std::shared_ptr<ParalleliaCore::OutputDataFlowTransformBlockCoreImplement<I,O> > m_input;
		std::shared_ptr<BufferBlock<O> > m_output;
		Concurrency::concurrent_vector<std::shared_ptr<IOutputDataFlowBlock<O> > > m_linktovector;
	};


	template<typename I, typename O> 
	TransformBlock<I,O>::TransformBlock(F func) :  m_input(new ParalleliaCore::OutputDataFlowTransformBlockCoreImplement<I,O>(func, DataflowBlockOptions::Default()))
												 , m_output(new Parallelia::BufferBlock<O>(DataflowBlockOptions::Default()))
	{ m_input->SetLink(m_output); }


	template<typename I, typename O> 
	TransformBlock<I,O>::TransformBlock(F func, const DataflowBlockOptions& options) : m_inputcoreimpl(new ParalleliaCore::DataFlowBlockCoreImplement<I>(func, options))
																					 , m_outputcoreimpl(new ParalleliaCore::DataFlowBlockCoreImplement<O>(func, options))
																					 , m_func(func)
	{}

	template<typename I, typename O> 
	TransformBlock<I,O>::~TransformBlock()
	{}

	template<typename I, typename O> 
	void TransformBlock<I,O>::DoComplete()
	{
		 m_input->Complete(); 
		 m_output->Complete();
	}

	//we are sure that block is completed if all source items have been processed (stored in output queue)
	template<typename I, typename O> 
	Concurrency::task_group& TransformBlock<I,O>::DoCompletion()
	{
		return m_input->Completion();
	}

	
	template<typename I, typename O> 
	bool TransformBlock<I,O>::Post(I item)
	{
		return DataFlowPostItemStatus::Accepted == TryPostItem(item);
	}

	//process item from input to output queue
	template<typename I, typename O> 
	void TransformBlock<I,O>::Transform(I item)
	{
		O result = m_func(item);
		m_output->TryPostItem(result);
	}


	//put item into input queue
	template<typename I, typename O> 
	DataFlowPostItemStatus TransformBlock<I,O>::DoTryPostItem(I item)
	{
		return m_input->TryPostItem(item);
	}

	template<typename I, typename O> 
	void TransformBlock<I,O>::DoLinkTo(std::shared_ptr<IOutputDataFlowBlock<O> >& outputBlock)
	{
		m_output->LinkTo(outputBlock);
	}

	template<typename I, typename O> 
	void TransformBlock<I,O>::StartDebug()
	{
		m_input->StartDebug();
		m_output->StartDebug();
	}

}


#endif