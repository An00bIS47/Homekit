
# Debugging using ESP-Prog

## Connecting

You need to connect these pins

| Name     | ESP32   | ESP-Prog     | Comment       |
|----------|---------|--------------|---------------|
| MTDI     | GPIO12  | TDI (ADBUS1) |               | 
| MTCK     | GPIO13  | TCK (ADBUS0) | <- LED PIN !  |
| MTMS     | GPIO14  | TMS (ADBUS3) |               |
| MTDO     | GPIO15  | TD0 (ADBUS2) |               |



## Start OpenOCD

`./openocd -f ../share/openocd/scripts/interface/ftdi/esp32_devkitj_v1.cfg -f ../share/openocd/scripts/board/esp-wroom-32.cfg`


## Configuration OpenOCD

Create a file called `file.elf` for the configuration of OpenOCD

```
target remote :3333 
set remote hardware-watchpoint-limit 2 
mon reset halt 
flushregs 
thb app_main 
c
```

## Start OpenOCD

Then start OpenOCD with the following command

`xtensa-esp32-elf-gdb -x gdbinit ~/path/to/elf/file.elf`