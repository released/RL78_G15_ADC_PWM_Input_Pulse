/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>

#include "inc_main.h"

#include "misc_config.h"
#include "custom_func.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/

struct flag_32bit flag_PROJ_CTL;
#define FLAG_PROJ_TIMER_PERIOD_1000MS                 	(flag_PROJ_CTL.bit0)
#define FLAG_PROJ_TIMER_PERIOD_SPECIFIC                 (flag_PROJ_CTL.bit1)
#define FLAG_PROJ_TRIG_BTN1                 	        (flag_PROJ_CTL.bit2)
#define FLAG_PROJ_TRIG_BTN2                    		    (flag_PROJ_CTL.bit3)
#define FLAG_PROJ_TRIG_ADC_CH                           (flag_PROJ_CTL.bit4)
#define FLAG_PROJ_GET_WIDTH_MEASUREMENT                 (flag_PROJ_CTL.bit5)
#define FLAG_PROJ_WIDTH_HIGH_TRIG                       (flag_PROJ_CTL.bit6)
#define FLAG_PROJ_WIDTH_LOW_TRIG                        (flag_PROJ_CTL.bit7)


#define FLAG_PROJ_TRIG_1                                (flag_PROJ_CTL.bit8)
#define FLAG_PROJ_TRIG_2                                (flag_PROJ_CTL.bit9)
#define FLAG_PROJ_TRIG_3                                (flag_PROJ_CTL.bit10)
#define FLAG_PROJ_TRIG_4                                (flag_PROJ_CTL.bit11)
#define FLAG_PROJ_TRIG_5                                (flag_PROJ_CTL.bit12)
#define FLAG_PROJ_PWM_DUTY_INC                          (flag_PROJ_CTL.bit13)
#define FLAG_PROJ_PWM_DUTY_DEC                          (flag_PROJ_CTL.bit14)
#define FLAG_PROJ_REVERSE15                             (flag_PROJ_CTL.bit15)

/*_____ D E F I N I T I O N S ______________________________________________*/

volatile unsigned int counter_tick = 0;
volatile unsigned int btn_counter_tick = 0;

#define BTN_PRESSED_LONG                                (2500)

#define INPUT_PULSE_CAPTURE_PORT                        (P2_bit.no3)
#define INPUT_PULSE_MEASURE_START(void)                 (R_Config_TAU0_4_Start())
#define INPUT_PULSE_MEASURE_STOP(void)                  (R_Config_TAU0_4_Stop())
#define INPUT_PULSE_IRQ_FLAG                            (TMIF04)

#define DUTY_RESOLUTION                                 (100)
volatile unsigned int pwm_duty[8] = {0};

#define WIDTH_MEASURE_TIME                              (4)
unsigned char g_times_low = 0;                          /* Measurement times counter (low level width) */
unsigned char g_times_high = 0;                         /* Measurement times counter (high level width) */
unsigned char g_count = WIDTH_MEASURE_TIME;             /* Number of measurement */
volatile unsigned char g_edge_flag = 0xFFU;             /* Edge flag */ 
unsigned char g_port_data[2] = {0U, 0U};                /* Store PULSE level */
unsigned int g_width_low[WIDTH_MEASURE_TIME] = {0U};    /* Store low level width */
unsigned int g_width_high[WIDTH_MEASURE_TIME] = {0U};   /* Store high level width */
unsigned char g_times_invalid = 0x00U;                  /* Invalid edge counter */

unsigned int pulse_high = 0;
unsigned int pulse_low = 0;
unsigned int pulse_total = 0;
// float cal_duty = 0;

// #define VBG_VOLTAGE                                     (0.815)  // v
#define VBG_VOLTAGE                                     (815)  // mv

/*G15 V_BGR : 0.815 V*/
unsigned long vdd_Vbgr = 0;
unsigned short adc_buffer[11] = {0};

// #define ENABLE_AVG

// #define ENABLE_BUTTON
#define ENABLE_LOG_ADC
// #define ENABLE_LOG_PWM
// #define ENABLE_LOG_CAPTURE

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/
extern volatile uint32_t g_tau0_ch4_width;

