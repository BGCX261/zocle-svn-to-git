include ../mak/UserDefine.mak

SUB_DIRS = src
MODULE = runtime
LIBS = ../osal/.libs/libosal.a ../util/.libs/libutil.a ../platform/.libs/libplatform.a

all:
	@make build_subdir
	@make .libs/lib$(MODULE).a

clean:
	@make clean_subdir
	@make clean_libs

install:
	@make install_module_library

include ../mak/Rules.mak
