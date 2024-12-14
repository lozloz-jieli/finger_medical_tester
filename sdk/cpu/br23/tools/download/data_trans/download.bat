@echo off

cd %~dp0

copy ..\..\tone.cfg .
copy ..\..\br23loader.bin .
copy ..\..\script.ver

..\..\isd_download.exe  ..\..\isd_config.ini -tonorflash -dev br23 -boot 0x12000 -div8 -wait 300 -uboot ..\..\uboot.boot -app ..\..\app.bin ..\..\cfg_tool.bin -ota_file ..\..\ota_all.bin  -key  168-AC690X-FA76.key  
:: -format all
::-reboot 2500

@rem ɾ����ʱ�ļ�-format all    -key  168-AC690X-FA76.key
if exist *.mp3 del *.mp3 
if exist *.PIX del *.PIX
if exist *.TAB del *.TAB
if exist *.res del *.res
if exist *.sty del *.sty

@rem add ver to jl_isd.fw
@rem generate upgrade file
..\..\fw_add.exe -noenc -fw jl_isd.fw  -add ..\..\ota_all.bin -name ota.bin -type 100 -out jl_isd_all.fw
..\..\fw_add.exe -noenc -fw jl_isd.fw  -add ..\..\ota_nor.bin -name ota.bin -type 100 -out jl_isd_nor.fw
@rem add ver to jl_isd.fw
..\..\fw_add.exe -noenc -fw jl_isd_all.fw -add script.ver -out jl_isd_all.fw
..\..\fw_add.exe -noenc -fw jl_isd_nor.fw -add script.ver -out jl_isd_nor.fw

..\..\ufw_maker.exe -fw_to_ufw jl_isd_all.fw
..\..\ufw_maker.exe -fw_to_ufw jl_isd_nor.fw

copy jl_isd_all.ufw update.ufw
copy jl_isd_nor.ufw nor_update.ufw
copy jl_isd_all.fw jl_isd.fw
del jl_isd_all.ufw jl_isd_nor.ufw jl_isd_all.fw jl_isd_nor.fw

@REM ���������ļ������ļ�
::ufw_maker.exe -chip AC800X %ADD_KEY% -output config.ufw -res bt_cfg.cfg

::IF EXIST jl_696x.bin del jl_696x.bin 

@rem ��������˵��
@rem -format vm        //����VM ����
@rem -format cfg       //����BT CFG ����
@rem -format 0x3f0-2   //��ʾ�ӵ� 0x3f0 �� sector ��ʼ�������� 2 �� sector(��һ������Ϊ16���ƻ�10���ƶ��ɣ��ڶ�������������10����)

ping /n 2 127.1>null
IF EXIST null del null

