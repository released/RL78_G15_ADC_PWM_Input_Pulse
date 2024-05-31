# RL78_G15_ADC_PWM_Input_Pulse
 RL78_G15_ADC_PWM_Input_Pulse

udpate @ 2024/05/31

1. use RL78 G15 EVB to test ADC , PWM , input pulse

	- use P03 , P04 , for UART terminal TX , RX 

	- use P121/TO07 , to generate 10K freq , and change duty by terminal , 

		- press digit:a/A(increase duty) , d/D(decrease duty)

		- change duty with step : 1 %

![image](https://github.com/released/RL78_G15_ADC_PWM_Input_Pulse/blob/main/scope.jpg)


	- use P23/TI04 , to capture input pulse , to calculate freqency and duty

		- Set the TI04 pin input valid edge to “Both edge”.
		
	- use P02/AIN1 , for ADC channel 

2. enable define ENABLE_LOG_ADC , ENABLE_LOG_PWM , ENABLE_LOG_CAPTURE to printf log in terminal 	
		
if enable ADC log

![image](https://github.com/released/RL78_G15_ADC_PWM_Input_Pulse/blob/main/ENABLE_LOG_ADC.jpg)		


if enable PWM log

![image](https://github.com/released/RL78_G15_ADC_PWM_Input_Pulse/blob/main/ENABLE_LOG_PWM.jpg)		


if enable capture log

![image](https://github.com/released/RL78_G15_ADC_PWM_Input_Pulse/blob/main/ENABLE_LOG_CAPTURE.jpg)		


