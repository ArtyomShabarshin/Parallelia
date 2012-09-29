#ifndef READYEVENTRECEIVER_T
#define READYEVENTRECEIVER_T


#include "IDataFlowBlock.h"
#include "IOutputDataFlowBlock.h"

#include "DataFlowBlockState.h"
#include "DataFlowBlockOptions.h"

#include "concurrent_queue.h"
#include "ppltasks.h"


namespace Parallelia
{

	namespace ParalleliaCore
	{
		typedef std::function<void()> F;
		class ReadyEventReceiver
		{
		public:
			virtual ~ReadyEventReceiver()
			{}
			ReadyEventReceiver(F func) : m_func(func)
			{}

			void Invoke() { m_func(); }

		private:
			//no copy
			ReadyEventReceiver(const ReadyEventReceiver&);
			ReadyEventReceiver& operator=(const ReadyEventReceiver&);
		private:
			F m_func;

		};
	}

}

#endif