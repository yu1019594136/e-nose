#include <QPainter>
#include <QDebug>
#include "plot_widget.h"


Plot_Widget::Plot_Widget(QWidget *parent)
{
    change = false;
    //qDebug() << "change = " << change << endl;
}
void Plot_Widget::paintEvent(QPaintEvent *event)
{
    int i;
    QPoint arry[100];

    QPainter painter(this);
//    qDebug() << "height = " << qMin(width(),height()) << endl;
//    qDebug() << "width = " << qMax(width(),height()) << endl;

    /* 开启抗锯齿效果 */
    //painter.setRenderHint(QPainter::Antialiasing);

    /* 设置视口（逻辑坐标） */
    painter.setWindow(-500, -32768, 1000, 65536);

    /* 坐标系平移,将y值取负号 */
    painter.translate(-500, 32768);

    /* 画坐标轴 */
//    painter.drawLine(QPoint(0, 0), QPoint(1000, 0));
//    painter.drawLine(QPoint(0, 0), QPoint(0, -65535));

    for(i = 0; i < 100; i++)
    {
        arry[i].setX(i * 10);
        arry[i].setY(i * -100);
    }
    painter.drawPoints(arry+50, 50);
}
