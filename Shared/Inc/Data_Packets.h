#ifndef DATA_PACKET_H
#define DATA_PACKET_H
#include <stdint.h>
#include <stdbool.h>

#pragma pack(push, 1)
typedef struct Information_Package_STR
{
    uint8_t Start;
    uint8_t S_Num;
    uint8_t Blade_Count;
    uint8_t Current_Limit_Steady;
    uint8_t Throttle;
    bool Arm_Check;
    bool Manual_Check;
    bool Start_Test;
    bool Ui_Mode;
    bool Save_To_Controller;
    bool Calibration;
    uint8_t Load_Cell_Thrust;
    uint8_t Load_Cell_Torque;
    uint8_t Stop;
}Information_Package_STR;

typedef struct Telemetry_Package_STR
{
    int16_t Voltage_I;
    uint16_t Voltage_F;
    int16_t Current_I;
    uint16_t Current_F;
    uint16_t RPM_I;
    uint16_t RPM_F;
    int16_t Thrust_I;
    uint16_t Thrust_F;
    int16_t Torque_I;
    uint16_t Torque_F;
    uint8_t Throttle;
    uint16_t PWM_Time;
    uint8_t Error_Code;
}Telemetry_Package_STR;
#pragma pack(pop)
extern  Information_Package_STR Information_Package;
extern  Telemetry_Package_STR Telemetry_Package;
extern volatile bool Com_Method;
extern volatile bool Data_Received_Flag;
#endif // DATA_PACKET_H
