/**
  *********************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   FreeRTOS V9.0.0 + STM32 �����ʱ��
  *********************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ��  STM32ȫϵ�п����� 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  **********************************************************************
  */ 
 
/*
*************************************************************************
*                             ������ͷ�ļ�
*************************************************************************
*/ 
/* FreeRTOSͷ�ļ� */
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
/* ������Ӳ��bspͷ�ļ� */
#include "stm32f7xx.h"
#include "main.h"
#include "bsp_led.h"
#include "bsp_key.h" 
#include "core_delay.h"   
#include "bsp_debug_usart.h"
/**************************** ������ ********************************/
/* 
 * ��������һ��ָ�룬����ָ��һ�����񣬵����񴴽���֮�����;�����һ��������
 * �Ժ�����Ҫ��������������Ҫͨ�������������������������������Լ�����ô
 * ����������ΪNULL��
 */
static TaskHandle_t AppTaskCreate_Handle = NULL;/* ���������� */

/********************************** �ں˶����� *********************************/
/*
 * �ź�������Ϣ���У��¼���־�飬�����ʱ����Щ�������ں˵Ķ���Ҫ��ʹ����Щ�ں�
 * ���󣬱����ȴ����������ɹ�֮��᷵��һ����Ӧ�ľ����ʵ���Ͼ���һ��ָ�룬������
 * �ǾͿ���ͨ��������������Щ�ں˶���
 *
 * �ں˶���˵���˾���һ��ȫ�ֵ����ݽṹ��ͨ����Щ���ݽṹ���ǿ���ʵ��������ͨ�ţ�
 * �������¼�ͬ���ȸ��ֹ��ܡ�������Щ���ܵ�ʵ��������ͨ��������Щ�ں˶���ĺ���
 * ����ɵ�
 * 
 */
static TimerHandle_t Swtmr1_Handle =NULL;   /* �����ʱ����� */
static TimerHandle_t Swtmr2_Handle =NULL;   /* �����ʱ����� */
/******************************* ȫ�ֱ������� ************************************/
/*
 * ��������дӦ�ó����ʱ�򣬿�����Ҫ�õ�һЩȫ�ֱ�����
 */
static uint32_t TmrCb_Count1 = 0; /* ��¼�����ʱ��1�ص�����ִ�д��� */
static uint32_t TmrCb_Count2 = 0; /* ��¼�����ʱ��2�ص�����ִ�д��� */

/******************************* �궨�� ************************************/
/*
 * ��������дӦ�ó����ʱ�򣬿�����Ҫ�õ�һЩ�궨�塣
 */

/*
*************************************************************************
*                             ��������
*************************************************************************
*/
static void AppTaskCreate(void);/* ���ڴ������� */

static void Swtmr1_Callback(void* parameter);
static void Swtmr2_Callback(void* parameter);

static void BSP_Init(void);/* ���ڳ�ʼ�����������Դ */

/*****************************************************************
  * @brief  ������
  * @param  ��
  * @retval ��
  * @note   ��һ����������Ӳ����ʼ�� 
            �ڶ���������APPӦ������
            ������������FreeRTOS����ʼ���������
  ****************************************************************/
int main(void)
{	
  BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
  
  /* ������Ӳ����ʼ�� */
  BSP_Init();
  
	printf("����һ��[Ұ��]-STM32ȫϵ�п�����-FreeRTOS�����ʱ��ʵ�飡\n");

  /* ����AppTaskCreate���� */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* ������ں��� */
                        (const char*    )"AppTaskCreate",/* �������� */
                        (uint16_t       )512,  /* ����ջ��С */
                        (void*          )NULL,/* ������ں������� */
                        (UBaseType_t    )1, /* ��������ȼ� */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* ������ƿ�ָ�� */ 
  /* ����������� */           
  if(pdPASS == xReturn)
    vTaskStartScheduler();   /* �������񣬿������� */
  else
    return -1;  
  
  while(1);   /* ��������ִ�е����� */    
}


