#include "scheduler.h"



Scheduler::Scheduler()
{

    taskCount = 0;


    for(int i=0;i<MAX_TASKS;i++)
    {
        tasks[i].active=false;
    }

}




bool Scheduler::addTask(
    uint32_t interval,
    TaskCallback callback
)
{

    if(taskCount>=MAX_TASKS)
        return false;



    tasks[taskCount].active=true;

    tasks[taskCount].interval=interval;

    tasks[taskCount].lastRun=0;

    tasks[taskCount].callback=callback;


    taskCount++;


    return true;

}




void Scheduler::run()
{

    uint32_t now=millis();



    for(int i=0;i<taskCount;i++)
    {

        if(!tasks[i].active)
            continue;



        if(now - tasks[i].lastRun >= tasks[i].interval)
        {

            tasks[i].lastRun=now;


            if(tasks[i].callback)
            {
                tasks[i].callback();
            }

        }

    }

}