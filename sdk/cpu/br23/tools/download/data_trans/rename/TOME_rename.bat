
set ictype=695x
set icflag=6951C8
set sdkver=V221
set customer="BL"
set project="HEXZS"
set Notes="BLE"
set fwType="烧录文件"
set bfuType="升级文件"
set Engineer="loz"



SET filename1=%customer%-%project%-%Notes%-%icflag%-%sdkver%-%fwType%-%Engineer%
SET filename2=%customer%-%project%-%Notes%-%icflag%-%sdkver%-%bfuType%-%Engineer%
SET shaolu_file=NULL
SET update_file=NULL

if %ictype%==692X (
SET shaolu_file=jl_isd.fw
SET update_file=updata.bfu

)
else if %ictype%==695X (
SET shaolu_file=jl_isd.fw
SET update_file=update.ufw

)
else if %ictype%==690X (
SET shaolu_file=jl_690x.bin
SET update_file=jl_690x.bfu

)
else if %ictype%==696X (
SET shaolu_file=jl_isd.fw
SET update_file=update.ufw

)
else if %ictype%==697X (
SET shaolu_file=jl_isd.fw
SET update_file=update.ufw

)
else if %ictype%==700X (
SET shaolu_file=jl_isd.fw
SET update_file=update.ufw

)
else if %ictype%==698X (
SET shaolu_file=jl_isd.fw
SET update_file=update.ufw

)

else if %ictype%==632X (
SET shaolu_file=jl_isd.fw
SET update_file=update.ufw

)

set date_str=%date:~,4%%date:~5,2%%date:~8,2%
set time_hh=%time:~0,2%
if /i %time_hh% LSS 10 (set time_hh=0%time:~1,1%)
set data_time_str=%date:~0,4%%date:~5,2%%date:~8,2%

del YIname.txt
"YIchecksum_2.1.8.exe" %shaolu_file% > "YIname.txt"
set "cmd_str=type YIname.txt ^| find "固件校验码""
FOR /F "usebackq" %%i IN (`%cmd_str%`) DO @set CheckCode=%%i
set "CheckCode=%CheckCode:~6,13%"
echo 校验码 %CheckCode%

::EDR
For /f "tokens=1* delims=:" %%i in ('Type YIname.txt^|Findstr /n ".*"') do (
If "%%i"=="6" Set BluetoothStr=%%j
) 
set "edrname=%BluetoothStr:~4,44%"
echo 蓝牙名 %edrname%


::BLE
For /f "tokens=1* delims=:" %%i in ('Type YIname.txt^|Findstr /n ".*"') do (
If "%%i"=="7" Set bleStr=%%j
) 
set "blename=%bleStr:~7,47%"
echo 蓝牙名 %blename%

del YIname.txt


REM EDR 经典蓝牙
ren %shaolu_file% %data_time_str%-S58-校验码(%CheckCode%)-蓝牙名("%edrname%")-%filename1%.fw
"C:\Program Files\WinRAR\Rar.exe" a  -k -r -s %data_time_str%-S58-校验码(%CheckCode%)-蓝牙名("%edrname%")-%filename2%.rar  %update_file%


REM BLE BLE蓝牙
::ren %shaolu_file% %data_time_str%-校验码(%CheckCode%)-BLE蓝牙名("%blename%")-%filename%.fw
::"C:\Program Files\WinRAR\Rar.exe" a  -k -r -s %data_time_str%-校验码(%CheckCode%)-BLE蓝牙名("%blename%")-%filename%.rar  %update_file%

REM EDR&BLE 双模蓝牙
::ren %shaolu_file% %data_time_str%-校验码(%CheckCode%)-蓝牙名("%edrname%")-BLE蓝牙名("%blename%")-%filename%.fw
::"C:\Program Files\WinRAR\Rar.exe" a  -k -r -s %data_time_str%-校验码(%CheckCode%)-蓝牙名("%edrname%")-BLE蓝牙名("%blename%")-%filename%.rar  %update_file%






