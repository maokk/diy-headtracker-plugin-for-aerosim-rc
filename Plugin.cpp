//-----------------------------------------------------------------------------
//  PluginDLL.cpp
//-----------------------------------------------------------------------------
//
//  Plugin for DIY HeadTracker from rcgroups
//  http://www.rcgroups.com/forums/showthread.php?t=1677559
//  (c) Andrey Prikupets, 2014
//-----------------------------------------------------------------------------
#include "Plugin.h"
#include "Serial.h"

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <stdio.h>
#include <stdlib.h>
#include "windows.h"
#include <direct.h>
#include <math.h>

// Custom Menu Item masks
#define MASK_MENU_ITEM__CHECKBOX_DIYHT_CONTROL_PITCH (1 << 0)
#define MASK_MENU_ITEM__CHECKBOX_DIYHT_CONTROL_ROLL  (1 << 1)
#define MASK_MENU_ITEM__CHECKBOX_DIYHT_CONTROL_PAN   (1 << 2)

const int MAX_CHARS = 80;

class HeadTrackerData {
public:
	float pitch;
	float roll;
	float pan;
	float txPitch;
	float txRoll;
	float txPan;
	BOOL controlPitch;
	BOOL controlRoll;
	BOOL controlPan;
	char buf[MAX_CHARS];
	int pos;

	CSerial port;

	DWORD resetMs;
	int retries;

	BOOL error;

	HeadTrackerData() {
		pitch = roll = pan = txPitch = txRoll = txPan = 0;
		pos = 0;
		error = FALSE;
		resetMs = 0;
		retries = 0;
	}
};

//-----------------------------------
//-----------------------------------
bool g_bFirstRun = true;          // Used to initialise menu checkboxes showing OSD with HUD

char g_strPortName[MAX_PATH] = "";
int g_intPortRate = 57600;
char g_strPluginFolder[MAX_PATH];

HeadTrackerData g_data;

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved)
{
  switch (ul_reason_for_call)
  {
 	  case DLL_PROCESS_ATTACH:
		  break;
	  case DLL_THREAD_ATTACH:
		  break;
	  case DLL_THREAD_DETACH:
		  break;
	  case DLL_PROCESS_DETACH:
      // Free here any resources used by the plugin
		  break;
  }
  return TRUE;
}

//-----------------------------------------------------------------------------
//
// This function is called once, just after the dll is loaded
//  - to initialise the top bar menu with your custom menu items
//  - to initialise any other resources (e.g. character map for OSD)
//
//-----------------------------------------------------------------------------
AeroSIMRC_DLL_EXPORT void AeroSIMRC_Plugin_Init(TPluginInit *ptPluginInit)
{
  //--------------------------------------------
  // Save paths for later use
  //--------------------------------------------
  strcpy(g_strPluginFolder, ptPluginInit->strPluginFolder);

  char strFullFileName[MAX_PATH];
  strcpy(strFullFileName, g_strPluginFolder);
  strcat(strFullFileName, "\\DIYHT.TXT");

  FILE *pFile = fopen(strFullFileName, "rb");
  if(pFile)
  {
	  fgets (g_strPortName, sizeof g_strPortName, pFile);
	  g_strPortName[strcspn(g_strPortName, "\n\r")] = '\0';

	  char strPortRate[MAX_PATH];
	  fgets (strPortRate, sizeof strPortRate, pFile);
	  g_intPortRate = atoi(strPortRate);

	  fclose(pFile);
  }
}

//-----------------------------------------------------------------------------


float map(float x, float xMin, float xMax, float normFactor) {
	if (x < xMin) 
		x = xMin;
	if (x > xMax) 
		x = xMax;
	x -= (xMax+xMin)/2.0; // Center value;
	return x*normFactor;
}

//-----------------------------------------------------------------------------

