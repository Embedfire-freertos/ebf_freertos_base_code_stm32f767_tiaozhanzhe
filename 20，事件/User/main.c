/**
  *********************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   FreeRTOS V9.0.0 + STM32 �¼�
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
static TaskHandle_t LED_Task_Handle = NULL;/* LED_Task������ */
static TaskHandle_t KEY_Task_Handle = NULL;/* KEY_Task������ */

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
static EventGroupHandle_t Event_Handle =NULL;

/******************************* ȫ�ֱ������� ************************************/
/*
 * ��������дӦ�ó����ʱ�򣬿�����Ҫ�õ�һЩȫ�ֱ�����
 */


/******************************* �궨�� ************************************/
/*
 * ��������дӦ�ó����ʱ�򣬿�����Ҫ�õ�һЩ�궨�塣
 */
#define KEY1_EVENT  (0x01 << 0)//�����¼������λ0
#define KEY2_EVENT  (0x01 << 1)//�����¼������λ1

/*
*************************************************************************
*                             ��������
*************************************************************************
*/
static void AppTaskCreate(void);/* ���ڴ������� */

static void LED_Task(void* pvParameters);/* LED_Task ����ʵ�� */
static void KEY_Task(void* pvParameters);/* KEY_Task ����ʵ�� */

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
	printf("����һ��[Ұ��]-STM32ȫϵ�п�����-FreeRTOS�¼���־��ʵ�飡\n");
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
  BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
  
  taskENTER_CRITICAL();           //�����ٽ���
  
  /* ���� Event_Handle */
  Event_Handle = xEventGroupCreate();	 
  if(NULL != Event_Handle)
    printf("Event_Handle �¼������ɹ�!\r\n");
    
  /* ����LED_Task���� */
  xReturn = xTaskCreate((TaskFunction_t )LED_Task, /* ������ں��� */
                        (const char*    )"LED_Task",/* �������� */
                        (uint16_t       )512,   /* ����ջ��С */
                        (void*          )NULL,	/* ������ں������� */
                        (UBaseType_t    )2,	    /* ��������ȼ� */
                        (TaskHandle_t*  )&LED_Task_Handle);/* ������ƿ�ָ�� */
  if(pdPASS == xReturn)
    printf("����LED_Task����ɹ�!\r\n");
  
  /* ����KEY_Task���� */
  xReturn = xTaskCreate((TaskFunction_t )KEY_Task,  /* ������ں��� */
                        (const char*    )"KEY_Task",/* �������� */
                        (uint16_t       )512,  /* ����ջ��С */
                        (void*          )NULL,/* ������ں������� */
                        (UBaseType_t    )3, /* ��������ȼ� */
                        (TaskHandle_t*  )&KEY_Task_Handle);/* ������ƿ�ָ�� */ 
  if(pdPASS == xReturn)
    printf("����KEY_Task����ɹ�!\n");
  
  vTaskDelete(AppTaskCreate_Handle); //ɾ��AppTaskCreate����
  
  taskEXIT_CRITICAL();            //�˳��ٽ���
}



/**********************************************************************
  * @ ������  �� LED_Task
  * @ ����˵���� LED_Task��������
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/
static void LED_Task(void* parameter)
{	
  EventBits_t r_event;  /* ����һ���¼����ձ��� */
  /* ������һ������ѭ�������ܷ��� */
  while (1)
	{
    /*******************************************************************
     * �ȴ������¼���־ 
     * 
     * ���xClearOnExit����ΪpdTRUE����ô��xEventGroupWaitBits()����֮ǰ��
     * �������ȴ�����������������ص�ԭ���ǳ�ʱ������ô���¼���������
     * ��uxBitsToWaitFor�е��κ�λ����������� 
     * ���xClearOnExit����ΪpdFALSE��
     * ���ڵ���xEventGroupWaitBits()ʱ����������¼��������õ�λ��
     *
     * xWaitForAllBits���xWaitForAllBits����ΪpdTRUE����uxBitsToWaitFor��
     * ������λ�����û�ָ���Ŀ�ʱ�䵽��ʱ��xEventGroupWaitBits()�ŷ��ء� 
     * ���xWaitForAllBits����ΪpdFALSE��������uxBitsToWaitFor�����õ��κ�
     * һ��λ��1 ��ָ���Ŀ�ʱ�䵽��ʱ��xEventGroupWaitBits()���᷵�ء� 
     * ����ʱ����xTicksToWait����ָ����          
      *********************************************************/
    r_event = xEventGroupWaitBits(Event_Handle,  /* �¼������� */
                                  KEY1_EVENT|KEY2_EVENT,/* �����̸߳���Ȥ���¼� */
                                  pdTRUE,   /* �˳�ʱ����¼�λ */
                                  pdTRUE,   /* �������Ȥ�������¼� */
                                  portMAX_DELAY);/* ָ����ʱ�¼�,һֱ�� */
                        
    if((r_event & (KEY1_EVENT|KEY2_EVENT)) == (KEY1_EVENT|KEY2_EVENT)) 
    {
      /* ���������ɲ�����ȷ */
      printf ( "KEY1��KEY2������\n");		
      LED1_TOGGLE;       //LED1	��ת
    }
    else
      printf ( "�¼�����\n");	
  }
}

/**********************************************************************
  * @ ������  �� KEY_Task
  * @ ����˵���� KEY_Task��������
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/
static void KEY_Task(void* parameter)
{	 
    /* ������һ������ѭ�������ܷ��� */
  while (1)
  {
    if( Key_Scan(KEY1_GPIO_PORT,KEY1_PIN) == KEY_ON )       //���KEY2������
		{
      printf ( "KEY1������\n" );
			/* ����һ���¼�1 */
			xEventGroupSetBits(Event_Handle,KEY1_EVENT);  					
		}
    
		if( Key_Scan(KEY2_GPIO_PORT,KEY2_PIN) == KEY_ON )       //���KEY2������
		{
      printf ( "KEY2������\n" );	
			/* ����һ���¼�2 */
			xEventGroupSetBits(Event_Handle,KEY2_EVENT); 				
		}
		vTaskDelay(20);     //ÿ20msɨ��һ��		
  }
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

