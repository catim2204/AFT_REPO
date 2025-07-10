/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "fatfs.h"
#include "i2c.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include "user.h"
#include <I2C_2.h>
#include <math.h>
#include <string.h>
#include <usbd_cdc_if.h>
#include "Data_Packets.h"
//#include <sdcard.h>
//#include "ff.h"
//#include <user_diskio.h>

//#include <ADC_Scan.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
  #define DEADBAND              2
  #define ADC_MAX               4095
  #define MAX_THRUST_LOSS       200
  #define CHANNEL_COUNT         5
  #define OVERSAMPLE_COUNT      16
  #define DMA_BUFFER_SIZE    (CHANNEL_COUNT * OVERSAMPLE_COUNT)
  #define MODE_UI               1
  #define MODE_INTERNAL         2           
  #define UI_NONE               0
  #define UI_TEST_MODE          1
  #define UI_CALIBRATE          2  
  #define UI_MANUAL_MODE        3 
  #define UI_ONBOARD_MODE       4
  #define LCD_DISPLAY_VALUES    1
  #define LCD_CHANGE_S_NUM      2
  #define LCD_CHANGE_C_STEADY   3
  #define LCD_CHANGE_C_BURST    4
  #define LCD_CHANGE_LC_THRUST  5
  #define LCD_CHANGE_LC_TORQUE  6
  #define LCD_CONFIRM_SCREEN    7
  #define LCD_SAVE_TO_SD_CARD   8
  #define MAX_NUMBER_OF_CELLS   13
  #define THRUST          (GPIOA), (GPIO_PIN_1)
  #define TORQUE          (GPIOA), (GPIO_PIN_2)
  #define V_BAT           (GPIOA), (GPIO_PIN_3)
  #define CSS             (GPIOA), (GPIO_PIN_4)
  #define SCK             (GPIOA), (GPIO_PIN_5)
  #define MISO            (GPIOA), (GPIO_PIN_6)
  #define MOSI            (GPIOA), (GPIO_PIN_7)
  #define USART_CLK       (GPIOA), (GPIO_PIN_8)
  #define USART_TX        (GPIOA), (GPIO_PIN_9)
  #define USART_RX        (GPIOA), (GPIO_PIN_10)
  #define DF_N            (GPIOA), (GPIO_PIN_11)
  #define DF_P            (GPIOA), (GPIO_PIN_12)
  #define JTMS            (GPIOA), (GPIO_PIN_13)
  #define JTCK            (GPIOA), (GPIO_PIN_14)
  #define JTDI            (GPIOA), (GPIO_PIN_15) 
  #define CURRENT_SENSOR  (GPIOB), (GPIO_PIN_0)
  #define POTENTIOMETER   (GPIOB), (GPIO_PIN_1)
  #define BOOT            (GPIOB), (GPIO_PIN_2)
  #define JTDO            (GPIOB), (GPIO_PIN_3)
  #define NJTRST          (GPIOB), (GPIO_PIN_4)
  #define BUTTON3         GPIO_PIN_5
  #define SDA             (GPIOB), (GPIO_PIN_6)
  #define SCL             (GPIOB), (GPIO_PIN_7)
  #define BUTTON5         GPIO_PIN_8
  #define BUTTON6         GPIO_PIN_9
  #define BUTTON7         GPIO_PIN_10
  #define BUTTON4         GPIO_PIN_11
  #define BUTTON1         GPIO_PIN_12
  #define BUTTON2         GPIO_PIN_13
  #define IR_SENS         GPIO_PIN_15
 
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
  Information_Package_STR Information_Package;
  Telemetry_Package_STR Telemetry_Package;
  uint16_t ADC_Values[5] = {0,0,0,0,0};
  uint32_t Cal_Int[5]   = {0,0,0,0,0};
  
  float Voltage=0;
  float Current_S=0;
  float Thrust=0;
  float Torque=0;
  float Diff_V1;
  
  volatile bool BUTTON_1 = false;
  bool Plus=false;
  bool Minus=false;
  bool OK=false;
  bool Abort=false;
  volatile bool LCD_Clear_Flag=false;
  volatile bool ARM=false;
  volatile bool Com_Method;

  int8_t  SD_ERROR_CODE;
  uint8_t Last_Throttle_Value;
  uint16_t filtered_adc[5];
  uint16_t last_value[5] = {0,0,0,0,0};
  uint16_t filtered_value[5]= {0,0,0,0,0};
  uint16_t ADC_Offset_Thrust;
  uint16_t ADC_Offset_Torque;
  uint16_t ADC_Offset_Current;
  uint16_t ADC_Offset_Voltage;

  uint64_t period_counts = 0;
  uint32_t Capture_Buffer[10];
  uint8_t capture_done = 0;

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);

  volatile uint8_t  Onboard_States=0;
  volatile uint8_t  UI_States = 0;
  volatile uint8_t  Current_State=0;
  volatile uint32_t pulse_count = 0;
  double rpm = 0;
  //volatile bool dataReceivedFlag = false;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
