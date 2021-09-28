# CPP Loop

Simple C++ message loop and timer implementation.

## Usage

```
CoreUtils::event_id_t eventId;
CoreUtils::MessageLoop loop;
void eventHandler()
{
...
}
# Creating an event handler as a limbda function
# The value returned is the event id. 
eventId = loop.addHandler(eventHandler);
loop.run(); 
```
The ```loop.run()``` blocks its calling thread until some other thread calls ```loop._stopped()```. Now you can post events to the loop from other threads:
```
loop.postEvent(eventId); 
```
This will cause the event's handler to be executed within the thread where the ```loop.run()``` was called.
If you need a handler tp be executed only once, you can provide it as the ```postRoutine()``` method:
```
   loop.postRoutine([](){...});
```
Use ```postMethodCall()``` to post the method of some object. These post methods return immediately after they have been called, and usually before the corresponding handler is executed. If you want to block the calling thread until the handler is executed, use ```postEventAndWait()```.
The Timer object will make the attached handler to be called periodically:
```
void handler()
{
...
}
CoreUtils::Timer timer(loop, 1000, handler);
...
timer.stop();
```
The timer starts after it is created.