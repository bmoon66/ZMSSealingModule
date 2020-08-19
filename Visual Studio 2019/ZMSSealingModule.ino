/*
 Name:		ZMSSealingModule.ino
 Created:	11/21/2018 10:33:58 AM
 Author:	bmoon
*/

#include <Servo.h>
//TODO: Set all delays in move steps
Servo LayflatServo;										// create servo object to control a servo
//int LayflatServoPos = 0;								// variable to store the servo position

														
// Define stepper pins:: PUL, DIR, ENA, CurrentPosition, AwayPosition 
unsigned int SmSealTable[5] = { 28,29,30,0,0 };			//Stepper motor seal module table:: PUL, DIR, ENA, CurrentPosition, AwayPosition
unsigned int SmLayflatFeed[5] = { 19,20,21,0,0 };		//Stepper motor lay flat feed:: PUL, DIR, ENA, CurrentPosition, AwayPosition
unsigned int SmLayflat[5] = { 25,26,27,0,0 };			//Stepper motor Jaw with suction cups:: PUL, DIR, ENA, CurrentPosition, AwayPosition
unsigned int SmEjectLayflat[5] = { 22,23,24,0,0 };		//Stepper motor To eject from table:: PUL, DIR, ENA, CurrentPosition, AwayPosition

// Define relay pins
const int PvLayflatCutter = 47;							//IsSet
const int PvSealerLH = 46;								//IsSet
const int PvSealerRH = 44;								//IsSet
const int PvSealerClamps = 45;							//IsSet
const int PvEjectwheelCylinder = 50;					//IsSet
const int PvSucCupVacuum = 49;							//IsSet
const int PvFeed = 48;									//IsSet
const int PSIPin = A0;									//IsSet
const int TblHeaterLH = 51;								//IsSet
const int TblHeaterRH = 52;								//IsSet

// Define sensor pins
const int ProxHome = 2;									//ProxHome pin
const int ProxAway = 3;									//ProxAway pin
const int ProxLayflatHome = 4;							//ProxLayflatHome pin
const int ProxLayflatAway = 5;							//ProxLayflatAway pin
int AtHome = 1;											//proximity var
int AtAway = 1;											//proximity var

const bool CW = 1;										//For stepper direction control
const bool CCW = 0;										//For stepper direction control

bool Enable = 1;										//Global enable or disable sealing module
//int Error = 0;										//Not yet used

int CurrCMD;											//Received Current CMD
unsigned int CurrData;									//Received Current Data
unsigned int CurrData2;									//Received Current Data 2
unsigned int CurrData3;									//Received Current Data 3
int PrevCMD;											//Previous Current CMD
unsigned int PrevData;									//Previous Current Data
unsigned int PrevData2;									//Previous Current Data 2
unsigned int PrevData3;									//Previous Current Data 3

int MyServo(9);											//Global Servo
float PSI = 0;											//Global PSI


