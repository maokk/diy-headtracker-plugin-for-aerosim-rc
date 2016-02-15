DIY Headtracker: http://www.rcgroups.com/forums/showthread.php?t=1677559

AeroSIM-RC: http://www.aerosimrc.com/chm/en/web_help.htm

**How to install**

  1. In the folder with AeroSIM-RC plugins create "DIYHT" subfolder, i.e.: "C:\Program Files (x86)\AeroSIM-RC\Plugin\DIYHT".
  1. Copy "bin\plugin\_AeroSIMRC.dll", "deploy\plugin.txt" and "deploy\DIYHT.txt" into "DIYHT" subfolder.
  1. Connect your DIYHT Headtracker via USB port and find out, which COM port it got (via My Computer - Manage - Ports).
  1. Modify DIYHT.txt file: write port name in the first line (i.e COM3). If baud rate of your DIYHT is different from standard 57600, enter in at second line of the file.

**How to run**

  1. Run AeroSIM-RC;
  1. Open "Plugin" menu;
  1. Click to "DIYHT - DIY Headtracker plugin";
  1. Menu "DIYHT - DIY Headtracker plugin" will appear. You can control, which FPV camera pan/tilt/roll axis are controlled by DIYHT;
  1. Switch to FPV camera (F4);
  1. Optionally reset your DIY Headtracker by pressing a button;
  1. Fly and look around!

**Note**

  1. To show raw values, FPV camera signals and DIYHT connection status please enable "Show debug info" command in "Plugin" menu. It will output over the screen.
  1. If your DIYHT is reversed by some axis, use its GUI utility to change reverse.