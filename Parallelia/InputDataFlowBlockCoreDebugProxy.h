#ifndef INPUTDATAFLOWBLOCKCOREDEBUGPROXY_H
#define INPUTDATAFLOWBLOCKCOREDEBUGPROXY_H

#include "DebugInfo.h"

namespace Parallelia
{
	namespace Utils
	{
		template<class T>
		class InputDataFlowBlockCoreDebugProxy
		{
		public:
			InputDataFlowBlockCoreDebugProxy() 
			{}

			virtual ~InputDataFlowBlockCoreDebugProxy()
			{}

			void StartConsumer(int consumerid);
			void FinishConsumer(int consumerid);
			void StartSendingItem(int consumerid, const T& item);
			void CompleteSendingItem(int consumerid, const T& item, int linkernumber, size_t maxcapacity);
			void NotCompleteSendingItem(int consumerid, const T& item, int linkernumber);
			void NotCompleteSendingItemSpinLock(int consumerid, const T& item);
			void GotoSleep(int consumerid);
			void NoDataGotoSleep(int consumerid);
			void Wokeup(int consumerid, const size_t& waitresult);
			void NoDataWokeup(int consumerid, const size_t& waitresult);
			void AddLink();
			void ReceivedItem(const T& item);
			void TryToWakeupConsumer();
			void StartDebug();
		private:
			//no copy
			InputDataFlowBlockCoreDebugProxy(const InputDataFlowBlockCoreDebugProxy&);
			InputDataFlowBlockCoreDebugProxy& operator=(const InputDataFlowBlockCoreDebugProxy&);


		private:
			DebugInfo<> m_debug;
		};

		template<class T>
		void InputDataFlowBlockCoreDebugProxy<T>::StartConsumer(int consumerid)
		{
#ifdef DEBUG_PDF_TRACE
			m_debug.Add(std::string("Start consumer ") + std::to_string(consumerid));
#endif
		}

		template<class T>
		void InputDataFlowBlockCoreDebugProxy<T>::FinishConsumer(int consumerid)
		{
#ifdef DEBUG_PDF_TRACE
			m_debug.Add(std::string("Finish consumer ") + std::to_string(consumerid));
#endif
		}

		template<class T>
		void InputDataFlowBlockCoreDebugProxy<T>::StartSendingItem(int consumerid, const T& item)
		{
#ifdef DEBUG_PDF_TRACE
			m_debug.Add(std::string("Start sending item [") + std::to_string(item) + std::string("]. consumer ") + std::to_string(consumerid));
#endif
		}

		template<class T>
		void InputDataFlowBlockCoreDebugProxy<T>::CompleteSendingItem(int consumerid, const T& item, int linkernumber, size_t maxcapacity)
		{
#ifdef DEBUG_PDF_TRACE
			m_debug.Add(std::string("Complete sending item [") + std::to_string(item) + std::string("]. to linker ") + std::to_string(linkernumber) + std::string(".(cap:") + std::to_string(maxcapacity) + std::string(") consumer ") + std::to_string(consumerid));
#endif
		}

		template<class T>
		void InputDataFlowBlockCoreDebugProxy<T>::NotCompleteSendingItem(int consumerid, const T& item, int linkernumber)
		{
#ifdef DEBUG_PDF_TRACE
			m_debug.Add(std::string("Not complete sending item [") + std::to_string(item) + std::string("]. to linker ") + std::to_string(linkernumber) + std::string(". consumer ") + std::to_string(consumerid));
#endif
		}

		template<class T>
		void InputDataFlowBlockCoreDebugProxy<T>::NotCompleteSendingItemSpinLock(int consumerid, const T& item)
		{
#ifdef DEBUG_PDF_TRACE
			m_debug.Add(std::string("Not complete sending item (spinlock timeout) [") + std::to_string(item) +  std::string("]. consumer ") + std::to_string(consumerid));
#endif
		}

		template<class T>
		void InputDataFlowBlockCoreDebugProxy<T>::GotoSleep(int consumerid)
		{
#ifdef DEBUG_PDF_TRACE
			m_debug.Add(std::string("Go to sleep. consumer ") + std::to_string(consumerid));
#endif
		}

		template<class T>
		void InputDataFlowBlockCoreDebugProxy<T>::Wokeup(int consumerid, const size_t& waitresult)
		{
#ifdef DEBUG_PDF_TRACE
			m_debug.Add(std::string("Woke up after no data with result ") 
						+ std::to_string(waitresult)
						+ std::string(". consumer ") + std::to_string(consumerid));
#endif
		}

		template<class T>
		void InputDataFlowBlockCoreDebugProxy<T>::NoDataGotoSleep(int consumerid)
		{
#ifdef DEBUG_PDF_TRACE
			m_debug.Add(std::string("No data. Go to sleep. consumer ") + std::to_string(consumerid));
#endif
		}

		template<class T>
		void InputDataFlowBlockCoreDebugProxy<T>::NoDataWokeup(int consumerid, const size_t& waitresult)
		{
#ifdef DEBUG_PDF_TRACE
			m_debug.Add(std::string("Woke up after no data. consumer ") + std::to_string(consumerid));
#endif
		}

		template<class T>
		void InputDataFlowBlockCoreDebugProxy<T>::AddLink()
		{
#ifdef DEBUG_PDF_TRACE	
			m_debug.Add(std::string("Add link"));
#endif
		}

		template<class T>
		void InputDataFlowBlockCoreDebugProxy<T>::ReceivedItem(const T& item)
		{
#ifdef DEBUG_PDF_TRACE	
			m_debug.Add(std::string("Received item [") + std::to_string(item) + std::string("]. (TryPostItem)"));
#endif
		}

		template<class T>
		void InputDataFlowBlockCoreDebugProxy<T>::TryToWakeupConsumer()
		{
#ifdef DEBUG_PDF_TRACE	
			m_debug.Add(std::string("Trying to wake up some consumer"));
#endif
		}

		template<class T>
		void InputDataFlowBlockCoreDebugProxy<T>::StartDebug()
		{
#ifdef DEBUG_PDF_TRACE	
			m_debug.StartDebug();
#endif
		}
	}

}


#endif