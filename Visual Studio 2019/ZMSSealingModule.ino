/*
 Name:		ZMSSealingModule.ino
 Created:	11/21/2018 10:33:58 AM
 Author:	bmoon
*/

// Define stepper pins:: PUL, DIR, ENA, CurrentPosition, AwayPosition 
unsigned int SmSealTable[5] = { 6,5,4,0,0 };			//Stepper motor seal module table:: PUL, DIR, ENA, CurrentPosition, AwayPosition
unsigned int SmLayflatFeed[5] = { 16,15,14,0,0 };		//Stepper motor lay flat feed:: PUL, DIR, ENA, CurrentPosition, AwayPosition
unsigned int SmLayflatLH[5] = { 9,8,7,0,0 };			//Stepper motor left hand of table:: PUL, DIR, ENA, CurrentPosition, AwayPosition
unsigned int SmLayflatRH[5] = { 12,11,10,0,0 };			//Stepper motor right hand of table:: PUL, DIR, ENA, CurrentPosition, AwayPosition

// Define relay pins
const int PvLayflatCutter = 40;							//Add pin number
const int PvSealerLH = 38;								//Add pin number
const int PvSealerRH = 42;								//Add pin number
const int PvSealerClamps = 39;							//Add pin number
const int PvSucCupCylinder = 41;						//Add pin number
const int PvSucCupVacume = 43;							//Add pin number

// Define sensor pins
const int ProxHome = 3;									//Add pin number
const int ProxAway = 2;									//Add pin number

const bool CW = 1;										//For stepper direction control
const bool CCW = 0;										//For stepper direction control

bool Enable = 0;										//Enable or disable sealing module
int Error = 0;											//Not yet used

int CurrCMD;											//Recieved Current CMD
unsigned int CurrData;									//Recieved Current Data
unsigned int CurrData2;									//Recieved Current Data 2
unsigned int CurrData3;									//Recieved Current Data 3
int PrevCMD;											//Previous Current CMD
unsigned int PrevData;									//Previous Current Data
unsigned int PrevData2;									//Previous Current Data 2
unsigned int PrevData3;									//Previous Current Data 3

int AtHome = 1;											//Global proximity var
int AtAway = 1;											//Global proximity var
											
void setup() {
	
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

	digitalWrite(SmLayflatLH[2], LOW);					//Disable Stepper Driver
	pinMode(SmLayflatLH[0], OUTPUT);					//Set stepper pin for output pinMode (PUL, OUTPUT);
	pinMode(SmLayflatLH[1], OUTPUT);					//Set stepper pin for output pinMode (DIR, OUTPUT);
	pinMode(SmLayflatLH[2], OUTPUT);					//Set stepper pin for output pinMode (ENA, OUTPUT);
	SmLayflatLH[3] = 0;									//Set current position to 0

	digitalWrite(SmLayflatRH[2], LOW);					//Disable Stepper Driver
	pinMode(SmLayflatRH[0], OUTPUT);					//Set stepper pin for output pinMode (PUL, OUTPUT);
	pinMode(SmLayflatRH[1], OUTPUT);					//Set stepper pin for output pinMode (DIR, OUTPUT);
	pinMode(SmLayflatRH[2], OUTPUT);					//Set stepper pin for output pinMode (ENA, OUTPUT);
	SmLayflatRH[3] = 0;									//Set current position to 0

	pinMode(PvLayflatCutter, OUTPUT);					//Setup pins for output
	digitalWrite(PvLayflatCutter, HIGH);				//Turn Cutter Off

	pinMode(PvSealerLH, OUTPUT);						//Setup pins for output
	digitalWrite(PvSealerLH, HIGH);						//Turn PvSealerLH Off

	pinMode(PvSealerRH, OUTPUT);						//Setup pins for output
	digitalWrite(PvSealerRH, HIGH);						//Turn PvSealerRH Off

	pinMode(PvSealerClamps, OUTPUT);					//Setup pins for output
	digitalWrite(PvSealerClamps, HIGH);					//Turn PvSealerClamps Off

	pinMode(PvSucCupCylinder, OUTPUT);					//Setup pins for output
	digitalWrite(PvSucCupCylinder, HIGH);				//Turn PvSucCupCylinder Off

	pinMode(PvSucCupVacume, OUTPUT);					//Setup pins for output
	digitalWrite(PvSucCupVacume, HIGH);					//Turn PvvSucCupVacume Off

	pinMode(ProxHome, INPUT);							//Setup pins for input
	pinMode(ProxAway, INPUT);							//Setup pins for input
		   
	Serial.begin(57600);								//Start serial port
}

