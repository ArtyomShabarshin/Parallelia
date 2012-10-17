#ifndef TRANSFORMBLOCK_T
#define TRANSFORMBLOCK_T


#include <functional>
#include <memory>
#include "concurrent_queue.h"
#include "concurrent_vector.h"
#include "ppltasks.h"

#include "BufferBlock.h"
#include "OutputDataFlowBlockCore.h"
#include "DataFlowBlockCompletion.h"


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
		size_t InputCount() { return m_input.Count(); }
		size_t OutputCount() { return m_output.Count(); }
		void StartDebug();
		long ProcessedItems() const { return m_input.ProcessedItems(); }
		long ProcessingItems() const { return m_input.ProcessingItems(); }
	private:
		TransformBlock(const TransformBlock&);
		TransformBlock& operator=(const TransformBlock&);
		//IDataFlowBlock
		virtual void DoComplete();
		virtual IDataFlowBlockCompletion& DoCompletion();

		//IOutputDataFlowBlock
		virtual DataFlowPostItemStatus DoTryPostItem(I item);
		virtual void DoRegisterReadyEventReceiver(std::shared_ptr<ParalleliaCore::ReadyEventReceiver > readyEventReveicer) { m_input.RegisterReadyEventReceiver(readyEventReveicer); }
		virtual size_t DoCapacityFactor(){ return m_input.CapacityFactor(); }

		//IInputDataFlowBlock
		virtual void DoLinkTo(std::shared_ptr<IOutputDataFlowBlock<O> >& outputBlock);

		void CompletionWait();
	private:
		ParalleliaCore::OutputDataFlowBlockCore<I,O> m_input;
		BufferBlock<O> m_output;
		Concurrency::concurrent_vector<std::shared_ptr<IOutputDataFlowBlock<O> > > m_linktovector;
		IDataFlowBlockCompletion* m_completion;
	};


	template<typename I, typename O> 
	TransformBlock<I,O>::TransformBlock(F func) :  m_input(func, DataflowBlockOptions::Default())
												 , m_output(DataflowBlockOptions::Default())
												 , m_completion(new ParalleliaCore::DataFlowBlockCompletion(std::bind(&TransformBlock<I,O>::CompletionWait, this)))
	{ m_input.SetLink(&m_output); }


	template<typename I, typename O> 
	TransformBlock<I,O>::TransformBlock(F func, const DataflowBlockOptions& options) : m_input(func, options)
																					 , m_output(options)
																					 , m_completion(new ParalleliaCore::DataFlowBlockCompletion(std::bind(&TransformBlock<I,O>::CompletionWait, this)))
	{ m_input.SetLink(&m_output); }

	template<typename I, typename O> 
	TransformBlock<I,O>::~TransformBlock()
	{}

	template<typename I, typename O> 
	void TransformBlock<I,O>::DoComplete()
	{
		 m_input.Complete(); 
		 m_output.Complete();
	}

	//we are sure that block is completed if all source items have been processed (stored in output queue)
	template<typename I, typename O> 
	IDataFlowBlockCompletion& TransformBlock<I,O>::DoCompletion()
	{
		return *m_completion;
	}

	
	template<typename I, typename O> 
	bool TransformBlock<I,O>::Post(I item)
	{
		return DataFlowPostItemStatus::Accepted == TryPostItem(item);
	}

	//put item into input queue
	template<typename I, typename O> 
	DataFlowPostItemStatus TransformBlock<I,O>::DoTryPostItem(I item)
	{
		return m_input.TryPostItem(item);
	}

	template<typename I, typename O> 
	void TransformBlock<I,O>::DoLinkTo(std::shared_ptr<IOutputDataFlowBlock<O> >& outputBlock)
	{
		m_output.LinkTo(outputBlock);
	}

	template<typename I, typename O> 
	void TransformBlock<I,O>::StartDebug()
	{
		m_input.StartDebug();
		m_output.StartDebug();
	}

	template<typename I, typename O> 
	void TransformBlock<I,O>::CompletionWait()
	{
		m_input.Completion().Wait();
		m_output.Complete();
		m_output.Completion().Wait();
	}

}


#endif