void control(const TDataFromAeroSimRC  *ptDataFromAeroSimRC, TDataToAeroSimRC *ptDataToAeroSimRC)
{
  #define CH_b0_FPV_CAM_PAN                11
  #define CH_b0_FPV_CAM_TILT               12
  #define CH_b0_FPV_CAM_ROLL               26

  if (!(g_data.controlPitch || g_data.controlRoll || g_data.controlPan)) {
    if (g_data.port.IsOpened()) {
      g_data.port.SendData("$PLEN\n");
      g_data.port.Close();
    }
    g_data.error = FALSE;
    return;
  }

  if (g_data.error)
	return;

  if (!g_data.port.IsOpened()) {
	if (!g_data.port.Open(g_strPortName, g_intPortRate)) {
		g_data.error = TRUE;
		return;
	}
  }

  // Check: buf='ABC'; pos=3;
  int bytesRead = g_data.port.ReadData(&g_data.buf[g_data.pos], MAX_CHARS-g_data.pos); // Check: buf='ABCD\nF'; bytesRead=3;
  if (bytesRead > 0) {
  	int oldPos = g_data.pos; // Check: oldPos=3;
  	g_data.pos += bytesRead; // position right after last read character; // Check: pos=6;
  	for(int i=oldPos; i<g_data.pos; i++) { // Check: i=3; i<6;
  		if (g_data.buf[i] == '\n') { // Check: buf[4] = '\n';
  			g_data.buf[i] = 0; // Check: buf[4] = 0;
  			// Parse text from 0 till the 0 character;
  			// printf("Parsing: %s\n", &g_data.buf);
			float pitch;
			float roll;
			float pan;
  			int found = sscanf(g_data.buf, "%f,%f,%f", &pitch, &roll, &pan);
  			if (found == 3) {
				g_data.pitch = pitch;
				g_data.roll = roll;
				g_data.pan = pan;
//--------------------
//     LEFT	MID	RIGHT
//Pan	90	180	270
//      -1  0   +1
//--------------------
//	    UP MID DOWN
//Pitch	0  90   180
//      1   0   -1
//--------------------
//	    LEFT MID RIGHT
//Roll	180	 90	 0		
//      -1    0  +1
//--------------------
				// tx values should be in [-1, 0, +1];
				g_data.txPitch = map(pitch, 0, 180, -1.0/90.0); // inverse;
				g_data.txRoll = map(roll, 0, 180, -1.0/90.0); // inverse;
				g_data.txPan = map(pan, 90, 270, 1.0/90.0);
///  				printf("pitch: %.2f, roll: %.2f, pan: %.2f\n", g_data.pitch, g_data.roll, g_data.pan);
  			} else {
///  				printf("Invalid data, found=%d\n", found);
  			}
  			// Delete parsed part from buffer;
  			g_data.pos -= (i+1); // Check: pos=1;
  			memmove(&g_data.buf[0], &g_data.buf[i+1], g_data.pos); // Check: '5', 1;
  			// Reset to 0 the read position;
  			i = 0;
  		}
  	}
  	if (g_data.pos >= MAX_CHARS) {
		// Discard the buffer;
///		printf("Buffer discarded: %s\n", g_data.buf);
		g_data.pos = 0;
  	}
  	g_data.resetMs = GetTickCount();
  } else {
  	DWORD currentMs = GetTickCount();
  	if (abs((long)(g_data.resetMs-currentMs)) > 1000) {
		g_data.retries++;
  		g_data.resetMs = currentMs;
  		g_data.port.SendData("$PLST\n");
///  		printf("Reset\n");
  	}
  }

  // Set new channel position to the returned struct
  // Tell the simulator to override this channel with our new value, so the model takes our input
  if (g_data.controlPan) {
    ptDataToAeroSimRC->Channel_abOverride_RX[CH_b0_FPV_CAM_PAN ] = true;
    ptDataToAeroSimRC->Channel_afNewValue_RX[CH_b0_FPV_CAM_PAN ] = g_data.txPan;
  }

  if (g_data.controlPitch) {
    ptDataToAeroSimRC->Channel_abOverride_RX[CH_b0_FPV_CAM_TILT] = true;
    ptDataToAeroSimRC->Channel_afNewValue_RX[CH_b0_FPV_CAM_TILT] = g_data.txPitch;
  }

  if (g_data.controlRoll) {
    ptDataToAeroSimRC->Channel_abOverride_RX[CH_b0_FPV_CAM_ROLL] = true;
    ptDataToAeroSimRC->Channel_afNewValue_RX[CH_b0_FPV_CAM_ROLL] = g_data.txRoll;
  }
}

