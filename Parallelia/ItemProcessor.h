#ifndef ITEMPROCESSOR_H
#define ITEMPROCESSOR_H

#include "IInputDataFlowBlock.h"
#include "InputDataFlowBlockCoreDebugProxy.h"
#include "concurrent_vector.h"
#include "concurrent_queue.h"

namespace Parallelia
{

	namespace ParalleliaCore
	{
		template<typename T> 
		class ItemProcessor
		{
		public:
			typedef IOutputDataFlowBlock<T>*  Linktype;
			typedef std::pair<Linktype, typename IInputDataFlowBlock<T>::Predicate> LinkStoreItem;
			typedef typename Concurrency::concurrent_vector<LinkStoreItem>::const_iterator LinkIterator;
			typedef Concurrency::concurrent_queue<std::shared_ptr<Concurrency::event> > ConsumerQueue;
		
			ItemProcessor(Parallelia::Utils::InputDataFlowBlockCoreDebugProxy<T>& debug);
			virtual ~ItemProcessor();

			void ProcessItem(const T& t, int consumerid, ConsumerQueue& consumers) { DoProcessItem(t, consumerid, consumers); }
			void AddLink(Linktype outputBlock, typename IInputDataFlowBlock<T>::Predicate predicate);
			long ProcessedItems() const { return m_processedItems; }
		protected:
			LinkIterator BeginLink() { return m_linktovector.begin(); }
			LinkIterator EndLink() { return m_linktovector.end(); }
			Parallelia::Utils::InputDataFlowBlockCoreDebugProxy<T>& Debug() { return m_debug; }
			void IncrementProcessedItems() { _InterlockedIncrement(&m_processedItems); }
		private:
			//no copy
			ItemProcessor(const ItemProcessor&);
			ItemProcessor& operator=(const ItemProcessor&);

			virtual void DoProcessItem(const T& t, int consumerid, ConsumerQueue& consumers) = 0;
			

		private:
			Concurrency::concurrent_vector<LinkStoreItem> m_linktovector;
			Parallelia::Utils::InputDataFlowBlockCoreDebugProxy<T>& m_debug;
			long m_processedItems;
		};

		template<typename T> 
		ItemProcessor<T>::ItemProcessor(Parallelia::Utils::InputDataFlowBlockCoreDebugProxy<T>& debug) : m_debug(debug)
																										,m_processedItems(0L)
		{}

		template<typename T> 
		ItemProcessor<T>::~ItemProcessor()
		{}

		template<typename T> 
		void ItemProcessor<T>::AddLink(Linktype outputBlock, typename IInputDataFlowBlock<T>::Predicate predicate)
		{
			LinkStoreItem link = std::make_pair(outputBlock, predicate);
			m_linktovector.push_back(link);
		}



	}


}


#endif