void set_TIMER_PERIOD_SPECIFIC(void)
{
    FLAG_PROJ_TIMER_PERIOD_SPECIFIC = 1;
}

void reset_TIMER_PERIOD_SPECIFIC(void)
{
    FLAG_PROJ_TIMER_PERIOD_SPECIFIC = 0;
}

bool Is_TIMER_PERIOD_SPECIFIC_Trig(void)
{
    return FLAG_PROJ_TIMER_PERIOD_SPECIFIC;
}

void set_TIMER_PERIOD_1000MS(void)
{
    FLAG_PROJ_TIMER_PERIOD_1000MS = 1;
}

void reset_TIMER_PERIOD_1000MS(void)
{
    FLAG_PROJ_TIMER_PERIOD_1000MS = 0;
}

bool Is_TIMER_PERIOD_1000MS_Trig(void)
{
    return FLAG_PROJ_TIMER_PERIOD_1000MS;
}

unsigned int btn_get_tick(void)
{
	return (btn_counter_tick);
}

void btn_set_tick(unsigned int t)
{
	btn_counter_tick = t;
}

void btn_tick_counter(void)
{
	btn_counter_tick++;
    if (btn_get_tick() >= 60000)
    {
        btn_set_tick(0);
    }
}

unsigned int get_tick(void)
{
	return (counter_tick);
}

void set_tick(unsigned int t)
{
	counter_tick = t;
}

void tick_counter(void)
{
	counter_tick++;
    if (get_tick() >= 60000)
    {
        set_tick(0);
    }
}

void delay_ms(unsigned int ms)
{
	#if 1
    unsigned int tickstart = get_tick();
    unsigned int wait = ms;
	unsigned int tmp = 0;
	
    while (1)
    {
		if (get_tick() > tickstart)	// tickstart = 59000 , tick_counter = 60000
		{
			tmp = get_tick() - tickstart;
		}
		else // tickstart = 59000 , tick_counter = 2048
		{
			tmp = 60000 -  tickstart + get_tick();
		}		
		
		if (tmp > wait)
			break;
    }
	
	#else
	TIMER_Delay(TIMER0, 1000*ms);
	#endif
}

void Timer_1ms_IRQ(void)
{
    tick_counter();

    if ((get_tick() % 1000) == 0)
    {
        set_TIMER_PERIOD_1000MS();
    }

    if ((get_tick() % 100) == 0)
    {
        set_TIMER_PERIOD_SPECIFIC();
    }

    if ((get_tick() % 50) == 0)
    {

    }	

    #if defined (ENABLE_BUTTON)
    Button_Process_long_counter();
    #endif
}


// unsigned short ADC_To_Voltage(unsigned short adc_value)
// {
// 	unsigned long volt = 0;

// 	// volt = (unsigned long) (vdd_Vbgr*adc_value)/1024;
// 	volt = (unsigned long) (vdd_Vbgr*adc_value) >> 10;
	
//     // printf(",0x%4X(%5d,%5d),",adc_value,adc_value , volt);

// 	return (unsigned short) volt;	
// }

void GetADC(e_ad_channel_t ch)
{
    unsigned short tmp_buffer = 0;

    // R_Config_ADC_Set_ADChannel(ch);
    FLAG_PROJ_TRIG_ADC_CH = 0;
    R_Config_ADC_Start();
    while(!FLAG_PROJ_TRIG_ADC_CH);
    R_Config_ADC_Get_Result_10bit(&tmp_buffer);
    R_Config_ADC_Stop();
    FLAG_PROJ_TRIG_ADC_CH = 0;

    adc_buffer[ch] = tmp_buffer;    

    // printf_tiny("ch[%d]:0x%04X\r\n",ch,adc_buffer[ch]);

}

