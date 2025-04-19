#include <iostream>
#include <fstream>
#include <memory>
#include <algorithm>
#include <vector>
#include <numeric> 
#include <string>

#include "ServiceInterface.h"


namespace Manifest_Core
{
	class Adder : public SubscriberInterface
	{
	private:
		std::vector<int> vals;
		int sum;
	public:
		const std::vector<int>& GetVals() const { return this->vals; };
		Adder(const Subscription* _sub) : sum(0), SubscriberInterface((Subscription*)_sub){};
		~Adder()
		{			
			if (isSubscribed)
			{
				std::cout << "adder is unsubscribing due to being deleted " << std::endl;
				Registration(false);
			}
		}
		void Update()
		{
			vals.emplace_back(vals.size());
			sum = std::accumulate(vals.begin(), vals.end(), 0);
			std::cout << "Current sum of values in Adder: " << sum << std::endl;
			if (vals.size() > 10)
				Registration(false);
		};
	};
}
