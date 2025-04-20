
#include "Useruart.h"
#include "HLW8032.h"


extern HLW8032_Obj hlw8032;

#define UART0_TX_BUFF_SIZE (128) // uart 缓冲区大小
#define UART0_PATTERN_CHR_NUM (3) // 连续接收触发中断的字符数
QueueHandle_t uart0_queue = NULL; // uart事件队列

/**
 * @brief UART0 的接收回调 
 * @note 用户逻辑代码在此处
 * @param data 接收的数据指针
 *
 */
static void inline Uart0_Rxcallbackevent(uint8_t *data)
{
    hlw8032.uartInterrupt_Callback(data);
}

static void uart0_event_task(void *pvParameters)
{
    uart_event_t event;
    size_t buffered_size;
    uint8_t *dtmp = (uint8_t *)malloc(UART0_TX_BUFF_SIZE);
    int pos = -1;
    for (;;)
    {
        // 等待 UART 事件
        if (xQueueReceive(uart0_queue, (void *)&event, (TickType_t)portMAX_DELAY))
        {
            bzero(dtmp, UART0_TX_BUFF_SIZE);
            switch (event.type)
            {
            // UART 数据接收事件
            case UART_DATA:
                uart_read_bytes(UART_NUM_0, dtmp, event.size, portMAX_DELAY); // 接收数据
                Uart0_Rxcallbackevent(dtmp);
                break;
            // 检测到硬件 FIFO 溢出事件
            case UART_FIFO_OVF:
                uart_flush_input(UART_NUM_0);
                xQueueReset(uart0_queue);
                break;
            // UART 环形缓冲区满事件
            case UART_BUFFER_FULL:
                uart_flush_input(UART_NUM_0);
                xQueueReset(uart0_queue);
                break;
            // 检测到 UART RX 中断事件
            case UART_BREAK:
                break;
            // 检测到 UART 校验错误事件
            case UART_PARITY_ERR:

                break;
            // 检测到 UART 帧错误事件
            case UART_FRAME_ERR:
                break;
            // 检测到 UART 模式匹配事件
            case UART_PATTERN_DET:
                pos = uart_pattern_pop_pos(UART_NUM_0); // 将此处移出 switch
                uart_get_buffered_data_len(UART_NUM_0, &buffered_size);
                if (pos == -1)
                {
                    uart_flush_input(UART_NUM_0);
                }
                else
                {
                    uart_read_bytes(UART_NUM_0, dtmp, pos, 100 / portTICK_PERIOD_MS);
                    uint8_t pat[UART0_PATTERN_CHR_NUM + 1];
                    memset(pat, 0, sizeof(pat));
                    uart_read_bytes(UART_NUM_0, pat, UART0_PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
                }
                break;
            default:
                break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

// 初始化 UART
void Base_Init_uart0()
{

    uart_config_t uart_config = {
        .baud_rate = HLW8032_Obj::__buadRate,   // 波特率
        .data_bits = UART_DATA_8_BITS,         // 数据位
        .parity = UART_PARITY_EVEN,             // 偶校验
        .stop_bits = UART_STOP_BITS_1,         // 停止位1位
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE, // 硬件流控制
        .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_DEFAULT,
        .flags = {
            .backup_before_sleep = false,
        }};

    uart_driver_install(UART_NUM_0, UART0_TX_BUFF_SIZE * 2, UART0_TX_BUFF_SIZE * 2, 20, &uart0_queue, 0);

    ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));

    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    xTaskCreate(uart0_event_task, "UART0Event_task", 1024, NULL, 12, NULL);
}