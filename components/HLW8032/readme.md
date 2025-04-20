
# HLW8032 驱动库说明文档

## 📌 简介

该库封装了 HLW8032 电能电量计芯片的串口通信协议与寄存器数据解析，支持电压、电流、功率等参数的读取与数据完整性校验，适用于 ESP32 或其他支持 UART 的嵌入式平台。

---

## 📁 文件结构

```plaintext
HLW8032/
│
├── HLW8032.h       // 头文件（接口定义）
├── HLW8032.cpp     // 源文件（实现）
```

---

## 🧩 依赖项

- UART 驱动（使用 ESP-IDF 中的 `driver/uart.h`）
- C 标准类型支持（`stdint-gcc.h`）
- 内部回调需在 UART 中断中注册调用 `uartInterrupt_Callback()`

---

## 🚀 使用方法

### 1️⃣ 引入头文件

```cpp
#include "HLW8032.h"
```

### 2️⃣ 创建对象

```cpp
HLW8032_Obj hlw;
```

### 3️⃣ 初始化设备

```cpp
hlw.begin(1.0f, 1.0f);  // 设置电压电流系数
// 或使用默认初始化
hlw.begin();
```

### 4️⃣ 在串口中断中调用接收回调

```cpp
void uart_isr_callback(uint8_t *rx_data)
{
    hlw.uartInterrupt_Callback(rx_data);
}
```

### 5️⃣ 获取数据（ADC 原始值或换算后的实际值）

```cpp
uint32_t voltageRaw, currentRaw, powerRaw;
hlw.getVoltageAndCurrent(&voltageRaw, &currentRaw, &powerRaw);

float voltage, current, power;
hlw.getVoltageAndCurrent(&voltage, &current, &power);
```

---

## 📘 接口说明

### 📄 `Status_t begin(float volCoefficient, float curCoefficient)`

初始化 HLW8032，设置电压/电流系数。

### 📄 `Status_t getVoltageAndCurrent(uint32_t*, uint32_t*, uint32_t*)`

获取原始 ADC 数值（未换算）。

### 📄 `Status_t getVoltageAndCurrent(float*, float*, float*)`

获取已乘以系数后的真实物理量（单位 V / A / W）。

### 📄 `void uartInterrupt_Callback(uint8_t* data)`

接收串口中断数据处理，需用户在串口回调中调用。

---

## 🔁 枚举类型定义

`enum class HLW8032_Obj::Status_t` 表示各操作状态：

| 名称                 | 含义                         |
|----------------------|------------------------------|
| `OK`                 | 操作成功                     |
| `ERROR`              | 一般错误                     |
| `UARTERROR`          | 串口错误                     |
| `DRDYERROR`          | 数据准备错误                 |
| `CHECKSUMERROR`      | 校验和错误                   |
| `NOTAVAILABLE`       | 数据不可用或芯片异常         |
| `VOLTAGEOVER`        | 电压数据溢出                 |
| `CURRENTOVER`        | 电流数据溢出                 |
| `POWEROVER`          | 功率数据溢出                 |

---

## 🧠 注意事项

- 本库假设使用 UART 串口通信，波特率为 `4800`，可通过 `__buadRate` 常量更改。
- 一定要将 `uartInterrupt_Callback()` 嵌入到 UART 接收中断回调函数中。
- 电压/电流系数建议由实际标定获取。

---

## 📞 示例代码

```cpp
HLW8032_Obj hlw;
hlw.begin(1.23f, 0.98f);

while (1) {
    float v, i, p;
    if (hlw.getVoltageAndCurrent(&v, &i, &p) == HLW8032_Obj::Status_t::OK) {
        printf("Voltage: %.2f V, Current: %.2f A, Power: %.2f W\n", v, i, p);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
}
```