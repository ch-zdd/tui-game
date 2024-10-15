LIB = crc
MAIN = main
COMPILE_TARGET = main


all:main
main: $(MAIN).o
	gcc $^ -o $(COMPILE_TARGET) -ltk_common -Lcommon-lib

%.o: %.c  
	gcc -c $< -o $@ -Wall

clean:
	rm -rf  *.o *.so main
