#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, dgtl1,  LEncoder,       sensorQuadEncoder)
#pragma config(Sensor, dgtl3,  REncoder,       sensorQuadEncoder)
#pragma config(Sensor, dgtl5,  LiftMainEncoder, sensorQuadEncoder)
#pragma config(Sensor, dgtl7,  LiftHoldEncoder, sensorQuadEncoder)
#pragma config(Sensor, dgtl9,  LiftHoldLimit,  sensorTouch)
#pragma config(Sensor, dgtl10, LiftMainLimit,  sensorTouch)
#pragma config(Sensor, I2C_1,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Motor,  port1,           BallGetMotor,  tmotorVex393_HBridge, openLoop)
#pragma config(Motor,  port2,           BR,            tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port3,           BL,            tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port4,           FL,            tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port5,           FR,            tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port6,           BallShootMotor, tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port7,           LiftMainMotor, tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port8,           ClawMotor,     tmotorVex393_MC29, openLoop, encoderPort, I2C_1)
#pragma config(Motor,  port9,           LiftHoldMotor, tmotorVex393_MC29, openLoop, reversed)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

//interval
#define FLASH_INTERVAL 1
//timeLimit
#define USER_TIME_LIMT 105000
#define AUTO_TIME_LIMIT 15000

//clawDef
#define CLAW_ACTION_BTN Btn7R
#define CLAW_REVERSE_SPEED 127
#define CLAW_REVERSE_TARGET -1557

//driveDef
#define JOY_THREASHOLD 20
#define JOY_VERTICAL_CH Ch3
#define JOY_HORIZONTAL_CH Ch4
#define JOY_ROTATE_CH Ch1
#define JOY_NORMAL_DIR_BTN Btn7U
#define JOY_REVERSE_DIR_BTN Btn7D

//getballDef
#define GETBALL_SPEED 127
#define GETBALL_BTN Btn6D
#define SPITBALL_BTN Btn5U

//shootBall
#define SHOOTBALL_SPEED 127
#define SHOOTBALL_BTN Btn5D

//liftDef
#define LIFT_MAIN_HIGH_BTN Btn8U
#define LIFT_MAIN_LOW_BTN Btn8D
#define LIFT_MAIN_RESET_BTN Btn8R
#define HOLD_MODE Btn8L
#define HOLD_INIT_BTN Btn6U
#define LIFT_MAIN_LOW 85
#define LIFT_MAIN_HIGH 122
#define MAPPING_A 2.35
#define OFFSET_HOLD 133
#define OFFSET_GET 238
#define OFFSET_GET_AND_HOLD -105

//lockRob
#define LOCK_POWER 5

//continueBtn
#define CONTINUE_BTN Btn7D

//emergencyStopBtn

float MainBattery;
float BackUpBattery;
int LiftMainTarget = 0;
int LiftHoldTarget = 0;
int LiftMainToHoldOffest = OFFSET_GET;
int ClawTarget = 0;
int ClawFlag = 0;
int HoldModeFlag = 1;
int HoldModeIndicator = 1;
int IsMappingDisabled = 0;
int IsClawDisabled = 0;
int IsDriveReversed = 0;

const float sFix = 0.86;

//pid struct and init
typedef struct
{
	float Kp;
	float Ki;
	float Kd;
	float Error;
	float LastError;
	float Integral;
	float Derivative;
	float IntegralLimit;
	float OutPut;
	float OutPutMax;
	float OutPutMin;
} PID;


PID LiftMainPid;
PID LiftHoldPid;
PID ClawPid;

//
//pid and basic function
//

void pidInit(PID pid, float kp, float ki, float kd, float integralLimit)
{
	pid.Kp = kp;
	pid.Ki = ki;
	pid.Kd = kd;
	pid.IntegralLimit = integralLimit;
	pid.LastError = 0;
	pid.Integral = 0;
}

