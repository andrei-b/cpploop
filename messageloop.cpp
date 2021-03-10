//
// Created by andrey on 2/5/21.
//

#include "messageloop.h"
#include "sleeper.h"
#include <spdlog/spdlog.h>
#include <utility>


CoreUtils::MessageLoop::MessageLoop() : registerLock(),baseEvent(0)
{
    addHandlerInternal([]{}, false);
}

event_id_t CoreUtils::MessageLoop::addHandler(std::function<void(void)> handler)
{
    return addHandlerInternal(handler, false);
}

void CoreUtils::MessageLoop::postEvent(event_id_t event)
{
    spdlog::trace("Message posted {}", event);
    events.put(event);
}

void CoreUtils::MessageLoop::postAndWait(event_id_t event)
{
    Sleeper s;
    postRoutine([&]{ handlerOf(event)(); s.wake(); });
    s.sleepForever();
}

void CoreUtils::MessageLoop::run()
{
    spdlog::trace("Enter message loop");
    while (!_exit) {
        auto e = events.take();
        spdlog::trace("Message received: {}", e);
        if (e == 0)
            break;
        spdlog::trace("Calling handler: {}", e);
        handlerOf(e)();
        dropOnceHandler(e);
    }
    spdlog::trace("Exit message loop");
}

void CoreUtils::MessageLoop::exit()
{
    postEvent(0);
    _exit = true;
}

event_id_t CoreUtils::MessageLoop::addHandlerInternal(std::function<void(void)> handler, bool once)
{
    std::lock_guard lck(registerLock);
    auto ne = newEvent();
    registredEvents.insert(std::make_pair<event_id_t, CoreUtils::EventItem>(reinterpret_cast<event_id_t &&>(ne), EventItem{ne, std::move(handler), once}));
    return ne;
}

void CoreUtils::MessageLoop::postRoutine(std::function<void(void)> routine)
{
    auto e = addHandlerInternal(std::move(routine), true);
    postEvent(e);
}

std::function<void(void)> & CoreUtils::MessageLoop::handlerOf(event_id_t e)
{
    std::lock_guard lck(registerLock);
    if (registredEvents.find(e)!= registredEvents.end())
        return registredEvents[e].handler;
    spdlog::error("No handler for event {}", e);
    return registredEvents[0].handler;
}

void CoreUtils::MessageLoop::dropOnceHandler(event_id_t e)
{
    std::lock_guard lck(registerLock);
    if (registredEvents.find(e)!= registredEvents.end())
        if (registredEvents[e].once)
            registredEvents.erase(e);
}

template <typename Result, typename... Params>
void CoreUtils::MessageLoop::postRoutine(std::function<Result(Params...)> routine, Params... args)
{
    postRoutine([&]{ routine(args...); });
}

template <typename Result, typename... Params>
Result CoreUtils::MessageLoop::postRoutineAndWait(std::function<Result(Params...)> routine, Params... args)
{
    Sleeper s;
    Result result;
    postRoutine([&]{ result = routine(args...); s.wake();});
    s.sleepForever();
    return result;
}

template <typename Class, typename Result, typename... Params>
void CoreUtils::MessageLoop::postMethodCall(std::function<Result(Params...)> method, Class * c, Params... args)
{
    auto f = std::bind(method, c);
    postRoutine<Result, Params...>(f, args...);
}

const event_id_t BaseEventHigh = 0x80000000;

event_id_t CoreUtils::MessageLoop::newEvent()
{
    if (baseEvent >= BaseEventHigh)
        for(auto i = baseEvent; i > 0; i--)
            if (registredEvents.find(i) == registredEvents.end())
                return i;
    return ++baseEvent;
}