void loop() {

	if (Enable == 1)														// Check if enabled
	{
		if (RecvCmdData() == true) {										// If Recieved data and CMD is correct

			if (CurrCMD == 1)												//Check if CMD enable CMD
			{
				RunCMD(CurrCMD, CurrData, CurrData2, CurrData3);			//Run Get Module Type Command
			}
							
			else {

				if (CurrCMD > 200 && CurrCMD < 299)							//Check if correct CMD numbers
				{
					RunCMD(CurrCMD, CurrData, CurrData2, CurrData3);		// Run Command
				}
				else
				{
					ReplyToCmdData(888, 8888, 8888, 8888);					//Send error code for CMD not > 100-200 <
				}
			}
		}
		else
		{
			delay(300);														//Wait a bit for new data
		}
	}
	else
	{
		if (RecvCmdData() == true) {										//If Recieved data and CMD is correct
			if (CurrCMD == 221 && CurrData == 1)											//Check if CMD enable CMD
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
bool RecvCmdData() {												// Recieve data 

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
			DataFromPC = atof(strtokIndx);							// convert this part to an integer copy to CmdFrom PC

			strtokIndx = strtok(NULL, ",");							// this continues where the previous call left off
			Data2FromPc = atof(strtokIndx);							// convert this part to an integer copy to Data2FromPC

			strtokIndx = strtok(NULL, ",");							// this continues where the previous call left off
			Data3FromPC = atof(strtokIndx);							// convert this part to an integer copy to Data2FromPC

			strtokIndx = strtok(NULL, ",");							// this continues where the previous call left off
			CheckSumFromPC = atof(strtokIndx);						// convert this part to an integer copy to CmdFrom PC

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
				ReplyToCmdData(999, 9999, 9999, 9999);				//Reply with bad data
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

	case 1:/// Send Microcontroller Type |--> RX: <1,0,0,0,1,> TX: <200,0,0,0,CheckSum,>
		ReplyToCmdData(200, 0, 0, 0);							//Send Status string
		break;
	
	case 201:/// Send Status |--> RX: <201,0,0,0,201,> TX: <111,1111,1111,1111,CheckSum,>
		ReplyToCmdData(111, 1111, 1111, 1111);							//Send Status string
		break;

	case 202:///Return previous CMD |--> RX: <202,0,0,0,202,> |--> TX: <PreCMD,PreData,PreData,PreData,CheckSum,>
		ReplyToCmdData(202, PrevCMD, 0, 0);									//Send previous CMD		
		break;

	case 203:/// Wait(NTime Mill Sec)|--> RX: <3,Time,0,0,3,>	|--> TX: <3,Time,0,0,CheckSum,>
		delay(Data);
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);			//Send completed string	
		break;

	case 204:///Enguage or disenguage PvSucCupVacume (1=on 0=off)|--> RX: <204,OnOff,0,0,204,>	|--> TX: <204,OnOff,0,0,CheckSum,>
		CmdPvSucCupVacume(Data);
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);		//Send completed string	
		break;

	case 205:///Enguage lay flat cutter for (Time) mill sec)|--> RX: <205,TimeOn,0,0,205,>	|--> TX: <205,TimeOn,0,0,CheckSum,>
		CmdLayFlatCutter(Data);
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);		//Send completed string	
		break;

	case 206:///Enguage or disenguage SucCupCylinder (1=on 0=off)|--> RX: <206,OnOff,0,0,206,>	|--> TX: <206,OnOff,0,0,CheckSum,>
		CmdPvSucCupCylinder(Data);
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);		//Send completed string	
		break;

	case 207:///Enguage or disenguage PvSealerLH (1=on 0=off)|--> RX: <207,OnOff,0,0,207,>	|--> TX: <207,OnOff,0,0,CheckSum,>
		CmdPvSealerLH(Data);
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);		//Send completed string	
		break;

	case 208:///Enguage or disenguage PvSealerRH (1=on 0=off)|--> RX: <208,OnOff,0,0,208,>	|--> TX: <208,OnOff,0,0,CheckSum,>
		CmdPvSealerRH(Data);
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);		//Send completed string	
		break;

	case 209:///Enguage PvSealer For (n Time)MilSec|--> RX: <209,TimeOn,0,0,209,>	|--> TX: <209,TimeOn,0,0,CheckSum,>
		CmdPvSealerLhRhTime(Data);
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);		//Send completed string	
		break;

	case 210:///MoveToPosition Feed Lh Rh  |--> RX: <210,Position,Speed,0,210,>	|--> TX: <210,pos,0,0,CheckSum,>
	{
		unsigned int pos = MoveToPositionFeedLhRh(Data, Data2);
		ReplyToCmdData(CurrCMD, pos, CurrData2, CurrData3);				//Send completed string
		pos = 0;
	}
		break;
		
	case 211:///Enguage or disenguage SealerClamps (1=on 0=off)|--> RX: <211,OnOff,0,0,211,>	|--> TX: <211,OnOff,0,0,CheckSum,>
		CmdPvSealerClamps(Data);
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);		//Send completed string	
		break;
	
	case 212: ///MoveToPosition Seal Table to  Position at Speed |--> RX: <212,Position,Speed,0,212,>	|--> TX: <212,pos,0,0,CheckSum,>			
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
	
	case 213:///MoveSteps: SmSealTable, Dir, Steps, Speed|--> RX: <213,Dir,Steps,Speed,213,>	|--> TX: <213,Dir,Steps,Speed,CheckSum,>	
		MoveSteps(SmSealTable, Data, Data2, Data3);						//Move seal table n steps
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);		//Send completed string	
		break;

	case 214:///Get SmSealTable Current Position	|--> RX: <214,0,0,0,214,> |--> TX: <214,Position,0,0,CheckSum,>											
		ReplyToCmdData(CurrCMD, SmSealTable[3], CurrData2, CurrData3);	//Send completed Current Position string
		break;

	case 215:/// Go to home position |--> RX: <215,0,0,0,215,> |--> TX: <215,0,0,0,CheckSum,>
		GoHome();
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);		//Send completed string	
		break;

	case 216:///MoveSteps: Lh Rh steppers , Dir, Steps, Speed |--> RX: <216,Dir,Steps,Speed,216,>	|--> TX: <216,Dir,Steps,Speed,CheckSum,>	
		MoveStepsLhRh(Data, Data2, Data3);
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);		//Send completed string

		break;

	case 217:///MoveSteps: Lh Rh and feed steppers , Dir, Steps, Speed |--> RX: <217,Dir,Steps,Speed,217,>	|--> TX: <217,Dir,Steps,Speed,CheckSum,>	
		MoveStepsFeedLhRh(Data, Data2, Data3);
		ReplyToCmdData(CurrCMD, CurrData, CurrData2, CurrData3);		//Send completed string
		break;
	case 218:///Get Lh Rh Feed position|--> RX: <218,0,0,0,218,> |--> TX: <218,LH pos,RH pos,Feed pos,CheckSum,>
		ReplyToCmdData(CurrCMD, SmLayflatLH[3], SmLayflatRH[3], SmLayflatFeed[3]);	//Send completed Current Position string
		break;
	
	case 219:///MoveToPosition Lh Rh  | -- > RX: <219,Position,Speed,0,219, > | -- > TX: <219,pos,0,0,CheckSum,>
		{
		unsigned int pos = MoveToPositionLhRh(Data, Data2);
		ReplyToCmdData(CurrCMD, pos, 0, 0);								//Send completed string
		pos = 0;
		}
		break;
	
	case 220:///Get SmSealTable Max Position	|--> RX: <220,0,0,0,220,> |--> TX: <220,Position,0,0,CheckSum,>											
		ReplyToCmdData(CurrCMD, SmSealTable[4], CurrData2, CurrData3);	//Send completed max Position string
		break;
	
	case 221:///Enable disable sealer	|--> RX: <221,CMD,0,0,220,> |--> TX: <221,On Off,0,0,CheckSum,>											
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
	
	default:
		return false;
		break;
	}

	return true;
}

