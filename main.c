#include "reg52.h"

//时间参数设置
#define WASHINE_TIME 30*60 //洗涤时间
#define RELEASE_TIME 2*60 //放水时间
#define DRY_TIME     5*60 //甩干时间
#define STOP_TIME    45 //等待刹车时间

//#define WASHINE_TIME 30 //洗涤时间
//#define RELEASE_TIME 10 //放水时间
//#define DRY_TIME     60 //甩干时间
//#define STOP_TIME    35 //等待刹车时间

//继电器引脚定义
sbit rotate1      = P1^3;
sbit rotate2      = P1^2;
sbit release      = P1^1;

//蜂鸣器引脚定义
sbit beep         = P1^5;

//时间状态变量，用于判断时间在洗涤时间，放水时间，脱水时间，从而控制电机
//unsigned int timeState = WASHINE_TIME + RELEASE_TIME + DRY_TIME + STOP_TIME;
unsigned int timeState=0;
unsigned char rotateChagneTime = 0;// 电机翻转时间
unsigned char Time0time = 0;  //定时器0 计时时间
unsigned char dryReadTime = 0; //甩干启动时间
//全部结束时蜂鸣器响标志
unsigned char beeFlag = 0;

//按键定义
sbit startStopKey = P3^7;
sbit releaseKey   = P3^5;

//延时函数
void delay(unsigned int i)
{
	while(i--);
}
//定时器初始化
void InitTimer0(void)//定时器初始化 定时 50,000ms
{
    TMOD = 0x01;
    TH0  = 0x4C;
    TL0  = 0x00;
    EA   = 1;
    ET0  = 1;
    TR0  = 1;
}
//设置启动或停止洗涤
void washineStartStopSetting(void)
{
	static char flag = 1;
	if(flag)
	{
		//重置时间参数，使其开始洗涤
		timeState = WASHINE_TIME + RELEASE_TIME + DRY_TIME + STOP_TIME;
		
		//蜂鸣器响
		beep = 0;
		delay(65535);delay(65535);delay(65535);
		beep = 1;
	}
	else
	{
		//重置时间参数，使其开始停止洗涤
		timeState = 0;
		
		//蜂鸣器响
		beep = 0;
		delay(65535);delay(65535);delay(65535);
		beep = 1;
		delay(65535);delay(65535);delay(65535);
		beep = 0;
		delay(65535);delay(65535);delay(65535);
		beep = 1;
	}
	flag=!flag;
}

