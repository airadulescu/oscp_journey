@ECHO OFF
:: This batch add suer to administrator and enable RDP
TITLE Add user
ECHO Adding user...
net user kali password /add
net localgroup Administrators kali /add
net localgroup "Remote Desktop Users" kali /add
ECHO Enabling RDP...
reg add "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Terminal Server" /v fDenyTSConnections /t REG_DWORD /d 0 /f
ECHO =============
ECHO User Added
ECHO =============
net users
