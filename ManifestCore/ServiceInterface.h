#pragma once
#include <iostream>
#include <memory>
#include <algorithm>
#include <vector>
#include <numeric> 
#include <string>


namespace Manifest_Core
{
	class Subscription;
	
	//wrapper for subscription services
	//added as base class to services and provides 
	class SubscriberInterface : public std::enable_shared_from_this<SubscriberInterface>
	{
		protected:
			bool isSubscribed = false;
			Subscription* subscription;
		public:
			SubscriberInterface(Subscription* _sub) : subscription(_sub)
			{
			};
			virtual void Update() = 0;//overwritten by derived classes update = service
			void Registration(const bool& isRegistering);//update the registration to un/subscribe
			const bool& SubscriptionStatus() const { return isSubscribed; };
			virtual ~SubscriberInterface();
	};

	class Subscription
	{
		private:
			//list of service subscribers
			std::vector<SubscriberInterface*> subscribers;		
		public:
			//add subscriber to service
			void Subscribe(const SubscriberInterface* _subscriber)
			{
				subscribers.emplace_back((SubscriberInterface*)_subscriber);
			}
			//search thru list of service subscribers and check for caller
			//if caller is found remove subscription
			void Unsubscribe(const SubscriberInterface* _subscriber)
			{			
				subscribers.erase(std::remove_if(subscribers.begin(), subscribers.end(),
						[&](SubscriberInterface* subscriber)
						{
							return _subscriber == subscriber;
						}),	subscribers.end());			
			}
			//itterate thru list of service subscribers and push service update to subscribed members
			void Notify()
			{
				for (auto& s : subscribers)
					if (s->SubscriptionStatus()) //check subscriber is still active
						s->Update();//publish to subscriber		
			}
			~Subscription()
			{
				std::cout << "Subscription deletd " << std::endl;
			}
	};
};