void setup() {

	LayflatServo.attach(MyServo);						// Attaches the servo on pin 9 to the servo object
	LayflatServo.write(120);							// tell servo to go to position 120
	digitalWrite(MyServo, LOW);							//Turn Servo Off


	digitalWrite(SmSealTable[2], LOW);					//Disable Stepper Driver	
	pinMode(SmSealTable[0], OUTPUT);					//Set stepper pin for output pinMode (PUL, OUTPUT);
	pinMode(SmSealTable[1], OUTPUT);					//Set stepper pin for output pinMode (DIR, OUTPUT);
	pinMode(SmSealTable[2], OUTPUT);					//Set stepper pin for output pinMode (ENA, OUTPUT);
	SmSealTable[3] = 0;									//Set current position to 0
	SmSealTable[4] = 0;									//Set current away position to 0

	digitalWrite(SmLayflatFeed[2], LOW);				//Disable Stepper Driver
	pinMode(SmLayflatFeed[0], OUTPUT);					//Set stepper pin for output pinMode (PUL, OUTPUT);
	pinMode(SmLayflatFeed[1], OUTPUT);					//Set stepper pin for output pinMode (DIR, OUTPUT);
	pinMode(SmLayflatFeed[2], OUTPUT);					//Set stepper pin for output pinMode (ENA, OUTPUT);
	SmLayflatFeed[3] = 0;								//Set current position to 0

	digitalWrite(SmLayflat[2], LOW);					//Disable Stepper Driver
	pinMode(SmLayflat[0], OUTPUT);						//Set stepper pin for output pinMode (PUL, OUTPUT);
	pinMode(SmLayflat[1], OUTPUT);						//Set stepper pin for output pinMode (DIR, OUTPUT);
	pinMode(SmLayflat[2], OUTPUT);						//Set stepper pin for output pinMode (ENA, OUTPUT);
	SmLayflat[3] = 0;									//Set current position to 0

	digitalWrite(SmEjectLayflat[2], LOW);				//Disable Stepper Driver
	pinMode(SmEjectLayflat[0], OUTPUT);					//Set stepper pin for output pinMode (PUL, OUTPUT);
	pinMode(SmEjectLayflat[1], OUTPUT);					//Set stepper pin for output pinMode (DIR, OUTPUT);
	pinMode(SmEjectLayflat[2], OUTPUT);					//Set stepper pin for output pinMode (ENA, OUTPUT);
	SmEjectLayflat[3] = 0;								//Set current position to 0
	
	
	
	pinMode(PvLayflatCutter, OUTPUT);					//Setup pins for output
	digitalWrite(PvLayflatCutter, HIGH);				//Turn Cutter Off

	pinMode(PvSealerLH, OUTPUT);						//Setup pins for output
	digitalWrite(PvSealerLH, HIGH);						//Turn PvSealerLH Off

	pinMode(PvSealerRH, OUTPUT);						//Setup pins for output
	digitalWrite(PvSealerRH, HIGH);						//Turn PvSealerRH Off

	pinMode(PvSealerClamps, OUTPUT);					//Setup pins for output
	digitalWrite(PvSealerClamps, HIGH);					//Turn PvSealerClamps Off

	pinMode(PvEjectwheelCylinder, OUTPUT);				//Setup pins for output
	digitalWrite(PvEjectwheelCylinder, HIGH);			//Turn PvEjectwheelCylinder Off

	pinMode(PvSucCupVacuum, OUTPUT);					//Setup pins for output
	digitalWrite(PvSucCupVacuum, HIGH);					//Turn PvvSucCupVacuum Off

	pinMode(PvFeed, OUTPUT);							//Setup pins for output
	digitalWrite(PvFeed, HIGH);							//Turn PvFeed Off

	pinMode(ProxHome, INPUT);							//Setup pins for input
	pinMode(ProxAway, INPUT);							//Setup pins for input
	pinMode(ProxLayflatHome, INPUT);					//Setup pins for input
	pinMode(ProxLayflatAway, INPUT);					//Setup pins for input
	pinMode(PSIPin, INPUT);								//Setup pins for input
	pinMode(TblHeaterLH, OUTPUT);						//Setup pins for output
	digitalWrite(TblHeaterLH, HIGH);					//Turn Heater Off
	pinMode(TblHeaterRH, OUTPUT);						//Setup pins for output
	digitalWrite(TblHeaterRH, HIGH);					//Turn Heater Off

	
	Serial.begin(57600);								//Start serial port
	RunCMD(222, 0, 0, 0);								//Run DisEnable Command
}


void loop() {
	
	if (Enable == 1) {														// Check if enabled
		
		if (RecvCmdData() == true) {										// If Received data and CMD is correct
											
			if (CurrCMD > 199 && CurrCMD < 300)								//Check if correct CMD numbers
			{
				RunCMD(CurrCMD, CurrData, CurrData2, CurrData3);			// Run Command
			}
			
			else
			{
				ReplyToCmdData(888, 8888, 8888, 8888);						//Send error code for CMD not > 199-299 <
			}
		}		
		else
		{
			delay(300);														//Pause a bit 		
		}
	}
	
	else 
	{
		if (RecvCmdData() == true) {										//If Received data and CMD is correct
			if (CurrCMD == 222 && CurrData == 1)							//Check if CMD enable CMD
			{
				RunCMD(CurrCMD, CurrData, CurrData2, CurrData3);			//Run Enable Command
			}
			
			else
			{
				ReplyToCmdData(777, 7777, 7777, 7777);						//Send error code for not enabled
			}
		}
		else
		{
			delay(300);														//Pause a bit 		
		}
	}
}


