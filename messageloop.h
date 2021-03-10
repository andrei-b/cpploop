//
// Created by andrey on 2/5/21.
//

#ifndef CORE_MESSAGELOOP_H
#define CORE_MESSAGELOOP_H
#include <functional>
#include <list>
#include "blockingqueue.h"
#include <mutex>
#include <map>
#include <condition_variable>
#include <thread>

using event_id_t = uint32_t;

namespace CoreUtils
{


    struct EventItem
    {
        event_id_t event  = 0;
        std::function<void(void)> handler;
        bool once = false;
    };

    class MessageLoop
    {
    public:
        MessageLoop();
        MessageLoop(const MessageLoop &) = delete;
        MessageLoop(MessageLoop &&) = delete;
        MessageLoop &operator=(const MessageLoop &rhs) = delete;
        void run();
        event_id_t addHandler(std::function<void(void)> handler);
        void postEvent(event_id_t event);
        void postAndWait(event_id_t event);
        void postRoutine(std::function<void(void)> routine);
        template<typename Result, typename ... Params>
        void postRoutine(std::function<Result(Params...)> routine, Params... args);
        template <typename Result, typename... Params>
        Result postRoutineAndWait(std::function<Result(Params...)> routine, Params... args);
        template<typename Class, typename Result, typename ... Params>
        void postMethodCall(std::function<Result(Params...)> method, Class * c, Params... args);
        [[nodiscard]] std::function<void(void)> &handlerOf(event_id_t e);
        void exit();
    private:
        event_id_t addHandlerInternal(std::function<void(void)> handler, bool once);
        void dropOnceHandler(event_id_t e);
        event_id_t newEvent();
        std::map<event_id_t, EventItem> registredEvents;
        std::mutex registerLock;
        BlockingQueue<event_id_t> events;
        event_id_t baseEvent;
        bool _exit = false;
    };
}

#endif  // CORE_MESSAGELOOP_H
