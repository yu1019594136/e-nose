#ifndef PLOT_WIDGET_H
#define PLOT_WIDGET_H

#include <QWidget>
#include "common.h"
#include "qcommon.h"

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
    PLOT_INFO plot_para;

public slots:
    /* 接收来自数据处理线程的采样数据绘图命令 */
    void recei_fro_datapro_dataplot(PLOT_INFO plot_info);

    /* 用户通过点击plot1和plot2来选择绘制哪个数据 */
    void recei_fro_GUI_PLOT_DATA_TYPE(int plot_data_type);
};

#endif // PLOT_WIDGET_H
