SET PATH=C:\Keil_v5\ARM\ARMCC\Bin;C:\ProgramData\Oracle\Java\javapath; C:\Xilinx\14.2\ISE_DS\ISE\bin\nt64\ise.exe\bin\nt64; C:\Xilinx\14.2\ISE_DS\ISE\bin\nt64\ise.exe\lib\nt64;C:\Xilinx\10.1\ISE\smartmodel\nt\installed_nt\lib\pcnt.lib;C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\Windows\System32\WindowsPowerShell\v1.0\;C:\Program Files (x86)\MiKTeX 2.9\miktex\bin\;D:\MATLAB\runtime\win64;D:\MATLAB\bin;C:\Program Files (x86)\Skype\Phone\;C:\Modeltech_pe_edu_10.3b\win32pe_edu;C:\Program Files (x86)\Google\google_appengine\
SET CPU_TYPE=ARMCM4
SET CPU_VENDOR=ARM
SET UV2_TARGET=Target 1
SET CPU_CLOCK=0x00B71B00
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\m0slave.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\my_sensors.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmAsm" --Via ".\objects\startup_armcm4._ia"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\system_armcm4.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmLink" --Via ".\Objects\collect_data_M4_sing.lnp"
