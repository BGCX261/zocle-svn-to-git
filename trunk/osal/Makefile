include ../mak/UserDefine.mak

SUB_DIRS = src
MODULE = osal
LIBS = 

all:
	@make build_subdir
	@make .libs/lib$(MODULE).a

clean:
	@make clean_subdir
	@make clean_libs

install:
	@make install_module_library

include ../mak/Rules.mak
