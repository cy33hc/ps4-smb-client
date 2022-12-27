# PS4 SMB Client

Simple SMB client for the PS4. Allows you to transfer files between the PS4 and your Windows Shares, Linux SMB Shares and NAS SMB shares

![Preview](/screenshot.jpg)

## Installation
Copy the **ps4_smb_client.pkg** in to a FAT32 format usb drive then install from package installer

## Controls
```
Triangle - Menu (after a file(s)/folder(s) is selected)
Cross - Select Button/TextBox
Circle - Un-Select the file list to navigate to other widgets
Square - Mark file(s)/folder(s) for Delete/Rename/Upload/Download
R1 - Navigate to the Remote list of files
L1 - Navigate to the Local list of files
TouchPad Button - Exit Application
```

## Remote Package Installer Feature
Remote Package Installation is supported if you setup SMB and a HTTP Server on the same machine. You also need to make sure the SMB and HTTP Server share the exact same folder.

**HELP - Need new translations for the added strings. Please help update the languages files. Use the [English.ini](https://github.com/cy33hc/ps4-smb-client/blob/master/data/assets/langs/English.ini) for reference**

**No need for "Remote Package Installer" homebrew and "Remote Package Sender" from PC.**

Example: If you share the folder C:\MyShare for SMB, then also make sure the C:\MyShare is the root folder for the HTTP Server.

**HTTP Servers known to work with RPI.**

[Leefull's exploit host](https://github.com/Leeful/leeful.github.io/blob/master/Exploit%20Host%20Server%20v1.2.exe) Place the "Exploit Host Server v1.2.exe" into the SMB share folder and execute.

Microsoft IIS

[npx serve](https://www.npmjs.com/package/serve)

## Known Issues
- Rename of files and folders in subdirectories fails for Windows Shares. Works with SMB shares.

## Multi Language Support
The appplication support following languages

The following languages are auto detected.
```
Dutch
English
French
German
Italiano
Japanese
Korean
Polish
Portuguese_BR
Russian
Spanish
Simplified Chinese
Traditional Chinese
```

The following aren't standard languages supported by the PS4, therefore requires a config file update.
```
Catalan
Croatian
Euskera
Galego
Greek
Hungarian
Indonesian
Ryukyuan
Thai
```
User must modify the file **/data/ps4-smb-client/config.ini** located in the ps4 hard drive and update the **language** setting to with the **exact** values from the list above.

**HELP:** There are no language translations for the following languages, therefore not support yet. Please help expand the list by submitting translation for the following languages. If you would like to help, please download this [Template](https://github.com/cy33hc/ps4-smb-client/blob/master/data/assets/langs/English.ini), make your changes and submit an issue with the file attached.
```
Finnish
Swedish
Danish
Norwegian
Turkish
Arabic
Czech
Romanian
Vietnamese
```
or any other language that you have a traslation for.

## Building
Before build the app, you need to build the dependencies first.
Clone the following Git repos and build them in order

openssl - https://github.com/cy33hc/ps4-openssl/tree/OpenSSL_1_1_1-ps4

libsmb2 - https://github.com/cy33hc/libsmb2/tree/ps4

```
1. Download the pacbrew-pacman from following location and install.
   https://github.com/PacBrew/pacbrew-pacman/releases
2. Run following cmds
   pacbrew-pacman -Sy
   pacbrew-pacman -S ps4-openorbis ps4-openorbis-portlibs
   chmod guo+x /opt/pacbrew/ps4/openorbis/ps4vars.sh
   source /opt/pacbrew/ps4/openorbis/ps4vars.sh
   mkdir build; cd build
   openorbis-cmake ..
   make
```

## Credits
The color theme was borrowed from NX-Shell on the switch.
