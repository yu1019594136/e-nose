#ifndef SHT21_H
#define SHT21_H

#ifdef __cplusplus
extern "C"
{
#endif

#define SHT21_HUMIDITY_FILEPATH "/sys/class/i2c-adapter/i2c-1/1-0040/humidity1_input"
#define SHT21_TEMPERTU_FILEPATH "/sys/class/i2c-adapter/i2c-1/1-0040/temp1_input"

float sht21_get_humidity_float();
float sht21_get_temp_float();
void sht21_get_humidity_string(char *humid_str);
void sht21_get_temp_string(char *temp_str);

#ifdef __cplusplus
}
#endif

#endif // SHT21_H
