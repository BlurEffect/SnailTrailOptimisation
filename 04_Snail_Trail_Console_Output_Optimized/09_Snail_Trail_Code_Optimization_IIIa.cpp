/*
09_Snail_Trail_Code_Optimization_IIIa
Kevin Meergans, 2013, based on the snail trail game by A. Oram
This continues to improve the performance of the game by optimising c++ code. This includes improvement of the 
initialisation part.
Changes (copied from report):
•	Improved the initialisation part by restructuring code and getting rid of some calls to “rand”. For that reason, 
	I had to record keys with the new version, as the game would no longer play the same with stuff placed at 
	different random positions. Although I ensured that every possible event occurred in the new recorded games, 
	the times achieved might not be fully comparable to those of earlier versions.
•	Speeded up the set garden code by getting rid of one for-loop and using “memset” to fill the rows of the garden, 
	or great parts of them, with one call each.
•	Running out of obvious speedups, I experimented with some different ways to store snail and frog positions like 
	creating structs for snail and frogs or going back to store all variables on their own, instead of in arrays. 
	Additionally, I tested whether it would be faster to have if-else statements instead of the switch-statement in 
	the move snail code. None of these things brought any measurable speedup, so I mostly reverted those changes.
*/

//---------------------------------
//include libraries
//include standard libraries
#include <iostream >         //for output and input
#include <iomanip>           //for formatted output in 'cout'
#include <conio.h>           //for getch
#include <fstream>           //for files
#include <sstream>
#include <queue>
#include <string>            //for string
#include "hr_time.h"         //for timers

using namespace std;

//include our own libraries
#include "RandomUtils.h"     //for Seed, Random,
#include "ConsoleUtils.h"    //for Clrscr, Gotoxy, etc.
#include "TimeUtils.h"       //for GetTime, GetDate, etc.

// global constants

// garden dimensions
const int SIZEY(20);						// vertical dimension
const int SIZEX(30);						// horizontal dimension




//constants used for the garden & its inhabitants
const char BLANK(' ');						// open space
const char PELLET ('-'); //(BLANK);			// should be blank) but test using a visible character.
const char LETTUCE ('@');					// a lettuce
const char SLIME ('.');						// snail produce
const char WALL('+');                       // garden wall
const char FROG ('M');
const char DEAD_FROG_BONES ('X');			// Dead frogs are marked as such in their 'y' coordinate
const char SNAIL('&');						// snail (player's icon)
const char DEADSNAIL ('o');					// just the shell left...

const int  SLIMELIFE (25);					// how long slime lasts (in keypresses)
const int  NUM_PELLETS (15);				// number of slug pellets scattered about
const int  PELLET_THRESHOLD (5);			// deadly threshold! Slither over this number and you die!
const int  LETTUCE_QUOTA (4);				// how many lettuces you need to eat before you win.
const int  NUM_FROGS (2);
const int  FROGLEAP (4);					// How many spaces do frogs jump when they move
const int  EagleStrike (30);				// There's a 1 in 'nn' chance of an eagle strike on a frog

// the keyboard arrow codes
const int UP    (72);						// up key
const int DOWN  (80);						// down key
const int RIGHT (77);						// right key
const int LEFT  (75);						// left key

// other command letters
const char QUIT('q');						//end the game

const int MLEFT (SIZEX+5);					//define left margin for messages (avoiding garden)

// needed for output with WriteConsoleOutput in paintgarden
const COORD OUTPUT_BUFFER_START = {0, 0}; 
const COORD OUTPUT_BUFFER_SIZE = {SIZEX, SIZEY};

// run through the prerecorded 120 main loop iterations this many times
const int numberOfCycles(1000000);

