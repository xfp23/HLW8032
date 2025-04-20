#include "HLW8032.h"

HLW8032_Obj::HLW8032_Obj()
{
    memset(&this->flag,0,sizeof(HLW8032_flag_t));
    this->DRDY = false;
    memset(this->Recive_buffer,0,sizeof(this->Recive_buffer));
}

HLW8032_Obj::~HLW8032_Obj()
{
}

HLW8032_Obj::Status_t HLW8032_Obj::begin(float volCoefficient, float curCoefficient)
{
    this->volCoefficient = volCoefficient;
    this->curCoefficient = curCoefficient;
    return Status_t::OK;
}

HLW8032_Obj::Status_t HLW8032_Obj::begin()
{
    this->volCoefficient = 1.88;
    this->curCoefficient = 1;
    return Status_t::OK;
}

HLW8032_Obj::Status_t HLW8032_Obj::checkStateReg(uint8_t value)
{
    if (value == 0x55)
        return Status_t::OK;

    if (value == 0xAA)
        return Status_t::NOTAVAILABLE;

    uint8_t data = value & 0xf0;
    if (data == 0xf0)
        return Status_t::OK;

    data = value & 0xf2;
    if (data == 0xf2)
        return Status_t::POWEROVER;

    data = value & 0xf4;
    if (data == 0xf4)
        return Status_t::CURRENTOVER;

    data = value & 0xf8;
    if (data == 0xf8)
        return Status_t::VOLTAGEOVER;

    return Status_t::ERROR;
}

void HLW8032_Obj::uartInterrupt_Callback(uint8_t *data)
{
    memcpy((void *)this->Recive_buffer, data, sizeof(this->Recive_buffer)); // 拷贝缓冲区
    this->DRDY = true;                                                      // 使能标志
}

HLW8032_Obj::Status_t HLW8032_Obj::CheckSum(uint8_t *data)
{
    uint32_t sum = 0;
    for (int i = 2; i < 22; i++)
    {
        sum += data[i];
    }
    if ((sum & 0x0F) != data[23])
        return Status_t::CHECKSUMERROR;

    return Status_t::OK;
}

HLW8032_Obj::Status_t HLW8032_Obj::getVoltageCurrentAndPower(uint32_t *Voltage_buffer, uint32_t *Current_buffer, uint32_t *Power_buffer)
{
    if (this->DRDY == false)
    {
        return Status_t::DRDYERROR;
    }

    this->DRDY = false;

    Status_t ret = this->CheckSum((uint8_t *)this->Recive_buffer);
    if (ret != Status_t::OK)
        return ret;
    ret = this->checkStateReg(Recive_buffer[0]);
    if (ret != Status_t::OK)
        return ret;
    this->checkData_Update(this->Recive_buffer[20]);

    if (Voltage_buffer == nullptr)
        return Status_t::ERROR;

    if (this->flag.Update_Voltage == ON)
    {
        this->flag.Update_Voltage = OFF;
        *Voltage_buffer = ((uint32_t)Recive_buffer[5] << 16 | (uint16_t)Recive_buffer[6] << 8) | Recive_buffer[7];
    }

    if (Current_buffer == nullptr)
        return Status_t::ERROR;

    if (this->flag.Update_Current == ON)
    {
        this->flag.Update_Current = OFF;
        *Current_buffer = ((uint32_t)Recive_buffer[11] << 16 | (uint16_t)Recive_buffer[12] << 8) | Recive_buffer[13];
    }

    if (Power_buffer == nullptr)
        return Status_t::ERROR;

    if (this->flag.Update_Power == ON)
    {
        this->flag.Update_Power = OFF;
        *Power_buffer = ((uint32_t)Recive_buffer[17] << 16 | (uint16_t)Recive_buffer[18] << 8) | Recive_buffer[19];
    }

    return Status_t::OK;
}

HLW8032_Obj::Status_t HLW8032_Obj::getVoltageCurrentAndPower(float *Voltage_buffer, float *Current_buffer, float *Power_buffer)
{
    if (this->DRDY == false)
    {
        return Status_t::DRDYERROR;
    }

    this->DRDY = false;

    Status_t ret = this->CheckSum((uint8_t *)this->Recive_buffer);
    if (ret != Status_t::OK)
    {
        return ret;
    }

    ret = this->checkStateReg(Recive_buffer[0]);
    if (ret != Status_t::OK)
    {
        return ret;
    }


    this->checkData_Update(this->Recive_buffer[20]);

    if (Voltage_buffer != nullptr && this->flag.Update_Voltage == ON)
    {
        this->flag.Update_Voltage = OFF;
        *Voltage_buffer = (float)((((uint32_t)Recive_buffer[2] << 16 | (uint16_t)Recive_buffer[3] << 8) | Recive_buffer[4]) / (((uint32_t)Recive_buffer[5] << 16 | (uint16_t)Recive_buffer[6] << 8) | Recive_buffer[7])) * this->volCoefficient;
    }

    if (Current_buffer != nullptr && this->flag.Update_Current == ON)
    {
        this->flag.Update_Current = OFF;
        *Current_buffer = (float)((((uint32_t)Recive_buffer[8] << 16 | (uint16_t)Recive_buffer[9] << 8) | Recive_buffer[10]) / (((uint32_t)Recive_buffer[11] << 16 | (uint16_t)Recive_buffer[12] << 8) | Recive_buffer[13])) * this->curCoefficient;
    }

    if (Power_buffer == nullptr && this->flag.Update_Power == ON)
    {
        this->flag.Update_Power = OFF;
        *Power_buffer = (float)((((uint32_t)Recive_buffer[14] << 16 | (uint16_t)Recive_buffer[15] << 8) | Recive_buffer[16]) / (((uint32_t)Recive_buffer[17] << 16 | (uint16_t)Recive_buffer[18] << 8) | Recive_buffer[19])) * this->curCoefficient * this->volCoefficient;
    }

    return Status_t::OK;
}

HLW8032_Obj::Status_t HLW8032_Obj::checkData_Update(uint8_t value)
{
    if ((value & 0x40) == 0x40)
    {
        this->flag.Update_Voltage = ON;
    }

    if ((value & 0x20) == 0x20)
    {
        this->flag.Update_Current = ON;
    }

    if ((value & 0x10) == 0x10)
    {
        this->flag.Update_Current = ON;
    }

    return Status_t::OK;
}