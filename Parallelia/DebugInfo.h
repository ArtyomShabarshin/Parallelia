#ifndef DEBUGINFO
#define DEBUGINFO

#include "concurrent_vector.h"
#include <string>

#include "PerfCounter.h"

namespace Parallelia
{
	namespace Utils
	{
		template<class T=std::string>
		class DebugInfo
		{
			struct DebugInfoItem
			{
				double dt;
				T info;
			};
		public:
			DebugInfo()
			{
				StartDebug();
			}

			virtual ~DebugInfo()
			{}

			void Add(const T& info);
			double GetCounter();
			void StartDebug();

		private:
			Concurrency::concurrent_vector<DebugInfoItem> m_store;
			PerfCounter m_counter;
		};

		//thread-safe store item to store
		template<class T>
		void DebugInfo<T>::Add(const T& info)
		{
			DebugInfoItem item;
			item.dt = GetCounter();
			item.info = info;
			m_store.push_back(item);
		}

		template<class T>
		double DebugInfo<T>::GetCounter()
		{
			return m_counter.GetCounter();
		}

		template<class T>
		void DebugInfo<T>::StartDebug()
		{
			m_counter.Start();
		}

	}

}


#endif