//-----------------------------------------------------------------------------
// The string returned in pucOnScreenDebugInfoText will be displayed in AeroSIMRC
// In this example, the all data received from AeroSIMRC is displayed on the screen
//-----------------------------------------------------------------------------
void InfoText(const TDataFromAeroSimRC *ptDataFromAeroSimRC, TDataToAeroSimRC *ptDataToAeroSimRC)
{
  char strDebugInfo[10000];  

  if (g_data.error) {
    sprintf(strDebugInfo,
      "--- HeadTracker -------------------------------------------------------------\n"
      "\n"
      "Error opening Port %s at rate %d",
      g_strPortName, g_intPortRate);
  } else {
    sprintf(strDebugInfo,
      "--- HeadTracker -------------------------------------------------------------\n"
      "\n"
      "Port %s %s, %d retries\n"
      "\n"
	  "Pitch   = %6.2f   Roll   = %6.2f   Pan   = %6.2f\n"
      "txPitch = %6.2f   txRoll = %6.2f   txPan = %6.2f\n", 
      g_strPortName, (g_data.port.IsOpened() ? "OPENED" : "CLOSED"), g_data.retries,
      g_data.pitch, g_data.roll, g_data.pan,
      g_data.txPitch, g_data.txRoll, g_data.txPan);
  }

  ptDataToAeroSimRC->Debug_pucOnScreenInfoText = strDebugInfo;
}



//-----------------------------------------------------------------------------
// This function is called at each program cycle from AeroSIM RC
//-----------------------------------------------------------------------------
AeroSIMRC_DLL_EXPORT void AeroSIMRC_Plugin_Run(const TDataFromAeroSimRC  *ptDataFromAeroSimRC,
                                                     TDataToAeroSimRC    *ptDataToAeroSimRC)
{
  //-----------------------------------
  // By default do not change the Menu Items of type CheckBox
  //-----------------------------------
  ptDataToAeroSimRC->Menu_nFlags_MenuItem_New_CheckBox_Status = ptDataFromAeroSimRC->Menu_nFlags_MenuItem_Status;

  //-----------------------------------
  // Tick checkboxe to Enable DIYHT at the start;
  //-----------------------------------
  if(g_bFirstRun)
  {
    ptDataToAeroSimRC->Menu_nFlags_MenuItem_New_CheckBox_Status = MASK_MENU_ITEM__CHECKBOX_DIYHT_CONTROL_PITCH;
    g_bFirstRun = false;
  }

  g_data.controlPitch = (ptDataFromAeroSimRC->Menu_nFlags_MenuItem_Status & MASK_MENU_ITEM__CHECKBOX_DIYHT_CONTROL_PITCH);
  g_data.controlRoll = (ptDataFromAeroSimRC->Menu_nFlags_MenuItem_Status & MASK_MENU_ITEM__CHECKBOX_DIYHT_CONTROL_ROLL);
  g_data.controlPan = (ptDataFromAeroSimRC->Menu_nFlags_MenuItem_Status & MASK_MENU_ITEM__CHECKBOX_DIYHT_CONTROL_PAN);

  //-----------------------------------
  // Extract Menu Commands from Flags
  //-----------------------------------
  control(ptDataFromAeroSimRC, ptDataToAeroSimRC);

  //-----------------------------------
  // debug info is shown on the screen
  //-----------------------------------
  InfoText(ptDataFromAeroSimRC, ptDataToAeroSimRC);
}
