//
// Created by Casper Thule on 16/08/2021.
//

#ifndef OSMCPROJECT_CALLBACKTIMER_H
#define OSMCPROJECT_CALLBACKTIMER_H
#include <atomic>
#include <thread>
#include <functional>
// https://stackoverflow.com/a/30425945
class CallBackTimer
{
public:
    CallBackTimer()
            :_execute(false)
    {}

    ~CallBackTimer() {
        if( _execute.load(std::memory_order_acquire) ) {
            stop();
        };
    }

    void stop()
    {
        _execute.store(false, std::memory_order_release);
        if( _thd.joinable() )
            _thd.join();
    }

    void start(int intervalMs, std::function<void(void)> func)
    {
        if( _execute.load(std::memory_order_acquire) ) {
            stop();
        };
        _execute.store(true, std::memory_order_release);
        _thd = std::thread([this, intervalMs, func]()
                           {
                               while (_execute.load(std::memory_order_acquire)) {
                                   func();
                                   std::this_thread::sleep_for(
                                           std::chrono::milliseconds(intervalMs));
                               }
                           });
    }

    bool is_running() const noexcept {
        return ( _execute.load(std::memory_order_acquire) &&
                 _thd.joinable() );
    }

private:
    std::atomic<bool> _execute;
    std::thread _thd;
};
#endif //OSMCPROJECT_CALLBACKTIMER_H
