/*
 * 	coded by hatchet June 2024
 *
 * 	This is my version of Space Invaders
 *
 * 	TODO
 * 	collission detection for when the aliens hit the player
 * 	color
 * 	have different levels
 * 	create an intro scren and exit screen
 *
 *
*/
 
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

// these are the colors to be used throughout the game
#define PLAYER_COLOR	1
#define BORDER_COLOR    2
#define SCORE_COLOR     3
#define ALIEN_COLOR     4

#define SCREEN_HEIGHT  	30
#define SCREEN_WIDTH    70

// the height and width of the invasion fleet
#define INVWIDTH	5
#define INVHEIGHT	3

// this struct is for the spaceships, both for the player and the aliens
typedef struct {
	// these three strings are the ascii art for the vessels
	char healthy[3][7];
	char dying[3][7];
	char dead[3][7];
	int health;
	int pos;
	int y;
	int x;

} spaceship;

// this is for a bullet, this is for both player and alien bullets
typedef struct {
	int y;
	int x;
	bool new;
} bullet;

// prints the intro screen
int introscreen();

// prints the game over screen and return 1 if the player wants to play again
// or returns 0 if they choose not to play again
int gameover(int code, int *score);

// draw the border
int drawborder();

// a function to draw ther players spaceship
int drawplayer(spaceship *playership);

// draw the players bullet
int drawplayerbullet(bullet *playerbullet, int pos);

// draw the alien spaceship
int drawalien(spaceship *alienship);

// draw the alien bullet
int drawalienbullet(spaceship invasion[][INVHEIGHT], bullet *alienbullet);

// Moves the invasion ships
// Takes an argument which is an int and indicates the direction
// The values for the argument invasiondirection are below.
// 0 - the invasion has already stepped down 2 blocks on the left side
// 1 - the invasion has already stepped down 1 blocks on the left side
// 2 - the invasion is travelling to the left side
// 3 - the invasion is traveling to the right side
// 4 - the invasion has already stepped down 1 block on the right side
// 5 - the invasion has already stepped down 2 block on the right side
int mvinvasion(spaceship *alienship, int *invasiondirection);

// returns TRUE if the playership has been hit by an alien bullet
// otherwise returns FALSE
bool plcollission(spaceship *playership, bullet *alienbullet);

// draws the alien ships
int drawinvasion(int direction);

// initiate the invasion fleet
int initinvasion(spaceship invasion[][INVHEIGHT]);

// move the invasion fleet
int invmv(spaceship invasion[][INVHEIGHT], spaceship *playership, int *invdirection);

// draw the invasion fleet
int invdraw(spaceship invasion[][INVHEIGHT], int *invdirection);

// change the invdirection variable for the invasion fleet
// 0 - the invasion has already stepped down 2 blocks on the left side
// 1 - the invasion has already stepped down 1 blocks on the left side
// 2 - the invasion is travelling to the left side
// 3 - the invasion is traveling to the right side
// 4 - the invasion has already stepped down 1 block on the right side
// 5 - the invasion has already stepped down 2 block on the right side
int invchkdirection(spaceship invasion[][INVHEIGHT], int *invdirection);

