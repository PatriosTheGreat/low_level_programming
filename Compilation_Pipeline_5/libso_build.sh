nasm -f elf64 -o libso.o libso.asm
ld -o main main.o -d libso.so

ld -shared -o libso.so libso.o --dynamic-linker=/lib64/ld-linux-x86-64.so.2
readelf -S libso.so