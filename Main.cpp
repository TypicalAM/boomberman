#include <thread>
#include <iostream>

void myThread() {
    for (int i = 0; i < 10; ++i) {
        std::cout << "Hehe mam " << i << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
}

void myThread2() {
    for (int i = 0; i < 10; ++i) {
        std::cout << "Hehe mam " << i << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
}

int main() {
    std::thread t1(myThread);
    std::thread t2(myThread2);

    t1.join();
    t2.join();

    std::cout << "Both ended" << std::endl;
}
