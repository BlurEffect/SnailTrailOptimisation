/*
10_Snail_Trail_Code_Optimization_IIIb
Kevin Meergans, 2013, based on the snail trail game by A. Oram
This version was an experiment to change the garden and other data structures from multidimensional to simple arrays.
The problem with this step was that one still has to maintain rows and columns in order to properly move frogs and 
the snail requiring to calculate the row and column from a single integer using modulus and integer division. 
I pre-calculated the row and column values for all 600 garden fields to perform lookups instead of calculations 
but the program as a whole still went slower, or at best about equally as fast, 
as “09_Snail_Trail_Code_Optimization_IIIa.cpp”, which is why I didn’t proceed with that version.
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
const int keys[270] = {0,2,2,0,0,0,0,2,0,0,3,3,1,1,1,1,1,2,1,3,3,0,3,3,3,3,3,0,2,0,0,0,0,0,0,0,0,1,3,3,3,3,1,2,1,1,1,3,1,2,2,2,2,2,1,2,2,2,2,2,2,2,0,2,2,2,2,0,3,0,0,0,2,0,3,0,0,0,0,0,0,0,0,1,1,2,1,1,3,3,1,1,3,3,1,1,4,4,1,1,2,2,2,2,2,2,1,3,1,1,1,1,3,1,1,1,3,3,1,3,0,3,3,1,3,0,0,0,0,2,0,0,0,0,2,1,1,1,1,1,2,2,1,1,2,1,1,2,2,2,2,2,0,0,0,2,1,1,2,1,1,1,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,1,1,3,3,3,3,3,3,1,1,1,1,3,3,3,3,3,3,0,0,0,2,2,2,2,2,2,2,2,0,3,0,2,2,2,2,1,1,1,1,1,1,1,1,1,1,3,3,3,3,3,3,3,3,3,3,3,3,2,1,2,2,1,1,1,2,0,0,0,0,0,3,3,3,3,3,0,1,1,1,5,5};

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
const int moveDirections[4] = {-1,1,-(SIZEX - 2),SIZEX - 2};

// pre-calculated numbers for rows and columns (index of garden array is used as index to these)
const int columns[504] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27};
const int rows[504] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17};

const int GARDENSIZE = 504;

// required for reading keys from the keys array (does not belong to the actual game)
unsigned int keyCount(0);
// the count of the current main loop iteration within the current cycle (does not belong to the actual game)
unsigned int iterationCount(0);

double accumulatedFramerates(0.0);
long long frameCount(0LL);

struct Position
{
	int row;
	int column;
};

// Start of the 'SNAIL TRAIL' listing
//---------------------------------
int main()
{
	
	//function prototypes
	void paintGame(int, int, const char []);
	int getKeyPress();
	void showFrameRate(double);
	int getRandomCoordinates();

	//local variables
	//arrays that store ...
	
	// garden no longer contains the walls
	char garden		  [GARDENSIZE];		// the game 'world' without the walls

	bool isSnailAlive(true);
	
	int snail;
	
	// keeps track of whether frogs are currently sitting on lettuces or not
	bool lettucesBlocked[2] = {false, false};

	// holds frog positions
	// [0] - y coordinate of frog 1
	// [1] - x coordinate of frog 1
	// [2] - y coordinate of frog 2
	// [3] - x coordinate of frog 2
	int frogs[2];

	// holds the position of each slime ball
	int slimeTrail[SLIMELIFE];

	//int	 snailStillAlive (true);				// snail starts alive!

	unsigned int counters[4] = {0,0,0,0}; // hold slime counter, count pellets eaten and lettuces eaten, and message ID

	// key can have the following values
	// 0 - Left
	// 1 - Right
	// 2 - Up
	// 3 - Down
	// 4 - Other
	// 5 - Quit
	unsigned int  key(4);
		
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
				slimeTrail[i] = -1;
			}

			//-----------------------------------------------------------------------------------
			// set garden

			// got completely rid of the inner loop and the branching within the loop
			memset(&garden[0], BLANK, GARDENSIZE);
			
			//-------------------------------------------------------------------------------------
			// place snail

			// set snail initial coordinates
			snail = rand() % GARDENSIZE;

			garden[snail] = SNAIL;

			//--------------------------------------------------------------------------------------

			int index(0);

			for (int slugP=0; slugP < NUM_PELLETS; ++slugP)								// scatter some slug pellets...
			{	
				do
				{
					index = rand() % GARDENSIZE;
				}while((index == snail) || garden [index] == PELLET); // avoid snail and other pellets

				garden [index] = PELLET;
			}
	
			//---------------------------------------------------------------------------------
			// scatter lettuces

			do
			{
				index = rand() % GARDENSIZE;
						  // avoid snail, pellets and other lettucii
			}while((index == snail) || garden [index] == PELLET || garden [index] == LETTUCE);
			 
			garden [index] = LETTUCE;

			do
			{
				index = rand() % GARDENSIZE;
						  // avoid snail, pellets and other lettucii
			}while((index == snail) || garden [index] == PELLET || garden [index] == LETTUCE);
			 
			garden [index] = LETTUCE;

			do
			{
				index = rand() % GARDENSIZE;
						  // avoid snail, pellets and other lettucii
			}while((index == snail) || garden [index] == PELLET || garden [index] == LETTUCE);
			 
			garden [index] = LETTUCE;

			do
			{
				index = rand() % GARDENSIZE;
						  // avoid snail, pellets and other lettucii
			}while((index == snail) || garden [index] == PELLET || garden [index] == LETTUCE);
			 
			garden [index] = LETTUCE;

			//-------------------------------------------------------------------------------
			//scatter frogs

			// unrolled loop

			// frog 1
			do
			{
				frogs[0] = rand() % GARDENSIZE;
			}while(frogs[0] == snail);  // avoid snail (no need to avoid other frog yet)

			//frog 2
			do
			{
				frogs[1] = rand() % GARDENSIZE;
			}while((frogs[1] == snail) || (frogs[1] == frogs[0]));


			lettucesBlocked[0] = garden[frogs[0]] == LETTUCE; // frog 1 is currently blocking a lettuce
			garden [frogs[0]] = FROG;		// put frog 1 on garden (this may overwrite a slug pellet)

			lettucesBlocked[1] = garden[frogs[1]] == LETTUCE; // frog 2 is currently blocking a lettuce
			garden [frogs[1]] = FROG;		// put frog 2 on garden (this may overwrite a slug pellet)

			//------------------------------------------------------------------------------
			// further initialising

			counters[0] = 0;
			counters[1] = 0;
			counters[2] = 0;
			counters[3] = 0;

			isSnailAlive = true;

			//paintGame( counters[3], counters[1], garden);			//display game info, garden & messages

			key = keys[keyCount++];//getKeyPress();							//get started or quit game

			/************************************************************************************************
			Game loop
			*************************************************************************************************/

			while ((key != 5) && isSnailAlive && counters[2] != LETTUCE_QUOTA)	//user not bored, and snail not dead or full (main game loop)
			{
				
				s.startTimer(); // not part of game
				// ************** code to be timed ***********************************************
				// now, everything is timed (including initialisation)
				


				if(key != 4)	// only move the snail if an arrow key was pressed
				{	
					/*********************************************************************************
					Move the snail
					**********************************************************************************/
					
					//char temp = garden[snailY + moveDirections[key][0]][snailX + moveDirections[key][1]];
					

					if(((abs(moveDirections[key]) == 28)&&(snail + moveDirections[key] >= 0)&&(snail + moveDirections[key] < GARDENSIZE))||(((snail + moveDirections[key]) / (SIZEX -2) == snail / (SIZEX -2)) && (abs(moveDirections[key]) == 1))) //|| (abs(moveDirections[key]) == 28))
					{
						char temp = garden[snail + moveDirections[key]];
						if(temp == BLANK || temp == PELLET || temp == LETTUCE || temp == DEAD_FROG_BONES)
						{
							garden[snail] = SLIME;	//lay a trail of slime

							slimeTrail[counters[0]] = snail;
		
							snail += moveDirections[key];	//go in direction indicated by keyMove

							if(temp == PELLET)
							{
								//printf("\a");									// produce a warning sound
								if (++counters[1] >= PELLET_THRESHOLD)				// aaaargh! poisoned!
								{	
									counters[3] = 1;
									//printf("\a\a\a\a");							// produce a death knell
									isSnailAlive = false;
								}
							}
							if(temp == LETTUCE)
							{
								if(++counters[2] != LETTUCE_QUOTA)
								{
									counters[3] = 3;
									//printf("\a");	
								}
								else
								{
									counters[3] = 2;
									//printf("\a\a\a\a\a\a\a");	
								}
							}
							// place snail (move snail in garden)
							garden[snail] = SNAIL;	

						}else if(temp == SLIME)
						{
							counters[3] = 4;
						}else
						{
							// frog
							garden[snail] = SLIME;				// lay a final trail of slime
							snail += moveDirections[key];							// go in direction indicated by keyMove
							counters[3] = 6;
							//printf("\a\a\a\a");								// produce a death knell
							isSnailAlive = false;
							// place snail (move snail in garden)
							garden[snail] = SNAIL;	
						}								
					}else
					{
						// wall
						//printf("\a");		//produce a warning sound
							counters[3] = 5;
					}
					

					
				}else
				{
					counters[3] = 7;
				}
				/*********************************************************************************
				Dissolve the slime
				**********************************************************************************/
				
				counters[0] = (++counters[0]) % SLIMELIFE;
				if(slimeTrail[counters[0]] != -1)
				{
					garden[slimeTrail[counters[0]]] = BLANK;
					slimeTrail[counters[0]] = -1;
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
						garden [frogs[0]] = BLANK;
					}else  
					{
						garden [frogs[0]] = LETTUCE;
						lettucesBlocked[0] = false; // no longer sitting on lettuce
					}
			
					// work out where to jump to depending on where the snail is...

					
					int direction = ((rows[snail] > rows[frogs[0]])) - ((rows[snail] < rows[frogs[0]]));
					
					if(((frogs[0] + (FROGLEAP * (SIZEX - 2)) * direction) < (SIZEY-2) * (SIZEX-2)) &&
						((frogs[0] + (FROGLEAP * (SIZEX - 2)) * direction) >= 0))
					{
						frogs[0] += (FROGLEAP * (SIZEX - 2)) * direction;
					}else
					{
						frogs[0] = columns[frogs[0]] + ((SIZEY - 3) * (SIZEX - 2))*(direction > 0);
					}


					// see which way to jump in the X direction (left and right)
					direction = (columns[snail] > columns[frogs[0]]) - ((columns[snail] < columns[frogs[0]]));

					if(rows[frogs[0]] == rows[frogs[0] + FROGLEAP * direction])
					{
						frogs[0] += FROGLEAP * direction;
					}else
					{
						frogs[0] = (rows[frogs[0]] * (SIZEX-2)) + ((SIZEX-3) * (direction > 0));
					}

					lettucesBlocked[0] = (garden [frogs[0]] == LETTUCE);

					if (((rand() % EagleStrike) + 1) != EagleStrike)  // not gotten by eagle?		
					{
						if (frogs[0] != snail)	// landed on snail? - grub up!
						{

							garden [frogs[0]] = FROG;				// display frog on garden (thus destroying any pellet that might be there).
						}
						else 
						{
							counters[3] = 8;
							//printf("\a\a\a\a");									// produce a death knell
							isSnailAlive = false;
						}
					}
					else 
					{
						if(!lettucesBlocked[0])
						{
							garden [frogs[0]] = DEAD_FROG_BONES;				// show remnants of frog in garden
						}else
						{
							// if the frog was sitting on a lettuce as he was killed, restore the lettuce
							garden [frogs[0]] = LETTUCE;				// show remnants of frog in garden
						}
			
						frogs[0] = -1;									// and mark frog as deceased
						counters[3] = 9;
						//printf("\a");												//produce a warning sound
					}
				}

				// frog 2
				if ((frogs[1] != -1) && isSnailAlive)		// if frog not been gotten by an eagle or GameOver
				{	
					// jump off garden (taking any slug pellet with it)... check it wasn't on a lettuce though...

					if (!lettucesBlocked[1])
					{
						garden [frogs[1]] = BLANK;
					}else  
					{
						garden [frogs[1]] = LETTUCE; 
						lettucesBlocked[1] = false; // no longer sitting on lettuce
					}
			
					// work out where to jump to depending on where the snail is...
					// see which way to jump in the Y direction (up and down)
					
					int direction = ((rows[snail] > rows[frogs[1]])) - ((rows[snail] < rows[frogs[1]]));

					if(((frogs[1] + (FROGLEAP * (SIZEX - 2)) * direction) < (SIZEY-2) * (SIZEX-2)) &&
						((frogs[1] + (FROGLEAP * (SIZEX - 2)) * direction) >= 0))
					{
						frogs[1] += (FROGLEAP * (SIZEX - 2)) * direction;
					}else
					{
						frogs[1] = columns[frogs[1]] + ((SIZEY - 3) * (SIZEX - 2))*(direction > 0);
					}

					// see which way to jump in the X direction (left and right)
					direction = (columns[snail] > columns[frogs[1]]) - ((columns[snail] < columns[frogs[1]]));
					
					if(rows[frogs[1]] == rows[frogs[1] + FROGLEAP * direction])
					{
						frogs[1] += FROGLEAP * direction;
					}else
					{
						frogs[1] = (rows[frogs[1]] * (SIZEX-2)) + ((SIZEX-3) * (direction > 0));
					}
					
					lettucesBlocked[1] = (garden [frogs[1]] == LETTUCE);

					if (((rand() % EagleStrike) + 1) != EagleStrike)  // not gotten by eagle?					
					{
						if (frogs[1] != snail)	// landed on snail? - grub up!
						{
							garden [frogs[1]] = FROG;				// display frog on garden (thus destroying any pellet that might be there).
						}
						else 
						{
							counters[3] = 8;
							//printf("\a\a\a\a");									// produce a death knell
							isSnailAlive = false;
						}
					}
					else 
					{

						if(!lettucesBlocked[1])
						{
							garden [frogs[1]] = DEAD_FROG_BONES;				// show remnants of frog in garden
				
						}else
						{
							// if the frog was sitting on a lettuce as he was killed, restore the lettuce
							garden [frogs[1]] = LETTUCE;				// show remnants of frog in garden
				
						}

						frogs[1] = -1;									// and mark frog as deceased
						counters[3] = 9;
						//printf("\a");												//produce a warning sound
					}
				}

				/*************************************************************************************
				Paint the game
				**************************************************************************************/

				//paintGame  ( counters[3], counters[1], garden);		// display game info, garden & messages
				counters[3] = 12; // reset message

				s.stopTimer(); // not part of game
				//*************** end of timed section ******************************************
				// now everything is timed
				
				
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
				garden[snail] = DEADSNAIL;
				counters[3] = 11;
			}else
			{
				// Survived
				counters[3] = 10;
			}

			//paintGame( counters[3], counters[1], garden);			//display final game info, garden & message
		
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
		counters[0] = 0;						
		counters[1] = 0;	
		counters[2] = 0;
		key = 4;
	}


	ofstream outFramerates;
	outFramerates.open("Framerates.txt");
	outFramerates << accumulatedFramerates <<"\n" << frameCount;
	outFramerates.close();

	return 0;
} //end main

