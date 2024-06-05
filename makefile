all: spaceinvaders.c
	gcc spaceinvaders.c -lncurses -o spaceinvaders

debug: spaceinvaders.c
	gcc spaceinvaders.c -lncurses -o spaceinvaders -g