/*----------------------------------------------------------------------------------------------------
 Function Name:		RecvCmdData
					Checks for opening char gets CMD Data Data2 and checksum from PC
					Clears the buffer and converts data to int then returns the data
					No data returns false
_______________________________________________________________________________________________________*/
bool RecvCmdData() {												// Receive data 
	
	char AddCmdData[37];											//Array place holder for Serial.readBytesUtill
	int CmdFromPC = 0;
	unsigned int DataFromPC = 0;									//Local vars to hold broken apart rx data
	unsigned int Data2FromPc = 0;									//Local vars to hold broken apart rx data
	unsigned int Data3FromPC = 0;									//Local vars to hold broken apart rx data
	unsigned int CheckSumFromPC = 0;								//Local vars to hold broken apart rx data
	char rc;

	if (Serial.available() > 0) {
		rc = Serial.read();											//Get Start
		if (rc == '<') {											// Check if = Start char

			if (Serial.available() > 0) {
				Serial.readBytesUntil('>', AddCmdData, 37);			//Read till ">"
			}

			char * strtokIndx;										// this is used by strtok() as an index

			strtokIndx = strtok(AddCmdData, ",");					// this continues where the previous call left off
			CmdFromPC = atoi(strtokIndx);							// convert this part to an integer copy to CmdFrom PC

			strtokIndx = strtok(NULL, ",");							// this continues where the previous call left off
			DataFromPC = atoi(strtokIndx);							// convert this part to an integer copy to CmdFrom PC

			strtokIndx = strtok(NULL, ",");							// this continues where the previous call left off
			Data2FromPc = atoi(strtokIndx);							// convert this part to an integer copy to Data2FromPC

			strtokIndx = strtok(NULL, ",");							// this continues where the previous call left off
			Data3FromPC = atoi(strtokIndx);							// convert this part to an integer copy to Data2FromPC

			strtokIndx = strtok(NULL, ",");							// this continues where the previous call left off
			CheckSumFromPC = atoi(strtokIndx);						// convert this part to an integer copy to CmdFrom PC

			unsigned int y = CmdFromPC + DataFromPC + Data2FromPc + Data3FromPC;	// Checksum Check


			if (y == CheckSumFromPC) {								//Checksum Check
				CurrCMD = CmdFromPC;								//copy CmdFromPC to global CurrCmd
				CurrData = DataFromPC;								//copy DataFromPC to global CurrData
				CurrData2 = Data2FromPc;							//copy Data2FromPC to global CurrData2
				CurrData3 = Data3FromPC;							//copy Data3FromPC to global CurrData3

				memset(AddCmdData, 0, sizeof(AddCmdData));			//clear RX array
				return true;										//Good data
			}

			else {													//if bad checksum
				memset(AddCmdData, 0, sizeof(AddCmdData));			//clear RX array
				ReplyToCmdData(999, 9999, 9999,8888);				//Reply with bad data
				return false;										//Bad data
			}

		}
		else {														//if bad Start Char 
			memset(AddCmdData, 0, sizeof(AddCmdData));				//clear RX array
			ReplyToCmdData(999, 9999, 9999, 9999);					//Reply with bad data
			return false;											//Bad data
		}
	}
	return false;
}


/*----------------------------------------------------------------------------------------------------
 Function Name:		ReplyToCmdData
					Returns CMD, Data, Data2, Data3 and Checksum to PC
					Updates previous CMD, Data, Data2, Data3
					Clears current CMD, Data, Data2, Data3
_______________________________________________________________________________________________________*/
void ReplyToCmdData(int CMD, unsigned int Data, unsigned int Data2, unsigned int Data3)
{
	unsigned int CheckSum = CMD + Data + Data2 + Data3;				//Build CheckSum

	String ReturnString = "<";																		//Build String
	ReturnString = ReturnString + CMD + ',' + Data + ',' + Data2 + ',' + Data3 + ',' + CheckSum + ',' + '>';

	Serial.print(ReturnString);						//Send return string 
	PrevCMD = CurrCMD;								//Update PrevCMD
	PrevData = CurrData;							//Update CurrData
	PrevData2 = CurrData2;							//Update CurrData2
	PrevData3 = CurrData3;							//Update CurrData3

	CurrCMD = 0;									//Clear Current CMD
	CurrData = 0;									//Clear Current Data
	CurrData2 = 0;									//Clear Current Data2
	CurrData3 = 0;									//Clear Current Data3
}


