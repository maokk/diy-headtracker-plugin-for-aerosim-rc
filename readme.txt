How to install:

1. In the folder with AeroSIM-RC plugins create "DIYHT" subfolder, i.e.: 

   "C:\Program Files (x86)\AeroSIM-RC\Plugin\DIYHT"

2. Copy "bin\plugin_AeroSIMRC.dll", "deploy\plugin.txt" and "deploy\DIYHT.txt" into "DIYHT" subfolder.

3. Connect your DIYHT Headtracker via USB port and find out, which COM port it got (via My Computer - Manage - Ports).

4. Modify DIYHT.txt file: write port name in the first line (i.e COM3). If baud rate of your DIYHT is different from standard 57600, enter in at second line of the file.

How to run:

1. Run AeroSIM-RC;
2. Open "Plugin" menu;
3. Click to "DIYHT - DIY Headtracker plugin";
4. Menu "DIYHT - DIY Headtracker plugin" will appear. You can control, which FPV camera axis are controlled by DIYHT;
5. Switch to FPV camera (F4);
6. Optionally reset you DIY Headtracker by pressing a button;
7. Fly!

Notes:

1. To show raw values, FPV camera signals and DIYHT connection status please enable "Show debug info" command in "Plugin" menu. It will output over the screen.
2. If your DIYHT is reversed by some axis, use its GUI utility to change reverse. 