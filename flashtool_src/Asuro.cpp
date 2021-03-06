/***************************************************************************
             Asuro.cpp  -  Main Class for ASURO Flash Tools
                             -------------------
    begin                : Die Aug 12 10:16:57 CEST 2003
    copyright            : (C) 2003-2004 DLR RM by Jan Grewe
    email                : jan.grewe@gmx.de
 ***************************************************************************/

/***************************************************************************

 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   any later version.                                                    *
 ***************************************************************************/

/***************************************************************************
	Ver      date       Author        comment
  -------------------------------------------------------------------------
	1.0   12.08.2003    Jan Grewe     build
	1.1	  11.12.2003    Jan Grewe     fixed error m_endPage
	1.2   27.08.2004    Jan Grewe     Port Scan included
 ***************************************************************************/ 
#include "Asuro.h"

#include <string.h>
#include <time.h>

#ifdef LINUX
#include "PosixSerial.h"
CPosixSerial Serial;
#elif defined  WINDOWS
#include "WinSerial.h"
CWinSerial Serial;
#else
#error Wrong OS (Linux or Windows) 
#endif

CAsuro::CAsuro()
{
	memset(m_ASUROfileName,0x00,sizeof(m_ASUROfileName));
	memset(m_ASUROCOMPort,0x00,sizeof(m_ASUROCOMPort));
	m_ASUROCancel = false;
	m_MaxTry = 10;
	m_TimeoutConnect = 10;
	m_TimeoutFlash = 1;
}


bool CAsuro::InitCAsuro(void)
{
	FILE* fp = NULL;
	char line[sizeof(m_ASUROfileName)],lineCount = 0;

	if (m_ASUROIniPath[0] != '\0') fp = fopen(m_ASUROIniPath,"r");
	if (fp != NULL) {

		rewind(fp);
		while (readLine(line,fp) != EOF) {
			switch (lineCount) {
				case 0: memcpy(m_ASUROfileName,line,strlen(line) -1); break;
				case 1: memcpy(m_ASUROCOMPort,line,strlen(line) -1); break;
				case 2: sscanf(line,"%d",(int *)&m_TimeoutConnect); break;
				case 3: sscanf(line,"%d",(int *)&m_TimeoutFlash); break;
				case 4: sscanf(line,"%d",(int *)&m_MaxTry); break; 
			}
			lineCount ++;
		}
		fclose(fp);
		return true;
	}
	return false;
}

CAsuro::~CAsuro()
{
	FILE* fp = NULL;
	if (m_ASUROIniPath[0] != '\0') fp = fopen(m_ASUROIniPath,"w");
	if (fp != NULL) {
		if (m_ASUROfileName == "") fprintf(fp,"Hex Filename \n");
		else fprintf(fp,"%s\n",m_ASUROfileName);
 		if (m_ASUROCOMPort == "") fprintf(fp,"Interface \n");
		else fprintf(fp,"%s\n",m_ASUROCOMPort);
		fprintf(fp,"%d \t\t#Timeout Connect\n",m_TimeoutConnect);
		fprintf(fp,"%d \t\t#Timeout Flash ('t')\n",m_TimeoutFlash);
		fprintf(fp,"%d \t\t#MaxTry for flashing\n",m_MaxTry);
		fclose(fp);
	}
}

bool CAsuro::Init()
{
	if (!Serial.Open(m_ASUROCOMPort)) {
		ErrorText((char*)"Failed !");
		return false;
	}
	Serial.Timeout(0);
	SuccessText((char*)"OK !");
	return true;
}

bool CAsuro::PortScan(char* portName, unsigned short number, unsigned short mode)
{
  return Serial.Scan(portName,number,mode);
}

bool CAsuro::Connect()
{
	char buf[11] = {0,0,0,0,0,0,0,0,0,'\0'};
	Serial.ClearBuffer();
	time_t t1,t2;

	time(&t1);
	time(&t2);
	MessageText((char*)"Connect to ASURO --> ");
	while (difftime(t2,t1) < m_TimeoutConnect) {
		time(&t2);
		if (m_ASUROCancel) {
			ErrorText((char*)"Cancel !");	
			return false;
		}
		Progress((unsigned int)(difftime(t2,t1) * 100.0) / m_TimeoutConnect);
		Serial.Write((char*)"Flash",5);
		Serial.Read(buf,10);
		TimeWait(100);
		ViewUpdate();
		if (strstr(buf,"ASURO")) {
			SuccessText((char*)"OK !");
			return true;
		}
	}
	Progress(100);
	ErrorText((char*)"Timeout !");
	return false;
}

