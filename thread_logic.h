#ifndef THREAD_LOGIC_H
#define THREAD_LOGIC_H

#include <QThread>
#include <QString>
#include <QTimer>

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

private slots:

};

#endif // THREAD_LOGIC_H
