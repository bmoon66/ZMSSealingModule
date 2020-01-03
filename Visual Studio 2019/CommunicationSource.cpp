//TODO: just checking

/*----------------------------------------------------------------------------------------------------
 Function Name:		RecvCmdData
					Checks for opening char gets CMD Data Data2 and checksum from PC
					Clears the buffer and converts data to int then returns the data
					No data returns false
_______________________________________________________________________________________________________*/
bool RecvCmdData() {											// Recieve data 

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
					Returns CMD Data Data2 and Checksum to PC
					Updates previous CMD Data Data2
					Clears current CMD Data Data2
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