/*----------------------------------------------------------------------------------------------------
 Function Name:		CmdLayFlatCutter
					Enguages lay flat cutter for (Time in mill seconds then disenguages
_______________________________________________________________________________________________________*/
void CmdLayFlatCutter(int Time) {
	digitalWrite(PvLayflatCutter, LOW);						//Turn on Lay Flat Cutter Cylinder
	delay(Time);											// For n-milsec
	digitalWrite(PvLayflatCutter, HIGH);					//Turn off Lay Flat Cutter Cylinder
}

/*----------------------------------------------------------------------------------------------------
 Function Name:		CmdPvSucCupVacume
					Enguages Suc Cup Vacume
_______________________________________________________________________________________________________*/
void CmdPvSucCupVacume(int Engage) {
	if (Engage == 1)
	{
		digitalWrite(PvSucCupVacume, LOW);					//Turn on Lay Flat Cutter Cylinder
	}
	else if (Engage == 0)
	{
		digitalWrite(PvSucCupVacume, HIGH);					//Turn off Lay Flat Cutter Cylinder
	}
}

/*----------------------------------------------------------------------------------------------------
 Function Name:		CmdPvSucCupCylinder
					Enguages Suc Cup Vacume cylinder
_______________________________________________________________________________________________________*/
void CmdPvSucCupCylinder(int Engage) {
	if (Engage == 1)
	{
		digitalWrite(PvSucCupCylinder, LOW);				//Turn on SucCupCylinder
	}
	else if (Engage == 0)
	{
		digitalWrite(PvSucCupCylinder, HIGH);				//Turn off SucCupCylinder
	}
}


