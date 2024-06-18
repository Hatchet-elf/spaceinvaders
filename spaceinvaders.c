/*
 * 	coded by hatchet June 2024
 *
 * 	This is my version of Space Invaders
 *
 * 	TODO
 * 	DONE collission detection for when the aliens hit the player
 * 	DONE color
 * 	DONE have different levels
 * 	DONE create an intro scren and exit screen
 * 	write the readme file
 * 	DONE make the player spaceship look better
 * 	check that the comments are all accurate
 * 	add a pause option
 * 	add a help screen
 * 	add the blocks that you can shoot between and which protect you
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

#define SCREEN_HEIGHT  	60
#define SCREEN_WIDTH    140

#define INVASIONWIDTH	10
#define INVASIONHEIGHT	15

#define BLOCKSHEIGHT	3
#define BLOCKSWIDTH	20

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

typedef struct {
	int y;
	int x;
	int active;
} blockarray;

int initblocks(blockarray blocks[][BLOCKSWIDTH]);

int drawblocks(blockarray blocks[][BLOCKSWIDTH]);

int hasbullethitblocks(blockarray blocks[][BLOCKSWIDTH], bullet *playerbullet, bullet *alienbullet);

void drawintroscreen();

// prints the game over screen and return 1 if the player wants to play again
// or returns 0 if they choose not to play again
int drawgameoverscreen(int code, int *score);

// gets called when the player has killed all aliens
// changes the variable currentgamelevel to the next game level
// when the player has finished the final level it will return:
// 	1 if they are playing again
// 	0 if they do not want to play again
int gotonextlevel(int *score, int *currentgamelevel);

int drawborder();

int drawplayer(spaceship *playership);

int drawplayerbullet(bullet *playerbullet, int pos);

int drawalienbullet(spaceship invasion[][INVASIONHEIGHT], bullet *alienbullet);

int moveinvasion(spaceship invasion[][INVASIONHEIGHT], spaceship *playership, int *invasiondirection, int currentgamelevel);

// returns TRUE if the playership has been hit by an alien bullet
// otherwise returns FALSE
bool isplayerhitbybullet(spaceship *playership, bullet *alienbullet);

bool isalienhitbybullet(spaceship invasion[][INVASIONHEIGHT], int *score, bullet *playerbullet, int currentgamelevel);

// initiate the invasion fleet
int initinvasion(spaceship invasion[][INVASIONHEIGHT]);

// draw the invasion fleet
// this function returns the number of invasion spaceships that have been moved
// when it returns 0 then there are no remaining invasion spaceships then they have all been
// killed by the player
int drawinvasion(spaceship invasion[][INVASIONHEIGHT], int *invasiondirection, int currentgamelevel);

// change the invasiondirection variable for the invasion fleet
int changeinvasiondirection(spaceship invasion[][INVASIONHEIGHT], int *invasiondirection, int currentgamelevel);

// this is a wrapper for the mvprintw
// makes it easy to print in the centre of the screen
int printincentreofscreen(int linetoprinton, char *stringtoprint);

// print in the centre of the screen with a number being passed by reference
int printincentreofscreenwithnumber(int linetoprinton, char *stringtoprint, int *number);

int main(int argc, char *argv[]){

	int inputchar;

	int x, y;
	int a, b;
	int score = 0;
	int lives = 5;

	// yes, its true. There is cheat mode in Space Invaders - gives you unlimited lives
	int cheatmode = 0;

	// this is used to store the current level in the game
	int currentgamelevel = 1;

	// this is for the timer, the clock to move everything at the right time
	// there are two timespect structures declared
	// the time is compared between them to control the movement of the bullets and aliens
	struct timespec moverecenttime, moveprevioustime;
	uint64_t moveelapsedtime = 0;

	// this time is for moving bullets
	struct timespec bulletrecenttime, bulletprevioustime;
	uint64_t bulletelapsedtime = 0;

	int invasiondirection = 3;
	spaceship invasion[INVASIONWIDTH][INVASIONHEIGHT];

	bullet playerbullet;
	bullet alienbullet;

	blockarray blocks[BLOCKSHEIGHT][BLOCKSWIDTH];
	initblocks(blocks);
	

	// declare and intialize the player spaceship
	// There needs to be a blank space at the start and end of each string otherwise
	// the screen flickers with each refresh
	spaceship playership;
	strncpy(playership.healthy[0], "   ^   ", 7);
	strncpy(playership.healthy[1], "  /^\\  ", 7);
	strncpy(playership.healthy[2], " /_|_\\   ", 7);

	spaceship alienship;
	
	// init the screen for curses and initialise a few things
	initscr();
	resize_term(SCREEN_HEIGHT, SCREEN_WIDTH);
	start_color();
	init_pair(PLAYER_COLOR, COLOR_GREEN, COLOR_BLACK);
	init_pair(BORDER_COLOR, COLOR_YELLOW, COLOR_BLACK);
	init_pair(ALIEN_COLOR, COLOR_RED, COLOR_BLACK);
	init_pair(SCORE_COLOR, COLOR_BLUE, COLOR_BLACK);

	drawintroscreen();

newgame:
	invasiondirection = 3;
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

	// this is the clock that is used to measure the time to MOVE everything andrefresh the screen
	clock_gettime(CLOCK_MONOTONIC, &moverecenttime);
	moveprevioustime.tv_sec = moverecenttime.tv_sec;
	moveprevioustime.tv_nsec = moverecenttime.tv_nsec;

	clock_gettime(CLOCK_MONOTONIC, &bulletrecenttime);
	bulletprevioustime.tv_sec = bulletrecenttime.tv_sec;
	bulletprevioustime.tv_nsec = bulletrecenttime.tv_nsec;

	inputchar = 0;
	score = 0;
	lives = 5;

	alienbullet.y = LINES + 2;
	alienbullet.x = 0;
	
	while(1){
		inputchar = getch();

		// work out the amount of time that has passed since each timer was reset
		// the time is compared between them to control the bullets and movement of aliens and the player
		
		clock_gettime(CLOCK_MONOTONIC, &moverecenttime);
		clock_gettime(CLOCK_MONOTONIC, &bulletrecenttime);
		
		// store the number of nanoseconds that have passed since both time and moveprevioustime were synced
		moveelapsedtime = 1000000000 * (moverecenttime.tv_sec - moveprevioustime.tv_sec) + moverecenttime.tv_nsec - moveprevioustime.tv_nsec;		
		bulletelapsedtime = 1000000000 * (bulletrecenttime.tv_sec - bulletprevioustime.tv_sec) + bulletrecenttime.tv_nsec - bulletprevioustime.tv_nsec;		


		// this if statment looks after the bullet movements and if the player or an alien ship have been hit
		if(bulletelapsedtime > 50000000){
			if(isplayerhitbybullet(&playership, &alienbullet)){
				lives--;
				playership.health--;
			}

			drawplayerbullet(&playerbullet, 0);
			drawalienbullet(invasion, &alienbullet);

			if(cheatmode){
				attron(COLOR_PAIR(BORDER_COLOR));
				mvprintw(0, 0, "O");
				attroff(COLOR_PAIR(BORDER_COLOR));
			}
			
			isalienhitbybullet(invasion, &score, &playerbullet, currentgamelevel);

			hasbullethitblocks(blocks, &playerbullet, &alienbullet);
			
			// reset the two timers and have them match each other
			clock_gettime(CLOCK_MONOTONIC, &bulletrecenttime);
			bulletprevioustime.tv_sec = bulletrecenttime.tv_sec;
			bulletprevioustime.tv_nsec = bulletrecenttime.tv_nsec;
		}

		// this if statement looks after the movement of the invasion
		if(moveelapsedtime > 100000000){
			
			// if moveinvasion() returns 1 then one of the aliens has collided with earth or the player
			// then drawgameoverscreen() is called
			if(moveinvasion(invasion, &playership, &invasiondirection, currentgamelevel)){
				// if cheatmode is active then goto next level
				if(cheatmode){
					currentgamelevel++;
					goto newgame;
				}

				// if the player has reached level the final level they can start again or quite
				if(drawgameoverscreen(2, &score)){
					currentgamelevel = 1;
					goto newgame;
				}else{
					endwin();
					return 0;
				}
				
			}
	
			// check and change the direction of the invasion
			changeinvasiondirection(invasion, &invasiondirection, currentgamelevel);

			// increase aliens health (or rather have them die further) if they have been previously hit
			// aliens start with health at zero
			// when they get hit by a player bullet the health counts up
			// depending on what level of health they have determines what image is drawn as they die
			for(y = 0; y < INVASIONWIDTH; y++){
				for(x = 0; x < currentgamelevel; x++){
					if((invasion[y][x].health > 0)){
						invasion[y][x].health++;
					}
				}
			}

			// check if the player has no lives left
			if((lives == 0) && (cheatmode == 0)){
				if(drawgameoverscreen(1, &score)){
					currentgamelevel = 1;
					goto newgame;
				}else{
					endwin();
					return 0;
				}
			}

			if(cheatmode){
				lives = 5;
			}
			//
			// start drawing everything on the screen
			erase();
			drawborder();
			attron(COLOR_PAIR(SCORE_COLOR));
			mvprintw(0, 2, " Score = %d ", score);
			mvprintw(0, 17, " Lives = %d ", lives);
			attroff(COLOR_PAIR(SCORE_COLOR));

			// print a "O" in the top left corner if cheatmode is active
			if(cheatmode){
				attron(COLOR_PAIR(BORDER_COLOR));
				mvprintw(0, 0, "O");
				attroff(COLOR_PAIR(BORDER_COLOR));
			}

			drawplayer(&playership);

			// draw the invasion and if there are no remaining aliens then goto next level
			if (!drawinvasion(invasion, &invasiondirection, currentgamelevel)){
				if(gotonextlevel(&score, &currentgamelevel)){
					goto newgame;
				}
				endwin();
				return 0;
			}

			drawblocks(blocks);
			
			// reset the two timers and have them match each other
			clock_gettime(CLOCK_MONOTONIC, &moverecenttime);
			moveprevioustime.tv_sec = moverecenttime.tv_sec;
			moveprevioustime.tv_nsec = moverecenttime.tv_nsec;

		}

		// have the aliens shoot bullets at the player
		for(x = 0; x < INVASIONWIDTH; x++){
			for(y = 0; y < currentgamelevel; y++){
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
				playership.pos++;

				// if the player has gone past the right hand wall then put the playership back where they were
				if(playership.pos > COLS - 8){
					playership.pos--;
					break;
				}

				drawborder();
				attron(COLOR_PAIR(SCORE_COLOR));
				mvprintw(0, 2, " Score = %d ", score);
				mvprintw(0, 17, " Lives = %d ", lives);
				attroff(COLOR_PAIR(SCORE_COLOR));

				drawplayer(&playership);

				break;

			case KEY_LEFT:
				playership.pos--;

				// if the player has gone past the left or right hand wall then put the playership back where they were
				if(playership.pos <= 0){
					playership.pos++;
					break;
				}

				drawborder();
				attron(COLOR_PAIR(SCORE_COLOR));
				mvprintw(0, 2, " Score = %d ", score);
				mvprintw(0, 17, " Lives = %d ", lives);
				attroff(COLOR_PAIR(SCORE_COLOR));
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
				attron(COLOR_PAIR(SCORE_COLOR));
				mvprintw(0, 2, " Score = %d ", score);
				mvprintw(0, 17, " Lives = %d ", lives);
				attroff(COLOR_PAIR(SCORE_COLOR));
				drawplayer(&playership);

				playerbullet.y = LINES - 5;
				drawplayerbullet(&playerbullet, playership.pos);
	
				break;

				// turn cheatmode on
			case 'g':
				cheatmode = 1;
				break;
				// turn cheatmode off
			case 'G':
				cheatmode = 0;
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

	attron(COLOR_PAIR(BORDER_COLOR));

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

	attroff(COLOR_PAIR(BORDER_COLOR));

        return 0;
}

int drawplayer(spaceship *playership){
	int a, b;
	int topofship = LINES - 4;

	// draw one character at a time with two for loops
        attron(COLOR_PAIR(PLAYER_COLOR));
	for(a = 0; a < 3; a++){
		for(b = 0; b < 7; b++){
			mvprintw(topofship + a, playership->pos + b, "%c", playership->healthy[a][b]);
		}
	}
        attroff(COLOR_PAIR(PLAYER_COLOR));

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

	if(playerbullet->y < 1){
		return 0;
	}

	// draw two bullets, it looks better
        attron(COLOR_PAIR(PLAYER_COLOR));
	mvprintw(playerbullet->y, playerbullet->x, "|");
	mvprintw(playerbullet->y + 1, playerbullet->x, "|");
        attroff(COLOR_PAIR(PLAYER_COLOR));

	return 0;
}

int drawalienbullet(spaceship invasion[][INVASIONHEIGHT], bullet *alienbullet){
	
	// if the bullet is still on the screen then simply move it down one space
	if(alienbullet->new == false){
		alienbullet->y++;
	}
	
        attron(COLOR_PAIR(ALIEN_COLOR));
	mvprintw(alienbullet->y, alienbullet->x, "|");

	// draw a second '|' above the bullet
	mvprintw(alienbullet->y - 1, alienbullet->x, "|");
        attroff(COLOR_PAIR(ALIEN_COLOR));

	return 0;
}

bool isplayerhitbybullet(spaceship *playership, bullet *alienbullet){
	int a, b;
	// x is the top of the player ship
	int topofship = LINES - 4;

	for(a = 0; a < 3; a++){
		for(b = 0; b < 7; b++){
			if((topofship + a == alienbullet->y) && (playership->pos + b == alienbullet->x)){

				// if the player ship has been hit then set alien bullet to -1 for y axis so it doesn't show on the screen
				alienbullet->x = -1;
				alienbullet->y = LINES + 2;
				
				// return 1 which is true because there has been a collission
				return true;
			}
		}
	}
	return false;
}


int initinvasion(spaceship invasion[][INVASIONHEIGHT]){
	int a, b;

	for(a = 0; a < INVASIONWIDTH; a++){
		for(b = 0; b < INVASIONHEIGHT; b++){
			// what to draw when the alien is healthy
			strncpy(invasion[a][b].healthy[0], "  ^ ^  ", 7);
			strncpy(invasion[a][b].healthy[1], "  0 0  ", 7);
			strncpy(invasion[a][b].healthy[2], " -___- ", 7);

			// the alien has been hit and is in the process of dying
			strncpy(invasion[a][b].dying[0], "  \\ /  ", 7);
			strncpy(invasion[a][b].dying[1], "-- *--  ", 7);
			strncpy(invasion[a][b].dying[2], "  / \\   ", 7);

			// the last scene of the alien dying
			strncpy(invasion[a][b].dead[0], "  ..   ", 7);
			strncpy(invasion[a][b].dead[1], ".:  :. ", 7);
			strncpy(invasion[a][b].dead[2], "  ..   ", 7);
			invasion[a][b].health = 0;
			invasion[a][b].y = 2 + (3 * b);
			invasion[a][b].x = 9 + (7 * a);
		}
	}

	return 0;

}

// returns an integer indicating the next direction that the invasion will move
int changeinvasiondirection(spaceship invasion[][INVASIONHEIGHT], int *invasiondirection, int currentgamelevel){
	int a, b;

	for(a = 0; a < INVASIONWIDTH; a++){
		for(b = 0; b < currentgamelevel; b++){

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
			if((invasion[a][b].x == 1) && (*invasiondirection == 2) && (invasion[a][b].health < 3)){
				*invasiondirection = 1;
				return 1;
			}

			if((invasion[a][b].x == 1) && (*invasiondirection == 1)&& (invasion[a][b].health < 3)){
				*invasiondirection = 0;
				return 0;
			}


			if((invasion[a][b].x == 1) && (*invasiondirection == 0) && (invasion[a][b].health < 3)){
				*invasiondirection = 3;
				return 3;
			}

			if((invasion[a][b].x == COLS - 8) && (*invasiondirection == 3) && (invasion[a][b].health < 3)){
				*invasiondirection = 4;
				return 4;
			}

			if((invasion[a][b].x == COLS - 8) && (*invasiondirection == 4) && (invasion[a][b].health < 3)){
				*invasiondirection = 5;
				return 5;
			}

			if((invasion[a][b].x == COLS - 8) && (*invasiondirection == 5) && (invasion[a][b].health < 3)){
				*invasiondirection = 2;
				return 2;
			}
			
		}
	}	
	// this should never be reached 
	return *invasiondirection;
}

// Takes an argument which is an int and indicates the direction
// The argument invasiondirection gets set by the function changeinvasiondirection()
int moveinvasion(spaceship invasion[][INVASIONHEIGHT], spaceship *playership, int *invasiondirection, int currentgamelevel){
	int a, b;

	for(a = 0; a < INVASIONWIDTH; a++){
		for(b = 0; b < currentgamelevel; b++){


			// if one of the aliens has reached the level of the screen that is the top of the 
			// player ship then return 1
			// this means that the aliens have rached Earth
			if((invasion[a][b].y == LINES - 7) && invasion[a][b].health == 0){
				return 1;
				break;
			}

			switch(*invasiondirection){
				// each switch statement moves the ship according to the invasiondirection variable
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

int drawinvasion(spaceship invasion[][INVASIONHEIGHT], int *invasiondirection, int currentgamelevel){
	int counter;
	int x, y;
	int a, b;
	int numberofinvasionspaceships = 0;

	attron(COLOR_PAIR(ALIEN_COLOR));

	for(y = 0; y < currentgamelevel; y++){
		for(x = 0; x < INVASIONWIDTH; x++){
			for(a = 0; a < 3; a++){
				for(b = 0; b < 7; b++){

					switch(invasion[x][y].health){
						// the reason for the empty switch statements is a hack to create a delay so you actually see
						// the ship change as it dies
						// otherwise it would all be over in 3 frames
						//
						// this works because the alien health is increased once per frame once the alien gets his by a 
						// player bullet, this is done in the main()function
						//
						// so each time this function is run the alien.healthy integer has been increased and the image that
						// is drawn changes appropriately
						case 0:
							mvprintw(invasion[x][y].y + a, invasion[x][y].x + b, "%c", invasion[x][y].healthy[a][b]);
							numberofinvasionspaceships++;
							break;
						case 1:
						case 2:
						case 3:
						case 4:
							mvprintw(invasion[x][y].y + a, invasion[x][y].x + b, "%c", invasion[x][y].dying[a][b]);
							numberofinvasionspaceships++;
							break;
						case 5:
						case 6:
						case 7:
						case 8:
							mvprintw(invasion[x][y].y + a, invasion[x][y].x + b, "%c", invasion[x][y].dead[a][b]);
							numberofinvasionspaceships++;
							break;
					}
				}
			}
		}
	}

	attroff(COLOR_PAIR(ALIEN_COLOR));

	return numberofinvasionspaceships;
}

void drawintroscreen(){
        clear();
        drawborder();

        attron(COLOR_PAIR(PLAYER_COLOR));
        printincentreofscreen(2, "Space Invaders");
        printincentreofscreen(3, "Prepare to save Earth !");
        printincentreofscreen(5, "Hit any key to start");
        printincentreofscreen(7, "coded by Hatchet");
        printincentreofscreen(8, "June 2024");

        attroff(COLOR_PAIR(PLAYER_COLOR));

        // turn off nodelay before calling getch
        // to ensure that the play has to hit a key and we have the delay
        nodelay(stdscr, FALSE);
        getch();
        nodelay(stdscr, TRUE);

        clear();
        return;
}

int drawgameoverscreen(int code, int *score){
        int inputchar;

        switch(code){
                // the player has zero lives left
                case 1:
                        clear();
                        drawborder();

        		attron(COLOR_PAIR(PLAYER_COLOR));
                        printincentreofscreen(2, "Game Over");
                        printincentreofscreen(3, "Zero lives left");
                        printincentreofscreen(5, "Nice try, better luck next time... ");
        		attroff(COLOR_PAIR(PLAYER_COLOR));

                        attron(COLOR_PAIR(SCORE_COLOR));
                        printincentreofscreenwithnumber(7, "Your score is ", score);
                        attroff(COLOR_PAIR(SCORE_COLOR));

        		attron(COLOR_PAIR(PLAYER_COLOR));
                        printincentreofscreen(9, "Want to play again?");
                        printincentreofscreen(10, "Hit 'Y' or 'N'");
        		attroff(COLOR_PAIR(PLAYER_COLOR));

                        refresh();

                        while(1){
                                nodelay(stdscr, FALSE);
                                inputchar = getch();

                                switch(inputchar){
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

        		attron(COLOR_PAIR(PLAYER_COLOR));
                        printincentreofscreen(2, "Game Over");
                        printincentreofscreen(3, "The aliens reached Earth");
                        printincentreofscreen(5, "Nice try, better luck next time... ");
        		attroff(COLOR_PAIR(PLAYER_COLOR));

                        attron(COLOR_PAIR(SCORE_COLOR));
                        printincentreofscreenwithnumber(7, "Your score is ", score);
                        attroff(COLOR_PAIR(SCORE_COLOR));

                        attron(COLOR_PAIR(PLAYER_COLOR));
                        printincentreofscreen(9, "Want to play again?");
                        printincentreofscreen(10, "Hit 'Y' or 'N'");
                        attroff(COLOR_PAIR(PLAYER_COLOR));

                        refresh();

                        while(1){
                                nodelay(stdscr, FALSE);
                                inputchar = getch();

                                switch(inputchar){
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

int gotonextlevel(int *score, int *currentgamelevel){
        int inputchar;
	int nextlevel = (*currentgamelevel + 1);

        switch(*currentgamelevel){
                case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
                        clear();
                        drawborder();

                        attron(COLOR_PAIR(PLAYER_COLOR));
                        printincentreofscreen(2, "Level Success !  ");
                        printincentreofscreen(3, "Zero aliens left");
                        printincentreofscreenwithnumber(7, "Your score is ", score);
                        printincentreofscreenwithnumber(9, "ready for level ", &nextlevel);
                        printincentreofscreen(10, "Hit 'n' for next level");
                        attroff(COLOR_PAIR(PLAYER_COLOR));

                        refresh();

                        while(1){
                                nodelay(stdscr, FALSE);
                                inputchar = getch();

                                switch(inputchar){
                                        case 'n':
                                        case 'N':
                                                clear();
						// increment the value that *currentgamelevel is pointing to
						(*currentgamelevel)++;
                                		nodelay(stdscr, TRUE);
						return *currentgamelevel;
					default:
						break;
                        	}
			}

			break;
		case 15:
                        clear();
                        drawborder();

                        attron(COLOR_PAIR(PLAYER_COLOR));
                        printincentreofscreen(2, "You win, you saved Earth from those nasty aliens  ");
                        printincentreofscreen(3, "Zero aliens left");
                        printincentreofscreenwithnumber(7, "Your score is d", score);
                        printincentreofscreen(10, "Want to play again?");
                        printincentreofscreen(11, "Hit 'Y' or 'N'");
                        attroff(COLOR_PAIR(PLAYER_COLOR));



                        refresh();

                        while(1){
                                nodelay(stdscr, FALSE);
                                inputchar = getch();

				// change *currentgamelevel to 1 if the player plays again
				// or make it 0 if they want to end the game
                                switch(inputchar){
                                        case 'y':
                                        case 'Y':
                                                clear();
                                		nodelay(stdscr, TRUE);
						*currentgamelevel = 1;
                                                return 1;

                                        case 'n':
                                        case 'N':
                                        case 'q':
                                        case 'Q':
						*currentgamelevel = 0;
                                                return 0;

					default:
						break;
                        	}
			}
	}
        return 0;
}

int printincentreofscreen(int linetoprinton, char *stringtoprint){
	int stringlength = 0;
	int x;
	int indentfromleft= 0;

	stringlength = strlen(stringtoprint);

	// if stringtoprint is longer than the screen width then print a line of "X"
	if(stringlength > COLS - 1){
		for(x = 0; x < COLS; x++){
			mvprintw(indentfromleft, x, "X");
		}
		return 1;
	}

	indentfromleft = (COLS / 2) - (stringlength / 2);
	mvprintw(linetoprinton, indentfromleft, "%s", stringtoprint);

	return stringlength;
}

int printincentreofscreenwithnumber(int linetoprinton, char *stringtoprint, int *number){
	int stringlength = 0;
	int x;
	int indentfromleft = 0;

	stringlength = strlen(stringtoprint);

	// if stringtoprint is longer than the screen width then print a line of "X"
	if(stringlength > COLS - 1){
		for(x = 0; x < COLS; x++){
			mvprintw(linetoprinton, x, "X");
		}
		return 1;
	}

	indentfromleft = (COLS / 2) - (stringlength / 2);
	mvprintw(linetoprinton, indentfromleft, "%s%d", stringtoprint, *number);

	return stringlength;
}

bool isalienhitbybullet(spaceship invasion[][INVASIONHEIGHT], int *score, bullet *playerbullet, int currentgamelevel){
	int x, y;
	int a, b;
	
	// this loop steps through eachship in the invasion and then each character of each ship that is still alive
	// if the ship is still alive then each character in the ship is compared with the location of the player bullet then:
	// the alienship has its health increased (which means it is now going to die)
	// the score is increased
	// the player bullet is moved off screen
	for(y = 0; y < INVASIONWIDTH; y++){
		for(x = 0; x < currentgamelevel; x++){
			for(a = 0; a < 3; a++){
				for(b = 0; b < 7; b++){
					if(invasion[y][x].health < 3){
						if((invasion[y][x].y + a == playerbullet->y) && (invasion[y][x].x + b == playerbullet->x)){
							invasion[y][x].health++;
							*score++;
							playerbullet->x = -1;
							playerbullet->y = -1;
						}
					}
				}
			}
		}
	}
	return 0;
}

int initblocks(blockarray blocks[][BLOCKSWIDTH]){
	int y, x;

	
	return 0;
}

int drawblocks(blockarray blocks[][BLOCKSWIDTH]){
	return 0;
}

int hasbullethitblocks(blockarray blocks[][BLOCKSWIDTH], bullet *playerbullet, bullet *alienbullet){
	return 0;
}


