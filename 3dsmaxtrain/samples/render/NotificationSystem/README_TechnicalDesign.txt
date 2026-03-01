
Author David Lanier

Some technical explanations to avoid code archeology.

The Notification API works like this.

There are 2 modes :
- Immediate to get notified as soon as a change happens.
- OnDemand, notification events are stored in the Notification client until you ask them. Asking the events flushes them from
	our local storage.

To get notifications, you create a client using a version number which can let the user ask for different notification 
engines using the same API. You can create several clients if needed.
INotificationClient* INotificationManager::RegisterNewImmediateClient(int version = -1);
INotificationClient* INotificationManager::RegisterNewOnDemandClient(int version = -1);

When the user asks the notification system to monitor something, we look for a Notifier or create one if no Notifier is created yet.
A Notifier is a class that may add a reference on the thing to monitor, or uses scene events or whatever way to get notified 
of a change for that thing.
See for example MaxNodeNotifier class.

A Notifier has listeners registered, listening to different messages from it. It can have at least one or many listeners. 
when a notification has been caught in Max, the notifier calls the listeners that this event has been caught using the HandleNotifyMessage function.
And the listeners call the user via the callback function provided in immediate mode or store the event in the Client in 
arrays when we use the Ondemand mode.