/*----------------------------------------------------------------------------------------------------
 Function Name:		RunCMD
					Runs CMDs received from PC
					Returns appropriate data to PC
_______________________________________________________________________________________________________*/
bool RunCMD(int CMD, unsigned int Data, unsigned int Data2, unsigned int Data3) {
	
	switch (CMD) {

	case 1:// Send Micro controller Type |--> RX: <1,0,0,0,1,> TX: <200,0,0,0,CheckSum,>
		ReplyToCmdData(200, 0, 0, 0);							//Send Type string
		break;
	
	case 226://Engage or disengage TblHeater (1=on 0=off)|--> RX: <226,OnOff,0,0,CheckSum,>	|--> TX: <226,OnOff,0,0,CheckSum,>
		CmdPvTblHeater(Data);
		ReplyToCmdData(CurrCMD, Data, CurrData2, CurrData3);		//Send completed string	
		break;

	case 201:// Send Status |--> RX: <201,0,0,0,201,> TX: <111,1111,1111,1111,CheckSum,>
		ReplyToCmdData(111, 1111, 1111, 1111);							//Send Status string
		break;

	case 202://Return previous CMD |--> RX: <202,0,0,0,202,> |--> TX: <PreCMD,PreData,PreData,PreData,CheckSum,>
		ReplyToCmdData(202, PrevCMD, 0, 0);									//Send previous CMD		
		break;

	case 203:// Wait(NTime Mill Sec)|--> RX: <203,Time,0,0,CheckSum,>	|--> TX: <3,Time,0,0,CheckSum,>
		delay(Data);
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);			//Send completed string	
		break;

	case 204://Engage or disEngage PvSucCupVacuum (1=on 0=off)|--> RX: <204,OnOff,0,0,CheckSum,>	|--> TX: <204,OnOff,0,0,CheckSum,>
		CmdPvSucCupVacuum(Data);
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);		//Send completed string	
		break;

	case 205://Engage lay flat cutter for (Time) mill sec)|--> RX: <205,TimeOn,0,0,CheckSum,>	|--> TX: <205,TimeOn,0,0,CheckSum,>
		CmdLayFlatCutter(Data);
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);		//Send completed string	
		break;

	case 206://Engage or disEngage PvEjectwheelCylinder (1=on 0=off)|--> RX: <206,OnOff,0,0,CheckSum,>	|--> TX: <206,OnOff,0,0,CheckSum,>
		CmdPvEjectwheelCylinder(Data);
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);		//Send completed string	
		break;

	case 207://Engage or disEngage PvSealerLH (1=on 0=off)|--> RX: <207,OnOff,0,0,CheckSum,>	|--> TX: <207,OnOff,0,0,CheckSum,>
		CmdPvSealerLH(Data);
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);		//Send completed string	
		break;

	case 208://Engage or disEngage PvSealerRH (1=on 0=off)|--> RX: <208,OnOff,0,0,CheckSum,>	|--> TX: <208,OnOff,0,0,CheckSum,>
		CmdPvSealerRH(Data);
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);		//Send completed string	
		break;

	case 209://Engage Both PvSealers For (n Time)MilSec|--> RX: <209,TimeOn,0,0,CheckSum,>	|--> TX: <209,TimeOn,0,0,CheckSum,>
		CmdPvSealerLhRhTime(Data);
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);		//Send completed string	
		break;

		case 210://Engage or disEngage SealerClamps (1=on 0=off)|--> RX: <210,OnOff,0,0,CheckSum,>	|--> TX: <211,OnOff,0,0,CheckSum,>
		CmdPvSealerClamps(Data);
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);		//Send completed string	
		break;
	
	case 211://Engage or disEngage Feed Roller (1=on 0=off)|--> RX: <211,OnOff,0,0,CheckSum,>	|--> TX: <211,OnOff,0,0,CheckSum,>
		CmdFeed(Data);
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);		//Send completed string	
		break;
	
	case 212://MoveToPosition Lay-flat  |--> RX: <212,Position,Speed,0,CheckSum,>	|--> TX: <212,pos,0,0,CheckSum,>
		{
		unsigned int pos = MoveToPosition(SmLayflat,Data, Data2);
		
		if (pos == 6666) {													//if requested pos to much or to little
			ReplyToCmdData(CurrCMD, pos, CurrData2, CurrData3);				//Send error string
		}
		else {																//if notCurrPosition = requested position
			ReplyToCmdData(CurrCMD, SmSealTable[3], CurrData2, CurrData3);	//Send completed string
		}
		}
		break;

	case 213: //MoveToPosition Seal Table to Position at Speed |--> RX: <213,Position,Speed,0,CheckSum,>	|--> TX: <213,pos,0,0,CheckSum,>			
		{
		unsigned int pos = MoveToPosition(SmSealTable, Data, Data2);
		
		if (pos == 6666) {													//if requested pos to much or to little
			ReplyToCmdData(CurrCMD, pos, CurrData2, CurrData3);				//Send error string
		}
		else {																//if notCurrPosition = requested position
			ReplyToCmdData(CurrCMD, SmSealTable[3], CurrData2, CurrData3);	//Send completed string
		}
		}
		break;
	
	case 214://MoveSteps: SmSealTable, Dir, Steps, Speed|--> RX: <214,Dir,Steps,Speed,CheckSum,>	|--> TX: <214,Dir,Steps,Speed,CheckSum,>	
		MoveSteps(SmSealTable, Data, Data2, Data3);						//Move seal table n steps
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);		//Send completed string	
		break;

	case 215://Get SmSealTable Current Position	|--> RX: <215,0,0,0,215,> |--> TX: <215,Position,0,0,CheckSum,>											
		ReplyToCmdData(CurrCMD, SmSealTable[3], CurrData2, CurrData3);	//Send completed Current Position string
		break;

	case 216:// Go to home position |--> RX: <216,0,0,0,216,> |--> TX: <216,0,0,0,CheckSum,>
		GoHome();
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);		//Send completed string	
		break;

	case 217://MoveStepsFeeder: (Dir, Steps, Speed) |--> RX: <217,Dir,Steps,Speed,CheckSum,>	|--> TX: <217,Dir,Steps,Speed,CheckSum,>	
		MoveStepsFeed(Data, Data2, Data3);
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);		//Send completed string
		break;

	case 218://MoveStepsLayflat: Lay-flat Stepper, Dir, Steps, Speed |--> RX: <218,Dir,Steps,Speed,CheckSum,>	|--> TX: <218,Dir,Steps,Speed,CheckSum,>	
		MoveStepsLayflat(Data, Data2, Data3);
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);		//Send completed string
		break;
	
	case 219://Get Lay-flat position|--> RX: <219,0,0,0,219,> |--> TX: <219,LH pos,RH pos,Feed pos,CheckSum,>
		ReplyToCmdData(CurrCMD,SmLayflat[3], SmLayflat[4],0);			//Send completed Current Position and Max position
		break;
	
	case 220://MoveStepsEject  RX:Stepper, Dir, Steps, Speed  <220,Dir,Steps,Speed,CheckSum, > | -- > TX: <220,pos,0,0,CheckSum,>
		MoveStepsEject(Data, Data2, Data3);
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);		//Send completed string
		break;
	
	case 221://Get SmSealTable Max Position	|--> RX: <221,0,0,0,220,> |--> TX: <221,Position,0,0,CheckSum,>											
		ReplyToCmdData(CurrCMD, SmSealTable[4], CurrData2, CurrData3);	//Send completed max Position string
		break;
	
	case 222://Enable disable sealer	|--> RX: <222,CMD,0,0,CheckSum,> |--> TX: <222,On Off,0,0,CheckSum,>											
		if (Data == 1)
		{
			Enable = 1;
		}
		else
		{
			Enable = 0;
		}
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);		//Send completed enable or disabled (1on or 0 off)
		break;
	
	case 223: //LayflatHome |--> RX: <223,0,0,0,223,> |--> TX: <223,0,0,0,CheckSum,>
		LayflatHome();
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);		//Send completed string	
		break;
	
	case 224: //CmdReadPSI |--> RX: <224,0,0,0,223,> |--> TX: <224,0,0,0,CheckSum,>
		CmdReadPSI();
		ReplyToCmdData(CurrCMD, PSI, CurrData2, CurrData3);				//Send completed string	
		break;
			
	case 225://MoveServoToPos|--> RX: <225,Pos,0,0,CheckSum,>	|--> TX: <225,Pos,0,0,CheckSum,>	
		{
		int z = MoveServoToPos(Data);									//Move servo to position
		ReplyToCmdData(CurrCMD, z, CurrData2, CurrData3);				//Send completed string	with position
		}
		break;

	default:
		return false;
		break;
	}

	return true;
}

