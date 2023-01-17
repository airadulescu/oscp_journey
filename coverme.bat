@ECHO OFF
:: This batch file add user to Administrator and enables RDP
TITLE Add User
ECHO Adding User...
net user coverme password /add
net localgroup Administrators coverme /add
net localgroup "Remote Desktop Users" coverme /add
ECHO Enabling RDP...
reg add "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Terminal Server" /v fDenyTSConnections /t REG_DWORD /d 0 /f
ECHO ===================
ECHO User Added
ECHO ===================
net users