/*
    COPY FROM R_Config_ADC_Create
    channel : 1
    P02
*/
void ADC_Channel_config_Init(void)
{
    volatile uint16_t w_count;

    ADCEN = 1U;    /* supply AD clock */
    ADMK = 1U;    /* disable INTAD interrupt */
    ADIF = 0U;    /* clear INTAD interrupt flag */
    /* Set INTAD priority */
    ADPR1 = 1U;
    ADPR0 = 1U;
    /* Set ANI1 pin */
    PMC0 |= 0x04U;
    PM0 |= 0x04U;
    ADM0 = _00_AD_CONVERSION_CLOCK_8 | _00_AD_TIME_MODE_NORMAL_1;
    ADM2 = _00_AD_RESOLUTION_10BIT;
    ADS = _01_AD_INPUT_CHANNEL_1;

    ADCE = 1U;

    /* Reference voltage stability wait time, 0.125us */
    for (w_count = 0U; w_count < AD_WAITTIME_B; w_count++ )
    {
        NOP();
    }
    
    // R_Config_ADC_Start();
}

void GetVREF(void)
{
    unsigned short tmp_buffer = 0;
    unsigned short adc_resolution = 1024;

    /*
        VDD = VBG * adc_resolution / ConversionResult
        VDD/VBG = adc_resolution / ConversionResult
    
    */
    // R_Config_ADC_Set_ADChannel(ADINTERREFVOLT);
    FLAG_PROJ_TRIG_ADC_CH = 0;
    R_Config_ADC_Start();    // to get ADC internal vref channel
    while(!FLAG_PROJ_TRIG_ADC_CH);
    R_Config_ADC_Get_Result_10bit(&tmp_buffer);
    R_Config_ADC_Stop();
    FLAG_PROJ_TRIG_ADC_CH = 0;

    vdd_Vbgr = (unsigned long) VBG_VOLTAGE*adc_resolution/tmp_buffer;

    // printf_tiny("Read VREF:%d(0x%04X),",tmp_buffer,tmp_buffer);
    // printf_tiny("VBGR:%4dmv\r\n",vdd_Vbgr);
}

/*
    COPY FROM R_Config_ADC_Create
    channel : internal reference voltage
*/
void ADC_VREF_config_Init(void)
{    
    volatile uint16_t w_count;

    ADCEN = 1U;    /* supply AD clock */
    ADMK = 1U;    /* disable INTAD interrupt */
    ADIF = 0U;    /* clear INTAD interrupt flag */
    /* Set INTAD priority */
    ADPR1 = 1U;
    ADPR0 = 1U;
    ADM0 = _00_AD_CONVERSION_CLOCK_8 | _00_AD_TIME_MODE_NORMAL_1;
    ADM2 = _00_AD_RESOLUTION_10BIT;
    ADS = _0D_AD_INPUT_INTERREFVOLT;

    ADCE = 1U;

    /* Reference voltage stability wait time, 0.125us */
    for (w_count = 0U; w_count < AD_WAITTIME_B; w_count++ )
    {
        NOP();
    }

    // R_Config_ADC_Start();  
}

void ADC_Process(void)
{    
    // get ADC channel
    GetADC(ADCHANNEL1);
    	
    #if defined (ENABLE_LOG_ADC)   //ADC debug log
    {
        printf_tiny("VREF:%d,",vdd_Vbgr);
        printf_tiny("ch[%d]:0x%04X",ADCHANNEL1,adc_buffer[ADCHANNEL1]);
        printf_tiny("(%d mv)", (vdd_Vbgr*adc_buffer[ADCHANNEL1]) >> 10);
        printf_tiny("\r\n");
    }
    #endif
    
}

void ADC_init(void)
{    
    // init internal vref channel
    ADC_VREF_config_Init();
    // get VREF
    GetVREF();

    // init normal adc channel
    ADC_Channel_config_Init();
}

void ADC_Process_in_IRQ(void)
{
    FLAG_PROJ_TRIG_ADC_CH = 1;
}

/*
    TAU0 PWM : 10K
        - SLAVE 1 : P07 , duty : 100%
*/

unsigned int get_TAU0_pwm_ch_duty(unsigned char ch)
{
	return (pwm_duty[ch]);
}

void set_TAU0_pwm_ch_duty(unsigned char ch ,unsigned int duty)
{
    pwm_duty[ch] = duty;
}