void pidSetOutputLimit(PID pid, int MaxOutput ,int MinOutput)
{
	pid.OutPutMax = MaxOutput;
	pid.OutPutMin = MinOutput;
}

void pidTempInit(PID pid)
{
	pid.LastError = 0;
	pid.Integral = 0;
}

task PIDErrorCalculation()
{
	while (true)
	{
		LiftMainPid.Error = LiftMainTarget - SensorValue[LiftMainEncoder];
		LiftHoldPid.Error = LiftHoldTarget - SensorValue[LiftHoldEncoder];
		ClawPid.Error = ClawTarget - getMotorEncoder(ClawMotor);
	}
}

void pidCalculateIME(PID pid, char MotorIndex)
{
	pidTempInit(pid);
	resetMotorEncoder(MotorIndex);
	while (true)
	{
		if (abs(pid.Error) < pid.IntegralLimit)
		{
			pid.Integral += pid.Error;
		}

		pid.Derivative = pid.Error - pid.LastError;
		pid.LastError = pid.Error;

		pid.OutPut = (pid.Kp * pid.Error +
					  pid.Ki * pid.Integral +
					  pid.Kd * pid.Derivative);

		if (pid.OutPut > pid.OutPutMax)
			pid.OutPut = pid.OutPutMax;
		if (pid.OutPut < pid.OutPutMin)
			pid.OutPut = pid.OutPutMin;

		motor[MotorIndex] = pid.OutPut;
	}
}

void pidCalculate(PID pid, char SensorIndex, char MotorIndex)
{
	pidTempInit(pid);
	SensorValue[SensorIndex] = 0;
	while (true)
	{
		if (abs(pid.Error) < pid.IntegralLimit)
		{
			pid.Integral += pid.Error;
		}

		pid.Derivative = pid.Error - pid.LastError;
		pid.LastError = pid.Error;

		pid.OutPut = (pid.Kp * pid.Error +
					  pid.Ki * pid.Integral +
					  pid.Kd * pid.Derivative);

		if (pid.OutPut > pid.OutPutMax)
			pid.OutPut = pid.OutPutMax;
		if (pid.OutPut < pid.OutPutMin)
			pid.OutPut = pid.OutPutMin;

		motor[MotorIndex] = pid.OutPut;
	}
}

void SelfCheck()
{
	motor[LiftHoldMotor] = -120;
	while(true)
	{
		if(SensorValue(LiftHoldLimit))
		{
			motor[LiftHoldMotor] = 0;
			break;
		}

	}
}

int BtnClick(char BtnIndex)
{
	if (vexRT[BtnIndex] == 1)
	{
		while (vexRT[BtnIndex]) {}
		return 1;
	}
	else
	{
		return 0;
	}
}

//pidTask

task LiftMain()
{
	pidCalculate(LiftMainPid, LiftMainEncoder, LiftMainMotor);
}

task LiftHold()
{
	pidCalculate(LiftHoldPid, LiftHoldEncoder ,LiftHoldMotor);
}

task Claw()
{
	pidCalculateIME(ClawPid, ClawMotor);
}

//
///MainController
//

void MappingUsageControl(int ErrorOffset, int ErrorThreashold)
{
	if (abs(LiftMainPid.Error) - ErrorOffset <= ErrorThreashold)
	{
		IsMappingDisabled = 1;
	}
	else
	{
		IsMappingDisabled = 0;
	}
}

void LiftMainControl()
{
	if (vexRT[LIFT_MAIN_HIGH_BTN])
	{
		LiftMainToHoldOffest = OFFSET_HOLD;
		HoldModeFlag = 0;
		LiftMainTarget = LIFT_MAIN_HIGH;
	}
	else if (vexRT[LIFT_MAIN_LOW_BTN])
	{
		LiftMainToHoldOffest = OFFSET_HOLD;
		HoldModeFlag = 0;
		LiftMainTarget = LIFT_MAIN_LOW;
	}
	else if (vexRT[LIFT_MAIN_RESET_BTN])
	{
		LiftMainToHoldOffest = OFFSET_GET;
		HoldModeFlag = 1;
		LiftMainTarget = 0;
	}
}

