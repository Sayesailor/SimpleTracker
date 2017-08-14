#ifndef SIMPLE_TRACKER_TIMETOOL_H_
#define SIMPLE_TRACKER_TIMETOOL_H_

#ifdef __linux__

#include <cstdio>
#include <cstring>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

class ClockTool {
   public:
    ClockTool() : begin_(0), end_(0) {}

    inline void begin() { begin_ = clock(); }

    inline void end() { end_ = clock(); }

    inline void reset() {
        begin_ = 0;
        end_ = 0;
    }

    float get_interval() {
        if (end_ < begin_) return 0;
        return static_cast<double>(end_ - begin_) / CLOCKS_PER_SEC;
    }

   private:
    clock_t begin_;
    clock_t end_;
};

class TimeTool {
   public:
    TimeTool() { reset(); }

    inline void begin() { gettimeofday(&begin_, NULL); }

    inline void end() { gettimeofday(&end_, NULL); }

    inline void reset() {
        memset(&begin_, 0, sizeof(struct timeval));
        memset(&end_, 0, sizeof(struct timeval));
    }

    float get_interval() {
        if (end_.tv_usec < begin_.tv_usec) {
            end_.tv_usec += 1000;
            end_.tv_sec = end_.tv_sec - 1;
        }
        return (end_.tv_sec - begin_.tv_sec) * 1000 +
               (end_.tv_usec - begin_.tv_usec);
    }

   private:
    struct timeval begin_;
    struct timeval end_;
};

#endif  // __linux__
#endif
