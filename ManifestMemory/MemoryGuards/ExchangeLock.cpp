#include "ExchangeLock.h"

using namespace Manifest_Memory;

//exchange spin lock - spin locally & back off
void ExchangeLock::Lock()
{
	MFu32 spinWait{ 0 };
	while (true)
	{//if currently unlocked !false->true, else remains locked
		if (!lock.exchange(LOCKED, std::memory_order_acquire))
			return;
		spinWait += 2;
		//just read lock value without write		
		while (lock.load(std::memory_order_relaxed))
			for (volatile MFu32 spin{ 0 }; spin < spinWait; ++spin);
	}
};

MFbool ExchangeLock::ProbeLock()
{
	return lock.load(std::memory_order_relaxed);
}

void ExchangeLock::Unlock()
{
	lock.store(UNLOCKED, std::memory_order_release);
};