/*----------------------------------------------------------------------------------------------------
 Function Name:		CmdCmdPvSealerLH
					Enguages Suc Cup Vacume cylinder
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
					Enguages Suc Cup Vacume cylinder
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
 Function Name:		CmdPvSealerLHTime
					Enguages Suc Cup Vacume cylinder
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
					Enguages Suc Cup Vacume cylinder
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
}

/*----------------------------------------------------------------------------------------------------
 Function Name:		MoveSteps
					Moves Stepper X Amount of steps CW or CCW
					Updates Current Position
______________________________________________________________________________________________________*/
unsigned int MoveSteps(unsigned int Stepper[5], bool Dir, unsigned int Steps,  int Speed) {
	
		if (Dir == CW) {
			digitalWrite(Stepper[1], LOW);						//Clockwise   
			digitalWrite(Stepper[2], HIGH);						//Enable Stepper Driver
			
			for (unsigned int i = 0; i < Steps; i++)			//Steps
			{
				digitalWrite(Stepper[0], HIGH);					//Pulse 
				delayMicroseconds(30);							//Pulse Delay
				digitalWrite(Stepper[0], LOW);					//Pulse
				delayMicroseconds(Speed);						//Time between steps
				Stepper[3] -= 1;								//Update Current Position
			}
			
			digitalWrite(Stepper[2], LOW);						//Disable Stepper Driver
			return Stepper[3];
		}

		else if (Dir == CCW) {
			digitalWrite(Stepper[1], HIGH);						//CounterClockwise   
			digitalWrite(Stepper[2], HIGH);						//Enable Stepper Driver
			
			for (unsigned int i = 0; i < Steps; i++)			//Steps
			{
				digitalWrite(Stepper[0], HIGH);					//Pulse 
				delayMicroseconds(30);							//Pulse Delay
				digitalWrite(Stepper[0], LOW);					//Pulse
				delayMicroseconds(Speed);						//Time between steps
				Stepper[3] += 1;								//Update Current Position
			}
			
			digitalWrite(Stepper[2], LOW);						//Disable Stepper Driver
			return Stepper[3];
		}
	
	return Stepper[3];
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
	
	while (AtHome == 1) {										// Finding Home
		AtHome = digitalRead(ProxHome);							// Checking switch untill its activated
		MoveSteps(SmSealTable, CW, 5, 600);						// Move stepper till switch is activated
	}
	
	MoveSteps(SmSealTable, CCW, 25, 600);						// Move stepper back n places
	SmSealTable[3] = 0;											// Record current position
	AtHome = 1;													// Clear AtHome Var
	
	while (AtAway == 1) {										// Finding Away
		AtAway = digitalRead(ProxAway);							// Checking switch untill its activated
		MoveSteps(SmSealTable, CCW, 5, 600);					// Move stepper till switch is activated
	}
	
	MoveSteps(SmSealTable, CW, 25, 600);						// Move stepper back n places
	SmSealTable[4] = SmSealTable[3];							// Record current position (away position)
	AtAway = 1;
	
	MoveSteps(SmSealTable, CW, 3000, 600);						// Move table to eject extrution
	MoveStepsLhRh(CW, 10000, 350);								// Move table steppers to eject layflat 

    MoveToPosition(SmSealTable, 5, 600);						// Move to home position
}




