#ifndef PLOT_WIDGET_H
#define PLOT_WIDGET_H

#include <QWidget>

#define WIDTH 219
#define LENGTH 481

class Plot_Widget : public QWidget
{
    Q_OBJECT

public:
    bool change;
    Plot_Widget(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *event);

private:
};

#endif // PLOT_WIDGET_H
