#ifndef __HLW8032_H
#define __HLW8032_H

/**
 * @file HLW8032.h
 * @author xfp23
 * @brief HLW8032电能电量计芯片驱动库
 * @note 使用此库请事先初始化好芯片串口功能，将此库回调方法加入中断回调中使用
 * @version 0.1
 * @date 2025-04-18
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "stdint-gcc.h"
#include "driver/uart.h"
#include "string.h"

class HLW8032_Obj
{
    enum
    {
        OFF = 0x00,
        ON = 0x01,
    };
public:
    enum class Status_t
    {
        OK,            // 操作成功
        ERROR,         // 一般错误
        UARTERROR,     // 串口错误
        DRDYERROR,     // 数据准备错误
        CHECKSUMERROR, // 校验和错误
        NOTAVAILABLE,  // 数据不可用 芯片误差修正功能失效
        VOLTAGEOVER,   // 电压寄存器溢出
        CURRENTOVER,   // 电流寄存器溢出
        POWEROVER,     // 功率寄存器溢出
    };                // 状态

private:
    typedef struct
    {
        uint8_t Update_Current : 1; // 电流更新
        uint8_t Update_Voltage : 1; // 电压更新
        uint8_t Update_Power : 1;   // 功率更新
        uint8_t Reserve_bits : 5;   // 保留
    } HLW8032_flag_t;

    HLW8032_flag_t flag;
    float volCoefficient = 0; // 电压系数
    float curCoefficient = 0; // 电流系数

    volatile bool DRDY = false; // 数据准备信号
    uint8_t Recive_buffer[24];  // 串口接收buffer

    /**
     * @brief 除状态寄存器(State REG)、检测寄存器(CheckREG)和校验和寄存器(CheckSumREG)之外的寄存器的每个字节数据相加和,取低8bit
     *
     * @param data
     * @return Status_t
     */
    Status_t CheckSum(uint8_t *data);

    /**
     * @brief 检查状态寄存器
     *
     * @param value
     * @return Status_t
     */
    Status_t checkStateReg(uint8_t value);

    Status_t checkData_Update(uint8_t value);

public:
   static const uint16_t __buadRate = 4800;
    HLW8032_Obj();
    ~HLW8032_Obj();

    /**
     * @brief 初始化
     * 
     * @param volCoefficient 电压系数
     * @param curCoefficient 电流系数
     * @return Status_t 操作状态
     */
    Status_t begin(float volCoefficient, float curCoefficient);

    /**
     * @brief 默认初始化
     * 
     * @return Status_t 操作状态
     */
    Status_t begin();
    /**
     * @brief 获取电压电流ADC原始值
     *
     * @param Voltage_buffer 电压buffer地址，uint32_t 类型
     * @param Current_buffer 电流buffer地址，uint32_t 类型
     * @return Status_t 操作状态
     */
    Status_t getVoltageAndCurrent(uint32_t *Voltage_buffer, uint32_t *Current_buffer, uint32_t *Power_buffer);

    /**
     * @brief 获取计算后的电压电流值
     *
     * @param Voltage_buffer 电压buffer地址，float 类型
     * @param Current_buffer 电流buffer地址，float 类型
     * @return Status_t 操作状态
     */
    Status_t getVoltageAndCurrent(float *Voltage_buffer, float *Current_buffer, float *Power_buffer);

    /**
     * @brief 串口中断回调，放在串口回调中
     * 
     * @param data 串口接收到的数据
     */
    virtual void uartInterrupt_Callback(uint8_t *data);
};

#endif