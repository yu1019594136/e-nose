#ifndef QCOMMON_H
#define QCOMMON_H

#include <QString>

typedef struct{
    int thermo_switch;//加热带电路开关，HIGH? LOW?
    float preset_temp;//预设温度
    //int hold_time;//蒸发时间
    //bool wait_temp_stable;//是否等待温度稳定在开始恒温
    bool thermo_inform_flag;//达到恒温状态后是否需要硬件线程返回信号通知逻辑线程
} THERMOSTAT;

enum SYSTEM_STATE{
    STANDBY = 0,    //待机
    PREHEAT,        //预热
    THERMO,         //恒温
    EVAPORATION,    //蒸发
    SAMPLING,       //采样
    CLEAR,          //清洗
    QUIT            //退出
};

typedef struct{
    float sample_freq;//每个通道的采样频率
    int sample_time;//每个通道的时间长度
    QString filename_prefix;
    bool sample_inform_flag;//采样完成后是否需要硬件线程返回信号通知逻辑线程
} SAMPLE;

/* 需要实时更新的信息 */
typedef struct{
    QString ds18b20_temp;
    QString sht21_temp;
    QString sht21_humid;
} GUI_REALTIME_INFO;

typedef struct{
    float preset_temp;//预设温度
    int hold_time;//蒸发时间
    int pump_up_time;//打气时间
    float sample_freq;//每个通道的采样频率
    int sample_time;//每个通道的时间长度
    int evapor_clear_time;//蒸发室清洗时间
    int reac_clear_time;//反应室清洗时间
    int sample_style;

    /* 呼吸时间设置 */
    unsigned int inhale_time;
    unsigned int inhale_wait_time;
    unsigned int exhale_time;
    unsigned int exhale_wait_time;
    unsigned int hale_count;

    QString data_file_path;
} SYSTEM_PARA_SET;

enum SAMPLE_STYLE{
    SINGLE = 0,
    CONTINUE
};

typedef struct{
    int standby_flag;
    int preheat_flag;
    int thermo_flag;
    int evaporation_flag;
    int sampling_flag;
    int clear_flag;
} OPERATION_FLAG;//各个标签用于保证某些状态中的操作仅仅执行一次

enum FLAG_STATUS{
    UN_SET = 0,//缺省设置状态
    AL_SET
};

typedef struct{
    bool pushButton_standby;
    bool pushButton_preheat;
    bool pushButton_thermo;
    bool pushButton_evaporation;
    bool pushButton_sampling;
    bool pushButton_clear;
} PUSHBUTTON_STATE;

typedef struct{
    int mode;       //使能按钮模式
    int enable_time;//使能时间，如果为0表示按钮没有使能时间计时
} USER_BUTTON_ENABLE;

enum BUTTON_MODE{
    SET_BUTTON = 0,    //使能set按钮

    OPEN_BUTTON,       //使能open按钮
    CLOSE_BUTTON,      //

    CLEAR_BUTTON,      //使能clear按钮
    CLEAR_BUTTON_DISABLE,
    PAUSE_BUTTON,
    PLOT_BUTTON,
    DONE_BUTTON,

    UNSET      //缺省设置状态
};
#endif // QCOMMON_H