//void HAL_UART_RxCpltCallback(USART_HandleTypeDef *huart);
  
  float Convert_ADC_Thrust();
  float Convert_ADC_Battery();
  float Convert_ADC_Current();
  double Calculate_RPM();
  
  void Convert_ADC_Torque();
  void Internal_Mode();
  void UI_Mode();
  void ADC_DeadBand_Filter();
  void Display_LCD_Values();
  void Safety_Check();
  void Define_States_UI();
  void Change_S_Num();
  void Change_C_Steady();
  void Change_C_Burst();
  void Change_LC_Thrust();
  void Change_LC_Torque();
  void Save_Log();
  void split_float(float value, int16_t *int_part, uint16_t *frac_part, int precision);
  void Test_Mode();
  void Manual_Mode();
  void Calibrate();
  void Transmit_Data();
  void Gather_Information();
 

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
 

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  MX_USB_DEVICE_Init();
  MX_ADC1_Init();
  MX_FATFS_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_ADCEx_Calibration_Start(&hadc1);
  HAL_ADC_Start_DMA(&hadc1,(uint32_t *)&ADC_Values,5); 
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);  
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
  ADC_DeadBand_Filter();
  Calibrate();
  LCD_Init(&hi2c1);
  LCD_Clear();
  HAL_UART_Receive_IT(&huart1, (uint8_t *)&Information_Package, sizeof(Information_Package_STR));
  Current_State=1;
  HAL_Delay(50);
  ADC_DeadBand_Filter();
  Calibrate();
  HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
  __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, 1000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while(1)
  {

    //rpm = Calculate_RPM();
    switch (Current_State)
    {

      //       Telemetry_Package.Throttle=Information_Package.Throttle;
      //   ADC_DeadBand_Filter();
      //   Voltage = Convert_ADC_Battery();
      //   split_float(Voltage,&Telemetry_Package.Voltage_I,&Telemetry_Package.Voltage_F,2);
      //   Current_S = Convert_ADC_Current();
      //   split_float(Current_S,&Telemetry_Package.Current_I,&Telemetry_Package.Current_F,2);
      //   Thrust    = Convert_ADC_Thrust();    
      //   split_float(Thrust, &Telemetry_Package.Thrust_I,&Telemetry_Package.Thrust_F,3);
      //   Convert_ADC_Torque();
      //   split_float(Torque, &Telemetry_Package.Torque_I,&Telemetry_Package.Torque_F,3);
      //   rpm= Calculate_RPM();
      //   split_float(rpm, &Telemetry_Package.RPM_I, &Telemetry_Package.RPM_F, 2);
      // Transmit_Data();
      // Last_Throttle_Value=Information_Package.Throttle;
      // HAL_Delay(50);
    case MODE_INTERNAL:
      Internal_Mode();
      break;
    case MODE_UI:
      UI_Mode();
      break;
    default:
      break;
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_ADC
                              |RCC_PERIPHCLK_USB;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV4;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
    /**
     * @brief This function will be enabled when the `Information_Package.Arm_Check` and the `Information_Package.Start_Test` flags are both true.
     *  Uses conversion functions to convert `values_adc` into the desired characteristics
     * @note This resets the `Information_Package.Start_Test` flag when it finishes 
    */
    void Test_Mode()
    { 
      if(Com_Method==false)
      { for(int i=0;i<=1000;i++)
        { 
        if(Abort==true)
        { 
          Abort=false;
          Information_Package.Start_Test =0;
          Current_State=0;
          break;
        }  
        ADC_DeadBand_Filter();
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, i+999);
        Voltage = Convert_ADC_Battery();
        split_float(Voltage,&Telemetry_Package.Voltage_I,&Telemetry_Package.Voltage_F,2);
        Current_S = Convert_ADC_Current();
        split_float(Current_S,&Telemetry_Package.Current_I,&Telemetry_Package.Current_F,2);
        Thrust    = Convert_ADC_Thrust();    
        split_float(Thrust, &Telemetry_Package.Thrust_I,&Telemetry_Package.Thrust_F,3);
        Convert_ADC_Torque();
        split_float(Torque, &Telemetry_Package.Torque_I,&Telemetry_Package.Torque_F,3);
        rpm= Calculate_RPM();
        split_float(rpm, &Telemetry_Package.RPM_I, &Telemetry_Package.RPM_F, 2);
        if(i%10==0)Telemetry_Package.Throttle=i/10;
        Telemetry_Package.PWM_Time = i+1000;
        Safety_Check();
        HAL_Delay(10);
        Transmit_Data();
        }
      }
      if(Com_Method==true)
      {
      for(int i=0;i<=1000;i+=10)
      { 
        if(Abort==true)
        { 
          Abort=false;
          Information_Package.Start_Test =0;
          Current_State=0;
          break;
        }  
        ADC_DeadBand_Filter();
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, i+999);
        Voltage = Convert_ADC_Battery();
        split_float(Voltage,&Telemetry_Package.Voltage_I,&Telemetry_Package.Voltage_F,2);
        Current_S = Convert_ADC_Current();
        split_float(Current_S,&Telemetry_Package.Current_I,&Telemetry_Package.Current_F,2);
        Thrust    = Convert_ADC_Thrust();    
        split_float(Thrust, &Telemetry_Package.Thrust_I,&Telemetry_Package.Thrust_F,3);
        Convert_ADC_Torque();
        split_float(Torque, &Telemetry_Package.Torque_I,&Telemetry_Package.Torque_F,3);
        if(i%10==0)Telemetry_Package.Throttle=i/10;
        Telemetry_Package.PWM_Time = i+1000;
        Safety_Check();
        HAL_Delay(100);
        Transmit_Data();
      }
    }
        Telemetry_Package.Error_Code     = 0;
        Information_Package.Start_Test = 0;
        Current_State = 1;
        UI_States = 0; 
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
}
void Manual_Mode()
{
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, Information_Package.Throttle*10);
  if(!(Information_Package.Throttle==Last_Throttle_Value))
  { 
      Telemetry_Package.Throttle=Information_Package.Throttle;
        ADC_DeadBand_Filter();
        Voltage = Convert_ADC_Battery();
        split_float(Voltage,&Telemetry_Package.Voltage_I,&Telemetry_Package.Voltage_F,2);
        Current_S = Convert_ADC_Current();
        split_float(Current_S,&Telemetry_Package.Current_I,&Telemetry_Package.Current_F,2);
        Thrust    = Convert_ADC_Thrust();    
        split_float(Thrust, &Telemetry_Package.Thrust_I,&Telemetry_Package.Thrust_F,3);
        Convert_ADC_Torque();
        split_float(Torque, &Telemetry_Package.Torque_I,&Telemetry_Package.Torque_F,3);
        rpm= Calculate_RPM();
        split_float(rpm, &Telemetry_Package.RPM_I, &Telemetry_Package.RPM_F, 2);
      Transmit_Data();
      Last_Throttle_Value=Information_Package.Throttle;
      HAL_Delay(50);
  }
}

  /**
   * @brief This function splits a `Float` into its `Integer` and `Fractional` parts to be sent as `uint16_t` values inside the `Telemetry_Package_STR`
   * 
   * @param value 
   * @param int_part 
   * @param frac_part 
   * @param precision 
   */
  void split_float(float value, int16_t *int_part, uint16_t *frac_part, int precision) 
  {
      *int_part = abs((int)value);
      float frac = value - *int_part;
      *frac_part = abs((int)(roundf(frac * powf(10, precision))));
  }
  
  /**
   * @brief Math for measuring the voltage via the onboard divider, measured resistances with multimeter 99000, 6210 Ohm respectively
   * 
   * @return float `Voltage` 
   */
  float Convert_ADC_Battery()
  {
    float Voltage;
    Voltage=(((float)filtered_value[2] / ADC_MAX) * 3.3)*16.9491;
    return Voltage;
  }
  /**
   * @brief  Math for converting the current sensor voltage output into current 
   * 
   * @return float 
   */
  float Convert_ADC_Current()
  {
    float Current;
    float Voltage;
    Voltage= (filtered_value[3])-ADC_Offset_Current;
    Current = ((Voltage)/ADC_MAX)*200;
    if(Current<0)
    return 0;
    else
    return Current;
  }
  /**
   * @brief Math for converting the load cell voltage output into thrust in KG
   * 
   * @return float 
   */
  float Convert_ADC_Thrust()
  { 
    float Thrust;
    Thrust   = 0.335*((float)filtered_value[1]-(float)ADC_Offset_Thrust); 
    //Thrust = 0.933*((float)(filtered_value[1]-700)/ADC_MAX*3.3);
    return Thrust/1000;
  }
  /**
   * @brief Deadband filter for the ADCs, slightly aleviates their swing
   * 
   */
  void ADC_DeadBand_Filter()
  {
      for (int i = 0; i < 5; i++)
      {
          uint16_t new_value = ADC_Values[i];
          
          if (abs((int)new_value - (int)last_value[i]) > DEADBAND)
          {
              filtered_value[i] = new_value;
              last_value[i] = new_value;
          }
          // else: keep previous filtered_value[i]
      }
  }
    /**
     * @brief Math for converting the lswitch (Com_Method)oad cell voltage output into Nm
     * 
     * @return float 
     */
    void Convert_ADC_Torque()
    { 
    Torque = 0.8653846*((float)filtered_value[0]-(float)ADC_Offset_Torque); 
    //Torque = 0.8653846f*(float)filtered_adc[0] - 1471.1538f;
    }
  /**
   * @brief Zeros out the ADC values and therfore the outputs, called everytime a test cycle is started
   * 
   * @return * void 
   */
  void Calibrate()
  {   
      ADC_DeadBand_Filter();
      ADC_Offset_Thrust  = ADC_Values[1];
      ADC_Offset_Torque  = ADC_Values[0];
      ADC_Offset_Voltage = 0;
      ADC_Offset_Current = ADC_Values[3];
      Information_Package.Calibration=0;
      Current_State=1;
      UI_States=0;
      HAL_Delay(1000);
      Transmit_Data();
  }
  /**
   * @brief Checks to see if the device operates under safe conditions every test cycle iteration
   * @note Limits the device to ESC maximum ratings,Minimum battery voltage as inputed from either the UI or onboard HDI
   * @note A battery is considered spent at a cell voltage of `3V`
   * @note If the device is not operating under safe conditions, it sets the `Telemetry_Package.Error_Code` to `1` or `2` and sets the `Abort` flag to true    
   */
  void Safety_Check()
  { 
        if(Voltage <= Information_Package.S_Num*3)
        {
          Telemetry_Package.Error_Code=1;
          Abort=true;
        } 
        if(Current_S > Information_Package.Current_Limit_Steady)
        {
          Telemetry_Package.Error_Code=2;
          Abort=true;
        }
  }
  /**
   * @brief Defines the state machine when operating in UI mode 
   * 
  */
  void Define_States_UI()
  {
      if(Information_Package.Arm_Check==1 && Information_Package.Start_Test==1)   UI_States = UI_TEST_MODE;
      if(Information_Package.Calibration==1 && Information_Package.Arm_Check==1 ) UI_States = UI_CALIBRATE;
      if(Information_Package.Arm_Check==1 && Information_Package.Manual_Check==1) UI_States = UI_MANUAL_MODE;
  }

  /**
   * @brief Calback function for the buttons, executes action based on pressed button
   * 
   * @param GPIO_Pin 
   */
  void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
  {
        if(GPIO_Pin == BUTTON1)
    {
        if(Onboard_States<9)
        Onboard_States++;
        LCD_Clear_Flag=true;
    }
        if(GPIO_Pin == BUTTON2)
    {
       if(Onboard_States>0)
      {
        Onboard_States--;
        LCD_Clear_Flag=true;
      }
    }
        if(GPIO_Pin == BUTTON3)
    {
        Plus=true;      
    }
        if(GPIO_Pin == BUTTON4)
    {
        OK=true;
    }
        if(GPIO_Pin == BUTTON5)
    {
        Minus=true;
    }
        if(GPIO_Pin == BUTTON6)
    {
      
    }  
        if(GPIO_Pin == BUTTON7)
    {
        ARM=!ARM;
        LCD_Clear_Flag=true;
    }
    if(GPIO_Pin == IR_SENS)
    {
      pulse_count++;
    }
    
    
  }


  /**
   *  @brief In HDI mode, state `LCD_DISPLAY_VALUES` Displays the 4 main values outputed from `TEST_MODE()` 
   *  @note `Voltage` 
   *  @note `Current` 
   *  @note `Thrust` 
   *  @note `Torque` 
   */
  void Display_LCD_Values()
  {
    if(LCD_Clear_Flag)
    { LCD_Clear();
      LCD_Clear_Flag=false;
    }
    Voltage = Convert_ADC_Battery();
    Current_S = Convert_ADC_Current();
    Thrust = Convert_ADC_Thrust();
    Convert_ADC_Torque();
    LCD_SetCursor(0,0);
    LCD_SendString("Voltage");
    split_float(Voltage,&Telemetry_Package.Voltage_I,&Telemetry_Package.Voltage_F,2);
    LCD_SetCursor(8,0);
    LCD_SendFloat(Telemetry_Package.Voltage_I,Telemetry_Package.Voltage_F);
    Define_States_UI();
    ADC_DeadBand_Filter();
    LCD_SetCursor(0,1);
    LCD_SendString("Current");
    split_float(Current_S,&Telemetry_Package.Current_I,&Telemetry_Package.Current_F,2);
    LCD_SetCursor(8,1);
    LCD_SendFloat(Telemetry_Package.Current_I,Telemetry_Package.Current_F);
    LCD_SetCursor(0,2);
    LCD_SendString("Thrust");
    LCD_SetCursor(7,2);
    split_float(Thrust,&Telemetry_Package.Thrust_I,&Telemetry_Package.Thrust_F,3);
    LCD_SendFloat(Telemetry_Package.Thrust_I,Telemetry_Package.Thrust_F);
    LCD_SetCursor(0,3);
    LCD_SendString("Torque");
    LCD_SetCursor(7,3);
    split_float(Torque,&Telemetry_Package.Torque_I,&Telemetry_Package.Torque_F,3);
    LCD_SendFloat(Telemetry_Package.Torque_I,Telemetry_Package.Torque_F);
    
  }

  /**
   * @brief State machine for the onboard HDI
   * 
   * 
   */
  void Internal_Mode()
  {

        if(LCD_Clear_Flag)
    { 
      LCD_Clear();
      LCD_Clear_Flag=false;
        if(ARM)
      {
        LCD_SetCursor(15,3);
        LCD_SendString("ARMED");
      }
    }

      
    switch (Onboard_States)
    {
    case LCD_DISPLAY_VALUES:
      Display_LCD_Values();
      break;
    case LCD_CHANGE_S_NUM:
      Change_S_Num();
      break;
    case LCD_CHANGE_C_STEADY:
      Change_C_Steady();
      break;
    case LCD_CHANGE_C_BURST:
      Change_C_Burst();
      break;
    case LCD_CHANGE_LC_THRUST:
      Change_LC_Thrust();
      break;
    case LCD_CHANGE_LC_TORQUE:
      Change_LC_Torque();
      break;
    default:

      break;
    }
  }
  void UI_Mode()
  {
    Define_States_UI();
    switch (UI_States)
    {      
    case UI_TEST_MODE:
      Test_Mode();
      break;
    case UI_CALIBRATE:
      Calibrate();
      break;
    case UI_MANUAL_MODE:
      Manual_Mode();
      break;
    default:
      break;
    }
  }

  /**
   * @brief From HDI, Inputs the ammount of cells of a battery for the `Safety_Check()` function
   *  
   * 
   */
  void Change_S_Num()
  {
    
        if(LCD_Clear_Flag)
    { LCD_Clear();
      LCD_Clear_Flag=false;
    }
    LCD_SetCursor(0,0);
    LCD_SendString("Number of cells");
    LCD_SetCursor(0,1);
    LCD_SendInt(Information_Package.S_Num);
    if(Plus==true && Information_Package.S_Num<MAX_NUMBER_OF_CELLS)
    {
      Information_Package.S_Num++;
      Plus=false;   
    }
    if(Minus==true && Information_Package.S_Num>0)
    {
      Minus=false;
      Information_Package.S_Num--;
    }
  }
  void Change_C_Steady()
  { 
        if(LCD_Clear_Flag)
    { LCD_Clear();
      LCD_Clear_Flag=false;
    }
    LCD_SetCursor(0,0);
    LCD_SendString("Current limit steady");
    LCD_SetCursor(0,1);
    LCD_SendInt(Information_Package.Current_Limit_Steady);
    if(Plus==true && Information_Package.Current_Limit_Steady<100)
    {
      Information_Package.Current_Limit_Steady++;
      Plus=false;   
    }
    if(Minus==true && Information_Package.Current_Limit_Steady>0)
    {
      Minus=false;
      Information_Package.Current_Limit_Steady--;
    }
  }
  void Change_C_Burst()
  {
        if(LCD_Clear_Flag)
    { LCD_Clear();
      LCD_Clear_Flag=false;
    }
    LCD_SetCursor(0,0);
    LCD_SendString("Current limit burst");
    LCD_SetCursor(0,1);
    LCD_SendInt(Information_Package.Blade_Count);
    if(Plus==true && Information_Package.Blade_Count<6)
    {
      Information_Package.Blade_Count++;
      Plus=false;   
    }
    if(Minus==true && Information_Package.Blade_Count>1)
    {
      Minus=false;
      Information_Package.Blade_Count--;
    }
  }
  void Change_LC_Thrust()
  {
        if(LCD_Clear_Flag)
    { LCD_Clear();
      LCD_Clear_Flag=false;
    }
    LCD_SetCursor(0,0);
    LCD_SendString("Load cell thrust");
    LCD_SetCursor(0,1);
    LCD_SendInt(Information_Package.Load_Cell_Thrust);
    if(Plus==true && Information_Package.Load_Cell_Thrust<20)
    {
      Information_Package.Load_Cell_Thrust++;
      Plus=false;   
    }
    if(Minus==true && Information_Package.Load_Cell_Thrust>0)
    {
      Minus=false;
      Information_Package.Load_Cell_Thrust--;
    }
  }
    void Change_LC_Torque()
  {
        if(LCD_Clear_Flag)
    { LCD_Clear();
      LCD_Clear_Flag=false;
    }
    LCD_SetCursor(0,0);
    LCD_SendString("Load cell torque");
    LCD_SetCursor(0,1);
    LCD_SendInt(Information_Package.Load_Cell_Torque);
    if(Plus==true && Information_Package.Load_Cell_Torque<20)
    {
      Information_Package.Load_Cell_Torque++;
      Plus=false;   
    }
    if(Minus==true && Information_Package.Load_Cell_Torque>0)
    {
      Minus=false;
      Information_Package.Load_Cell_Torque--;
    }
  }
  void Save_Log()
  {

  }
  void Transmit_Data()
  {
    switch(Com_Method)
    {
    case false:
        CDC_Transmit_FS((uint8_t *)&Telemetry_Package,sizeof(Telemetry_Package_STR));
    break;
    case true:
    HAL_UART_Transmit(&huart1,(uint8_t *)&Telemetry_Package,sizeof(Telemetry_Package_STR),HAL_MAX_DELAY);
    break;
    default:
      break;
    }
              
  }

  void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart==&huart1)
  {   
      Com_Method=true;
      HAL_UART_Receive_IT(&huart1,(uint8_t*)&Information_Package, sizeof(Information_Package_STR));
  }
}
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) 
{
  if(htim == &htim3)  // Check if the interrupt is from TIM3
    {
  
    //   if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)  // Check if the interrupt is from TIM3 Channel 1
    // {
        // if(capture_done == 0) 
        // {
        //     capture1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        //     capture_done = 1;
        // } else 
        // {
        //     capture2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        //     if(capture2 > capture1)
        //           period_counts = capture2 - capture1;
        //     else // timer overflow
        //         period_counts = (htim->Instance->ARR - capture1) + capture2 + 1;
        //     capture_done = 2;
        // }
        // 
        if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
        {
            if(capture_done < 10) 
            {

              Capture_Buffer[capture_done] = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1); 
              capture_done++;
              
            }
            else
            {
                // Reset the capture buffer if it is full
                rpm = Calculate_RPM();
                capture_done = 0;
                period_counts = 0;
            }

        }

    }
  
    }
  

double Calculate_RPM() 
{

    for(int i = 0; i < capture_done - 1; i++) 
    {
        if (Capture_Buffer[i + 1] >= Capture_Buffer[i]) 
        {
            period_counts += Capture_Buffer[i + 1] - Capture_Buffer[i];
        } 
        else 
        {
            period_counts += (htim3.Instance->ARR - Capture_Buffer[i]) + Capture_Buffer[i + 1] + 1;
        }
    }
    return 600000.0f / (period_counts / 9 * 3); // 9 because we have 10 captures, and we want the average period

}


/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  Telemetry_Package.Error_Code=3;
  CDC_Transmit_FS((uint8_t *)&Telemetry_Package,sizeof(Telemetry_Package_STR));
  __disable_irq();
  while (1)
  { 
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
