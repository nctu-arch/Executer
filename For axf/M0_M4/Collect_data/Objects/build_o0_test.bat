D:\Keil_v5\ARM\ARMCC\bin\armlink --cpu=cortex-m0 -O0 --ro-base=0x0 --info=totals --map --first="test.o(vectors)" -o test.elf test.o
D:\Keil_v5\ARM\ARMCC\bin\fromelf --bin --output test.bin test.elf
D:\Keil_v5\ARM\ARMCC\bin\fromelf -c -o test.txt test.elf