int main(int argc, char *argv[]){

	int inputchar;

	int x, y;
	int a, b;
	int score = 0;
	int lives = 5;

	int timetomove = 0;


	// this is used to store the current level in the game
	int currentgamelevel = 0;

	// this is for the timer, the clock to move everything at the right time
	// there are two timespect structures declared
	// the time is compared between them to control the movement of the bullets and aliens
	struct timespec timer, lasttime;
	uint64_t elapsedtime = 0;

	// initiate the invasion
	int invdirection = 3;
	spaceship invasion[INVWIDTH][INVHEIGHT];

	bullet playerbullet;

	bullet alienbullet;

	// declare and intialize the player spaceship
	// There needs to be a blank space at the start and end of each string otherwise
	// the screen flickers with each refresh
	spaceship playership;
	strncpy(playership.healthy[0], "   ^   ", 7);
	strncpy(playership.healthy[1], "  / \\  ", 7);
	strncpy(playership.healthy[2], " // \\\\   ", 7);

	spaceship alienship;
	
	// init the screen for curses and initialise a few things
	initscr();

	resize_term(SCREEN_HEIGHT, SCREEN_WIDTH);
	introscreen();
newgame:
	memset(&alienbullet, 0, sizeof(playerbullet));
	memset(&playerbullet, 0, sizeof(playerbullet));

	initinvasion(invasion);

	clear();
	playership.pos = COLS / 2;	// have the players ship start in the middle of the screen
	curs_set(0);			// turn off the flashing curser
	keypad(stdscr, TRUE);		// allow the key pad to be used
	noecho();			// don't echo characters to the screen

	nodelay(stdscr, TRUE);		// don't wait for getch() to return a value


	// draw everything for the start of the game
	drawborder();
	drawplayer(&playership);
	refresh();

	// this is the clock that is used to measure the time to refresh the screen
	clock_gettime(CLOCK_MONOTONIC, &timer);
	lasttime.tv_sec = timer.tv_sec;
	lasttime.tv_nsec = timer.tv_nsec;

	inputchar = 0;
	score = 0;
	lives = 5;

	alienbullet.y = LINES + 2;
	alienbullet.x = 0;
	

	while(1){
		inputchar = getch();

		// work out the amount of time that has passed since struct timer and struct lasttime were synched
		// there are two timespec structures
		// the time is compared between them to control the movement of the bullets and aliens
		//
		// reset timer
		clock_gettime(CLOCK_MONOTONIC, &timer);
		// store the number of nanoseconds that have passed since both time and lasttime were synced
		elapsedtime = 1000000000 * (timer.tv_sec - lasttime.tv_sec) + timer.tv_nsec - lasttime.tv_nsec;		

		// if this many nanoseconds have passed then move things in the game
		if(elapsedtime > 100000000){
	
			// check if the any alien ships have been hit by a player bullet
			// I should really put this in a function of its own
			for(y = 0; y < INVWIDTH; y++){
				for(x = 0; x < INVHEIGHT; x++){
					for(a = 0; a < 3; a++){
						for(b = 0; b < 7; b++){
							if(invasion[y][x].health < 3){
								if((invasion[y][x].y + a == playerbullet.y) && (invasion[y][x].x + b == playerbullet.x)){
									invasion[y][x].health++;
									score++;
									playerbullet.x = -1;
									playerbullet.y = -1;
								}
							}
						}
					}
				}
			}
			
			// move the invasion
			// if imv() returns 1 then one of the aliens has collided with earth or the player
			// then gameover() is called
			if(invmv(invasion, &playership, &invdirection)){
				if(gameover(2, &score)){
						goto newgame;
				}else{
					endwin();
					return 0;
				}
			}
	
			// check and change the direction of the invasion
			invchkdirection(invasion, &invdirection);

			// increase the health (or rather have them die further) if they have been previously hit
			for(y = 0; y < INVWIDTH; y++){
				for(x = 0; x < INVHEIGHT; x++){
					if((invasion[y][x].health > 0)){
						invasion[y][x].health++;
					}
				}
			}

			// check if the player has no lives left
			if(lives == 0){
				if(gameover(1, &score)){
					goto newgame;
				}else{
					endwin();
					return 0;
				}
			}
		
			// check if alienbullet has hit the player
			if(plcollission(&playership, &alienbullet)){
				lives--;
				playership.health--;
			}

			// draw everthing on the screen
			erase();
			drawborder();
			mvprintw(0, 2, " Score = %d ", score);
			mvprintw(0, 17, " Lives = %d ", lives);
			drawplayer(&playership);
			invdraw(invasion, &invdirection);
			drawplayerbullet(&playerbullet, 0);
			drawalienbullet(invasion, &alienbullet);
			// reset the two timers and have them match each other
			clock_gettime(CLOCK_MONOTONIC, &timer);
			lasttime.tv_sec = timer.tv_sec;
			lasttime.tv_nsec = timer.tv_nsec;
		}

		// have the aliens shoot bullets at the player
		for(y = 0; y < INVHEIGHT; y++){
			for(x = 0; x < INVWIDTH; x++){
				// only allow a bullet to be fired if the below conditions are true
				// if the alienship is directly above the player AND
				// if the alien bullet has already gone off the bottom of the screen AND
				// if the alien has not yet been hit by a player bullet
				if((invasion[x][y].x == playership.pos) && (alienbullet.y >= LINES) && (invasion[x][y].health == 0)){
					alienbullet.y = invasion[x][y].y;
					alienbullet.x = invasion[x][y].x + 3;
				}
			}
		}
 
		switch(inputchar){
			case KEY_RIGHT:
				// move the player right
				playership.pos++;

				// if the player has gone past the left or right hand wall then put the playership back where they were
				if((playership.pos <= 1) || (playership.pos > COLS - 8)){
					playership.pos--;
					break;
				}

				drawborder();
				mvprintw(0, 2, " Score = %d ", score);
				mvprintw(0, 17, " Lives = %d ", lives);
				drawplayer(&playership);

				break;

			case KEY_LEFT:
				// move the player left
				playership.pos--;
				// if the player has gone past the left or right hand wall then put the playership back where they were
				if((playership.pos <= 0) || (playership.pos > COLS - 7)){
					playership.pos++;
					break;
				}
				drawborder();
				mvprintw(0, 2, " Score = %d ", score);
				mvprintw(0, 17, " Lives = %d ", lives);
				drawplayer(&playership);

				break;

				// hitting 'q' ends the game
			case 'q':
			case 'Q':
				endwin();
				return 0;

				break;
				
				// shoot a bullet when spacebar is hit
			case ' ':
				if(playerbullet.y > -1){
					break;
				}

				drawborder();
				mvprintw(0, 2, " Score = %d ", score);
				mvprintw(0, 17, " Lives = %d ", lives);
				drawplayer(&playership);

				playerbullet.y = LINES - 5;
				drawplayerbullet(&playerbullet, playership.pos);
	
				break;

				// this is debug code, hold 'd' whilst the game is playing keep on the screen
			case 'd':
				mvprintw(2, 2, "playership.pos:	%d", playership.pos);
				mvprintw(3, 2, "playerbullet.y	%d", playerbullet.y);
				mvprintw(4, 2, "playerbullet.x	%d", playerbullet.x);
				mvprintw(5, 2, "elapsedtime	%li", elapsedtime);
				break;
				
			default:
				break;
		}
	}

	endwin();

	return 0;

}

