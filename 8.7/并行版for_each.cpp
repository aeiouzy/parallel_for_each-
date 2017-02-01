#include <thread>
#include <vector>
#include <future>

template<typename Iterator,typename Func>
void parallel_for_each(Iterator first, Iterator last, Func f)
{
	unsigned long const length = std::distance(first, last);
	if (!length)
		return;
	unsigned long const min_per_thread = 25;
	unsigned long const max_threads =
		(length + min_per_thread - 1) / min_per_thread;

	unsigned long const hardware_threads =
		std::thread::hardware_concurrency();

	unsigned long const num_threads =
		std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);

	unsigned long const block_size = length / num_threads;

	std::vector<std::future<void>> futures(num_threads - 1);//1
	std::vector<std::thread> threads(num_threads - 1);
	join_threads joiner(threads);

	Iterator block_start = first;
	for (unsigned long i = 0; i < (num_threads - 1); ++i)
	{
		Iterator block_end = block_start;
		std::advance(block_end, block_size);
		std::packaged_task<void(void)> task(	//2
			[=]() {std::for_each(block_start, block_end, f); }
		);
		futures[i] = task.get_future();
		threads[i] = std::thread(std::move(task));//3
		block_start = block_end;
	}
	std::for_each(block_satrt, last, f);
	for (unsigned long i = 0; i < (num_threads), ++i)
		futures[i].get();	//4
}

class join_threads
{
	std::vector<std::thread>& threads;
public:
	explicit join_threads(std::vector<std::thread> threads_):threads(threads_){}
	~join_threads()
	{
		for (unsigned long i = 0; i < threads.size(); ++i)
			if (threads[i].joinable())
				threads[i].join();
	}
};