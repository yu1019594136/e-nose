#include "thread_data_proc.h"

/*********************数据处理线程*****************************/
DataProcessThread::DataProcessThread(QObject *parent) :
    QThread(parent)
{
    //stopped = false;
}

void DataProcessThread::run()
{
//    while (!stopped)
//    {
//        dosomething();
//    }

    stopped = false;
}

void DataProcessThread::stop()
{
    stopped = true;
}



