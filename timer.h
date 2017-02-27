
//timer functions
#include <sys/time.h>
#include <unistd.h>
class Timer{
    private:
    struct timeval _start, _end;

public:
    Timer(){}
    void start(){
        gettimeofday(&_start, NULL);
    }
    void end(){
        gettimeofday(&_end, NULL);
    }
    double getSecs(){
    return double(((_end.tv_sec  - _start.tv_sec) * 1000 + (_end.tv_usec - _start.tv_usec)/1000.0) + 0.5)/1000.;
    }

};