// read in keys from this array, faster than reading from file
// -> input operation does not pollute measurement to a greater extent 
const int keys[360] = {3,3,3,3,0,0,0,0,0,0,0,0,0,0,3,3,3,0,2,2,2,2,2,2,0,0,0,0,0,0,0,2,2,2,0,0,0,2,2,2,1,1,1,2,0,2,2,2,2,0,0,2,0,2,2,2,2,2,2,0,0,2,1,1,1,1,3,3,2,0,2,2,2,1,2,2,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,3,1,1,1,1,4,4,3,1,1,1,1,1,1,1,1,1,3,3,3,1,3,0,3,3,3,3,3,3,0,3,0,0,0,0,0,0,2,0,0,0,0,0,0,0,2,2,0,2,2,2,2,2,2,2,1,1,1,2,2,2,0,0,0,0,0,0,0,0,0,0,0,2,2,0,2,2,2,3,0,3,0,0,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,0,3,3,3,0,0,3,3,3,1,3,1,1,1,3,3,3,3,0,0,3,0,0,0,0,0,0,0,0,2,0,2,2,2,2,2,2,1,2,2,1,1,1,2,1,1,2,1,1,1,0,1,1,1,1,0,0,2,2,2,1,2,2,2,2,1,1,1,1,1,1,2,2,2,1,1,3,3,1,1,1,1,1,1,1,1,1,2,2,0,0,0,0,2,0,0,0,0,0,3,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,3,0,0,3,2,3,3,3,5,5};

// all possible messages
const char* messages[13] = {"READY TO SLITHER!? PRESS A KEY...", 
								"TOO MANY PELLETS SLITHERED OVER!",
								"LAST LETTUCE EATEN",
								"LETTUCE EATEN",
								"TRY A DIFFERENT DIRECTION",
								"THAT'S A WALL!",
								"OOPS! ENCOUNTERED A FROG!",
								"INVALID KEY",
								"FROG GOT YOU!",
								"EAGLE GOT A FROG",
								"WELL DONE, YOU'VE SURVIVED",
								"REST IN PEAS.",
								""};

// all possible move "vectors" for the snail
const int moveDirections[4][2] = {{0,-1},{0,1},{-1,0},{1,0}};

// required for reading keys from the keys array (does not belong to the actual game)
unsigned int keyCount(0);
// the count of the current main loop iteration within the current cycle (does not belong to the actual game)
unsigned int iterationCount(0);

double accumulatedFramerates(0.0);
long long frameCount(0LL);

