nasm -felf64 -gdwarf test.asm -o test.o
ld -o test test.o
chmod u+x test