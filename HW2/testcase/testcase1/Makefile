OPT = ../../opt
OPT_FLAG =  -load /home/chihmin/llvm-homework/build/lib/LLVMDataFlow.so -dataflow 
CC = clang
CC_FLAG = -c -emit-llvm
NAME = A
SRC = $(NAME).c
TAR = $(NAME).bc

all:
	$(CC) -o $(TAR) $(CC_FLAG) $(SRC)
	$(CC) $(CC_FLAG) -S $(SRC)
	$(OPT) $(OPT_FLAG) $(TAR)