void LiftMainToHoldMapping()
{
	if (!IsMappingDisabled)
	{
		LiftHoldTarget = MAPPING_A * (SensorValue[LiftMainEncoder]) + LiftMainToHoldOffest;
	}
}

void ClawReverseControl()
{
	if (!IsClawDisabled)
	{
		if (BtnClick(CLAW_ACTION_BTN))
		{
			if (ClawFlag == 1)
			{
				if(HoldModeFlag == 1)
				{
					LiftHoldTarget += OFFSET_GET_AND_HOLD;
					HoldModeFlag = 0;
				}
				ClawTarget = 0;
				ClawFlag = 0;
			}
			else
			{
				if(HoldModeFlag == 1)
				{
					LiftHoldTarget += OFFSET_GET_AND_HOLD;
					HoldModeFlag = 0;
				}
				ClawTarget = CLAW_REVERSE_TARGET;
				ClawFlag = 1;
			}
		}
	}
}

void HoldModeControl()
{
	if (BtnClick(HOLD_MODE))
	{
		if (LiftHoldTarget != 0)
		{
			if (HoldModeFlag == 1)
			{
				LiftHoldTarget += OFFSET_GET_AND_HOLD;
				HoldModeFlag = 0;
			}
			else
			{
				LiftHoldTarget -= OFFSET_GET_AND_HOLD;
				HoldModeFlag = 1;
			}
		}
	}
}

void HoldInitControl()
{
	if (BtnClick(HOLD_INIT_BTN))
	{
		if (HoldModeIndicator)
		{
			suspendTask(LiftHold);
			SelfCheck();
			resumeTask(LiftHold);
			pidInit(LiftHoldPid, 2.5, 0, 0, 5);
			LiftHoldTarget = 0;
			HoldModeIndicator = 0;
		}
		else
		{
			if (HoldModeFlag == 1)
			{
				LiftHoldTarget = MAPPING_A * (SensorValue[LiftMainEncoder]) + OFFSET_GET;
				HoldModeIndicator = 1;
			}
			else
			{
				LiftHoldTarget = MAPPING_A * (SensorValue[LiftMainEncoder]) + OFFSET_HOLD;
				HoldModeIndicator = 1;
			}
		}
	}
}

///
///autofunction
///

void autoMoveStraight(int dis, int speed)
{
	SensorValue(LEncoder) = 0;
	SensorValue(REncoder) = 0;
	while (abs(SensorValue[LEncoder]) < abs(dis) && abs(SensorValue[REncoder]) < abs(dis))
	{
		motor[FL] = motor[BL] = speed * sFix;
		motor[FR] = motor[BR] = speed;
	}
	motor[FL] = motor[BL] = motor[FR] = motor[BR] = 0;
}

void autoMoveStraightWithShoot(int dis, int speed, int ShootSpeed)
{
	SensorValue(LEncoder) = 0;
	SensorValue(REncoder) = 0;
	while (abs(SensorValue[LEncoder]) < abs(dis) && abs(SensorValue[REncoder]) < abs(dis))
	{
		motor[FL] = motor[BL] = speed * sFix;
		motor[FR] = motor[BR] = speed;
		motor[BallShootMotor] = ShootSpeed;
	}
	motor[FL] = motor[BL] = motor[FR] = motor[BR] = 0;
}

void autoMoveParallel(int dis, int speed)
{
	SensorValue(LEncoder) = 0;
	SensorValue(REncoder) = 0;
	while (abs(SensorValue[LEncoder]) < abs(dis) && abs(SensorValue[REncoder]) < abs(dis))
	{
		motor[FL] = motor[BR] = speed;
		motor[BL] = motor[FR] = -speed;
	}
	motor[FL] = motor[BR] = motor[BL] = motor[FR] = 0;
}

