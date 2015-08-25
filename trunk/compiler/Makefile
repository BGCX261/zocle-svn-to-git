include ../mak/UserDefine.mak

SUB_DIRS = src
MODULE = compiler
LIBS += -L$(PROJECT_ROOT_DIR)compiler_frontend/.libs/ -lcompiler_frontend -L$(PROJECT_ROOT_DIR)osal/.libs/ -losal -L$(PROJECT_ROOT_DIR)container/.libs/ -lcontainer -L$(PROJECT_ROOT_DIR)util/.libs/ -lutil

all:
	@make build_subdir
	@make module_executable

clean:
	@make clean_subdir
	@make clean_libs

install:
	@make install_module_library

include ../mak/Rules.mak
