#ifndef SCHEDULER_H
#define SCHEDULER_H


#include <Arduino.h>


#define MAX_TASKS 20


typedef void (*TaskCallback)();



struct Task
{

    bool active;

    uint32_t interval;

    uint32_t lastRun;

    TaskCallback callback;

};



class Scheduler
{

public:


    Scheduler();


    bool addTask(
        uint32_t interval,
        TaskCallback callback
    );


    void run();



private:

    Task tasks[MAX_TASKS];

    uint8_t taskCount;


};



#endif