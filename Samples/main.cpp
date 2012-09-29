#define DEBUG_PDF_TRACE


#include<windows.h>
#include "ActionBlock.h"
#include "BufferBlock.h"


using namespace Parallelia;


void helloWorld();

int main()
{
	helloWorld();
}


void longplay()
{
	double mult = 0;
	for(int i = 0; i < 1000; ++i)
	{
		for(int j = 0; j < 1000000; ++j)
		{
			mult += i * j;
		}
	}
}


void helloWorld()
{
	DataflowBlockOptions options;
	options.MaxDegreeOfParallelism = 4;
	options.Capacity = 8;

	ActionBlock<int> ab([=](int item)
	{ 
		longplay();
		printf("ab processed %d\n", item);
	}, options
	);

	DataflowBlockOptions options2;
	options2.MaxDegreeOfParallelism = 1;

	BufferBlock<int> bb(options2);

	bb.LinkTo(std::shared_ptr<IOutputDataFlowBlock<int> >(&ab));

	bb.StartDebug();
	ab.StartDebug();

	for(int i = 0; i < 10; ++i)
	{
		bb.Post(i);
	}



	printf("complete\n");
	getchar();
}


