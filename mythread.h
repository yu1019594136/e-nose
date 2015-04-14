#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>
#include <QString>

/*********************逻辑控制线程*****************************/
class LogicControlThread : public QThread
{
    Q_OBJECT
public:
    explicit LogicControlThread(QObject *parent = 0);
    void stop();

protected:
    void run();

private:
    volatile bool stopped;

signals:

public slots:

};

/*********************硬件控制线程*****************************/
class HardWareControlThread : public QThread
{
    Q_OBJECT
public:
    explicit HardWareControlThread(QObject *parent = 0);
    void stop();

protected:
    void run();

private:
    volatile bool stopped;

signals:
    void send_ds18b20_temp(QString);

public slots:

};

/*********************数据处理线程*****************************/
class DataProcessThread : public QThread
{
    Q_OBJECT
public:
    explicit DataProcessThread(QObject *parent = 0);
    void stop();

protected:
    void run();

private:
    volatile bool stopped;

signals:

public slots:

};

/*********************dosomething()*****************************/
void dosomething();

#endif // MYTHREAD_H
