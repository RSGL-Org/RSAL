ARGS = -c
OS = linux
SOURCEDIR = source
SOURCE = $(SOURCEDIR)/rsal.cpp
LIBDIR = $(SOURCEDIR)/deps
LIBS = $(LIBDIR)/libpthread-2.33.so

GXX = g++
build:
	@$(GXX) $(ARGS) $(SOURCE)
	ar rsv libRSAL.a $(LIBS) *.o
	rm rsal.o


install:
	sudo mv source/deps/ffplay /usr/bin/
	@make build
	@sudo cp libRSAL.a /usr/lib/libRSAL.a
	@rm libRSAL.a
	@mkdir ./RSAL
	@cp -r include RSAL.hpp ./RSAL
	@sudo cp -r ./RSAL /usr/include/
	@rm -r ./RSAL


all:
	make build
	sudo make install