void autoTurn(int dis, int speed)
{
	SensorValue(LEncoder) = 0;
	SensorValue(REncoder) = 0;
	while (abs(SensorValue[LEncoder]) < abs(dis) && abs(SensorValue[REncoder]) < abs(dis))
	{
		motor[FL] = motor[BL] = speed * sFix;
		motor[BR] = motor[FR] = -speed;
	}
	motor[FL] = motor[BR] = motor[BL] = motor[FR] = 0;
}


void autoClimbTime(int time, int speed)
{
	SensorValue(LEncoder) = 0;
	SensorValue(REncoder) = 0;
	clearTimer(T1);
	while (time1[T1] <= time)
	{
		motor[FL] = motor[BL] = speed * sFix;
		motor[BR] = motor[FR] = speed;
		//motor[BallGetMotor] = GETBALL_SPEED;
	}
	motor[FL] = motor[BR] = motor[BL] = motor[FR] = 0;
	//motor[BallGetMotor] = 0;
}

void autoShootBall(int delayTime, int speed)
{
	motor[BallShootMotor] = speed;
	wait1Msec(delayTime);
	motor[BallShootMotor] = 0;
}

//
//userTask
//

task ShootBall()
{
	while (true)
	{
		if (vexRT[SHOOTBALL_BTN])
		{
			motor[BallShootMotor] = SHOOTBALL_SPEED;
		}
		else
		{
			motor[BallShootMotor] = 0;
		}
	}
}

task GetBall()
{
	while (true)
	{
		if (vexRT[GETBALL_BTN])
		{
			motor[BallGetMotor] = GETBALL_SPEED;
		}
		else if (vexRT[SPITBALL_BTN])
		{
			motor[BallGetMotor] = -GETBALL_SPEED;
		}
		else
		{
			motor[BallGetMotor] = 0;
		}
	}
}

task Drive()
{

	int X2 = 0, Y1 = 0, X1 = 0;
	while (true)
	{
		if (abs(vexRT[JOY_VERTICAL_CH]) > JOY_THREASHOLD)
		{
			Y1 = vexRT[JOY_VERTICAL_CH];
		}
		else
		{
			Y1 = 0;
		}

		if (abs(vexRT[JOY_HORIZONTAL_CH]) > JOY_THREASHOLD)
		{
			X1 = vexRT[JOY_HORIZONTAL_CH];
		}
		else
		{
			X1 = 0;
		}

		if (abs(vexRT[JOY_ROTATE_CH]) > JOY_THREASHOLD)
		{
			if (IsDriveReversed)
			{
				X2 = -vexRT[JOY_ROTATE_CH];
			}
			else
			{
				X2 = vexRT[JOY_ROTATE_CH];
			}
		}
		else
		{
			X2 = 0;
		}

		motor[FR] = Y1 - X2 - X1;
		motor[BR] = Y1 - X2 + X1;
		motor[FL] = Y1 + X2 + X1;
		motor[BL] = Y1 + X2 - X1;

		if (vexRT[JOY_REVERSE_DIR_BTN] == 1)
		{
			IsDriveReversed = 1;
			bMotorReflected[BL] = true;
			bMotorReflected[BR] = true;
			bMotorReflected[FL] = true;
			bMotorReflected[FR] = true;
		}
		if (vexRT[JOY_NORMAL_DIR_BTN] == 1)
		{
			IsDriveReversed = 0;
			bMotorReflected[BL] = false;
			bMotorReflected[BR] = false;
			bMotorReflected[FL] = false;
			bMotorReflected[FR] = false;
		}

		//poslock

	}
}

////
// autoFunctionMain
////
// move :encoder 648 = 1ge
// turn :encoder 590 = 90du
// parallel right +
// turn right +