/*----------------------------------------------------------------------------------------------------
 Function Name:		MoveToPositionLhRh
					Move steppers on table to position
______________________________________________________________________________________________________*/
unsigned int MoveToPositionLhRh(unsigned int Position, int Speed) {

	unsigned int ThePosition;
	unsigned int x;
	unsigned int y;

	if (SmLayflatLH[3] == Position)								// Check if requested pos is Curr position
	{
		return Position;										// Return Position
	}

	if (Position >= 0);											// Check if requested pos to little
	{
		if (Position > SmLayflatLH[3]) {						// If move to position is grater than current position 
			x = Position - SmLayflatLH[3];						// Subtract current position from move to position
			ThePosition = MoveStepsLhRh(CCW, x, Speed);			// Move that amount of steps CCW
			return ThePosition;									// Return Position
		}
		else if (Position < SmLayflatLH[3]) {					// If move to position is grater than current position 
			y = SmLayflatLH[3] - Position;						// Subtract move to position from current position
			ThePosition = MoveStepsLhRh(CW, y, Speed);			// Move that amount of steps CW
			return ThePosition;									// Return Position	
		}

		else
			return 6666;										//Return request position error
	}
}


/*----------------------------------------------------------------------------------------------------
 Function Name:		MoveToPositionFeedLhRh
					Move steppers on table and feed stepper to position
______________________________________________________________________________________________________*/
unsigned int MoveToPositionFeedLhRh(unsigned int Position, int Speed) {
		
	unsigned int ThePosition;
	
	SmLayflatLH[3] = 0;											//Clear CurrPosiotion
	SmLayflatRH[3] = 0;											//Clear CurrPosiotion
	SmLayflatFeed[3] = 0;										//Clear CurrPosiotion
		
	ThePosition = MoveStepsFeedLhRh(CCW, Position, Speed);		// Move that amount of steps CCW
	return ThePosition;											// Return Position
}


/*----------------------------------------------------------------------------------------------------
 Function Name:		MoveStepsLhRh
					Move steppers on table n steps
______________________________________________________________________________________________________*/
unsigned int MoveStepsLhRh(bool Dir, unsigned int Steps, int Speed) {
	
		if (Dir == CW) {
			digitalWrite(SmLayflatLH[1], LOW);						//Clockwise   
			digitalWrite(SmLayflatRH[1], LOW);						//Clockwise
			digitalWrite(SmLayflatLH[2], HIGH);						//Enable Stepper Driver
			digitalWrite(SmLayflatRH[2], HIGH);						//Enable Stepper Driver
			
			for (unsigned int i = 0; i < Steps; i++)				//Steps
			{
				digitalWrite(SmLayflatLH[0], HIGH);					//Pulse SmLayflatLH
				digitalWrite(SmLayflatRH[0], HIGH);					//Pulse SmLayflatRH 
				delayMicroseconds(30);								//Pulse Delay
				digitalWrite(SmLayflatLH[0], LOW);					//Pulse SmLayflatLH
				digitalWrite(SmLayflatRH[0], LOW);					//Pulse SmLayflatRH
				delayMicroseconds(Speed);							//Time between steps
				
				SmLayflatLH[3] -= 1;								//Update Current SmLayflatLH Position
				SmLayflatRH[3] -= 1;								//Update Current SmLayflatRH Position
			}

			digitalWrite(SmLayflatLH[2], LOW);						//Disable Stepper Driver
			digitalWrite(SmLayflatRH[2], LOW);						//Disable Stepper Driver

			return SmLayflatLH[3];
		}

		else if (Dir == CCW) {
			digitalWrite(SmLayflatLH[1], HIGH);						//Clockwise   
			digitalWrite(SmLayflatRH[1], HIGH);						//Clockwise
			digitalWrite(SmLayflatLH[2], HIGH);						//Enable Stepper Driver
			digitalWrite(SmLayflatRH[2], HIGH);						//Enable Stepper Driver

			for (unsigned int i = 0; i < Steps; i++)				//Steps
			{
				digitalWrite(SmLayflatLH[0], HIGH);					//Pulse SmLayflatLH
				digitalWrite(SmLayflatRH[0], HIGH);					//Pulse SmLayflatRH
				delayMicroseconds(30);								//Pulse Delay
				digitalWrite(SmLayflatLH[0], LOW);					//Pulse SmLayflatLH
				digitalWrite(SmLayflatRH[0], LOW);					//Pulse SmLayflatRH
				delayMicroseconds(Speed);							//Time between steps
				
				SmLayflatLH[3] += 1;								//Update Current Position
				SmLayflatRH[3] += 1;								//Update Current Position
			}

			digitalWrite(SmLayflatLH[2], LOW);						//Disable Stepper Driver
			digitalWrite(SmLayflatRH[2], LOW);						//Disable Stepper Driver
			return SmLayflatLH[3];
		}

	return SmLayflatLH[3];
}


