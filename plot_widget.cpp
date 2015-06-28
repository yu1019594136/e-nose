#include <QPainter>
#include <QDebug>
#include "plot_widget.h"

/* 该变量在数据处理线程的实现文件中声明 */
//extern u_int16_t **p_data;

Plot_Widget::Plot_Widget(QWidget *parent)
{
    /* Qt color */
/*
Constant        Value       Description
Qt::white       3           White (#ffffff)
Qt::black       2           Black (#000000)
Qt::red         7           Red (#ff0000)
Qt::darkRed     13          Dark red (#800000)
Qt::green       8           Green (#00ff00)
Qt::darkGreen	14          Dark green (#008000)
Qt::blue        9           Blue (#0000ff)
Qt::darkBlue	15          Dark blue (#000080)
Qt::cyan        10          Cyan (#00ffff)
Qt::darkCyan	16          Dark cyan (#008080)
Qt::magenta     11          Magenta (#ff00ff)
Qt::darkMagenta	17          Dark magenta (#800080)
Qt::yellow      12          Yellow (#ffff00)
Qt::darkYellow	18          Dark yellow (#808000)
Qt::gray        5           Gray (#a0a0a4)
Qt::darkGray	4           Dark gray (#808080)
Qt::lightGray	6           Light gray (#c0c0c0)
Qt::transparent	19          a transparent black value (i.e., QColor(0, 0, 0, 0))
Qt::color0      0           0 pixel value (for bitmaps)
Qt::color1      1           1 pixel value (for bitmaps)
*/
    color[0] = Qt::black;
    color[1] = Qt::red;
    color[2] = Qt::green;
    color[3] = Qt::blue;
    color[4] = Qt::cyan;
    color[5] = Qt::magenta;
    color[6] = Qt::yellow;
    color[7] = Qt::gray;
    color[8] = Qt::darkBlue;
    color[9] = Qt::darkRed;

    /* 默认情况下，横坐标和纵坐标等于屏幕的宽、高像素点 */
    plot_width = 1000;
    plot_height = 65536;
    sample_count_real = 0;
}

void Plot_Widget::recei_fro_datapro_dataplot(PLOT_INFO plot_info)
{
    /* 更新绘图尺寸 */
//    plot_width = plot_info.width;
//    plot_height = plot_info.height;
//    sample_count_real = plot_info.sample_count_real;

//    /* 没有数据或者没有采样时不应该进行绘图，否则程序会运行出错 */
//    if(p_data && sample_count_real)
//        update();
}

void Plot_Widget::paintEvent(QPaintEvent *event)
{
//    int i;
//    int j;
//    long x_axis = 0;

    QPainter painter(this);

//    qDebug() << "height = " << qMin(width(),height()) << endl;
//    qDebug() << "width = " << qMax(width(),height()) << endl;

    /* 开启抗锯齿效果 */
    //painter.setRenderHint(QPainter::Antialiasing);

    /* 画坐标轴 */
//    painter.drawLine(QPoint(0, 0), QPoint(1000, 0));
//    painter.drawLine(QPoint(0, 0), QPoint(0, -65535));

//    /* 设置视口（逻辑坐标） */
//    painter.setWindow(-plot_width/2, -plot_height/2, plot_width, plot_height);

//    /* 坐标系平移,将y值取负号 */
//    painter.translate(-plot_width/2, plot_height/2);

//    if(sample_count_real && sample_count_real < plot_width)
//    {
//        for(j = 0; j < 10; j++)
//        {
//            painter.setPen(QPen(color[j]));

//            for(i = 0; i < sample_count_real; i++)
//            {
//                painter.drawPoint(i, -p_data[i][j]);
//            }
//        }
//    }
//    else if(sample_count_real)
//    {
//        for(j = 0; j < 10; j++)
//        {
//            painter.setPen(QPen(color[j]));

//            for(i = sample_count_real - plot_width; i < sample_count_real; i++)
//            {
//                painter.drawPoint(x_axis++, -p_data[i][j]);
//            }
//            x_axis = 0;
//        }
//    }

}