/***********************************************************************
  * @ ������  �� AppTaskCreate
  * @ ����˵���� Ϊ�˷���������е����񴴽����������������������
  * @ ����    �� ��  
  * @ ����ֵ  �� ��
  **********************************************************************/
static void AppTaskCreate(void)
{
  taskENTER_CRITICAL();           //�����ٽ���
    
  /************************************************************************************
   * ����������ڶ�ʱ��
   * ����ԭ��
   * TimerHandle_t xTimerCreate(	const char * const pcTimerName,
								const TickType_t xTimerPeriodInTicks,
								const UBaseType_t uxAutoReload,
								void * const pvTimerID,
                TimerCallbackFunction_t pxCallbackFunction )
    * @uxAutoReload : pdTRUEΪ����ģʽ��pdFALSΪ����ģʽ
   * ���ζ�ʱ��������(1000��ʱ�ӽ���)������ģʽ
   *************************************************************************************/
  Swtmr1_Handle=xTimerCreate((const char*		)"AutoReloadTimer",
                            (TickType_t			)1000,/* ��ʱ������ 1000(tick) */
                            (UBaseType_t		)pdTRUE,/* ����ģʽ */
                            (void*				  )1,/* Ϊÿ����ʱ������һ��������ΨһID */
                            (TimerCallbackFunction_t)Swtmr1_Callback); 
  if(Swtmr1_Handle != NULL)                          
  {
    /***********************************************************************************
     * xTicksToWait:����ڵ���xTimerStart()ʱ��������������tickΪ��λָ����������Ӧ����
     * ��Blocked(����)״̬�Եȴ�start����ɹ����͵�timer������е�ʱ�䡣 
     * ������������ȳ���֮ǰ����xTimerStart()�������xTicksToWait�����������õȴ�ʱ��Ϊ0.
     **********************************************************************************/
    xTimerStart(Swtmr1_Handle,0);	//�������ڶ�ʱ��
  }                            
  /************************************************************************************
   * ����������ڶ�ʱ��
   * ����ԭ��
   * TimerHandle_t xTimerCreate(	const char * const pcTimerName,
								const TickType_t xTimerPeriodInTicks,
								const UBaseType_t uxAutoReload,
								void * const pvTimerID,
                TimerCallbackFunction_t pxCallbackFunction )
    * @uxAutoReload : pdTRUEΪ����ģʽ��pdFALSΪ����ģʽ
   * ���ζ�ʱ��������(5000��ʱ�ӽ���)������ģʽ
   *************************************************************************************/
	Swtmr2_Handle=xTimerCreate((const char*			)"OneShotTimer",
                             (TickType_t			)5000,/* ��ʱ������ 5000(tick) */
                             (UBaseType_t			)pdFALSE,/* ����ģʽ */
                             (void*					  )2,/* Ϊÿ����ʱ������һ��������ΨһID */
                             (TimerCallbackFunction_t)Swtmr2_Callback); 
  if(Swtmr2_Handle != NULL)
  {
   /***********************************************************************************
   * xTicksToWait:����ڵ���xTimerStart()ʱ��������������tickΪ��λָ����������Ӧ����
   * ��Blocked(����)״̬�Եȴ�start����ɹ����͵�timer������е�ʱ�䡣 
   * ������������ȳ���֮ǰ����xTimerStart()�������xTicksToWait�����������õȴ�ʱ��Ϊ0.
   **********************************************************************************/   
    xTimerStart(Swtmr2_Handle,0);	//�������ڶ�ʱ��
  } 
  
  vTaskDelete(AppTaskCreate_Handle); //ɾ��AppTaskCreate����
  
  taskEXIT_CRITICAL();            //�˳��ٽ���
}

