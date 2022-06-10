ARGS = -Wall -fPIC -O -g
SOURCE  = source/*.cpp
LIBS =  source/deps/*.so source/deps/*.34 source/deps/*.0 source/deps/*.7

GXX = g++
build:
	@$(GXX) -c $(ARGS) $(SOURCE) -c --no-warnings
	@$(GXX) -shared *.o  $(LIBS) -o libRSAL.so 
	@rm *.o;

install:
	@make build
	@sudo cp libRSAL.so /usr/lib/libRSAL.so
	@rm libRSAL.so
	@mkdir ./RSAL
	@cp -r include RSGL.hpp ./Examples ./RSGL
	@sudo cp -r ./RSAL /usr/include/
	@sudo rm -r ./RSAL

all:
	build
	install

uninstall:
	rm /usr/lib/libRSAL.so

update:
	rm /usr/lib/libRSAL.so
	@make install