#include <iostream>
#include <thread>
#include "timed_waiter.h"

class TimerDemoThread {
private:
    timed_waiter m_timer;
    bool cancel = false;

    void main_loop();

public:
    void stop();

    std::thread run();
};

void TimerDemoThread::main_loop() {
    std::cout << "thread main_loop start" << std::endl;

    while (m_timer.wait_for(std::chrono::seconds(20))) {
        if (cancel) break;
        std::cout << "thread main_loop loop" << std::endl;
    }

    std::cout << "thread main_loop end" << std::endl;
}

void TimerDemoThread::stop() {
    cancel = true;
    m_timer.interrupt();
}

std::thread TimerDemoThread::run() {
    return std::thread([this] { this->main_loop(); });
}

int main() {
    timed_waiter m_timer;
    std::cout << "Timed_Waiter example starts!" << std::endl;

    auto timerDemo_obj = new TimerDemoThread();
    auto timerDemo_thread = timerDemo_obj->run();

    if (m_timer.wait_for(std::chrono::seconds(30))) {
        std::cout << "30 Seconds later main says hello again!" << std::endl;
        timerDemo_obj->stop();
    }

    if (timerDemo_thread.joinable()) {
        timerDemo_thread.join();
    }

    delete timerDemo_obj;

    return 0;
}