int drawborder(){

        int counter;

       // attron(COLOR_PAIR(BORDER_COLOR));

        // this draws the line across the top and then the line on the bottom
        for(counter = 0; counter < COLS; counter++){
                mvprintw(0, counter, "*");
                mvprintw(LINES - 1, counter, "*");
        }

        // this draws the line on the left hand side and then the line on the right hand border
        for(counter = 0; counter < LINES; counter++){
                mvprintw(counter, 0, "*");
                mvprintw(counter, COLS - 1, "*");
        }

//        attroff(COLOR_PAIR(BORDER_COLOR));

        return 0;
}

int drawplayer(spaceship *playership){
	int a, b;
	int line = LINES - 4;

	// draw one character at a time with two for loops
	for(a = 0; a < 3; a++){
		for(b = 0; b < 7; b++){
			mvprintw(line + a, playership->pos + b, "%c", playership->healthy[a][b]);
		}
	}

	return 0;
}


int drawplayerbullet(bullet *playerbullet, int pos){

	// if the variable pos is not zero then a new bullet has been fired
	// allocate the starting location to y and x in playerbullet
	if(pos != 0){
		playerbullet->y = LINES - 5;
		playerbullet->x = pos + 3;
	}

	// if the variable pos is zero then the bullet is simply moving up the screen one space
	if(pos == 0)
		playerbullet->y--;

	// draw two bullets, it looks better
	mvprintw(playerbullet->y, playerbullet->x, "|");
	mvprintw(playerbullet->y + 1, playerbullet->x, "|");

	return 0;
}