/*----------------------------------------------------------------------------------------------------
 Function Name:		GoHome
					Moves Stepper X Amount of steps CW or CCW
					Updates Current Position
					Settings based on divide by 1 (IMPORTANT)
______________________________________________________________________________________________________*/
void GoHome() {
	
	digitalWrite(PvLayflatCutter, LOW);							// Turn on cutter valve
	delay(500);													// Wait
	digitalWrite(PvLayflatCutter, HIGH);						// Turn off cutter valve

	 do {														// Finding Home
		AtHome = digitalRead(ProxHome);							// Checking switch until its activated
		MoveSteps(SmSealTable, CW, 5, 325);						// Move stepper till switch is activated
		}while (AtHome == 1);

	MoveSteps(SmSealTable, CCW, 25, 325);						// Move stepper back n places
	SmSealTable[3] = 0;											// Record current position
	AtHome = 1;													// Clear AtHome Var
	
	do {														// Finding Away
		AtAway = digitalRead(ProxAway);							// Checking switch until its activated
		MoveSteps(SmSealTable, CCW, 5, 325);					// Move stepper till switch is activated
		} while (AtAway == 1);

	MoveSteps(SmSealTable, CW, 25, 325);						// Move stepper back n places
	SmSealTable[4] = SmSealTable[3];							// Record current position (away position)
	AtAway = 1;

	MoveToPosition(SmSealTable, SmSealTable[4] / 3, 325);		// Move table to eject Lay-flat
	MoveSteps(SmEjectLayflat, 1, 10000, 325);							// Eject Lay-flat
		
	MoveToPosition(SmSealTable, 25, 325);						// Move to home position

	LayflatHome();												//Home Lay-flat
}