/******************************************************************************************
Output the game
*******************************************************************************************/

int getRandomCoordinates()
{
	int randomNumber = rand() % 504; // 504 -> fields that are not walls
	return randomNumber + SIZEX + 1 + (randomNumber/(SIZEX - 2)) * 2; // translate into garden coordinates
}

void paintGame (int msgId, int pellets, const char garden[])
{

	void paintGarden( const char []);
	void showInformation(int, int);

	
	Clrscr();
	paintGarden ( garden);		// display garden contents
	showInformation(pellets, msgId); // show title, options, pellet count, data and time and message

} //end of paintGame

void paintGarden( const char garden[])
{ 
	//display garden content on screen

	// use native windows function WriteConsoleOutput, faster than cout/printf/putch
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms687404%28v=vs.85%29.aspx
	// store garden in buffer and output everything together in one call
	// still filling buffer every time :/
	
	SMALL_RECT OUTPUT_BUFFER_RECT = {0, 2, SIZEX, SIZEY + 1};
	
	CHAR_INFO buffer[600];

	for (int i(0); i < (SIZEX); ++i)		//for each row
	{
		buffer[i].Char.UnicodeChar = WALL;
		buffer[i].Attributes = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY;
	}

	for (int y(0); y < (SIZEY - 1); ++y)		//for each row
	{	
		buffer[y * SIZEX].Char.UnicodeChar = WALL;
		buffer[y * SIZEX].Attributes = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY;
		for (int x(0); x < (SIZEX - 2); ++x)	//for each col
		{			
			// add new CHAR_INFO structure to the buffer
			buffer[x + 1 + (y + 1) * SIZEX].Char.UnicodeChar = garden[x + y * 28];
			// set bright white colour for every character
			buffer[x + 1 + (y + 1) * SIZEX].Attributes = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY;
		}
		buffer[y * SIZEX + SIZEX - 1].Char.UnicodeChar = WALL;
		buffer[y * SIZEX + SIZEX - 1].Attributes = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY;
	}

	for (int i(SIZEX * SIZEY - SIZEX - 1); i < (SIZEX * SIZEY); ++i)		//for each row
	{
		buffer[i].Char.UnicodeChar = WALL;
		buffer[i].Attributes = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY;
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
	

	//return keys[keyCount++];

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

