// #include "ADC_Scan.h"
// #include "adc.h"
// #include "dma.h"
// // Global variables
// uint16_t adcValues[ADC_CHANNEL_COUNT];

// //ADC_HandleTypeDef hadc1;
// extern DMA_HandleTypeDef hdma_adc1;


// // Function to initialize and read ADC in scan mode
// HAL_StatusTypeDef ADC_ScanMode_InitAndRead(void) {
//     HAL_StatusTypeDef status;

//     // ADC configuration
//     hadc1.Instance = ADC1;
//     hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE; // Enable scan mode
//     hadc1.Init.ContinuousConvMode = ENABLE; // Continuous conversion
//     hadc1.Init.DiscontinuousConvMode = DISABLE;
//     hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START; // Software trigger
//     hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
//     hadc1.Init.NbrOfConversion = ADC_CHANNEL_COUNT; // Number of channels
//     if (HAL_ADC_Init(&hadc1) != HAL_OK) {
//         return HAL_ERROR;
//     }

//     // Configure ADC channels (example: channels 0, 1, 2, 3)
//     ADC_ChannelConfTypeDef sConfig = {0};
//     uint32_t channels[] ={ADC_CHANNEL_1, ADC_CHANNEL_2, ADC_CHANNEL_3,ADC_CHANNEL_8,ADC_CHANNEL_9};
//     for (uint8_t i = 0; i < ADC_CHANNEL_COUNT; i++) {
//         sConfig.Channel = channels[i];
//         sConfig.Rank = i + 1; // Correct rank setup (NOT ADC_REGULAR_RANK_1 + i)
//         hadc1.Init.NbrOfConversion = ADC_CHANNEL_COUNT;
//         sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5; // Adjust as needed
//         if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
//             return HAL_ERROR;
//         }
//     }

//     // DMA configuration
//     __HAL_RCC_DMA1_CLK_ENABLE(); // Enable DMA clock
//     hdma_adc1.Instance = DMA1_Channel1; // DMA channel for ADC1
//     hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
//     hdma_adc1.Init.PeriphInc = DISABLE;
//     hdma_adc1.Init.MemInc = ENABLE;
//     hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
//     hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
//     hdma_adc1.Init.Mode = DMA_CIRCULAR; // Circular mode for continuous transfer
//     hdma_adc1.Init.Priority = DMA_PRIORITY_HIGH;
//     if (HAL_DMA_Init(&hdma_adc1) != HAL_OK) {
//         return HAL_ERROR;
//     }

//     // Link DMA to ADC
//     __HAL_LINKDMA(&hadc1, DMA_Handle, hdma_adc1);

//     // Start ADC with DMA
//     status = HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adcValues, ADC_CHANNEL_COUNT);
//     if (status != HAL_OK) {
//         return status;
//     }

//     return HAL_OK;
// }

// // Callback function for DMA transfer complete
// void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
//     // ADC values are now available in adcValues array
//     // Process adcValues[0] to adcValues[ADC_CHANNEL_COUNT-1] as needed
// }