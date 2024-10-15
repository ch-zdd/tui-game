.PHONY: all clean 

CFALGS += -Wall -fPIC
  
all: libtk_common.a  
  
libtk_common.a: data_handle.o
	ar -r libtk_common.a data_handle.o
  
data_handle.o: data_handle.c  
	gcc -c data_handle.c $(CFALGS)
  
clean:  
	rm -rf *.o
