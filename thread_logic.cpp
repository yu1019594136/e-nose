#include "thread_logic.h"

/*********************逻辑控制线程*****************************/
LogicControlThread::LogicControlThread(QObject *parent) :
    QThread(parent)
{
    stopped = false;
}

void LogicControlThread::run()
{
//    while (!stopped)
//    {
        //dosomething();
//    }

    stopped = false;
}
void LogicControlThread::stop()
{
    stopped = true;
}