/*----------------------------------------------------------------------------------------------------
 Function Name:		LayflatHome
					Moves Stepper X Amount of steps CW or CCW
					Updates Current Position
					Settings based on divide by 2 (IMPORTANT)
______________________________________________________________________________________________________*/
void LayflatHome() {
	int LayflatAtHome = 1;										//proximity var
	int LayflatAtAway = 1;										//proximity var

	while (LayflatAtHome == 1) {								// Finding Away
		LayflatAtHome = digitalRead(ProxLayflatHome);			// Checking switch until its activated
		MoveSteps(SmLayflat, CW, 5, 350);						// Move stepper till switch is activated
	}

	MoveSteps(SmLayflat, CCW, 50, 350);							// Move stepper back n places
	SmLayflat[3] = 0;											// Record current position
	LayflatAtHome = 1;

	while (LayflatAtAway == 1) {								// Finding Home
		LayflatAtAway = digitalRead(ProxLayflatAway);			// Checking switch until its activated
		MoveSteps(SmLayflat, CCW, 5, 350);						// Move stepper till switch is activated
	}

	MoveSteps(SmLayflat, CW, 50, 350);							// Move stepper back n places
	SmLayflat[4] = SmLayflat[3];								// Record current position (away position)
	LayflatAtAway = 1;											// Clear AtHome Var

	MoveToPosition(SmLayflat, 5, 350);							// Move to home position

}
/*----------------------------------------------------------------------------------------------------
 Function Name:		CmdLayFlatCutter
					Engage lay flat cutter for (Time in mill seconds then disengages
_______________________________________________________________________________________________________*/
void CmdLayFlatCutter(int Time) {
	digitalWrite(PvLayflatCutter, LOW);						//Turn on Lay Flat Cutter Cylinder
	delay(Time);											//For n-mil sec
	digitalWrite(PvLayflatCutter, HIGH);					//Turn off Lay Flat Cutter Cylinder
}


/*----------------------------------------------------------------------------------------------------
 Function Name:		CmdPvSucCupVacuum
					Engage Suction Cup Vacuum
_______________________________________________________________________________________________________*/
void CmdPvSucCupVacuum(int Engage) {
	if (Engage == 1)
	{
		digitalWrite(PvSucCupVacuum, LOW);					//Turn on PvSucCupVacuum Cylinder
	}
	else if (Engage == 0)
	{
		digitalWrite(PvSucCupVacuum, HIGH);					//Turn off PvSucCupVacuum Cylinder
	}
}


