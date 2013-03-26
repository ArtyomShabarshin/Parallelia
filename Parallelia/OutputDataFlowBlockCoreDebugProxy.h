#ifndef OUTPUTDATAFLOWBLOCKCOREDEBUGPROXY_H
#define OUTPUTDATAFLOWBLOCKCOREDEBUGPROXY_H

#include "DebugInfo.h"

namespace Parallelia
{
	namespace Utils
	{
		template<class T>
		class OutputDataFlowBlockCoreDebugProxy
		{
		public:
			OutputDataFlowBlockCoreDebugProxy() 
			{}

			virtual ~OutputDataFlowBlockCoreDebugProxy()
			{}

			void StartConsumer(int consumerid);
			void FinishConsumer(int consumerid);
			void StartProcessingItem(int consumerid, const T& item);
			void CompleteProcessingItem(int consumerid, const T& item);
			void SendReadyEventToProducer(int consumerid);
			void DeclineTryPostItem(const T& item);
			void ReceivedItem(const T& item);
			void TryToWakeupConsumer(const size_t& size);
			void StartDebug(); 
			void GotoSleep(int consumerid);
			void Wokeup(int consumerid, const size_t& waitresult);
			void SendItemToLinkConsumer(int consumerid, const T& item);

		private:
			//no copy
			OutputDataFlowBlockCoreDebugProxy(const OutputDataFlowBlockCoreDebugProxy&);
			OutputDataFlowBlockCoreDebugProxy& operator=(const OutputDataFlowBlockCoreDebugProxy&);


		private:
			DebugInfo<> m_debug;
		};

		template<class T>
		void OutputDataFlowBlockCoreDebugProxy<T>::StartConsumer(int consumerid)
		{
#ifdef DEBUG_PDF_TRACE
			m_debug.Add(std::string("Start consumer ") + std::to_string(consumerid));
#endif
		}

		template<class T>
		void OutputDataFlowBlockCoreDebugProxy<T>::FinishConsumer(int consumerid)
		{
#ifdef DEBUG_PDF_TRACE
			m_debug.Add(std::string("Finish consumer ") + std::to_string(consumerid));
#endif
		}

		template<class T>
		void OutputDataFlowBlockCoreDebugProxy<T>::StartProcessingItem(int consumerid, const T& item)
		{
#ifdef DEBUG_PDF_TRACE
			m_debug.Add(std::string("Start processing item [") + std::to_string(item) + std::string("]. consumer ") + std::to_string(consumerid));
#endif
		}

		template<class T>
		void OutputDataFlowBlockCoreDebugProxy<T>::CompleteProcessingItem(int consumerid, const T& item)
		{
#ifdef DEBUG_PDF_TRACE
			m_debug.Add(std::string("Complete processing item [") + std::to_string(item) + std::string("]. consumer ") + std::to_string(consumerid));
#endif
		}

		template<class T>
		void OutputDataFlowBlockCoreDebugProxy<T>::SendReadyEventToProducer(int consumerid)
		{
#ifdef DEBUG_PDF_TRACE
			m_debug.Add(std::string("Send ready event to producer. consumer ") + std::to_string(consumerid));
#endif
		}

		template<class T>
		void OutputDataFlowBlockCoreDebugProxy<T>::DeclineTryPostItem(const T& item)
		{
#ifdef DEBUG_PDF_TRACE				
			m_debug.Add(std::string("Decline trypostitem (overflow) item[") +  std::to_string(item) + std::string("]."));
#endif
		}

		template<class T>
		void OutputDataFlowBlockCoreDebugProxy<T>::ReceivedItem(const T& item)
		{
#ifdef DEBUG_PDF_TRACE		
			m_debug.Add(std::string("received item from producer [")+  std::to_string(item) + std::string("]."));
#endif
		}

		template<class T>
		void OutputDataFlowBlockCoreDebugProxy<T>::TryToWakeupConsumer(const size_t& size)
		{
#ifdef DEBUG_PDF_TRACE	
			m_debug.Add(std::string("try to wake up consumer (consumer wait queue:") + std::to_string(size) + std::string(")"));
#endif
		}


		template<class T>
		void OutputDataFlowBlockCoreDebugProxy<T>::StartDebug()
		{
#ifdef DEBUG_PDF_TRACE	
			m_debug.StartDebug();
#endif
		}

		template<class T>
		void OutputDataFlowBlockCoreDebugProxy<T>::GotoSleep(int consumerid)
		{
#ifdef DEBUG_PDF_TRACE
			m_debug.Add(std::string("No data. Go to sleep. consumer ") + std::to_string(consumerid));
#endif
		}

		template<class T>
		void OutputDataFlowBlockCoreDebugProxy<T>::Wokeup(int consumerid, const size_t& waitresult)
		{
#ifdef DEBUG_PDF_TRACE
			m_debug.Add(std::string("Woke up after no data with result ") 
						+ std::to_string(waitresult)
						+ std::string(". consumer ") + std::to_string(consumerid));
#endif
		}

		template<class T>
		void OutputDataFlowBlockCoreDebugProxy<T>::SendItemToLinkConsumer(int consumerid, const T& item)
		{
#ifdef DEBUG_PDF_TRACE
			m_debug.Add(std::string("Send item to link. item [") + std::to_string(item) + std::string("]. consumer ") + std::to_string(consumerid));
#endif
		}

	}


}

#endif