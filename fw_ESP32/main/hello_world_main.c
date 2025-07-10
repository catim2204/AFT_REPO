#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"
#include "nvs_flash.h"
#include "esp_gap_bt_api.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_intr_alloc.h"
#include "esp_wifi.h"
#include "Data_Packets.h"
static const char *TAG = "BT_SPP_EXAMPLE";

#define SPP_SERVER_NAME "SPP_SERVER"
#define DEVICE_NAME "ESP32_SPP"
#define RX_BUF_SIZE 2048
#define UART_QUEUE_SIZE 20
#define TXD_PIN 17 // UART TX pin
#define RXD_PIN 16 // UART RX pin
#define BATCH_SIZE 10

// Declare connection handle globally
static uint32_t spp_conn_handle = 0;
const uart_port_t UART_NUM = UART_NUM_2;
uint8_t payload_index = 0;
// UART queue handle
static QueueHandle_t uart_queue;
Telemetry_Package_STR Telemetry_Package;
Information_Package_STR Information_Package;

// Data structures
// #pragma pack(push,1)
// typedef struct Telemetry_Package_STR {
//     int16_t Voltage_I;
//     uint16_t Voltage_F;
//     int16_t Current_I;
//     uint16_t Current_F;
//     uint32_t RPM;
//     int16_t Thrust_I;
//     uint16_t Thrust_F;
//     int16_t Torque_I;
//     uint16_t Torque_F;
//     uint8_t Throttle;
//     uint16_t PWM_Time;
//     uint8_t Error_Code;
// } Telemetry_Package_STR;
// #pragma pack(pop)

// #pragma pack(push,1)
// typedef struct Information_Package_STR {
//     uint8_t Start;
//     uint8_t S_NUM;
//     uint8_t Current_Limit_Burst;
//     uint8_t Current_Limit_Steady;
//     uint8_t Throttle;
//     bool ARM_Check;
//     bool MANUAL_Check;
//     bool Start_Test;
//     bool UI_Mode;
//     bool Save_To_Controller;
//     bool Calibration;
//     uint8_t Load_Cell_Thrust;
//     uint8_t Load_Cell_Torque;
//     uint8_t Stop;
// } Information_Package_STR;
// #pragma pack(pop)

// Telemetry_Package_STR Telemetry_Package;
// Telemetry_Package_STR payload[BATCH_SIZE];
// Information_Package_STR Information_Package;

static void uart_task(void *arg);
static void spp_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);