/*copy from R_Config_TAU0_6_Create*/
void generate_TAU0_pwm_ch(void)
{
    TOM0 |= _0080_TAU_CH7_SLAVE_OUTPUT;
    TOL0 &= (uint16_t)~_0080_TAU_CH7_OUTPUT_LEVEL_L;
    TO0 &= (uint16_t)~_0080_TAU_CH7_OUTPUT_VALUE_1;
    TOE0 |= _0080_TAU_CH7_OUTPUT_ENABLE;
}

void PWM_Process_Adjust(void)
{
    int temp_duty = 0;
    unsigned int duty_hex = 0;
    unsigned short data_reg_default = get_TAU0_pwm_ch_data_reg_default();
    // unsigned short data_reg_default = _0640_TAU_TDR07_VALUE;
    unsigned short tmp = 0;

    if (FLAG_PROJ_PWM_DUTY_INC)
    {
        FLAG_PROJ_PWM_DUTY_INC = 0;

        temp_duty = get_TAU0_pwm_ch_duty(7);
        #if defined (ENABLE_LOG_PWM)
        {
            printf_tiny("+duty1:0x%02X,0x%02X\r\n",temp_duty ,data_reg_default);
        }
        #endif

        duty_hex = (data_reg_default / DUTY_RESOLUTION) * 1;   //0.1 %
        temp_duty = (temp_duty >= data_reg_default) ? (data_reg_default) : (temp_duty + duty_hex ) ;  
        #if defined (ENABLE_LOG_PWM)
        {
            printf_tiny("+duty2:0x%02X,0x%02X\r\n",temp_duty ,duty_hex);
        }
        #endif

        set_TAU0_pwm_ch_duty(7,temp_duty);
        #if defined (ENABLE_LOG_PWM)
        {
            // printf_tiny("+duty:0x%02X(%2.2f)\r\n",temp_duty , (float) temp_duty/data_reg_default*100 );
        }
        #endif

        tmp = get_TAU0_pwm_ch_duty(7);
        set_TAU0_pwm_ch_data_reg(tmp);
        generate_TAU0_pwm_ch();
    }
    if (FLAG_PROJ_PWM_DUTY_DEC)
    {
        FLAG_PROJ_PWM_DUTY_DEC = 0;

        temp_duty = get_TAU0_pwm_ch_duty(7);
        #if defined (ENABLE_LOG_PWM)
        {
            printf_tiny("-duty1:0x%02X,0x%02X\r\n",temp_duty ,data_reg_default);
        }
        #endif

        duty_hex = (data_reg_default / DUTY_RESOLUTION) * 1;   //0.1 %
        temp_duty = (temp_duty <= 0) ? (0) : (temp_duty - duty_hex ) ;   
        #if defined (ENABLE_LOG_PWM)
        {
            printf_tiny("-duty2:0x%02X,0x%02X\r\n",temp_duty ,duty_hex); 
        }
        #endif

        set_TAU0_pwm_ch_duty(7,temp_duty);
        #if defined (ENABLE_LOG_PWM)
        {
            // printf_tiny("-duty:0x%02X(%2.2f)\r\n",temp_duty , (float) temp_duty/data_reg_default*100 );
        }
        #endif

        tmp = get_TAU0_pwm_ch_duty(7);
        set_TAU0_pwm_ch_data_reg(tmp);
        generate_TAU0_pwm_ch();
    }
}



