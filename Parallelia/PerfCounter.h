#ifndef PERFCOUNTER
#define PERFCOUNTER

#include<windows.h>
#include "Winbase.h"

namespace Parallelia
{
	namespace Utils
	{
		class PerfCounter
		{
		public:
			PerfCounter()
			{}

			virtual ~PerfCounter()
			{}

			void Start()
			{
				LARGE_INTEGER li;
				QueryPerformanceFrequency(&li);
				m_pcFreq= double(li.QuadPart)/1000.0;
				QueryPerformanceCounter(&li);
				m_counterStart = li.QuadPart;
			}

			float GetCounter()
			{
				LARGE_INTEGER li;
				QueryPerformanceCounter(&li);
				double res =  double(li.QuadPart-m_counterStart) / m_pcFreq;
				return (float)res;
			}

		private:
			double m_pcFreq;
			__int64 m_counterStart;
		};
	}

}

#endif