void app_main(void)
{
    ESP_LOGI(TAG, "Starting application initialization");

    ESP_LOGI(TAG, "Initialized Information_Package: Start=%u, Stop=%u", Information_Package.Start, Information_Package.Stop);

    // Configure UART
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    esp_err_t ret = uart_driver_install(UART_NUM, RX_BUF_SIZE * 2, RX_BUF_SIZE * 2, UART_QUEUE_SIZE, &uart_queue, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART driver installation failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "UART driver installed");

    ret = uart_param_config(UART_NUM, &uart_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART configuration failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "UART configured with baud rate %d", uart_config.baud_rate);

    ret = uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART pin configuration failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "UART pins configured: TXD=%d, RXD=%d", TXD_PIN, RXD_PIN);

    // Create UART task
    BaseType_t task_ret = xTaskCreate(uart_task, "uart_task", 8192, NULL, 10, NULL);
    if (task_ret != pdPASS) 
    {
        ESP_LOGE(TAG, "Failed to create UART task");
        return;
    }
    ESP_LOGI(TAG, "UART task created successfully");

    // Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS initialization failed, erasing and retrying");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS initialization failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "NVS initialized successfully");

    // Release BLE memory as only Classic BT is used
    ret = esp_bt_controller_mem_release(ESP_BT_MODE_BLE);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(TAG, "BLE memory release failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "BLE memory released");

    // If controller is already initialized, deinit it first
    if (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_ENABLED ||
        esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_INITED) 
        {
        ESP_LOGW(TAG, "BT controller already initialized, deinitializing...");
        esp_bt_controller_disable();
        esp_bt_controller_deinit();
        }

    // Initialize BT controller with default config
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(TAG, "BT controller init failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "BT controller initialized");

    ret = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(TAG, "BT controller enable failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "BT controller enabled in BTDM mode");

    // Initialize and enable Bluedroid stack
    ret = esp_bluedroid_init();
    if (ret != ESP_OK) 
    {
        ESP_LOGE(TAG, "Bluedroid init failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "Bluedroid initialized");

    ret = esp_bluedroid_enable();
    if (ret != ESP_OK) 
    {
        ESP_LOGE(TAG, "Bluedroid enable failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "Bluedroid enabled");

    // Register SPP callback and initialize SPP
    ret = esp_spp_register_callback(spp_callback);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(TAG, "SPP callback registration failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "SPP callback registered");

    ret = esp_spp_init(ESP_SPP_MODE_CB);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(TAG, "SPP initialization failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "SPP initialized");

    // Set device name
    ret = esp_bt_dev_set_device_name(DEVICE_NAME);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(TAG, "Failed to set device name: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "Device name set to %s", DEVICE_NAME);

    ret = esp_bt_gap_set_scan_mode(1, 2);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(TAG, "Failed to set BT scan mode: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "BT scan mode set");

    ESP_LOGI(TAG, "Bluetooth SPP application fully initialized");
}


/**
 * @brief UART task to handle UART events and forward data to SPP
 * This task listens for UART events, reads data from the UART buffer,
 * and forwards it to the SPP connection if available.
 * It handles various UART events such as data reception, FIFO overflow,
 * and buffer full conditions.
 *  The task runs indefinitely, processing events from the UART queue.         
 * @param arg 
 */
static void uart_task(void *arg)
{
    uart_event_t event;
    uint8_t* rx_buffer = (uint8_t*) malloc(RX_BUF_SIZE);
    if (rx_buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for UART receive buffer");
        vTaskDelete(NULL);
    }

    ESP_LOGI(TAG, "UART task started");
    
while (1) 
{
    if (xQueueReceive(uart_queue, (void *)&event, (TickType_t)portMAX_DELAY)) 
    {
    switch (event.type) 
    {
    case UART_DATA:
        ESP_LOGD(TAG, "UART data event received, size=%d", event.size);
        if (spp_conn_handle != 0) 
        {
            int len = uart_read_bytes(UART_NUM, &Telemetry_Package, sizeof(Telemetry_Package_STR), portMAX_DELAY);
            if (len == sizeof(Telemetry_Package_STR)) 
            {  
                ESP_LOG_BUFFER_HEX(TAG, &Telemetry_Package, sizeof(Telemetry_Package_STR));
                esp_err_t ret = esp_spp_write(spp_conn_handle, sizeof(Telemetry_Package_STR),(const char *)&Telemetry_Package);
                if (ret == ESP_OK) 
                {
                    ESP_LOGI(TAG, "Forwarded %d bytes from UART to SPP", len);
                }
                    else 
                {
                    ESP_LOGE(TAG, "Failed to send UART data to SPP: %s", esp_err_to_name(ret));
                }
            } 
            else 
            {
                ESP_LOGW(TAG, "No data read from UART");
            }
        } 
        else 
        {
            ESP_LOGW(TAG, "Received UART data but no SPP connection active");
        }
        break;
    case UART_FIFO_OVF:
        ESP_LOGE(TAG, "UART FIFO overflow");
        uart_flush_input(UART_NUM);
        break;
    case UART_BUFFER_FULL:
        ESP_LOGE(TAG, "UART buffer full");
        uart_flush_input(UART_NUM);
        break;
    default:
        ESP_LOGD(TAG, "Unhandled UART event type: %d", event.type);
        break;
        }
    }
}
free(rx_buffer);
vTaskDelete(NULL);
}

/**
 * @brief SPP callback function to handle SPP events
 * This function processes various SPP events such as initialization,
 * starting the SPP server, client connections, data reception,
 *  congestion status changes, and disconnections.
 *
 * @param event 
 * @param param 
 */
static void spp_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    switch (event) {
    case ESP_SPP_INIT_EVT:
        ESP_LOGI(TAG, "SPP initialized");
        esp_err_t ret = esp_bt_dev_set_device_name(DEVICE_NAME);
        if (ret == ESP_OK)
        {
            ESP_LOGI(TAG, "Device name set to %s", DEVICE_NAME);
        } 
        else 
        {
            ESP_LOGE(TAG, "Failed to set device name: %s", esp_err_to_name(ret));
        }
        ret = esp_spp_start_srv(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0, SPP_SERVER_NAME);
        if (ret == ESP_OK) 
        {
            ESP_LOGI(TAG, "SPP server started with name %s", SPP_SERVER_NAME);
        } 
        else 
        {
            ESP_LOGE(TAG, "Failed to start SPP server: %s", esp_err_to_name(ret));
        }
        break;

    case ESP_SPP_START_EVT:
        ESP_LOGI(TAG, "SPP server started");
        break;

    case ESP_SPP_SRV_OPEN_EVT:
        ESP_LOGI(TAG, "Client connected, handle=%u", param->srv_open.handle);
        spp_conn_handle = param->srv_open.handle;
        break;
    case ESP_SPP_CONG_EVT:
        ESP_LOGI(TAG, "SPP congestion status changed, handle=%u, congested=%d", 
                 param->cong.handle, param->cong.cong);
        if (param->cong.cong) 
        {
            ESP_LOGW(TAG, "SPP connection is congested");
        } 
        else 
        {
            ESP_LOGI(TAG, "SPP connection is no longer congested");
        }
        break;
    case ESP_SPP_CLOSE_EVT:
        ESP_LOGI(TAG, "Client disconnected");
        spp_conn_handle = 0;
        break;

    case ESP_SPP_DATA_IND_EVT:
        ESP_LOGI(TAG, "Received %d bytes over SPP", param->data_ind.len);
        ESP_LOG_BUFFER_HEX(TAG, param->data_ind.data, param->data_ind.len);
        // Forward SPP data to UART
        if (param->data_ind.len >= sizeof(Information_Package_STR)) 
        {
            memcpy(&Information_Package, param->data_ind.data, param->data_ind.len);
            uart_write_bytes(UART_NUM,(const char*)&Information_Package,sizeof(Information_Package_STR));
            ESP_LOGD(TAG,"Sent to UART");
            ESP_LOGD(TAG, "Received valid data structure: Start=%u, S_Num =%u, Throttle=%u",
                     Information_Package.Start, Information_Package.S_Num, Information_Package.Throttle);
        } 
        else 
        {
            ESP_LOGW(TAG, "Received SPP data size %d does not match expected %d",
                     param->data_ind.len, sizeof(Information_Package_STR));
        }
        break;

    case ESP_SPP_WRITE_EVT:
        ESP_LOGI(TAG, "SPP write completed, handle=%u, status=%d", param->write.handle, param->write.status);
        if (param->write.status == ESP_SPP_SUCCESS) 
        {
            ESP_LOGI(TAG, "Data written successfully");
        } 
        else
        {
            ESP_LOGE(TAG, "Failed to write data over SPP: %s", esp_err_to_name(param->write.status));
        }
        break;
    default:
        ESP_LOGD(TAG, "Unhandled SPP event: %d", event);
        break;
    }
}