/*
    Set the TI00 pin input valid edge to “Both edge”.
*/
void PulseWidthCapture_Process_in_IRQ(void)    
{
    g_port_data[0] = INPUT_PULSE_CAPTURE_PORT;//P23
    // NOP();
    // NOP();
    // NOP();
    // NOP();
    g_port_data[1] = INPUT_PULSE_CAPTURE_PORT;//P23

    if ((0U == INPUT_PULSE_IRQ_FLAG) && ((0x01U & g_port_data[0]) == (0x01U & g_port_data[1])))
    {
        g_edge_flag = (g_port_data[0] & 0x01U);    /* Set edge flag */

        if (0U == g_edge_flag)    /* Store high level width */
        {
            #if defined (ENABLE_AVG)
            if (0U < g_times_high)    /* 4 times measurement finished ? */
            {
                g_width_high[g_count - g_times_high] = g_tau0_ch4_width;    /* Store capture value */
                g_times_high--;
            }
            else
            {
                FLAG_PROJ_WIDTH_HIGH_TRIG = 1;
            }
            #else
            pulse_high = g_tau0_ch4_width;
            FLAG_PROJ_WIDTH_HIGH_TRIG = 1;

            #endif
        }
        else    /* Store low level width */
        {
            #if defined (ENABLE_AVG)
            if (0U < g_times_low)     /* 4 times measurement finished ? */
            {
                g_width_low[g_count-g_times_low] = g_tau0_ch4_width;    /* Store capture value */
                g_times_low--;
            }
            else
            {
                FLAG_PROJ_WIDTH_LOW_TRIG = 1;
            }
            #else
            pulse_low = g_tau0_ch4_width;
            FLAG_PROJ_WIDTH_LOW_TRIG = 1;
            #endif
         }
    }
    else     /* Interrupt request is generated */
    {
        g_times_invalid++;      /* Increment counter of invalid edge */
        INPUT_PULSE_IRQ_FLAG = 0U;            /* Clear interrupt request */
    }
}

void PulseWidthCapture_Process_logging(void)
{

    if (FLAG_PROJ_GET_WIDTH_MEASUREMENT)
    {
        FLAG_PROJ_GET_WIDTH_MEASUREMENT = 0;
        pulse_total = pulse_high + pulse_low;

        #if defined (ENABLE_LOG_CAPTURE)
        // printf_tiny("H:%4d(0x%04X),L:%4d(0x%04X),total:%4d,duty:%2.1f\r\n" , 
        // pulse_high,pulse_high,
        // pulse_low,pulse_low,
        // pulse_total,(float) pulse_high/pulse_total*100); 

        printf_tiny("H:%4d(0x%04X),L:%4d(0x%04X),Total:%4d\r\n" ,pulse_high,pulse_high,pulse_low,pulse_low,pulse_total); 

        printf_tiny("\r\n");
        #endif

        pulse_high = 0;
        pulse_low = 0;
        pulse_total = 0;

    }
}

/*
    refer to 
    RL78/G23 
    Timer Array Unit (Pulse Interval Measurement: Width) 
*/
void PulseWidthCapture_Process_in_polling(void) //duty
{
    #if 1
    // unsigned short  i = 0;

    if (INPUT_PULSE_IRQ_FLAG)
    {
        INPUT_PULSE_IRQ_FLAG = 0;

        g_times_low = g_count;     /* Set number of measurement */
        g_times_high = g_count;

        #if 1
        do
        {
            /* code */
        } while ( !FLAG_PROJ_WIDTH_HIGH_TRIG && !FLAG_PROJ_WIDTH_LOW_TRIG );

        INPUT_PULSE_MEASURE_STOP();    /* Stop timer counter */
        FLAG_PROJ_WIDTH_HIGH_TRIG = 0;
        FLAG_PROJ_WIDTH_LOW_TRIG = 0;
        
        #else
        while(1)
        {
            if ((0U == g_times_low) && (0U == g_times_high))
            {
                INPUT_PULSE_MEASURE_STOP();    /* Stop timer counter */
                break;
            }
        }
        #endif

        #if defined (ENABLE_AVG)
        // sum
        for (i = 0 ; i < WIDTH_MEASURE_TIME ; i++)
        {
            pulse_high += g_width_high[i];
            pulse_low += g_width_low[i];          
        }

        // avg
        pulse_high = pulse_high / WIDTH_MEASURE_TIME;
        pulse_low = pulse_low / WIDTH_MEASURE_TIME;
        #endif

        // calculate
        // cal_duty = (float) pulse_high/pulse_total;
        // cal_duty *= 100;

        FLAG_PROJ_GET_WIDTH_MEASUREMENT = 1;

        INPUT_PULSE_MEASURE_START(); 
    }


    #else
    while (0U == INPUT_PULSE_IRQ_FLAG)
    {
        HALT();    /* Wait first edge detection */
    }
    INPUT_PULSE_IRQ_FLAG = 0U;    /* Clear interrupt flag */
    
    g_times_low = g_count;     /* Set number of measurement */
    g_times_high = g_count;
    
    EI();         /* Enable interrupt */

    /* Wait for the measurement to finish */
    while (1U)
    {
        HALT();
        if ((0U == g_times_low) && (0U == g_times_high))
        {
            INPUT_PULSE_MEASURE_STOP();    /* Stop timer counter */
        }
    }
    #endif
}






