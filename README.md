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
Hungarian
Indonesian
Ryukyuan
```
User must modify the file **/data/ps4-ftp-client/config.ini** located in the ps4 hard drive and update the **language** setting to with the **exact** values from the list above.

**HELP:** There are no language translations for the following languages, therefore not support yet. Please help expand the list by submitting translation for the following languages. If you would like to help, please download this [Template](https://github.com/cy33hc/ps4-ftp-client/blob/master/data/assets/langs/English.ini), make your changes and submit an issue with the file attached.
```
Greek
Finnish
Swedish
Danish
Norwegian
Turkish
Arabic
Czech
Romanian
Thai
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