bool CAsuro::BuildRAM()
{
	FILE *fp = NULL;
	unsigned int address = 0, type = 0, data = 0, cksum = 0, cksumFile = 0, recordLength = 0, i;
	char tmp[256],line[1024];
	m_startPage = END_PAGE;
	m_endPage = START_PAGE;
	
	MessageText((char*)"Bulding  RAM --> ");
	if (m_ASUROfileName[0] != '\0') fp = fopen(m_ASUROfileName,"r");

	if (fp != NULL) {
		rewind(fp);
		while (readLine(line,fp) != EOF) {
			if (line[0] != ':') {
				fclose(fp);
				ErrorText((char*)"Wrong file format !");
				return false;
			}
			cksum = 0;
			sscanf(&line[1],"%02X",&recordLength);
			sscanf(&line[3],"%04X",&address);
			sscanf(&line[7],"%02X",&type);
			// get start and end pages
			if ((unsigned int) ((address / (PAGE_SIZE - 2)) +0.5) < m_startPage)
				m_startPage = (unsigned int) ((address / (PAGE_SIZE - 3 )) +0.5);
			if ((unsigned int) ((address / (PAGE_SIZE - 2)) +0.5) > m_endPage)
				m_endPage = (unsigned int) ((address / (PAGE_SIZE - 3 )) +0.5);
			cksum = recordLength +(unsigned char) ((address & 0xFF00) >> 8)
							+ (unsigned char) (address & 0x00FF) + type;
			
			if (type == 0x00) { // data Record
				int header = HEX_HEADER;
#ifdef WINDOWS 
				// Windows \n\r = 1 char as EOL
				if (line[strlen(line)-1] == 0x0a) header ++;
#else
				// Unix \n = 1 char as EOL reading Windows file with \n\r as EOL is 2 char
				if (line[strlen(line)-1] == 0x0a) header += 2;
#endif
				if (strlen(line) != recordLength * 2 + header) { 
					ErrorText((char*)"HEX file line length ERROR !");
					return false;
				} 

				for ( i = 0; i < recordLength; i++) {
					sscanf(&line[9 + i*2],"%02X",&data);
					cksum += data;
					tmp[i] = data;
				}
				cksum = ((~cksum & 0xFF) + 1) & 0xFF;
				sscanf(&line[9 + i*2],"%02X",&cksumFile);
				if (cksum != cksumFile) {
					fclose(fp);
					ErrorText((char*)"HEX file chksum ERROR !");
					return false;
				}

				if (address + recordLength > (MAX_PAGE * PAGE_SIZE) - BOOT_SIZE) {
					fclose(fp);
					ErrorText((char*)"Hex file too large or address error !!");
					return false;
				}
				memcpy(&m_RAM[0][0]+address,&tmp[0],recordLength);
			}
			if (type == 0x01) { // End of File Record
				SuccessText((char*)"OK !");
				fclose(fp);
				return true;
			}
		}
	}
	sprintf(line,"%s does not exist !",m_ASUROfileName);
	ErrorText(line);
	return false;
}