// Start of the 'SNAIL TRAIL' listing
//---------------------------------
int main()
{
	//function prototypes
	void paintGame(int, int, const char [][SIZEX]);
	int getKeyPress();
	void showFrameRate(double);

	//local variables
	//arrays that store ...
	char garden		  [SIZEY][SIZEX];		// the game 'world'

	bool isSnailAlive(true);

	// the position of the snail
	// [0] - y coordinate
	// [1] - x coordinate
	int snail[2];
	
	//int snailY;
	//int snailX;
	

	// keeps track of whether frogs are currently sitting on lettuces or not
	bool lettucesBlocked[2] = {false, false};

	// holds frog positions
	// [0] - y coordinate of frog 1
	// [1] - x coordinate of frog 1
	// [2] - y coordinate of frog 2
	// [3] - x coordinate of frog 2
	int frogs[4];


	// holds the position of each slime ball
	int slimeTrail[SLIMELIFE][2];

	int counters[4] = {0,0,0,0}; // hold message ID, slime counter, count pellets eaten and lettuces eaten

	// key can have the following values
	// 0 - Left
	// 1 - Right
	// 2 - Up
	// 3 - Down
	// 4 - Other
	// 5 - Quit
	int  key(4);
		
	CStopWatch s;							// create a stopwatch for timing
	

	for(int i = 0; i < numberOfCycles; ++i)
	{
		// Now start the game...

		//Seed();									//seed the random number generator
		srand(256); 

		while (key != 5)		// keep playing games
		{	
			//Clrscr();

			/**********************************************************************************************
			Initialisation 
			***********************************************************************************************/
			
			//------------------------------------------------------------------------------
			// initialise slime trail
			
			for(int i = 0; i < SLIMELIFE; ++i)
			{
				slimeTrail[i][0] = -1;
				slimeTrail[i][1] = -1;
			}

			//-----------------------------------------------------------------------------------
			// set garden

			// got completely rid of the inner loop and the branching within the loop
			memset(&garden[0][0], WALL, SIZEX);
			for (int row(1); row < SIZEY - 1; ++row)			
			{	
				garden[row][0] = WALL;
				memset(&garden[row][1], BLANK, SIZEX-2);
				garden[row][SIZEX - 1] = WALL;
			}
			memset(&garden[SIZEY - 1][0], WALL, SIZEX);

			//-------------------------------------------------------------------------------------
			// place snail

			// set snail initial coordinates
			snail[0] = (rand() % (SIZEY-2)) + 1;		// vertical coordinate in range [1..(SIZEY - 2)]
			snail[1] = (rand() % (SIZEX-2)) + 1;		// horizontal coordinate in range [1..(SIZEX - 2)]

			garden[snail[0]][snail[1]] = SNAIL;

			//--------------------------------------------------------------------------------------
			// scatter pellets
			int	y = (0);
			int x = (0);

			for (int slugP=0; slugP < NUM_PELLETS; ++slugP)								// scatter some slug pellets...
			{	
				do
				{
					x = ((rand() % (SIZEX-2)) + 1);
					y = ((rand() % (SIZEY-2)) + 1);
				}while(((y == snail[0]) && (x == snail[1])) || garden [y][x] == PELLET); // avoid snail and other pellets

				garden [y][x] = PELLET;
			}
	
			//---------------------------------------------------------------------------------
			// scatter lettuces

			do
			{
				y = ((rand() % (SIZEY-2)) + 1);
				x = ((rand() % (SIZEX-2)) + 1);
						  // avoid snail, pellets and other lettucii
			}while(((y == snail[0]) && (x == snail[1])) || garden [y][x] == PELLET); // no need to check for lettuces already
			 
			garden [y][x] = LETTUCE;

			do
			{
				y = ((rand() % (SIZEY-2)) + 1);
				x = ((rand() % (SIZEX-2)) + 1);
						  // avoid snail, pellets and other lettucii
			}while(((y == snail[0]) && (x == snail[1])) || garden [y][x] == PELLET || garden [y][x] == LETTUCE);
			 
			garden [y][x] = LETTUCE;

			do
			{
				y = ((rand() % (SIZEY-2)) + 1);
				x = ((rand() % (SIZEX-2)) + 1);
						  // avoid snail, pellets and other lettucii
			}while(((y == snail[0]) && (x == snail[1])) || garden [y][x] == PELLET || garden [y][x] == LETTUCE);
			 
			garden [y][x] = LETTUCE;

			do
			{
				y = ((rand() % (SIZEY-2)) + 1);
				x = ((rand() % (SIZEX-2)) + 1);
						  // avoid snail, pellets and other lettucii
			}while(((y == snail[0]) && (x == snail[1])) || garden [y][x] == PELLET || garden [y][x] == LETTUCE);
			 
			garden [y][x] = LETTUCE;

			//-------------------------------------------------------------------------------
			//scatter frogs

			// unrolled for loop

			// frog 1
			do
			{
				frogs[0] = ((rand() % (SIZEY-2)) + 1);
				frogs[1] = ((rand() % (SIZEX-2)) + 1);
			}while((frogs[0] == snail[0]) && (frogs[1] == snail[1]));  // avoid snail (no need to avoid other frog yet)

			//frog 2
			do
			{
				frogs[2] = ((rand() % (SIZEY-2)) + 1);
				frogs[3] = ((rand() % (SIZEX-2)) + 1);
			}while(((frogs[2] == snail[0]) && (frogs[3] == snail[1])) || ((frogs[2] == frogs[0]) && (frogs[3] == frogs[1])));  // avoid snail and existing frog
			
			lettucesBlocked[0] = garden[frogs[0]][frogs[1]] == LETTUCE; // frog 1 is currently blocking a lettuce
			garden [frogs[0]][frogs[1]] = FROG;		// put frog 1 on garden (this may overwrite a slug pellet)

			lettucesBlocked[1] = garden[frogs[2]][frogs[3]] == LETTUCE; // frog 2 is currently blocking a lettuce
			garden [frogs[2]][frogs[3]] = FROG;		// put frog 2 on garden (this may overwrite a slug pellet)

			//------------------------------------------------------------------------------
			// further initialising

			counters[1] = 0;
			counters[2] = 0;
			counters[3] = 0;
			counters[0] = 0;

			isSnailAlive = true;

			//paintGame( counters[0], counters[2], garden);			//display game info, garden & messages

			key = keys[keyCount++];//getKeyPress();							//get started or quit game

			/************************************************************************************************
			Game loop
			*************************************************************************************************/

			while ((key != 5) && isSnailAlive && counters[3] != LETTUCE_QUOTA)	//user not bored, and snail not dead or full (main game loop)
			{
				s.startTimer(); // not part of game
				// ************** code to be timed ***********************************************
				// now, everything is timed (including initialisation)
				
				if(key != 4)	// only move the snail if an arrow key was pressed
				{	
					/*********************************************************************************
					Move the snail
					**********************************************************************************/
					
					char temp = garden[snail[0] + moveDirections[key][0]][snail[1] + moveDirections[key][1]];
					if(temp == BLANK || temp == PELLET || temp == LETTUCE || temp == DEAD_FROG_BONES)
					{
						garden[snail[0]][snail[1]] = SLIME;	//lay a trail of slime

						slimeTrail[counters[1]][0] = snail[0];
						slimeTrail[counters[1]][1] = snail[1];
			
						snail[0] += moveDirections[key][0];	//go in direction indicated by keyMove
						snail[1] += moveDirections[key][1];

						if(temp == PELLET)
						{
							//printf("\a");									// produce a warning sound
							if (++counters[2] >= PELLET_THRESHOLD)				// aaaargh! poisoned!
							{	
								counters[0] = 1;
								//printf("\a\a\a\a");							// produce a death knell
								isSnailAlive = false;
							}
						}
						if(temp == LETTUCE)
						{
							if(++counters[3] != LETTUCE_QUOTA)
							{
								counters[0] = 3;
								//printf("\a");	
							}
							else
							{
								counters[0] = 2;
								//printf("\a\a\a\a\a\a\a");	
							}
						}
						// place snail (move snail in garden)
						garden[snail[0]][snail[1]] = SNAIL;
					}else if(temp == WALL)
					{
							//printf("\a");		//produce a warning sound
							counters[0] = 5;
					}else if(temp == SLIME)
					{
						counters[0] = 4;			// no need to place snail for slime and wall (no movement)
					}else
					{
						// frog
						garden[snail[0]][snail[1]] = SLIME;				// lay a final trail of slime
						snail[0] += moveDirections[key][0];							// go in direction indicated by keyMove
						snail[1] += moveDirections[key][1];
						counters[0] = 6;
						//printf("\a\a\a\a");								// produce a death knell
						isSnailAlive = false;
						// place snail (move snail in garden)
						garden[snail[0]][snail[1]] = SNAIL;
					}
					
					
					/*
					switch( garden[snail[0] + moveDirections[key][0]][snail[1] + moveDirections[key][1]]) //depending on what is at target position
					{
						case BLANK:
							{
							garden[snail[0]][snail[1]] = SLIME;				//lay a trail of slime

							slimeTrail[counters[1]][0] = snail[0];
							slimeTrail[counters[1]][1] = snail[1];
			
							snail[0] += moveDirections[key][0];							//go in direction indicated by keyMove
							snail[1] += moveDirections[key][1];
							break;
							}
						case PELLET:		// increment pellet count and kill snail if > threshold
							{
							garden[snail[0]][snail[1]] = SLIME;				// lay a trail of slime

							slimeTrail[counters[1]][0] = snail[0];
							slimeTrail[counters[1]][1] = snail[1];
			
							snail[0] += moveDirections[key][0];							// go in direction indicated by keyMove
							snail[1] += moveDirections[key][1];
							//++counters[2];
							//printf("\a");									// produce a warning sound
							if (++counters[2] >= PELLET_THRESHOLD)				// aaaargh! poisoned!
							{	
								counters[0] = 1;
								//message = "TOO MANY PELLETS SLITHERED OVER!";
								//printf("\a\a\a\a");							// produce a death knell
								//snailStillAlive = false;					// game over
								isSnailAlive = false;
							}
							break;
							}
						case LETTUCE:		// increment lettuce count and win if snail is full
							{
							garden[snail[0]][snail[1]] = SLIME;				//lay a trail of slime

							slimeTrail[counters[1]][0] = snail[0];
							slimeTrail[counters[1]][1] = snail[1];
			
							snail[0] += moveDirections[key][0];							//go in direction indicated by keyMove
							snail[1] += moveDirections[key][1];

							//++counters[3];								// keep a count

							if(++counters[3] != LETTUCE_QUOTA)
							{
								counters[0] = 3;
								//message = "LETTUCE EATEN";
								//printf("\a");	
							}
							else
							{
								counters[0] = 2;
								//message = "LAST LETTUCE EATEN";
								//printf("\a\a\a\a\a\a\a");	
							}

							break;
							}
						case SLIME:
							{
								counters[0] = 4;
								//message = "TRY A DIFFERENT DIRECTION";
							}
						case WALL:				//oops, garden wall
							//printf("\a");		//produce a warning sound
							counters[0] = 5;
							//message = "THAT'S A WALL!";	
							break;				//& stay put
						case DEAD_FROG_BONES:		//its safe to move over dead/missing frogs too
							{
							garden[snail[0]][snail[1]] = SLIME;				//lay a trail of slime

							slimeTrail[counters[1]][0] = snail[0];
							slimeTrail[counters[1]][1] = snail[1];
			
							snail[0] += moveDirections[key][0];							//go in direction indicated by keyMove
							snail[1] += moveDirections[key][1];
							break;
							}
						case FROG:			//	kill snail if it throws itself at a frog!
							{
							garden[snail[0]][snail[1]] = SLIME;				// lay a final trail of slime
							snail[0] += moveDirections[key][0];							// go in direction indicated by keyMove
							snail[1] += moveDirections[key][1];
							counters[0] = 6;
							//message = "OOPS! ENCOUNTERED A FROG!";
							//printf("\a\a\a\a");								// produce a death knell
							//snailStillAlive = false;						// game over
							isSnailAlive = false;
							break;
							}
					}*/
					
					
				}else
				{
					counters[0] = 7;
					//message = "INVALID KEY";
				}
				/*********************************************************************************
				Dissolve the slime
				**********************************************************************************/
				
				counters[1] = (++counters[1]) % SLIMELIFE;
				if(slimeTrail[counters[1]][0] != -1)
				{
					garden[slimeTrail[counters[1]][0]][slimeTrail[counters[1]][1]] = BLANK;
					slimeTrail[counters[1]][0] = -1;
				}

				/*********************************************************************************
				Move the frogs
				**********************************************************************************/

				// frog 1
				if ((frogs[0] != -1) && isSnailAlive)		// if frog not been gotten by an eagle or GameOver
				{	
					// jump off garden (taking any slug pellet with it)... check it wasn't on a lettuce though...

					if (!lettucesBlocked[0])
					{
						garden [frogs[0]][frogs[1]] = BLANK;
					}else  
					{
						garden [frogs[0]][frogs[1]] = LETTUCE;
						lettucesBlocked[0] = false; // no longer sitting on lettuce
					}
			
					// work out where to jump to depending on where the snail is...
					// see which way to jump in the Y direction (up and down)
		
					
					if (snail[0] - frogs[0] > 0) 
				{frogs[0] += FROGLEAP;  if (frogs[0] >= SIZEY-1) frogs[0]=SIZEY-2;} // don't go over the garden walls!
			else if (snail[0] - frogs[0] < 0) 
				{frogs[0] -= FROGLEAP;  if (frogs[0] < 1) frogs[0]=1;		 };
			
			// see which way to jump in the X direction (left and right)

			if (snail[1] - frogs[1] > 0) 
				{frogs[1] += FROGLEAP;  if (frogs[1] >= SIZEX-1) frogs[1]=SIZEX-2;}
			else if (snail[1] - frogs[1] < 0) 
				{frogs[1] -= FROGLEAP;  if (frogs[1] < 1)	frogs[1]=1;		 };
			


					/*
					// direction is 0, -1 or 1
					int direction = ((snail[0] > frogs[0])) - ((snail[0] < frogs[0]));
					((frogs[0] + FROGLEAP * direction < SIZEY - 1) && (frogs[0] + FROGLEAP * direction >= 1)) ? (frogs[0] += FROGLEAP * direction) : (frogs[0] = 1 + (SIZEY - 3) * (direction > 0));
					
					// see which way to jump in the X direction (left and right)
					// direction is 0, -1 or 1
					direction = ((snail[1] > frogs[1])) - ((snail[1] < frogs[1]));
					((frogs[1] + FROGLEAP * direction < SIZEX - 1) && (frogs[1] - FROGLEAP * direction >= 1)) ? (frogs[1] += FROGLEAP * direction) : (frogs[1] = 1 + (SIZEX - 3) * (direction > 0));
					*/

					// if frog jumps onto a lettuce, remember the lettuce to restore it later
					lettucesBlocked[0] = (garden [frogs[0]][frogs[1]] == LETTUCE);
			

					if (((rand() % EagleStrike) + 1) != EagleStrike)  // not gotten by eagle?		
					{
						if (frogs[0] != snail[0] || frogs[1] != snail[1])	// landed on snail? - grub up!
						{
							garden [frogs[0]][frogs[1]] = FROG;				// display frog on garden (thus destroying any pellet that might be there).
						}
						else 
						{
							counters[0] = 8;
							//message = "FROG GOT YOU!";
							//printf("\a\a\a\a");									// produce a death knell
							//snailStillAlive = false;								// snail is dead!
							isSnailAlive = false;
						}
					}
					else 
					{
						if(!lettucesBlocked[0])
						{
							garden [frogs[0]][frogs[1]] = DEAD_FROG_BONES;				// show remnants of frog in garden
						}else
						{
							// if the frog was sitting on a lettuce as he was killed, restore the lettuce
							garden [frogs[0]][frogs[1]] = LETTUCE;				// show remnants of frog in garden
						}
			
						frogs[0] = -1;									// and mark frog as deceased
						counters[0] = 9;
						//printf("\a");												//produce a warning sound
					}
				}

				// frog 2
				if ((frogs[2] != -1) && isSnailAlive)		// if frog not been gotten by an eagle or GameOver
				{	
					// jump off garden (taking any slug pellet with it)... check it wasn't on a lettuce though...

					if (!lettucesBlocked[1])
					{
						garden [frogs[2]][frogs[3]] = BLANK;
					}else  
					{
						garden [frogs[2]][frogs[3]] = LETTUCE; 
						lettucesBlocked[1] = false; // no longer sitting on lettuce
					}
			
					// work out where to jump to depending on where the snail is...
					
					if (snail[0] - frogs[2] > 0) 
				{frogs[2] += FROGLEAP;  if (frogs[2] >= SIZEY-1) frogs[2]=SIZEY-2;} // don't go over the garden walls!
			else if (snail[0] - frogs[2] < 0) 
				{frogs[2] -= FROGLEAP;  if (frogs[2] < 1) frogs[2]=1;		 };
			
			// see which way to jump in the X direction (left and right)

			if (snail[1] - frogs[3] > 0) 
				{frogs[3] += FROGLEAP;  if (frogs[3] >= SIZEX-1) frogs[3]=SIZEX-2;}
			else if (snail[1] - frogs[3] < 0) 
				{frogs[3] -= FROGLEAP;  if (frogs[3] < 1)	frogs[3]=1;		 };
				
					// It seems the code below is not working correctly, that's why I brought back the
					// original frog move code above

					/* 
					// direction is 0, -1 or 1
					int direction = ((snail[0] > frogs[2])) - ((snail[0] < frogs[2]));
					((frogs[2] + FROGLEAP * direction < SIZEY - 1) && (frogs[2] + FROGLEAP * direction >= 1)) ? (frogs[2] += FROGLEAP * direction) : (frogs[2] = 1 + (SIZEY - 3) * (direction > 0));
					
					// see which way to jump in the X direction (left and right)
					// direction is 0, -1 or 1
					direction = ((snail[1] > frogs[3])) - ((snail[1] < frogs[3]));
					((frogs[3] + FROGLEAP * direction < SIZEX - 1) && (frogs[3] - FROGLEAP * direction >= 1)) ? (frogs[3] += FROGLEAP * direction) : (frogs[3] = 1 + (SIZEX - 3) * (direction > 0));
					*/

					// if frog jumps onto a lettuce, remember the lettuce to restore it later
					lettucesBlocked[1] = (garden [frogs[2]][frogs[3]] == LETTUCE);

					if (((rand() % EagleStrike) + 1) != EagleStrike)  // not gotten by eagle?					
					{
						if (frogs[2] != snail[0] || frogs[3] != snail[1])	// landed on snail? - grub up!
						{
							garden [frogs[2]][frogs[3]] = FROG;				// display frog on garden (thus destroying any pellet that might be there).
						}
						else 
						{
							counters[0] = 8;
							//printf("\a\a\a\a");									// produce a death knell
							isSnailAlive = false;
						}
					}
					else 
					{

						if(!lettucesBlocked[1])
						{
							garden [frogs[2]][frogs[3]] = DEAD_FROG_BONES;				// show remnants of frog in garden
				
						}else
						{
							// if the frog was sitting on a lettuce as he was killed, restore the lettuce
							garden [frogs[2]][frogs[3]] = LETTUCE;				// show remnants of frog in garden
				
						}

						frogs[2] = -1;									// and mark frog as deceased
						counters[0] = 9;
						//printf("\a");												//produce a warning sound
					}
				}

				/*************************************************************************************
				Paint the game
				**************************************************************************************/

				//paintGame  ( counters[0], counters[2], garden);		// display game info, garden & messages
				counters[0] = 12; // reset message

				
				//*************** end of timed section ******************************************
				// now everything is timed
				s.stopTimer(); // not part of game
				


				//showFrameRate ( s.getElapsedTime());		// display frame rate - not part of game

				if(s.getElapsedTime() > 0.0)
				{
					accumulatedFramerates += s.getElapsedTime();
					++frameCount;
				}
				++iterationCount;

				key = keys[keyCount++];//getKeyPress();						// display menu & read in next option
			}
			
			if (!isSnailAlive)
			{
				// Dead
				garden[snail[0]][snail[1]] = DEADSNAIL;
				counters[0] = 11;
			}else
			{
				// Survived
				counters[0] = 10;
			}

			//paintGame( counters[0], counters[2], garden);			//display final game info, garden & message
		
			// another go
			key = keys[keyCount++];//getKeyPress();
			/*
			SelectBackColour( clRed);
			SelectTextColour( clYellow);
			Gotoxy(MLEFT, 18);
			printf("PRESS 'Q' TO QUIT OR ANY KEY TO CONTINUE");
			SelectBackColour( clBlack);
			SelectTextColour( clWhite);
			*/
		} 
		// finally done

		// reset the iteration count
		iterationCount = 0;

		// Reset game state for the next cycle
		keyCount = 0;
		counters[1] = 0;						
		counters[2] = 0;	
		counters[3] = 0;
		key = 4;
	}

	//s.stopTimer(); // not part of game

	ofstream outFramerates;
	outFramerates.open("Framerates.txt");
	outFramerates << accumulatedFramerates <<"\n" << frameCount;
	outFramerates.close();

	return 0;
} //end main