/*
    G15 target board
    LED1 connected to P66, LED2 connected to P67
*/
void LED_Toggle(void)
{
    // PIN_WRITE(2,0) = ~PIN_READ(2,0);
    // PIN_WRITE(2,1) = ~PIN_READ(2,1);
    P2_bit.no0 = ~P2_bit.no0;
    P2_bit.no1 = ~P2_bit.no1;
}

void loop(void)
{
	// static unsigned int LOG1 = 0;

    if (Is_TIMER_PERIOD_1000MS_Trig())
    {
        reset_TIMER_PERIOD_1000MS();

        // printf_tiny("log(timer):%4d\r\n",LOG1++);
        LED_Toggle();             
    }

    if (Is_TIMER_PERIOD_SPECIFIC_Trig())
    {
        reset_TIMER_PERIOD_SPECIFIC();

        PulseWidthCapture_Process_logging();
        
        ADC_Process();  // P02
    }    


    #if defined (ENABLE_BUTTON)
    Button_Process_in_polling();
    #endif
    
    PulseWidthCapture_Process_in_polling();

    PWM_Process_Adjust();
}

#if defined (ENABLE_BUTTON)
// G15 EVB , P137/INTP0 , set both edge 
void Button_Process_long_counter(void)
{
    if (FLAG_PROJ_TRIG_BTN2)
    {
        btn_tick_counter();
    }
    else
    {
        btn_set_tick(0);
    }
}

void Button_Process_in_polling(void)
{
    static unsigned char cnt = 0;

    if (FLAG_PROJ_TRIG_BTN1)
    {
        FLAG_PROJ_TRIG_BTN1 = 0;
        printf_tiny("BTN pressed(%d)\r\n",cnt);

        if (cnt == 0)   //set both edge  , BTN pressed
        {
            FLAG_PROJ_TRIG_BTN2 = 1;
        }
        else if (cnt == 1)  //set both edge  , BTN released
        {
            FLAG_PROJ_TRIG_BTN2 = 0;
        }

        cnt = (cnt >= 1) ? (0) : (cnt+1) ;
    }

    if ((FLAG_PROJ_TRIG_BTN2 == 1) && 
        (btn_get_tick() > BTN_PRESSED_LONG))
    {         
        printf_tiny("BTN pressed LONG\r\n");
        btn_set_tick(0);
        FLAG_PROJ_TRIG_BTN2 = 0;
    }
}

// G15 EVB , P137/INTP0
void Button_Process_in_IRQ(void)    
{
    FLAG_PROJ_TRIG_BTN1 = 1;
}
#endif

void UARTx_Process(unsigned char rxbuf)
{    
    if (rxbuf > 0x7F)
    {
        printf_tiny("invalid command\r\n");
    }
    else
    {
        printf_tiny("press:%c(0x%02X)\r\n" , rxbuf,rxbuf);   // %c :  C99 libraries.
        switch(rxbuf)
        {
            case 'a':
            case 'A':
                FLAG_PROJ_PWM_DUTY_INC = 1;
                break;
            case 'd':
            case 'D':
                FLAG_PROJ_PWM_DUTY_DEC = 1;
                break;

            // case '1':
            //     FLAG_PROJ_TRIG_1 = 1;
            //     break;
            // case '2':
            //     FLAG_PROJ_TRIG_2 = 1;
            //     break;
            // case '3':
            //     FLAG_PROJ_TRIG_3 = 1;
            //     break;
            // case '4':
            //     FLAG_PROJ_TRIG_4 = 1;
            //     break;
            // case '5':
            //     FLAG_PROJ_TRIG_5 = 1;
            //     break;

            case 'X':
            case 'x':
                RL78_soft_reset(7);
                break;
            case 'Z':
            case 'z':
                RL78_soft_reset(1);
                break;
        }
    }
}

