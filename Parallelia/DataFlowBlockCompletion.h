#ifndef DATAFLOWBLOCKCOMPLETION_H
#define DATAFLOWBLOCKCOMPLETION_H

#include "IDataFlowBlockCompletion.h"


namespace Parallelia
{

	namespace ParalleliaCore
	{

		class DataFlowBlockCompletion : public IDataFlowBlockCompletion
		{
			typedef std::function<void()> F;
		public:
			DataFlowBlockCompletion(F func) : m_func(func)
			{}

			virtual ~DataFlowBlockCompletion()
			{}
		private:
			//no copy
			DataFlowBlockCompletion(DataFlowBlockCompletion&);
			DataFlowBlockCompletion& operator=(DataFlowBlockCompletion&);

			//IDataFlowBlockCompletion
			virtual void DoWait();
		private:
			F m_func;
		};

		void DataFlowBlockCompletion::DoWait()
		{
			m_func();
		}

	}

}

#endif