/*----------------------------------------------------------------------------------------------------
 Function Name:		MoveStepsFeedLhRh
					Move steppers on table and feed stepper n steps
______________________________________________________________________________________________________*/
unsigned int MoveStepsFeedLhRh(bool Dir, unsigned int Steps, int Speed) {

	if (Dir == CW) {
		digitalWrite(SmLayflatLH[1], LOW);						//Clockwise   
		digitalWrite(SmLayflatRH[1], LOW);						//Clockwise
		digitalWrite(SmLayflatFeed[1], LOW);					//Clockwise
		digitalWrite(SmLayflatLH[2], HIGH);						//Enable Stepper Driver
		digitalWrite(SmLayflatRH[2], HIGH);						//Enable Stepper Driver
		digitalWrite(SmLayflatFeed[2], HIGH);					//Enable Stepper Driver
		
		for (unsigned int i = 0; i < Steps; i++)				//Steps
		{
			digitalWrite(SmLayflatLH[0], HIGH);					//Pulse SmLayflatLH
			digitalWrite(SmLayflatRH[0], HIGH);					//Pulse SmLayflatRH 
			digitalWrite(SmLayflatFeed[0], HIGH);				//Pulse SmLayflatFeed 
			delayMicroseconds(50);								//Pulse Delay
			digitalWrite(SmLayflatLH[0], LOW);					//Pulse SmLayflatLH
			digitalWrite(SmLayflatRH[0], LOW);					//Pulse SmLayflatRH
			digitalWrite(SmLayflatFeed[0], LOW);				//Pulse SmLayflatFeed 
			delayMicroseconds(Speed);							//Time between steps

			SmLayflatLH[3] -= 1;								//Update Current SmLayflatLH Position
			SmLayflatRH[3] -= 1;								//Update Current SmLayflatRH Position
			SmLayflatFeed[3] -= 1;								//Update Current SmLayflatRH Position
		}

		digitalWrite(SmLayflatLH[2], LOW);						//Disable Stepper Driver
		digitalWrite(SmLayflatRH[2], LOW);						//Disable Stepper Driver
		digitalWrite(SmLayflatFeed[2], LOW);					//Disable Stepper Driver

		return SmLayflatLH[3];
	}

	else if (Dir == CCW) {
		digitalWrite(SmLayflatLH[1], HIGH);						//Clockwise   
		digitalWrite(SmLayflatRH[1], HIGH);						//Clockwise
		digitalWrite(SmLayflatFeed[1], HIGH);					//Clockwise
		digitalWrite(SmLayflatLH[2], HIGH);						//Enable Stepper Driver
		digitalWrite(SmLayflatRH[2], HIGH);						//Enable Stepper Driver
		digitalWrite(SmLayflatFeed[2], HIGH);					//Enable Stepper Driver

		for (unsigned int i = 0; i < Steps; i++)				//Steps
		{
			digitalWrite(SmLayflatLH[0], HIGH);					//Pulse SmLayflatLH
			digitalWrite(SmLayflatRH[0], HIGH);					//Pulse SmLayflatRH
			digitalWrite(SmLayflatFeed[0], HIGH);				//Pulse SmLayflatFeed
			delayMicroseconds(50);								//Pulse Delay
			digitalWrite(SmLayflatLH[0], LOW);					//Pulse SmLayflatLH
			digitalWrite(SmLayflatRH[0], LOW);					//Pulse SmLayflatRH
			digitalWrite(SmLayflatFeed[0], LOW);				//Pulse SmLayflatFeed
			delayMicroseconds(Speed);							//Time between steps

			SmLayflatLH[3] += 1;								//Update Current Position
			SmLayflatRH[3] += 1;								//Update Current Position
			SmLayflatFeed[3] += 1;								//Update Current Position
		}

		digitalWrite(SmLayflatLH[2], LOW);						//Disable Stepper Driver
		digitalWrite(SmLayflatRH[2], LOW);						//Disable Stepper Driver
		digitalWrite(SmLayflatFeed[2], LOW);					//Disable Stepper Driver
		return SmLayflatLH[3];
	}

	return SmLayflatLH[3];

}