int drawalienbullet(spaceship invasion[][INVHEIGHT], bullet *alienbullet){

	// if the bullet is still on the screen then simply move it down one space
	if(alienbullet->new == false){
		alienbullet->y++;
	}
	
	mvprintw(alienbullet->y, alienbullet->x, "|");
	mvprintw(alienbullet->y - 1, alienbullet->x, "|");

	return 0;
}

int mvinvasion(spaceship *alienship, int *invasiondirection){

	// this is a bit complex looking
	// switch statements 2 and 3 move the invasion left and right
	// the statements 0, 1, 4 and 5 move the invasion down at the edges of the screen
	switch(*invasiondirection){
		case 0:
			alienship->x++;
			*invasiondirection = 3;
			break;
		case 1:
			alienship->y++;
			*invasiondirection = 0;
			break;
		case 2:
			if(alienship->x == 1){
				alienship->y++;
				*invasiondirection = 1;
				break;
			}
			alienship->x--;
			break;
		case 3:
			if(alienship->x == COLS - 8){
				alienship->y++;
				*invasiondirection = 4;
				break;
			}
			alienship->x++;
			break;
		case 4:
			alienship->y++;
			*invasiondirection = 5;
			break;
		case 5:
			alienship->x--;
			*invasiondirection = 2;
			break;
	}

	return 0;
}


bool plcollission(spaceship *playership, bullet *alienbullet){
	int a, b;
	// x is the top of the player ship
	int x = LINES - 4;

	for(a = 0; a < 3; a++){
		for(b = 0; b < 7; b++){
			if((x + a == alienbullet->y) && (playership->pos + b == alienbullet->x)){

				// if the player ship has been hit then set alien bullet to -1 for y axis so it doesn't show on the screen
				alienbullet->x = -1;
				alienbullet->y = LINES + 2;
				
				// return 1 which is true because there has been a collission
				return 1;
			}
		}
	}

	return 0;
}


int initinvasion(spaceship invasion[][INVHEIGHT]){
	int x, y;

	for(y = 0; y < INVHEIGHT; y++){
		for(x = 0; x < INVWIDTH; x++){
					// what to draw when the alien is healthy
			     	  	strncpy(invasion[x][y].healthy[0], "  ^ ^  ", 7);
					strncpy(invasion[x][y].healthy[1], "  0 0  ", 7);
					strncpy(invasion[x][y].healthy[2], " -___- ", 7);

					// the alien has been hit and is in the process of dying
					strncpy(invasion[x][y].dying[0], "  \\ /  ", 7);
					strncpy(invasion[x][y].dying[1], "-- *--  ", 7);
					strncpy(invasion[x][y].dying[2], "  / \\   ", 7);

					// the last scene of the alien dying
					strncpy(invasion[x][y].dead[0], "  ..   ", 7);
					strncpy(invasion[x][y].dead[1], ".:  :. ", 7);
					strncpy(invasion[x][y].dead[2], "  ..   ", 7);
					invasion[x][y].health = 0;
					invasion[x][y].y = 2 + (3 * y);
					invasion[x][y].x = 9 + (7 * x);
		}
	}

	return 0;

}

