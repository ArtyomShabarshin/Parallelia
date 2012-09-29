#ifndef DEBUGINFO
#define DEBUGINFO

#include "concurrent_vector.h"
#include <string>
#include <ctime>

#include "Winbase.h"

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
			double m_pcFreq;
			__int64 m_counterStart;
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
			LARGE_INTEGER li;
			QueryPerformanceCounter(&li);
			double res =  double(li.QuadPart-m_counterStart) / m_pcFreq;
			return res;
		}

		void DebugInfo::StartDebug()
		{
			LARGE_INTEGER li;
			QueryPerformanceFrequency(&li);
			m_pcFreq= double(li.QuadPart)/1000.0;
			QueryPerformanceCounter(&li);
			m_counterStart = li.QuadPart;
		}

	}

}


#endif