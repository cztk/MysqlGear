//
// Created by zysik on 17.11.20.
//

#ifndef TIMEDWAITER_TIMED_WAITER_H
#define TIMEDWAITER_TIMED_WAITER_H

/// @cond
#include <mutex>
#include <condition_variable>
/// @endcond

struct timed_waiter {
    void interrupt() {
        {
            std::lock_guard<std::mutex> l(m_);
            stop_ = true;
        }
        c_.notify_one();
    }

    // returns false if interrupted
    template<class Duration>
    bool wait_for(Duration duration) {
        std::unique_lock<std::mutex> l(m_);
        return !c_.wait_for(l, duration, [this]() { return stop_; });
    }

private:
    std::condition_variable c_;
    std::mutex m_;
    bool stop_ = false;
};


#endif //TIMEDWAITER_TIMED_WAITER_H