/*----------------------------------------------------------------------------------------------------
 Function Name:		CmdPvEjectwheelCylinder
					Engage suction Cup Vacuum cylinder
_______________________________________________________________________________________________________*/
void CmdPvEjectwheelCylinder(int Engage) {
	if (Engage == 1)
	{
		digitalWrite(PvEjectwheelCylinder, LOW);				//Turn on PvEjectwheelCylinder
	}
	else if (Engage == 0)
	{
		digitalWrite(PvEjectwheelCylinder, HIGH);				//Turn off PvEjectwheelCylinder
	}
}


/*----------------------------------------------------------------------------------------------------
 Function Name:		CmdCmdPvSealerLH
					Engage suction Cup Vacuum cylinder
_______________________________________________________________________________________________________*/
void CmdPvSealerLH(int Engage) {
	if (Engage == 1)
	{
		digitalWrite(PvSealerLH, LOW);						//Turn on PvSealerLH
	}
	else if (Engage == 0)
	{
		digitalWrite(PvSealerLH, HIGH);						//Turn off PvSealerLH
	}
}


/*----------------------------------------------------------------------------------------------------
 Function Name:		CmdCmdPvSealerRH
					Engage suction Cup Vacuum cylinder
_______________________________________________________________________________________________________*/
void CmdPvSealerRH(int Engage) {
	if (Engage == 1)
	{
		digitalWrite(PvSealerRH, LOW);						//Turn on PvSealerRH
	}
	else if (Engage == 0)
	{
		digitalWrite(PvSealerRH, HIGH);						//Turn off PvSealerRH
	}
}


/*----------------------------------------------------------------------------------------------------
 Function Name:		CmdPvSealer LH RH Time
					Engage suction Cup Vacuum cylinder
_______________________________________________________________________________________________________*/
void CmdPvSealerLhRhTime(int Time) {

	digitalWrite(PvSealerLH, LOW);							//Turn on PvSealerLH
	digitalWrite(PvSealerRH, LOW);							//Turn on PvSealerRH
	delay(Time);
	digitalWrite(PvSealerLH, HIGH);							//Turn off PvSealerLH
	digitalWrite(PvSealerRH, HIGH);							//Turn off PvSealerRH
}


/*----------------------------------------------------------------------------------------------------
 Function Name:		CmdCmdPvSealerClamps
					Engage Sealer Clamps cylinder
_______________________________________________________________________________________________________*/
void CmdPvSealerClamps(int Engage) {
	if (Engage == 1)
	{
		digitalWrite(PvSealerClamps, LOW);						//Turn on PvSealerClamps
	}
	else if (Engage == 0)
	{
		digitalWrite(PvSealerClamps, HIGH);						//Turn PvSealerClamps
	}
}


/*----------------------------------------------------------------------------------------------------
 Function Name:		CmdPvTblHeater
					Engage Table Heater
_______________________________________________________________________________________________________*/
void CmdPvTblHeater(int Engage) {
	if (Engage == 1)
	{
		digitalWrite(TblHeaterLH, LOW);							//Turn on Table Heater
		digitalWrite(TblHeaterRH, LOW);							//Turn on Table Heater
	}
	else if (Engage == 0)
	{
		digitalWrite(TblHeaterLH, HIGH);						//Turn off Table Heater
		digitalWrite(TblHeaterRH, HIGH);						//Turn off Table Heater
	}
}

/*----------------------------------------------------------------------------------------------------
 Function Name:		CmdReadPSI
					Read Sealer PSI
______________________________________________________________________________________________________*/
void CmdReadPSI() {

	int AnalogReading = analogRead(PSIPin);
	PSI = (AnalogReading - 102) * 175 / (921 - 102);

}


/*----------------------------------------------------------------------------------------------------
 Function Name:		CmdFeed
					Engage Feed Roller cylinder
_______________________________________________________________________________________________________*/
void CmdFeed(int Engage) {
	if (Engage == 1)
	{
		digitalWrite(PvFeed, LOW);						//Turn on Feed Cylinder
	}
	else if (Engage == 0)
	{
		digitalWrite(PvFeed, HIGH);						//Turn off Feed Cylinder
	}
}


