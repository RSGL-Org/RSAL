ARGS = -Wall -fPIC -O -g
OS = linux
SOURCEDIR = source
SOURCE = $(SOURCEDIR)/rsal.cpp
LIBDIR = $(SOURCEDIR)/deps
LIBS = $(LIBDIR)/libpthread-2.33.so

GXX = g++
build:
	@$(GXX) -c $(ARGS) $(SOURCE) -o rsal.o
	@$(GXX) -shared rsal.o  $(LIBS) -o libRSAL.so 
	@rm rsal.o


install:
	@make build
	@sudo cp libRSAL.so /usr/lib/libRSAL.so
	@rm libRSAL.so
	@mkdir ./RSAL
	@cp -r include RSAL.hpp ./RSAL
	@sudo cp -r ./RSAL /usr/include/
	@rm -r ./RSAL


all:
	make build
	sudo make install
	g++ main.cpp -lRSAL
	./a.out