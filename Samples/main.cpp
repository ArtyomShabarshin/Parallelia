//#define DEBUG_PDF_TRACE


//
#include "PerfCounter.h"
#include "ActionBlock.h"
#include "BufferBlock.h"
#include "TransformBlock.h"
#include "NullBlock.h"
#include <iostream>


using namespace Parallelia;
using namespace Parallelia::Utils;


void helloWorld();
void speedActionBlock();
void speedTransformBlock();
void linkedpredicate();
void nullblocksample();

void her(int& g)
{
	
}


int main()
{
	DataflowBlockOptions options;
	options.MaxDegreeOfParallelism = 1;
	options.Capacity = -1;


	TransformBlock<float, long> tb([=](float v){
		return (long)v;
	}, options);

	
	ActionBlock<long> ab2([=](long item)
	{ 
		
	}, options);

	tb.LinkTo(&ab2);

	for(long i =0; i < 10; ++i)
	{
		if(DataFlowPostItemStatus::Accepted !=  tb.TryPostItem((float)i))
		{
			break;
		}

	}

	tb.Complete();
	
	tb.Completion().Wait();

	int g = 0;







	//nullblocksample();
	//linkedpredicate();
	//helloWorld();
	//speedActionBlock();
	//speedTransformBlock();
}

volatile long counter1 = 0;

void nullblocksample()
{
	DataflowBlockOptions options;
	options.MaxDegreeOfParallelism = 4;
	ActionBlock<int> ab([=](int i){ _InterlockedIncrement(&counter1); });
	BufferBlock<int> bb(options);
	NullBlock<int> nb;
	bb.LinkTo(&ab, [=](int item) { return item > 0;});
	bb.LinkTo(&nb);
	bb.Post(1);
	bb.Post(-1);
	bb.Post(3);
	while(true)
	{
		int g = 0;
	}
}


volatile long g = 0;



void linkedpredicate()
{
	DataflowBlockOptions options;
	options.MaxDegreeOfParallelism = 4;
	ActionBlock<int> ab([=](int i){ _InterlockedIncrement(&counter1); });
	BufferBlock<int> bb(options);
	bb.LinkTo(&ab, [=](int item) { return item > 0;});
	bb.Post(1);
	bb.Post(-1);
	bb.Post(3);

	//bb.Complete();
	while(true)
	{
		int g = 0;
	}
}


void longplay()
{
	double mult = 0;
	for(int i = 0; i < 1000; ++i)
	{
		for(int j = 0; j < 10000; ++j)
		{
			mult += i * j;
		}
	}
	if(mult > 0)
	{
		_InterlockedIncrement(&g);
	}
	
}

long longtransform(float v)
{
	double mult = v;
	for(int i = 0; i < 10; ++i)
	{
		for(int j = 0; j < 1000; ++j)
		{
			mult += i * j;
		}
	}
	if(mult > 0)
	{
		_InterlockedIncrement(&g);
	}
	return g;
}

void speedTransformBlock()
{
	PerfCounter counter;
	DataflowBlockOptions options;
	options.MaxDegreeOfParallelism = 1;
	options.Capacity = -1;
	TransformBlock<float, long> tb([=](float v){
		return longtransform(v);
	}, options);
	counter.Start();
	long numItems = 10;
	tb.StartDebug();
	for(long i =0; i < numItems; ++i)
	{
		if(DataFlowPostItemStatus::Accepted !=  tb.TryPostItem((float)i))
		{
			break;
		}

	}
	volatile long sum = 0;
	ActionBlock<long> ab([=, &sum](long item)
	{ 
		sum += item;
	}, options);

	

	tb.Complete();
	tb.LinkTo(&ab);
	tb.Completion().Wait();
	float duration = counter.GetCounter();
	char c;
	
	std::cout << "size " << tb.ProcessedItems() << " processed items\n";
	std::cout << "Post " << numItems << " items\n";
	std::cout << "duration: " << duration << " ms\n";
	float speed = (float)numItems * 1000.0f / duration ;
	std::cout << "post speed " << speed << " items per second\n";
	std::cout << "output size " << tb.OutputCount() << " items\n";
	std::cout << g;
	std::cout << "press c to continue";
	std::cin >> c;

	

	
}

void speedActionBlock()
{
	PerfCounter counter;
	DataflowBlockOptions options;
	options.MaxDegreeOfParallelism = 8;
	options.Capacity = -1;
	//Parallelia::ParalleliaCore::OutputDataFlowBlockCoreImplement<int>core([=](int item)
	ActionBlock<int> core([=](int item)
	{ 
		//longplay();
	}, options);

	
	counter.Start();
	long numItems = 10000;
	for(long i =0; i < numItems; ++i)
	{
		if(DataFlowPostItemStatus::Accepted !=  core.TryPostItem((int)i))
		{
			break;
		}

	}



	
	core.Complete();
	core.Completion().Wait();
	float duration = counter.GetCounter();
	char c;
	
	std::cout << "size " << core.ProcessedItems() << " processed items\n";
	std::cout << "Post " << numItems << " items\n";
	std::cout << "duration: " << duration << " ms\n";
	float speed = (float)numItems * 1000.0f / duration ;
	std::cout << "post speed " << speed << " items per second\n";
	std::cout << g;
	std::cout << "press c to exit";
	std::cin >> c;
}


void helloWorld()
{

	ActionBlock<int> ab([=](int item)
	{ 
		longplay();
	});

	for(int i = 0; i < 100; ++i)
	{
		ab.Post(i);

	}
	ab.Complete();
	ab.Completion().Wait();



	DataflowBlockOptions options2;
	options2.MaxDegreeOfParallelism = 1;

	BufferBlock<int> bb(options2);

	bb.LinkTo(&ab);

	bb.StartDebug();
	ab.StartDebug();

	for(int i = 0; i < 10; ++i)
	{
		bb.Post(i);
	}

	

	printf("complete\n");
	getchar();
}


