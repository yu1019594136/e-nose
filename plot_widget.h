#ifndef PLOT_WIDGET_H
#define PLOT_WIDGET_H

#include <QWidget>
#include "common.h"

#define WIDTH 1000      //将宽度等分成WIDTH份
#define HEIGHT 65536     //将高度等分成HEIGH份

class Plot_Widget : public QWidget
{
    Q_OBJECT

public:
    Plot_Widget(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QColor color[10];
    long plot_width;
    long plot_height;
    unsigned long sample_count_real;

public slots:
    /* 接收来自数据处理线程的采样数据绘图命令 */
    void recei_fro_datapro_dataplot(PLOT_INFO plot_info);
};

#endif // PLOT_WIDGET_H
