#include "SubscriptionSubscriber.h"

using namespace Manifest_Core;

void SubscriberInterface::Registration(const bool& isRegistering)
{
	isSubscribed = isRegistering;

	if (isRegistering)	
		subscription->Subscribe(this);
	else
		subscription->Unsubscribe(this);		
}

SubscriberInterface::~SubscriberInterface()
{	
}