void BlueBack()
{
	autoMoveParallel(100,-127)
	autoMoveStraightWithShoot(550,-127，0);
	autoShootBall(1000,127);
	autoTurn(600,-127);
	LiftHoldTarget = OFFSET_GET;
	autoMoveStraight(1050,-127);
	wait1Msec(800);
	LiftHoldTarget = OFFSET_HOLD;
	ClawTarget = CLAW_REVERSE_TARGET;
	wait1Msec(1500);
	LiftHoldTarget = OFFSET_GET;
	wait1Msec(1000);
	autoMoveStraight(110,127);
	wait1Msec(500);
	LiftHoldTarget = 0;
	autoTurn(550,127);
	autoMoveStraight(500,127);
	autoClimbTime(2200,127);
}

void RedBack()
{
	autoMoveStraight(550,-127);
	autoTurn(600,-127);
	LiftHoldTarget = OFFSET_GET;
	autoMoveStraight(1050,-127);
	wait1Msec(800);
	LiftHoldTarget = OFFSET_HOLD;
	ClawTarget = CLAW_REVERSE_TARGET;
	wait1Msec(1500);
	LiftHoldTarget = OFFSET_GET;
	wait1Msec(1000);
	autoMoveStraight(110,127);
	wait1Msec(500);
	LiftHoldTarget = 0;
	autoTurn(550,127);
	autoMoveStraight(500,127);
	autoClimbTime(2200,127);
}

/*void BlueFront()
{
	LiftHoldTarget = OFFSET_GET;
	autoMoveStraight(150, 60);
	autoShootBall(1500,127);
	autoMoveStraight(1150,127);
	autoMoveStraight(680,-127);
	autoMoveParallel(100,127);
	autoTurn(570,127);
	autoMoveStraight(520,-127);
	autoMoveStraight(300,-50);
	wait1Msec(1000);
	LiftHoldTarget = OFFSET_HOLD;
	ClawTarget = CLAW_REVERSE_TARGET;
	wait1Msec(1300);
	LiftHoldTarget = OFFSET_GET;
	autoMoveStraight(60,127);
}


void RedFront()
{
	LiftHoldTarget = OFFSET_GET;
	autoMoveStraight(150, 60);
	autoShootBall(1500,0);
	autoMoveStraight(1150,127);
	autoMoveStraight(680,-127);
	autoMoveParallel(100,127);
	autoTurn(570,-127);
	autoMoveStraight(520,-127);
	autoMoveStraight(300,-50);
	wait1Msec(1000);
	LiftHoldTarget = OFFSET_HOLD;
	ClawTarget = CLAW_REVERSE_TARGET;
	wait1Msec(2000);
	LiftHoldTarget = OFFSET_GET;
	autoMoveStraight(60,127);
}*/

task main()
{

	SelfCheck();
    
	//startPidSystems
	pidSetOutputLimit(LiftMainPid,127, -40);
	pidSetOutputLimit(ClawPid, 127, -127);
	pidSetOutputLimit(LiftHoldPid, 90, -127);
	pidInit(LiftMainPid, 20, 0, 0, 5);
	pidInit(LiftHoldPid, 3, 0, 0, 5);
	pidInit(ClawPid, 1, 0, 0, 5);
	startTask(PIDErrorCalculation);
	startTask(LiftMain);
	startTask(LiftHold);
	startTask(GetBatteryStatue);
	startTask(Claw);

	//BlueBack();
	//BlueFront();
	RedBack();
	//RedFront();

	startTask(ShootBall);
	startTask(GetBall);
	startTask(Drive);

	LiftHoldTarget = OFFSET_GET;
	LiftMainTarget = 0;

	while (true)
	{

		MappingUsageControl(1, 5);
		LiftMainControl();
		LiftMainToHoldMapping();
		HoldModeControl();
		ClawReverseControl();
		HoldInitControl();

	}
}
