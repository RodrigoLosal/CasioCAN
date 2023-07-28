/**------------------------------------------------------------------------------------------------
 * File with the functions of the auxiliary incilizations of the library.
-------------------------------------------------------------------------------------------------*/
#include "app_bsp.h"
#include "hel_lcd.h"

extern void HAL_MspInit( void );
extern void HAL_FDCAN_MspInit( FDCAN_HandleTypeDef *hfdcan );
extern void HAL_RTC_MspInit( RTC_HandleTypeDef *hrtc );
extern void HEL_LCD_MspInit( LCD_HandleTypeDef *hlcd );
extern void HAL_SPI_MspInit( SPI_HandleTypeDef *hspi );

/**
 * @brief   **Function to set the uC CPU to 64 MHz & the APB Clock to 32 MHz.**
 */

void HAL_MspInit( void )
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    HAL_StatusTypeDef Status;

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    /** Configure the main internal regulator output voltage*/

    /*The function is used and its result is verified.*/
    Status = HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, PWR_RET_ERROR );

    /* Initializes the RCC Oscillators according to the specified parameters in the RCC_OscInitTypeDef structure
    The frequency set is 64MHz with the internal 16MHz HSI oscilator. According to the formulas:
    fVCO = fPLLIN x ( N / M ) = 16MHz x (8 / 1) = 128MHz
    fPLLP = fVCO / P = 128MHz / 2 = 64MHz
    fPLLQ = fVCO / Q = 128MHz / 2 = 64MHz
    fPLLR = fVCO / R = 128MHz / 2 = 64MHz
    */
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSIDiv              = RCC_HSI_DIV1;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM            = RCC_PLLM_DIV1;
    RCC_OscInitStruct.PLL.PLLN            = 8;
    RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ            = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR            = RCC_PLLR_DIV2;

    /*The function is used and its result is verified.*/
    Status = HAL_RCC_OscConfig( &RCC_OscInitStruct );
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, RCC_RET_ERROR );

    /** Initializes the CPU, AHB and APB buses clocks*/
    RCC_ClkInitStruct.ClockType       = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource    = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider   = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider  = RCC_HCLK_DIV2;

    /*The function is used and its result is verified.*/
    Status = HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_2 );
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, RCC_RET_ERROR );
}

/**
 * @brief   **Function to initialice the Port D as the CAN Tx & Rx, and it's interrupts.**
 */

/* cppcheck-suppress misra-c2012-2.7 ; Function defined by the HAL library. */
void HAL_FDCAN_MspInit( FDCAN_HandleTypeDef *hfdcan )
{
    GPIO_InitTypeDef GpioCanStruct;

    __HAL_RCC_FDCAN_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    
    GpioCanStruct.Mode = GPIO_MODE_AF_PP;
    GpioCanStruct.Alternate = GPIO_AF3_FDCAN1;
    GpioCanStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GpioCanStruct.Pull = GPIO_NOPULL;
    GpioCanStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init( GPIOD, &GpioCanStruct );

    HAL_NVIC_SetPriority(TIM16_FDCAN_IT0_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(TIM16_FDCAN_IT0_IRQn);
}

/* cppcheck-suppress misra-c2012-2.7 ; Function defined by the HAL library. */
void HAL_RTC_MspInit( RTC_HandleTypeDef *hrtc )
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    HAL_StatusTypeDef Status;

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    /*The function is used and its result is verified.*/
    Status = HAL_PWREx_ControlVoltageScaling( PWR_REGULATOR_VOLTAGE_SCALE1 );
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, PWR_RET_ERROR );

    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_LSEDRIVE_CONFIG( RCC_LSEDRIVE_LOW );

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_NONE;

    /*The function is used and its result is verified.*/
    Status = HAL_RCCEx_PeriphCLKConfig ( &PeriphClkInitStruct );
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, RCC_RET_ERROR );

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.LSIState = RCC_LSI_OFF;

    /*The function is used and its result is verified.*/
    Status = HAL_RCC_OscConfig( &RCC_OscInitStruct );
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, RCC_RET_ERROR );

    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;

    /*The function is used and its result is verified.*/
    Status = HAL_RCCEx_PeriphCLKConfig( &PeriphClkInitStruct );
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, RCC_RET_ERROR );

    __HAL_RCC_RTC_ENABLE();
    __HAL_RCC_RTCAPB_CLK_ENABLE();
}

/*cppcheck-suppress misra-c2012-8.4 ; Function provided by the HAL library.*/
/**
 * @brief Ports and Pins configuration for the LCD.
 */

void HEL_LCD_MspInit( LCD_HandleTypeDef *hlcd ){
    //CS = GPIO_PIN_3
    //RST = GPIO_PIN_2
    //RS = GPIO_PIN_4
    GPIO_InitTypeDef GPIO_Init;  /*Gpios initial structure*/
    
    __HAL_RCC_GPIOD_CLK_ENABLE( ); /* Enable port D clock */
    __HAL_RCC_GPIOB_CLK_ENABLE( ); /* Enable port B clock */

    GPIO_Init.Pin   = hlcd->CSPin | hlcd->RSTPin| hlcd->RSPin; /*Pins configuration*/
    GPIO_Init.Mode  = GPIO_MODE_OUTPUT_PP; /*Push-pull output*/
    GPIO_Init.Pull  = GPIO_NOPULL;         /*Pin without pull-up or pull-down*/
    GPIO_Init.Speed = GPIO_SPEED_FREQ_LOW; /*Pin to low frecuency*/
    /*Initialize pins with the previous parameters*/
    HAL_GPIO_Init( hlcd->RSTPort, &GPIO_Init );
    HAL_GPIO_Init( hlcd->CSPort, &GPIO_Init );
    HAL_GPIO_Init( hlcd->RSPort, &GPIO_Init);

    GPIO_Init.Pin   = hlcd->BKLPin;         /*Pins configuration*/
    GPIO_Init.Mode  = GPIO_MODE_OUTPUT_PP; /*Push-pull output*/
    GPIO_Init.Pull  = GPIO_NOPULL;         /*Pin without pull-up or pull-down*/
    GPIO_Init.Speed = GPIO_SPEED_FREQ_LOW; /*Pin to low frecuency*/
    /*Initialize pins with the previous parameters*/
    HAL_GPIO_Init( hlcd->BKLPort, &GPIO_Init );

    /* Apply the configuration to spi 1 but before we make sure that the slave is disabled pin D3 in alt*/
    HAL_GPIO_WritePin( hlcd->CSPort, hlcd->CSPin, SET );
    HAL_GPIO_WritePin( hlcd->BKLPort, hlcd->BKLPin, SET );
}

/*cppcheck-suppress misra-c2012-8.4 ; Function provided by the HAL library.*/
/**
 * @brief Configuration of the SPI module.
 */

void HAL_SPI_MspInit( SPI_HandleTypeDef *hspi ) {
    (void)hspi;
    /* B5, B6 and B8 pins in altern function of SPI */
    GPIO_InitTypeDef GPIO;
    __GPIOD_CLK_ENABLE();
    __SPI1_CLK_ENABLE();

    GPIO.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_8;
    GPIO.Mode = GPIO_MODE_AF_PP;
    GPIO.Pull = GPIO_PULLUP;
    GPIO.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO.Alternate = GPIO_AF1_SPI1;
    HAL_GPIO_Init(GPIOD, &GPIO);
}