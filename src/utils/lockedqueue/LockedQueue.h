//
// Created by zysik on 17.11.20.
//

#ifndef LOCKEDQUEUE_LOCKEDQUEUE_H
#define LOCKEDQUEUE_LOCKEDQUEUE_H
/// @cond
#include <mutex>
#include <queue>
#include <list>
/// @endcond
namespace utils {
    // Simple mutex-guarded queue
    template<typename T>
    class LockedQueue {
    private:
        std::mutex mutex;
        std::queue<T> queue;
    public:
        void push(T value) {
            std::unique_lock<std::mutex> lock(mutex);
            queue.push(value);
        };

        // Get top message in the queue
        // Note: not exception-safe (will lose the message)
        T pop() {
            std::unique_lock<std::mutex> lock(mutex);
            T value;// = std::move(queue.front());
            //if (!queue.empty()) {
            std::swap(value, queue.front());
            queue.pop();
            //}
            return value;
        };

        void clear() {
            std::unique_lock<std::mutex> lock(mutex);
            // or use reflection to swap with new empty
            while (!queue.empty()) {
                queue.pop();
            }
        }

        bool empty() {
            std::unique_lock<std::mutex> lock(mutex);
            return queue.empty();
        }

        size_t size() {
            std::unique_lock<std::mutex> lock(mutex);
            return queue.size();
        }
    };
}
#endif //LOCKEDQUEUE_LOCKEDQUEUE_H