int invchkdirection(spaceship invasion[][INVHEIGHT], int *invdirection){
	int a, b;

	for(a = 0; a < INVWIDTH; a++){
		for(b = 0; b < INVHEIGHT; b++){

			// each of these if statements handles what to do with each direction
			//
			// The values for the argument invasiondirection are below.
			// 0 - the invasion has already stepped down 2 blocks on the left side
			// 1 - the invasion has already stepped down 1 blocks on the left side
			// 2 - the invasion is travelling to the left side
			// 3 - the invasion is traveling to the right side
			// 4 - the invasion has already stepped down 1 block on the right side
			// 5 - the invasion has already stepped down 2 block on the right side
			//
			// the reason for the check on the invasion.health is to ensure that the location is 
			// not checked if the array is already dead
			// this way you can shoot all the aliens on one side so that the remaining aliens
			// will still to right up to the wall
			if((invasion[a][b].x == 1) && (*invdirection == 2) && (invasion[a][b].health < 3)){
				*invdirection = 1;
				return 1;
			}

			if((invasion[a][b].x == 1) && (*invdirection == 1)&& (invasion[a][b].health < 3)){
					*invdirection = 0;
					return 0;
				}else


			if((invasion[a][b].x == 1) && (*invdirection == 0) && (invasion[a][b].health < 3)){
					*invdirection = 3;
					return 3;
				}

			if((invasion[a][b].x == COLS - 8) && (*invdirection == 3) && (invasion[a][b].health < 3)){
				*invdirection = 4;
				return 4;
			}

			if((invasion[a][b].x == COLS - 8) && (*invdirection == 4) && (invasion[a][b].health < 3)){
				*invdirection = 5;
				return 5;
			}

			if((invasion[a][b].x == COLS - 8) && (*invdirection == 5) && (invasion[a][b].health < 3)){
				*invdirection = 2;
				return 2;
			}
			
		}
	}	
	// this should never be used
	return *invdirection;
}

// move the invasion fleet
int invmv(spaceship invasion[][INVHEIGHT], spaceship *playership, int *invdirection){
	int a, b;

	for(b = 0; b < INVHEIGHT; b++){
		for(a = 0; a < INVWIDTH; a++){
			/*nodelay(stdscr, FALSE);
			mvprintw(2, 2, "invasion[a][b].x %d", invasion[a][b].x);
			mvprintw(3, 2, "playership->pos %d", playership->pos);
			refresh();
			getch();
			nodelay(stdscr, TRUE);
*/
			if((invasion[a][b].y == LINES - 7) && invasion[a][b].health == 0){
				return 1;
				break;
			}
			switch(*invdirection){
				// each switch statement moves the ship according to the invdirection variable
				// 0 - the invasion has already stepped down 2 blocks on the left side
				// 1 - the invasion has already stepped down 1 blocks on the left side
		 	 	// 2 - the invasion is travelling to the left side
		 		// 3 - the invasion is traveling to the right side
				// 4 - the invasion has already stepped down 1 block on the right side
				// 5 - the invasion has already stepped down 2 block on the right side
				case 0:
					invasion[a][b].y++;
					break;
				case 1:
					invasion[a][b].y++;
					break;
				case 2:
					invasion[a][b].x--;
					break;
				case 3:
					invasion[a][b].x++;
					break;
				case 4:
					invasion[a][b].y++;
					break;
				case 5:
					invasion[a][b].y++;
					break;
			}
		}
	}

	return 0;
}

// draw the invasion
int invdraw(spaceship invasion[][INVHEIGHT], int *invdirection){
	int counter;
	int x, y;
	int a, b;

	for(y = 0; y < INVHEIGHT; y++){
		for(x = 0; x < INVWIDTH; x++){
			for(a = 0; a < 3; a++){
				for(b = 0; b < 7; b++){
					switch(invasion[x][y].health){
						// the reason for the empty switch statements is a hack to create a delay so you actually see
						// the ship change as it dies
						// otherwise it would all be over in 3 frames
						case 0:
							mvprintw(invasion[x][y].y + a, invasion[x][y].x + b, "%c", invasion[x][y].healthy[a][b]);
							break;
						case 1:
						case 2:
						case 3:
						case 4:
							mvprintw(invasion[x][y].y + a, invasion[x][y].x + b, "%c", invasion[x][y].dying[a][b]);
							break;
						case 5:
						case 6:
						case 7:
						case 8:
							mvprintw(invasion[x][y].y + a, invasion[x][y].x + b, "%c", invasion[x][y].dead[a][b]);
							break;
					}
				}
			}
		}
	}

	return 0;
}

