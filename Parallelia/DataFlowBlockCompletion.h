#ifndef DATAFLOWBLOCKCOMPLETION_H
#define DATAFLOWBLOCKCOMPLETION_H

#include "IDataFlowBlockCompletion.h"


namespace Parallelia
{

	namespace ParalleliaCore
	{
		template<typename T> 
		class DataFlowBlockCompletion : public IDataFlowBlockCompletion
		{
			typedef std::function<T()> F;
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

		template<typename T> 
		void DataFlowBlockCompletion<T>::DoWait()
		{
			m_func();
		}

	}

}

#endif