/***********************************************************************
  * @ ������  �� Swtmr1_Callback
  * @ ����˵���� �����ʱ��1 �ص���������ӡ�ص�������Ϣ&��ǰϵͳʱ��
  *              �����ʱ���벻Ҫ��������������Ҳ��Ҫ������ѭ����Ӧ������
  * @ ����    �� ��  
  * @ ����ֵ  �� ��
  **********************************************************************/
static void Swtmr1_Callback(void* parameter)
{		
  TickType_t tick_num1;

  TmrCb_Count1++;						/* ÿ�ص�һ�μ�һ */

  tick_num1 = xTaskGetTickCount();	/* ��ȡ�δ�ʱ���ļ���ֵ */
  
  LED1_TOGGLE;
  
  printf("Swtmr1_Callback����ִ�� %d ��\n", TmrCb_Count1);
  printf("�δ�ʱ����ֵ=%d\n", tick_num1);
}

/***********************************************************************
  * @ ������  �� Swtmr2_Callback
  * @ ����˵���� �����ʱ��2 �ص���������ӡ�ص�������Ϣ&��ǰϵͳʱ��
  *              �����ʱ���벻Ҫ��������������Ҳ��Ҫ������ѭ����Ӧ������
  * @ ����    �� ��  
  * @ ����ֵ  �� ��
  **********************************************************************/
static void Swtmr2_Callback(void* parameter)
{	
  TickType_t tick_num2;

  TmrCb_Count2++;						/* ÿ�ص�һ�μ�һ */

  tick_num2 = xTaskGetTickCount();	/* ��ȡ�δ�ʱ���ļ���ֵ */

  printf("Swtmr2_Callback����ִ�� %d ��\n", TmrCb_Count2);
  printf("�δ�ʱ����ֵ=%d\n", tick_num2);
}



/***********************************************************************
  * @ ������  �� BSP_Init
  * @ ����˵���� �弶�����ʼ�������а����ϵĳ�ʼ�����ɷ��������������
  * @ ����    ��   
  * @ ����ֵ  �� ��
  *********************************************************************/
static void BSP_Init(void)
{
  /* ϵͳʱ�ӳ�ʼ����400MHz */
	SystemClock_Config();
  
  /* ��ʼ��SysTick */
  HAL_SYSTICK_Config( HAL_RCC_GetSysClockFreq() / configTICK_RATE_HZ );	

	/* Ӳ��BSP��ʼ��ͳͳ�����������LED�����ڣ�LCD�� */
    
	/* LED �˿ڳ�ʼ�� */
	LED_GPIO_Config();	
	
	/* usart �˿ڳ�ʼ�� */
  DEBUG_USART_Config();

  /* KEY �˿ڳ�ʼ�� */
  Key_GPIO_Config();
  
}
 

/**
  * @brief  System Clock ����
  *         system Clock �������� : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 216000000
  *            HCLK(Hz)                       = 216000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 25
  *            PLL_N                          = 432
  *            PLL_P                          = 2
  *            PLL_Q                          = 9
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 7
  * @param  ��
  * @retval ��
  */
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;

  /* ʹ��HSE������HSEΪPLL��ʱ��Դ������PLL�ĸ��ַ�Ƶ����M N P Q 
	 * PLLCLK = HSE/M*N/P = 25M / 25 *432 / 2 = 216M
	 */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  
  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }
  
  /* ���� OverDrive ģʽ�Դﵽ216MƵ��  */  
  ret = HAL_PWREx_EnableOverDrive();
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }
  
  /* ѡ��PLLCLK��ΪSYSCLK�������� HCLK, PCLK1 and PCLK2 ��ʱ�ӷ�Ƶ���� 
	 * SYSCLK = PLLCLK     = 216M
	 * HCLK   = SYSCLK / 1 = 216M
	 * PCLK2  = SYSCLK / 2 = 108M
	 * PCLK1  = SYSCLK / 4 = 54M
	 */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2; 
  
  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }  
}

/*********************************************END OF FILE**********************/