/*
    Reset Control Flag Register (RESF) 
    BIT7 : TRAP
    BIT6 : 0
    BIT5 : 0
    BIT4 : WDCLRF
    BIT3 : 0
    BIT2 : 0
    BIT1 : IAWRF
    BIT0 : LVIRF
*/
// void check_reset_source(void)
// {
//     /*
//         Internal reset request by execution of illegal instruction
//         0  Internal reset request is not generated, or the RESF register is cleared. 
//         1  Internal reset request is generated. 
//     */
//     uint8_t src = RESF;
//     printf_tiny("Reset Source <0x%08X>\r\n", src);

//     #if 1   //DEBUG , list reset source
//     if (src & BIT0)
//     {
//         printf_tiny("0)voltage detector (LVD)\r\n");       
//     }
//     if (src & BIT1)
//     {
//         printf_tiny("1)illegal-memory access\r\n");       
//     }
//     if (src & BIT2)
//     {
//         printf_tiny("2)EMPTY\r\n");       
//     }
//     if (src & BIT3)
//     {
//         printf_tiny("3)EMPTY\r\n");       
//     }
//     if (src & BIT4)
//     {
//         printf_tiny("4)watchdog timer (WDT) or clock monitor\r\n");       
//     }
//     if (src & BIT5)
//     {
//         printf_tiny("5)EMPTY\r\n");       
//     }
//     if (src & BIT6)
//     {
//         printf_tiny("6)EMPTY\r\n");       
//     }
//     if (src & BIT7)
//     {
//         printf_tiny("7)execution of illegal instruction\r\n");       
//     }
//     #endif

// }

/*
    7:Internal reset by execution of illegal instruction
    1:Internal reset by illegal-memory access
*/
//perform sofware reset
void _reset_by_illegal_instruction(void)
{
    static const unsigned char illegal_Instruction = 0xFF;
    void (*dummy) (void) = (void (*)(void))&illegal_Instruction;
    dummy();
}
void _reset_by_illegal_memory_access(void)
{
    // #if 1
    // const unsigned char ILLEGAL_ACCESS_ON = 0x80;
    // IAWCTL |= ILLEGAL_ACCESS_ON;            // switch IAWEN on (default off)
    // *(__far volatile char *)0x00000 = 0x00; //write illegal address 0x00000(RESET VECTOR)
    // #else
    // signed char __far* a;                   // Create a far-Pointer
    // IAWCTL |= _80_CGC_ILLEGAL_ACCESS_ON;    // switch IAWEN on (default off)
    // a = (signed char __far*) 0x0000;        // Point to 0x000000 (FLASH-ROM area)
    // *a = 0;
    // #endif
}

void RL78_soft_reset(unsigned char flag)
{
    switch(flag)
    {
        case 7: // do not use under debug mode
            _reset_by_illegal_instruction();        
            break;
        case 1:
            _reset_by_illegal_memory_access();
            break;
    }
}

// retarget printf
int __far putchar(int c)
{
    // G15 , UART0
    STMK0 = 1U;    /* disable INTST0 interrupt */
    TXD0 = (unsigned char)c;
    while(STIF0 == 0)
    {

    }
    STIF0 = 0U;    /* clear INTST0 interrupt flag */
    return c;
}

void hardware_init(void)
{
    // const unsigned char indicator[] = "hardware_init";
    BSP_EI();
    R_Config_UART0_Start();         // UART , P03 , P04
    R_Config_TAU0_1_Start();        // TIMER

    #if defined (ENABLE_BUTTON)
    R_Config_INTC_INTP0_Start();    // BUTTON , P137 
    #endif

    R_Config_TAU0_6_Start();        // PWM output : P121/TO07
    
    INPUT_PULSE_MEASURE_START();    // capture : P23/TI04

    ADC_init();                     // ADC : P02/AIN1
    
    // check_reset_source();
    // printf("%s finish\r\n\r\n",__func__);
    printf_tiny("%s finish\r\n\r\n",__func__);
}