/******************************************************************************************
Output the game
*******************************************************************************************/

void paintGame (int msgId, int pellets, const char garden[][SIZEX])
{

	void paintGarden( const char [][SIZEX]);
	void showInformation(int, int);

	
	Clrscr();
	paintGarden ( garden);		// display garden contents
	showInformation(pellets, msgId); // show title, options, pellet count, data and time and message

} //end of paintGame

void paintGarden( const char garden[][SIZEX])
{ 
	//display garden content on screen

	// use native windows function WriteConsoleOutput, faster than cout/printf/putch
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms687404%28v=vs.85%29.aspx
	// store garden in buffer and output everything together in one call
	// still filling buffer every time :/
	
	SMALL_RECT OUTPUT_BUFFER_RECT = {0, 2, SIZEX, SIZEY + 1};
	
	CHAR_INFO buffer[600];

	for (int y(0); y < (SIZEY); ++y)		//for each row
	{	
		for (int x(0); x < (SIZEX); ++x)	//for each col
		{			
			// add new CHAR_INFO structure to the buffer
			buffer[x + y * SIZEX].Char.UnicodeChar = garden[y][x];
			// set bright white colour for every character
			buffer[x + y * SIZEX].Attributes = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY;
		}
	}

	WriteConsoleOutput(GetStdHandle(STD_OUTPUT_HANDLE), buffer, OUTPUT_BUFFER_SIZE, OUTPUT_BUFFER_START, &OUTPUT_BUFFER_RECT);
	
} //end of paintGarden

