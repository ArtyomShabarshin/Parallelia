#ifndef IDATAFLOWBLOCKCOMPLETION_H
#define IDATAFLOWBLOCKCOMPLETION_H




namespace Parallelia
{
	class IDataFlowBlockCompletion
	{
	public:
		virtual ~IDataFlowBlockCompletion()
		{}

		void Wait() { DoWait(); }

	private:
		virtual void DoWait() = 0;

	};

}


#endif