/*----------------------------------------------------------------------------------------------------
 Function Name:		MoveToPosition
					Moves Stepper to a location between home and away
					Checks to see if it's all ready there
					Checks to see if move CW or CCW
______________________________________________________________________________________________________*/
unsigned int MoveToPosition(unsigned int Stepper[5], unsigned int Position,  int Speed) {
	unsigned int ThePosition;
	unsigned int x;
	unsigned int y;
		
	if (Stepper[3] == Position)									// Check if requested pos is Curr position
	{
		return Position;										// Return Position
	}
	
	if (Position <= Stepper[4] && Position >= 0);				// Check if requested pos to much or to little
	{
		if (Position > Stepper[3]) {							// If move to position is grater than current position 
			x = Position - Stepper[3];							// Subtract current position from move to position
			ThePosition = MoveSteps(Stepper, CCW, x, Speed);	// Move that amount of steps CCW
			return ThePosition;									// Return Position
		}
		else if (Position < Stepper[3]) {						// If move to position is grater than current position 
			y = Stepper[3] - Position;							// Subtract move to position from current position
			ThePosition = MoveSteps(Stepper, CW, y, Speed);		// Move that amount of steps CW
			return ThePosition;									// Return Position	
	}
	
	else
		return 6666;											//Return request position error
	}
	return Position;
}


/*----------------------------------------------------------------------------------------------------
 Function Name:		MoveSteps
					Moves Stepper X Amount of steps CW or CCW
					Updates Current Position
______________________________________________________________________________________________________*/
unsigned int MoveSteps(unsigned int Stepper[5], bool Dir, unsigned int Steps,  int Speed) {
	
		if (Dir == CCW) {
			digitalWrite(Stepper[1], LOW);						//CCW   
			digitalWrite(Stepper[2], HIGH);						//Enable Stepper Driver
			
			for (unsigned int i = 0; i < Steps; i++)			//Steps
			{
				digitalWrite(Stepper[0], LOW);					//Pulse 
				delayMicroseconds(50);							//Pulse Delay
				digitalWrite(Stepper[0], HIGH);					//Pulse
				delayMicroseconds(Speed);						//Time between steps
				Stepper[3] += 1;								//Update Current Position
			}
			
			digitalWrite(Stepper[0], LOW);						//Pulse pin low  
			digitalWrite(Stepper[2], LOW);						//Disable Stepper Driver
			return Stepper[3];
		}

		else if (Dir == CW) {
			digitalWrite(Stepper[1], HIGH);						//CW   
			digitalWrite(Stepper[2], HIGH);						//Enable Stepper Driver
			
			for (unsigned int i = 0; i < Steps; i++)			//Steps
			{
				digitalWrite(Stepper[0], LOW);					//Pulse 
				delayMicroseconds(50);							//Pulse Delay
				digitalWrite(Stepper[0], HIGH);					//Pulse
				delayMicroseconds(Speed);						//Time between steps
				Stepper[3] -= 1;								//Update Current Position
			}
			
			digitalWrite(Stepper[0], LOW);						//Pulse pin low 
			digitalWrite(Stepper[2], LOW);						//Disable Stepper Driver
			return Stepper[3];
		}
	
	return Stepper[3];
}


/*----------------------------------------------------------------------------------------------------
 Function Name:		MoveFeed n Steps
					Move feed stepper
______________________________________________________________________________________________________*/
unsigned int MoveStepsFeed(bool Dir, unsigned int Steps, int Speed)
{
	MoveSteps(SmLayflatFeed, Dir, Steps, Speed);
}


/*----------------------------------------------------------------------------------------------------
 Function Name:		MoveSteps Lay-flat
					Move Lay-flat stepper on table
______________________________________________________________________________________________________*/
unsigned int MoveStepsLayflat(bool Dir, unsigned int Steps, int Speed) {

	MoveSteps(SmLayflat, Dir, Steps, Speed);					// Move that amount of steps
}


/*----------------------------------------------------------------------------------------------------
 Function Name:		MoveStepsEject
					Move Eject stepper on table
______________________________________________________________________________________________________*/
unsigned int MoveStepsEject(bool Dir, unsigned int Steps, int Speed) {

	MoveSteps(SmEjectLayflat, Dir, Steps, Speed);					// Move that amount of steps CCW
}


/*----------------------------------------------------------------------------------------------------
 Function Name:		MoveServoToPos
					Move Servo To position
______________________________________________________________________________________________________*/
int MoveServoToPos(int Pos) {
		
	LayflatServo.write(Pos);						// tell servo to go to position in variable 'pos'
	delay(15);						
	digitalWrite(MyServo,LOW);
	return Pos;
	
}