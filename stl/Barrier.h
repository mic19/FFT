#ifndef FFT_BARRIER_H
#define FFT_BARRIER_H

#include <mutex>
#include <condition_variable>
#include <iostream>


class Barrier {
public:
    Barrier(int count):
            limit(count),
            count(count),
            generation(0) {}

    void wait() {
        std::unique_lock<std::mutex> lLock{mtx};
        auto lGen = generation;
        if (!--count) {
            ++generation;
            count = limit;
            cv.notify_all();
        } else {
            cv.wait(lLock, [this, lGen] { return lGen != generation; });
        }
    }

    std::size_t getCount() const{
        return count;
    }

private:
    std::mutex mtx;
    std::condition_variable cv;
    std::size_t limit;
    int count;
    std::size_t generation;
};




#endif