bool CAsuro::SendPage()
{
	unsigned int crc,i,j;
	char sendData[PAGE_SIZE]; 
	char getData[3],tmpText[256];
	time_t t1,t2;
	
	m_endPage++; // fixed 11.12.2003 
	Serial.Timeout(100);
	for (i = m_startPage; i <= m_endPage + 1; i++) {
		sendData[0] = i; // PageNo.
		crc = 0;
		memcpy(&sendData[1],&m_RAM[i][0],PAGE_SIZE - 3);
		//Build CRC16
		for (j = 0; j < PAGE_SIZE - 2; j++) // -2 CRC16
			crc = CRC16(crc,sendData[j]);
		memcpy(&sendData[j],&crc,2);
		// Last page was send
		if (i == m_endPage + 1) 
			memset(sendData,0xFF,PAGE_SIZE);
		else {
			sprintf(tmpText,"Sending Page %03d of %03d --> ",i,m_endPage);
			MessageText(tmpText);
		}

		// Try MAX_TRY times before giving up sendig data
		for (j = 0; j < m_MaxTry; j ++) {
			memset(getData,0x00,3);
			Serial.Write(sendData,PAGE_SIZE);
			Serial.ClearBuffer();
			if ( i == m_endPage + 1) {
				MessageText((char*)""); // just for \n
				SuccessText((char*)"All Pages flashed !!");
				MessageText((char*)""); // just for \n
				SuccessText((char*)"ASURO ready to start !!");
				Progress(100);
				return true;
			}
			time(&t1);
			time(&t2);
			do {
				time(&t2);
				if (m_ASUROCancel == true) {
					ErrorText((char*)"Cancel !");
					return false;
				}
        Serial.Read(getData,2);
				Serial.ClearBuffer();
				ViewUpdate();
			} while ((strcmp(getData,"CK") != 0) &&
					 (strcmp(getData,"OK") != 0) &&
					 (strcmp(getData,"ER") != 0) &&
					 difftime(t2,t1) <= m_TimeoutFlash);
			Progress(i*100/(m_endPage - m_startPage + 1));
#ifdef LINUX
      TimeWait(350);
#endif
      if (getData[0] == 'O' && getData[1] == 'K') {
				SuccessText((char*)" flashed !");
				break; // Page sended succssesfull
			}
			if (i <= m_endPage) {
				if (getData[0] == 'C' && getData[1] == 'K') WarningText((char*)"c"); 
				else if (getData[0] == 'E' && getData[1] == 'R') WarningText((char*)"v");
				else WarningText((char*)"t");  
			}
		}

		if (j == m_MaxTry) {
			MessageText((char*)""); // just for \n
			ErrorText((char*)"TIMEOUT !");	
			MessageText((char*)""); // just for \n
			ErrorText((char*)" ASURO dead --> FLASH damaged !!");
			return false;
		}
	}
	return false;
}

void CAsuro::Programm()
{
char tmp[255];
	m_ASUROCancel = false;
	Progress(0);
	sprintf(tmp,"Try to initialise %s ",m_ASUROCOMPort);
	Status(tmp);
	sprintf(tmp,"Open %s --> ",m_ASUROCOMPort);  
	MessageText(tmp);
	ViewUpdate();
	if (Init()) {
		Status((char*)"Building  RAM !" );
		ViewUpdate();
		if (BuildRAM()) {
			Status((char*)"Try to connect ASURO !" );
			ViewUpdate();
			Progress(0);
			if (Connect()) {
				Status((char*)"Flashing Firmware !" );
				ViewUpdate();
				if (SendPage())	Status((char*)"ASURO ready to start !" );
				else Status((char*)"ASURO dead ?!?! (Firmware damaged try again !)" );
			}
			else Status((char*)"Connect to ASURO failed !");
		}
		else Status((char*)"Building  RAM failed !" );
	}
	else {
    sprintf(tmp,"Can't initialise %s !",m_ASUROCOMPort);
    Status(tmp);
  }
	Serial.Close();
}


unsigned int CAsuro::CRC16(unsigned int crc, unsigned char data)
{
const unsigned int CRCtbl[ 256 ] = {                                 
   0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,   
   0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,   
   0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,   
   0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,   
   0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,   
   0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,   
   0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,   
   0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,   
   0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,   
   0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,   
   0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,   
   0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,   
   0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,   
   0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,   
   0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,   
   0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,   
   0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,   
   0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,   
   0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,   
   0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,   
   0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,   
   0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,   
   0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,   
   0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,   
   0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,   
   0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,   
   0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,   
   0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,   
   0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,   
   0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,   
   0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,   
   0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040 }; 

   return ( crc >> 8 ) ^ CRCtbl[ ( crc & 0xFF ) ^ data];     
}

char CAsuro::readLine(char* line, FILE *fp)
{
	char c;
	unsigned int i = 0;
	do {
		c = fgetc(fp);
		if (c == EOF) 
			return EOF;
		line[i++] = c;
	} while (c != '\n');
	line[i] = '\0';
	return true;
}
