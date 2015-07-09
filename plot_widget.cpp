#include <QPainter>
#include <QDebug>
#include <qcommon.h>
#include "plot_widget.h"
#include <QImage>

/*-----------------plot-----------------*/
extern PLOT_INFO p_sample_data;
extern PLOT_INFO p_clear_data;

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
    plot_para.height = 65536;
    plot_para.width = 1000;
    plot_para.p_data = NULL;
}

void Plot_Widget::recei_fro_datapro_dataplot(PLOT_INFO plot_info)
{
    plot_para = plot_info;

    /* 指针为空时不应该进行绘图，否则程序会运行出错 */
    if(plot_para.p_data)
        update();
    else
    {
        qDebug("plot_para.p_data = NULL, plot operation was cancled!\n");
    }
}

/* 用户通过点击plot1和plot2来选择绘制哪个数据 */
void Plot_Widget::recei_fro_GUI_PLOT_DATA_TYPE(int plot_data_type)
{
    if(plot_data_type == SAMPLE_DATA)
        plot_para = p_sample_data;
    else if(plot_data_type == CLEAR_DATA)
        plot_para = p_clear_data;

    /* 指针为空时不应该进行绘图，否则程序会运行出错 */
    if(plot_para.p_data)
        update();
    else
    {
        qDebug("plot_para.p_data = NULL, plot operation was cancled!\n");
    }
}

void Plot_Widget::paintEvent(QPaintEvent *event)
{
    int i;
    int j;

    QPainter painter(this);

    /* 开机时候有可能点开绘图选项卡，此时会执行绘图事件，所以绘图时间函数中也应该对指针进行判断 */
    if(plot_para.p_data)
    {
/*
("Bitstream Charter", "Clean", "Clearlyu", "Clearlyu Alternate Glyphs", "Clearlyu Arabic",
"Clearlyu Arabic Extra", "Clearlyu Devanagari", "Clearlyu Devangari Extra", "Clearlyu Ligature",
"Clearlyu Pua", "Courier 10 Pitch", "Fangsong Ti", "Fixed [Jis]", "Fixed [Misc]", "Fixed [Sony]",
"Gothic", "Mincho", "Newspaper", "Nil", "Song Ti", "Standard Symbols L")

*/
        /* 先画数据文件名称，在转换坐标 */
        QRect rect(10, 10, 460, 20);
        QFont font("Clearlyu", 12);
        painter.setFont(font);
        painter.drawText(rect, Qt::AlignHCenter, plot_para.pic_name.remove(SYS_FILE_PATH));

        /* 设置视口（逻辑坐标） */
        painter.setWindow(0, 0, plot_para.width, plot_para.height);
        /* 坐标系平移 */
        painter.translate(0, plot_para.height);

        qDebug("plot_para.height = %ld, plot_para.width = %ld\n", plot_para.height, plot_para.width);

        for(i = 0; i < 10; i++)
        {
            painter.setPen(QPen(color[i]));

            for(j = 0; j < plot_para.width; j++)
            {
                painter.drawPoint(j, -plot_para.p_data[j][i]);
            }
        }
    }
    else
    {
        qDebug("plot_para.p_data = NULL, cann't plot!\n");

        /* 不显示数据时，显示一张图片 */
        QImage pic;

        if(pic.load(QString(E_NOSE_LOGO)))
        {
            qDebug() << "picture size:" << pic.size() << endl;
            painter.drawImage(QPoint(30, 30), pic);
        }


    }
}
