include ../../mak/UserDefine.mak

PROJECT_ROOT_DIR=$(shell pwd)/../../
export PROJECT_ROOT_DIR

SUB_DIRS = \
$(PROJECT_ROOT_DIR)osal \
$(PROJECT_ROOT_DIR)util \
$(PROJECT_ROOT_DIR)container \
$(PROJECT_ROOT_DIR)runtime \
$(PROJECT_ROOT_DIR)platform \
$(PROJECT_ROOT_DIR)compiler_frontend \
$(PROJECT_ROOT_DIR)compiler_backend \
$(PROJECT_ROOT_DIR)compiler_middleend \
$(PROJECT_ROOT_DIR)compiler

all:
	@make build_subdir

clean:
	@make clean_subdir

install:
	@make install_subdir

include ../../mak/Rules.mak
