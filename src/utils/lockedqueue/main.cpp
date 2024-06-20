#include <iostream>
#include "LockedQueue.h"

int main() {
    std::cout << "Hello, World!" << std::endl;

    utils::LockedQueue<int> intDataQueue;

    if(intDataQueue.empty()) {
        std::cout << "my bag is empty, lets fill" << std::endl;
    }

    for(int i = 0;i<10;i++) {
        intDataQueue.push(i);
        std::cout << intDataQueue.size() << " integers sitting in my bar, add one integer " << std::endl;
    }

    std::cout << intDataQueue.size() << " integers sitting in my bar" << std::endl;

    std::cout << "take one integer" << std::endl;

    auto taken = intDataQueue.pop();

    std::cout << intDataQueue.size() << " integers sitting in my bar, last one gone was " << taken << std::endl;

    intDataQueue.clear();

    return 0;
}