void showInformation(int pellets, int msgId)
{	
	// show title
	SelectBackColour( clBlack);
	SelectTextColour( clYellow);
	Gotoxy(0, 0);
	printf("...THE SNAIL TRAIL...\n");
	
	// show options
	SelectBackColour( clRed);
	SelectTextColour( clYellow);
	Gotoxy(MLEFT, 12);
	printf("TO MOVE USE ARROW KEYS - EAT ALL LETTUCES (%c)", LETTUCE);
	Gotoxy(MLEFT, 13);
	printf("TO QUIT USE 'Q'");

	/*
	// show date and time
	SelectBackColour( clWhite);
	SelectTextColour( clBlack);
	Gotoxy(MLEFT, 1);
	printf("DATE: %s", GetDate());
	Gotoxy(MLEFT, 2);
	printf("TIME: %s", GetTime());
	*/

	//display current message
	SelectBackColour( clBlack);
	SelectTextColour( clWhite);
	Gotoxy(MLEFT, 15);
	printf(messages[msgId]);	

	// show pellet count
	Gotoxy(MLEFT, 17);
	printf("SLITHERED OVER %d PELLETS SO FAR!", pellets);

	//reset message to blank
	//message = "";
}

void showFrameRate ( double timeSecs)
{ // show time for one iteration of main game loop

	SelectBackColour( clBlack);
	SelectTextColour( clWhite);
	Gotoxy(MLEFT, 6);
	printf("FRAME RATE = %.3f at %.3f s/frame", (double) 1.0/timeSecs, timeSecs);
} // end of showFrameRate

/***************************************************************************************
Get user input
****************************************************************************************/

//not called anyway
int getKeyPress()
{
	/*
	int command;

	// read keys from file
	ostringstream oss;

	bool doStop = false;
	while(inKeys.good() && !doStop)
	{
		char ch = inKeys.get();
		if(ch != ',')
		{
			oss << ch;
		}else
		{
			doStop = true;
		}
	}

	if(inKeys.good())
	{
		istringstream iss(oss.str());
		iss >> command;
	}else
	{
		command = 113; // quit
	}
	*/
		
	
	// the old manual input, just for testing
	int command;
	//read in the selected option
	command = _getch();  	// to read arrow keys
	while ( command == 224)	// to clear extra info from buffer
		command = _getch();
	

	return keys[keyCount++];

	// the way keys are used was changed, this is needed to use the new system without having to change/rerecord
	// the keys that are read in
	
	switch(command)
	{
	case DOWN:
		return 3;
	case UP:
		return 2;
	case LEFT:
		return 0;
	case RIGHT:
		return 1;
	case 113:  //Quit
		return 5;
	default:
		return 4;
	}
	
} //end of getKeyPress


// End of the 'SNAIL TRAIL' listing