//设置启动或停止放水/脱水
void releaseStartStopSetting(void)
{
	static char flag = 1;
	if(flag)
	{
		//重置时间参数，使其开始放水和脱水
		timeState =  RELEASE_TIME + DRY_TIME + STOP_TIME;
		
		//蜂鸣器响
		beep = 0;
		delay(65535);delay(65535);delay(65535);
		beep = 1;
	}
	else
	{
		if(timeState>(DRY_TIME + STOP_TIME))//如果正在放水，直接停止
		{
			//重置时间参数，使其停止工作
			timeState =  0;
		}
		else//如果正在甩干，直接进入等待刹车
		{
			
			timeState =  STOP_TIME;//等待甩干停止
		}
		//蜂鸣器响
		beep = 0;
		delay(65535);delay(65535);delay(65535);
		beep = 1;
		delay(65535);delay(65535);delay(65535);
		//蜂鸣器响
		beep = 0;
		delay(65535);delay(65535);delay(65535);
		beep = 1;
	}
	
	flag=!flag;
}
void main(void)
{
	InitTimer0();
	beep = 0;
	delay(65535);delay(65535);delay(65535);
	delay(65535);delay(65535);delay(65535);
	delay(65535);delay(65535);delay(65535);
	beep = 1;
	while(1)
	{
		//启动停止按下
		if(startStopKey==0)
		{
			delay(5000);//延时消抖
			if(startStopKey==0)
			{
				//如果没有在脱水，才开始洗涤，避免快速切换状态导致损坏
				if(timeState==0 || timeState>(RELEASE_TIME + DRY_TIME + STOP_TIME))
				{
					washineStartStopSetting();//启动或停止洗涤
					rotateChagneTime = 0;
					Time0time=0;
					
					rotate1 = 1;
					rotate2 = 1;
					release = 1;
					
				}
			}
			while(startStopKey==0);
		}
		
		//放水/脱水按钮按下
		if(releaseKey==0)
		{
			delay(5000);//延时消抖
			if(releaseKey==0)
			{
				releaseStartStopSetting();//启动或停止放水/脱水
				
				rotate1 = 1;
				rotate2 = 1;
				release = 1;
				
				
			}
			while(releaseKey==0);
		}
	}
}
void Timer0Interrupt(void) interrupt 1
{
	
	
	
    TH0 = 0x4C;//延时 50,000ms
    TL0 = 0x00;
	
	Time0time++;
	if(Time0time>=20)//计时1秒//
	{
		Time0time=0;//
		
		rotateChagneTime++;
		
		if(timeState!=0)
		{
			timeState--;
		}
		
		dryReadTime++;
	}
	if(timeState > RELEASE_TIME + DRY_TIME + STOP_TIME)//洗涤
	{
		if(rotateChagneTime==0){
			rotate1 = 1;
			rotate2 = 0;
		}
		else if(rotateChagneTime==5)
		{
			rotate1 = 1;
			rotate2 = 1;
		}
		else if(rotateChagneTime==8)
		{
			rotate1 = 0;
			rotate2 = 1;
		}
		else if(rotateChagneTime==13)
		{
			rotate1 = 1;
			rotate2 = 1;
		}
		else if(rotateChagneTime>=16)
		{
			rotateChagneTime=0;
		}
	}
	else if(timeState > DRY_TIME + STOP_TIME)//放水
	{
			rotate1 = 1;
			rotate2 = 1;
			
			release = 0;
		
			dryReadTime=0 ;//脱水时间置0
	}
	else if(timeState > STOP_TIME)//脱水
	{
			if(dryReadTime==0)
			{	
				rotate1 = 0;
				rotate2 = 1;
				
				release = 0;
			}
			else if(dryReadTime==2)
			{	
				rotate1 = 1;
				rotate2 = 1;
				
				release = 0;
			}
			else if(dryReadTime==5)
			{	
				rotate1 = 0;
				rotate2 = 1;
				
				release = 0;
			}
			else if(dryReadTime==7)
			{	
				rotate1 = 1;
				rotate2 = 1;
				
				release = 0;
			}
			else if(dryReadTime==10)
			{	
				rotate1 = 0;
				rotate2 = 1;
				
				release = 0;
			}
			else if(dryReadTime==12)
			{	
				rotate1 = 1;
				rotate2 = 1;
				
				release = 0;
			}
			else if(dryReadTime==15)
			{	
				rotate1 = 0;
				rotate2 = 1;
				
				release = 0;
			}
			else if(dryReadTime==17)
			{	
				rotate1 = 1;
				rotate2 = 1;
				
				release = 0;
			}
			else if(dryReadTime==20)
			{	
				rotate1 = 0;
				rotate2 = 1;
				
				release = 0;
			}
			else if(dryReadTime==22)
			{	
				rotate1 = 1;
				rotate2 = 1;
				
				release = 0;
			}
			else if(dryReadTime==25)
			{	
				rotate1 = 0;
				rotate2 = 1;
				
				release = 0;
			}
			else if(dryReadTime==27)
			{	
				rotate1 = 1;
				rotate2 = 1;
				
				release = 0;
			}
			else if(dryReadTime>=30)
			{	
				dryReadTime=30;
				rotate1 = 0;
				rotate2 = 1;
				
				release = 0;
			}
		
	}
	else if(timeState > 0)//放水，等待甩干停止
	{
		rotate1 = 1;
		rotate2 = 1;
		
		release = 0;
		
		beeFlag = 1;//准备全部结束时蜂鸣器响
	}
	else //所有 结束
	{
		rotate1 = 1;
		rotate2 = 1;
		
		release = 1;
		
		//结束时 蜂鸣器 滴滴 几 声
		if(beeFlag==1)
		{
			beeFlag = 0;
			
			beep = 0;
			delay(65535);delay(65535);delay(65535);
			beep = 1;
			delay(65535);delay(65535);delay(65535);
			beep = 0;
			delay(65535);delay(65535);delay(65535);
			beep = 1;
			delay(65535);delay(65535);delay(65535);
			beep = 0;
			delay(65535);delay(65535);delay(65535);
			beep = 1;
			delay(65535);delay(65535);delay(65535);
			beep = 0;
			delay(65535);delay(65535);delay(65535);
			beep = 1;
			delay(65535);delay(65535);delay(65535);
			beep = 0;
			delay(65535);delay(65535);delay(65535);
			beep = 1;
			delay(65535);delay(65535);delay(65535);
			beep = 0;
			delay(65535);delay(65535);delay(65535);
			beep = 1;

		}
		
	}
}
