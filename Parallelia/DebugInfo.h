#ifndef DEBUGINFO
#define DEBUGINFO

#include "concurrent_vector.h"
#include <string>

#include "PerfCounter.h"

namespace Parallelia
{
	namespace Utils
	{

		class DebugInfo
		{
			typedef std::string ItemDataType;

			struct DebugInfoItem
			{
				double dt;
				ItemDataType info;
			};
		public:
			DebugInfo()
			{
				StartDebug();
			}

			virtual ~DebugInfo()
			{}

			void Add(const ItemDataType& info);
			double GetCounter();
			void StartDebug();

		private:
			Concurrency::concurrent_vector<DebugInfoItem> m_store;
			PerfCounter m_counter;
		};

		//thread-safe store item to store
		void DebugInfo::Add(const ItemDataType& info)
		{
			DebugInfoItem item;
			item.dt = GetCounter();
			item.info = info;
			m_store.push_back(item);
		}

		double DebugInfo::GetCounter()
		{
			return m_counter.GetCounter();
		}

		void DebugInfo::StartDebug()
		{
			m_counter.Start();
		}

	}

}


#endif