int introscreen(){

        clear();
        drawborder();


//        attron(COLOR_PAIR(FRUIT_COLOR));
        mvprintw(2, 2, "                          Space Invaders");
        mvprintw(3, 2, "                     Prepare to save Earth !");
        mvprintw(5, 2, "                        Hit any key to start");
        mvprintw(7, 2, "                         coded by Hatchet");
        mvprintw(8, 2, "                             June 2024");


//        attroff(COLOR_PAIR(FRUIT_COLOR));

        // turn off nodelay before calling getch
        // to ensure that the play has to hit a key and we have the delay
        nodelay(stdscr, FALSE);
        getch();
        nodelay(stdscr, TRUE);

        clear();
        return 0;
}

int gameover(int code, int *score){
        int input;


        switch(code){
                // the player has zero lives left
                case 1:
                        clear();
                        drawborder();

                        //attron(COLOR_PAIR(FRUIT_COLOR));
                        mvprintw(2, 2, "                          Game Over");
                        mvprintw(3, 2, "                        Zero lives left");
                        mvprintw(5, 2, "               Nice try, better luck next time... ");
                        //attroff(COLOR_PAIR(FRUIT_COLOR));

                        //attron(COLOR_PAIR(SCORE_COLOR));
                        mvprintw(7, 2, "                     Your score is %d", *score);
                        //attroff(COLOR_PAIR(SCORE_COLOR));

                        //attron(COLOR_PAIR(FRUIT_COLOR));
                        mvprintw(9, 2, "                      Want to play again?");
                        mvprintw(10, 2, "                       Hit 'Y' or 'N'");
                        //attroff(COLOR_PAIR(FRUIT_COLOR));

                        refresh();

                        while(1){
                                nodelay(stdscr, FALSE);
                                input = getch();

                                switch(input){
                                        case 'y':
                                        case 'Y':
                                                clear();
                                		nodelay(stdscr, TRUE);
                                                return 1;

                                        case 'n':
                                        case 'N':
                                        case 'q':
                                        case 'Q':
                                                return 0;
                                }
                        }

                        break;

                // The aliens have reached the player
                case 2:
                        clear();
                        drawborder();

                        //attron(COLOR_PAIR(FRUIT_COLOR));
                        mvprintw(2, 2, "                          Game Over");
                        mvprintw(3, 2, "                     The aliens reached Earth");
                        mvprintw(5, 2, "               Nice try, better luck next time... ");
                        //attroff(COLOR_PAIR(FRUIT_COLOR));

                        //attron(COLOR_PAIR(SCORE_COLOR));
                        mvprintw(7, 2, "                     Your score is %d", *score);
                        //attroff(COLOR_PAIR(SCORE_COLOR));

                        //attron(COLOR_PAIR(FRUIT_COLOR));
                        mvprintw(9, 2, "                      Want to play again?");
                        mvprintw(10, 2, "                       Hit 'Y' or 'N'");
                        //attroff(COLOR_PAIR(FRUIT_COLOR));

                        refresh();

                        while(1){
                                nodelay(stdscr, FALSE);
                                input = getch();

                                switch(input){
                                        case 'y':
                                        case 'Y':
                                                clear();
                                		nodelay(stdscr, TRUE);
                                                return 1;

                                        case 'n':
                                        case 'N':
                                        case 'q':
                                        case 'Q':
                                                return 0;
                                }
                        }


                // this should never be called
                default:
                        return 0;
        }

        return 0;
}

