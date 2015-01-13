/*
08_Snail_Trail_Code_Optimization_III
Kevin Meergans, 2013, based on the snail trail game by A. Oram
This continues to improve the performance of the game by optimising c++ code.
Changes (copied from report):
•	Got rid of “newgame” variable, use key instead
•	Key no longer holds values of the keycodes but values from zero to five that are, in case of an arrow key being 
	pressed, used as indices to an array that holds the four possible moves of the snail. This way, the move variable 
	and the “analyseKey” part could be removed, also getting rid of the branching due to the switch statement in that 
	part.
•	Got rid of full lettuce variable, check the amount of lettuces eaten against the maximal value instead
•	Unrolled the scatter lettuces code in the initialisation
•	Changed the string message variable into an integer holding an ID for the current message to be shown. That ID is 
	also the index to a constant array of all possible messages.
•	Moved pellet, lettuce, slime counter and message ID into one array of integers.
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

/*
const char gardenSymbols[9] = {' ', '-', '@', '.', '+', 'M', 'X', '&', 'o'};

const char BLANK(0);						// open space
const char PELLET (1); //(BLANK);			// should be blank) but test using a visible character.
const char LETTUCE (2);					// a lettuce
const char SLIME (3);						// snail produce
const char WALL(4);                       // garden wall
const char FROG (5);
const char DEAD_FROG_BONES (6);			// Dead frogs are marked as such in their 'y' coordinate

const char SNAIL(7);						// snail (player's icon)
const char DEADSNAIL (8);					// just the shell left...
*/

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

//const char NO_LETTUCE (BLANK);				// guess!
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


// stuff for the test harness

// The streams used to read in keys and to record frame rates for each iteration of the main game loop
ifstream inKeys; // used to read in key presses from a file
ofstream outFramerates("Framerates.txt"); // used to store the time needed for an iteration of the main loop

// the count of the current main loop iteration within the current cycle
int iterationCount(0);
// run through the prerecorded 120 main loop iterations this many times
const int numberOfCycles(1000000);
// accumulates the framerates for each of the 120 main loop iterations if there are multiple cycles
//double accumulatedFramerates[120] = {0.0f};

const char* const messages[13] = {"READY TO SLITHER!? PRESS A KEY...", 
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

	// garden data stores the following data
	// [0] snail position
	// [1] move vector
	// [2] position of frog No. 1
	// [3] position of frog No. 2
	// [4] position of lettuce blocked by frog 1
	// [5] position of lettuce blocked by frog 2
	// int gardenData[6][2];

	//int move[2];
	int snail[2];
	bool lettucesBlocked[2] = {false, false};

	// holds all frog data
	// [0] - y coordinate of frog 1
	// [1] - x coordinate of frog 1
	// [2] - y coordinate of frog 2
	// [3] - x coordinate of frog 2
	int frogs[8];

	int slimeTrail[SLIMELIFE][2];
	

	//char slimeTrail	  [SIZEY][SIZEX];		// lifetime of slime counters overlay
	//char lettucePatch [SIZEY][SIZEX];		// remember where the lettuces are planted

	//int  snail[2];							// the snail's current position
	//int  frogs [NUM_FROGS][2];				// coordinates of the frog contingent
	//int  move[2];							// the requested move direction


	//define a few global control constants
	int	 snailStillAlive (true);				// snail starts alive!
	//int  pellets( 0);							// number of times snail slimes over a slug pullet
	//int  lettucesEaten ( 0);					// win when this reaches LETTUCE_QUOTA
	//int slimeCounter(0);
	
	int counters[4] = {0,0,0,0}; // hold slime counter, count pellets eaten and lettuces eaten, and message ID


	static const int moveDirections[5][2] = {{0,-1},{0,1},{-1,0},{1,0},{0,0}};

	// key can have the following values
	// 0 - Left
	// 1 - Right
	// 2 - Up
	// 3 - Down
	// 4 - Other
	// 5 - Quit
	int  key(4);
	int newGame (!QUIT);				// start new game by not quitting initially!
		
	for(int i = 0; i < numberOfCycles; ++i)
	{
		// Now start the game...
		inKeys.open("Keys.txt");

		//Seed();									//seed the random number generator
		srand(256); 

		CStopWatch s;							// create a stopwatch for timing
		
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

			for ( int row( 0); row < SIZEY; ++row)			//for each row
			{	
				for ( int col( 0); col < SIZEX; ++col)		//for each col
				{	
					if (( row != 0) && (row != SIZEY-1) && ( col != 0) && (col != SIZEX-1))	//top & bottom walls, left & right walls
						garden[row][col] = BLANK;			// draw a space
					else
						garden[row][col] = WALL;			//draw a garden wall symbol
				}
			}

			//-------------------------------------------------------------------------------------
			// place snail

			// set snail initial coordinates
			snail[0] = (rand() % (SIZEY-2)) + 1;		// vertical coordinate in range [1..(SIZEY - 2)]
			snail[1] = (rand() % (SIZEX-2)) + 1;		// horizontal coordinate in range [1..(SIZEX - 2)]

			garden[snail[0]][snail[1]] = SNAIL;

			//--------------------------------------------------------------------------------------
			// scatter pellets
			
			for (int slugP=0; slugP < NUM_PELLETS; ++slugP)								// scatter some slug pellets...
			{	
				int x((rand() % (SIZEX-2)) + 1), y((rand() % (SIZEY-2)) + 1);
				while (( (y = (rand() % (SIZEY-2)) + 1) == snail[0]) && ((x=(rand() % (SIZEX-2)) + 1) == snail[1]) 
						|| garden [y][x] == PELLET) ;								// avoid snail and other pellets 
				garden [y][x] = PELLET;												// hide pellets around the garden
			}
	
			//---------------------------------------------------------------------------------
			// scatter lettuces

			int x((rand() % (SIZEX-2)) + 1), y((rand() % (SIZEY-2)) + 1);
			while (( (y = (rand() % (SIZEY-2)) + 1) == snail[0]) && ((x=(rand() % (SIZEX-2)) + 1) == snail[1]) 
					|| garden [y][x] == PELLET || garden [y][x] == LETTUCE) ;	// avoid snail, pellets and other lettucii 
			garden [y][x] = LETTUCE;

			x = ((rand() % (SIZEX-2)) + 1);
			y = ((rand() % (SIZEY-2)) + 1);
			while (( (y = (rand() % (SIZEY-2)) + 1) == snail[0]) && ((x=(rand() % (SIZEX-2)) + 1) == snail[1]) 
					|| garden [y][x] == PELLET || garden [y][x] == LETTUCE) ;	// avoid snail, pellets and other lettucii 
			garden [y][x] = LETTUCE;
			
			x = ((rand() % (SIZEX-2)) + 1);
			y = ((rand() % (SIZEY-2)) + 1);
			while (( (y = (rand() % (SIZEY-2)) + 1) == snail[0]) && ((x=(rand() % (SIZEX-2)) + 1) == snail[1]) 
					|| garden [y][x] == PELLET || garden [y][x] == LETTUCE) ;	// avoid snail, pellets and other lettucii 
			garden [y][x] = LETTUCE;

			x = ((rand() % (SIZEX-2)) + 1);
			y = ((rand() % (SIZEY-2)) + 1);
			while (( (y = (rand() % (SIZEY-2)) + 1) == snail[0]) && ((x=(rand() % (SIZEX-2)) + 1) == snail[1]) 
					|| garden [y][x] == PELLET || garden [y][x] == LETTUCE) ;	// avoid snail, pellets and other lettucii 
			garden [y][x] = LETTUCE;


			//-------------------------------------------------------------------------------
			//scatter frogs

			// frog 1
			frogs[1] = ((rand() % (SIZEX-2)) + 1);
			frogs[0] = ((rand() % (SIZEY-2)) + 1);	// prime coords before checking

			while (( (frogs[0] = (rand() % (SIZEY-2)) + 1) == snail[0]) && ((frogs[1]=(rand() % (SIZEX-2)) + 1) == snail[1])
					|| garden [y][x] == FROG) ;				// avoid snail and existing frogs 

			// frog 2
			frogs[3] = ((rand() % (SIZEX-2)) + 1);
			frogs[2] = ((rand() % (SIZEY-2)) + 1);	// prime coords before checking

			while (( (frogs[2] = (rand() % (SIZEY-2)) + 1) == snail[0]) && ((frogs[3]=(rand() % (SIZEX-2)) + 1) == snail[1])
					|| garden [y][x] == FROG) ;				// avoid snail and existing frogs 

			
			if(garden[frogs[0]][frogs[1]] == LETTUCE)
				lettucesBlocked[0] = true; // frog 1 is currently blocking a lettuce
			
			garden [frogs[0]][frogs[1]] = FROG;		// put frog 1 on garden (this may overwrite a slug pellet)

			if(garden[frogs[2]][frogs[3]] == LETTUCE)
				lettucesBlocked[1] = true; // frog 2 is currently blocking a lettuce
			
			garden [frogs[2]][frogs[3]] = FROG;		// put frog 2 on garden (this may overwrite a slug pellet)

			//------------------------------------------------------------------------------
			// further initialising

			snailStillAlive = true;					// bring snail to life!

			counters[0] = 0;
			counters[1] = 0;
			counters[2] = 0;
			counters[3] = 0;
			//pellets = 0;							// no slug pellets slithered over yet
			//lettucesEaten = 0;								// reset number of lettuces eaten
			//slimeCounter = 0;

			key = 4;

			//const char* message("READY TO SLITHER!? PRESS A KEY...");

			//paintGame( counters[3], counters[1], garden);			//display game info, garden & messages

			key = getKeyPress();							//get started or quit game

			/************************************************************************************************
			Game loop
			*************************************************************************************************/

			while ((key != 5) && snailStillAlive && counters[2] != LETTUCE_QUOTA)	//user not bored, and snail not dead or full (main game loop)
			{
				s.startTimer(); // not part of game

				// ************** code to be timed ***********************************************
		
				/*********************************************************************************
				Analyse the user input
				**********************************************************************************/
				
				if(key != 4)	// only move the snail if an arrow key was pressed
				{	
					/*********************************************************************************
					Move the snail
					**********************************************************************************/

					switch( garden[snail[0] + moveDirections[key][0]][snail[1] + moveDirections[key][1]]) //depending on what is at target position
					{
						case BLANK:
							{
							garden[snail[0]][snail[1]] = SLIME;				//lay a trail of slime

							slimeTrail[counters[0]][0] = snail[0];
							slimeTrail[counters[0]][1] = snail[1];
			
							snail[0] += moveDirections[key][0];							//go in direction indicated by keyMove
							snail[1] += moveDirections[key][1];
							break;
							}
						case PELLET:		// increment pellet count and kill snail if > threshold
							{
							garden[snail[0]][snail[1]] = SLIME;				// lay a trail of slime

							slimeTrail[counters[0]][0] = snail[0];
							slimeTrail[counters[0]][1] = snail[1];
			
							snail[0] += moveDirections[key][0];							// go in direction indicated by keyMove
							snail[1] += moveDirections[key][1];
							//printf("\a");									// produce a warning sound
							if (++counters[1] >= PELLET_THRESHOLD)				// aaaargh! poisoned!
							{	
								counters[3] = 1;
								//printf("\a\a\a\a");							// produce a death knell
								snailStillAlive = false;					// game over
							}
							break;
							}
						case LETTUCE:		// increment lettuce count and win if snail is full
							{
							garden[snail[0]][snail[1]] = SLIME;				//lay a trail of slime

							slimeTrail[counters[0]][0] = snail[0];
							slimeTrail[counters[0]][1] = snail[1];
			
							snail[0] += moveDirections[key][0];							//go in direction indicated by keyMove
							snail[1] += moveDirections[key][1];

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

							break;
							}
						case SLIME:
							{
								counters[3] = 4;
							}
						case WALL:				//oops, garden wall
							//printf("\a");		//produce a warning sound
							counters[3] = 5;
							break;				//& stay put
						case FROG:			//	kill snail if it throws itself at a frog!
							{
							garden[snail[0]][snail[1]] = SLIME;				// lay a final trail of slime
							snail[0] += moveDirections[key][0];							// go in direction indicated by keyMove
							snail[1] += moveDirections[key][1];
							counters[3] = 6;
							//printf("\a\a\a\a");								// produce a death knell
							snailStillAlive = false;						// game over
							break;
							}
						case DEAD_FROG_BONES:		//its safe to move over dead/missing frogs too
							{
							garden[snail[0]][snail[1]] = SLIME;				//lay a trail of slime

							slimeTrail[counters[0]][0] = snail[0];
							slimeTrail[counters[0]][1] = snail[1];
			
							snail[0] += moveDirections[key][0];							//go in direction indicated by keyMove
							snail[1] += moveDirections[key][1];
							break;
							}
					}

					// place snail (move snail in garden)
					garden[snail[0]][snail[1]] = SNAIL;
				}else
				{
					counters[3] = 7;
				}
				/*********************************************************************************
				Dissolve the slime
				**********************************************************************************/
				
				counters[0] = (++counters[0])%SLIMELIFE;
				if(slimeTrail[counters[0]][0] != -1)
				{
					garden[slimeTrail[counters[0]][0]][slimeTrail[counters[0]][1]] = BLANK;
					slimeTrail[counters[0]][0] = -1;
				}

				/*********************************************************************************
				Move the frogs
				**********************************************************************************/

				// frog 1
				if ((frogs[0] != -1) && snailStillAlive)		// if frog not been gotten by an eagle or GameOver
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
		
					// direction is 0, -1 or 1
					int direction = ((snail[0] - frogs[0]) > 0) - ((snail[0] - frogs[0]) < 0);

					// I tried to get rif of some of the branching but this didn't give any noticeable
					// speedup.

					frogs[0] += FROGLEAP * direction;
					if((frogs[0] >= SIZEY - 1) || (frogs[0] < 1))
					{
						frogs[0] = 1 + (SIZEY - 3) * (direction > 0);
					}

					// see which way to jump in the X direction (left and right)
					// direction is 0, -1 or 1
					direction = ((snail[1] - frogs[1]) > 0) - ((snail[1] - frogs[1]) < 0);

					frogs[1] += FROGLEAP * direction;
					if((frogs[1] >= SIZEX - 1) || (frogs[1] < 1))
					{
						frogs[1] = 1 + (SIZEX - 3) * (direction > 0);
					}
		
					if (((rand() % EagleStrike) + 1) != EagleStrike)  // not gotten by eagle?
					{
						if (frogs[0] != snail[0] || frogs[1] != snail[1])	// landed on snail? - grub up!
						{
							// if frog jumps onto a lettuce, remember the lettuce to restore it later
							
							// got rid of the branching in here
							lettucesBlocked[0] = (garden [frogs[0]][frogs[1]] == LETTUCE);
							
							garden [frogs[0]][frogs[1]] = FROG;				// display frog on garden (thus destroying any pellet that might be there).
						}
						else 
						{
							counters[3] = 8;
							//printf("\a\a\a\a");									// produce a death knell
							snailStillAlive = false;								// snail is dead!
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
						counters[3] = 9;
						//printf("\a");												//produce a warning sound
					}
				}

				// frog 2
				if ((frogs[2] != -1) && snailStillAlive)		// if frog not been gotten by an eagle or GameOver
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
					// see which way to jump in the Y direction (up and down)

					int direction = ((snail[0] - frogs[2]) > 0) - ((snail[0] - frogs[2]) < 0);

					frogs[2] += FROGLEAP * direction;
					if((frogs[2] >= SIZEY - 1) || (frogs[2] < 1))
					{
						frogs[2] = 1 + (SIZEY - 3) * (direction > 0);
					}

					// see which way to jump in the X direction (left and right)
					direction = ((snail[1] - frogs[3]) > 0) - ((snail[1] - frogs[3]) < 0);

					frogs[3] += FROGLEAP * direction;
					if((frogs[3] >= SIZEX - 1) || (frogs[3] < 1))
					{
						frogs[3] = 1 + (SIZEX - 3) * (direction > 0);
					}
		
					if (((rand() % EagleStrike) + 1) != EagleStrike)  // not gotten by eagle?						
					{
						if (frogs[2] != snail[0] || frogs[3] != snail[1])	// landed on snail? - grub up!
						{
							// if frog jumps onto a lettuce, remember the lettuce to restore it later
							lettucesBlocked[1] = (garden [frogs[2]][frogs[3]] == LETTUCE);
				
							garden [frogs[2]][frogs[3]] = FROG;				// display frog on garden (thus destroying any pellet that might be there).
						}
						else 
						{
							counters[3] = 8;
							//printf("\a\a\a\a");									// produce a death knell
							snailStillAlive = false;								// snail is dead!
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
						counters[3] = 9;
						//printf("\a");												//produce a warning sound
					}
				}

				/*************************************************************************************
				Paint the game
				**************************************************************************************/

				//paintGame  ( counters[3], counters[1], garden);		// display game info, garden & messages
				counters[3] = 12; // reset message

				//*************** end of timed section ******************************************

				s.stopTimer(); // not part of game
				if(s.getElapsedTime() > 0.0)
				{
					accumulatedFramerates += s.getElapsedTime();
					++frameCount;
				}
				//showFrameRate ( s.getElapsedTime());		// display frame rate - not part of game


				++iterationCount;

				key = getKeyPress();						// display menu & read in next option
			}
			//							If alive...								If dead...
			(snailStillAlive) ? counters[3] = 10 : counters[3] = 11;//message = "WELL DONE, YOU'VE SURVIVED" : message = "REST IN PEAS.";
			if (!snailStillAlive)
			{
				garden[snail[0]][snail[1]] = DEADSNAIL;
			}
			//paintGame( counters[3], counters[1], garden);			//display final game info, garden & message
		
			key = getKeyPress();
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

		// close the stream to be able to read from anew in the next cycle
		inKeys.close();
		// reset the iteration count
		iterationCount = 0;

		// Reset game state for the next cycle
		snailStillAlive = true;		
		counters[0] = 0;						
		counters[1] = 0;	
		counters[2] = 0;
		key = 4;
	}

	// print the recorded and accumulated times for the 120 frames to a file
	
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

int anotherGo (void)
{ //show end message and hold output screen
	
	int getKeyPress();
	/*
	SelectBackColour( clRed);
	SelectTextColour( clYellow);
	Gotoxy(MLEFT, 18);
	printf("PRESS 'Q' TO QUIT OR ANY KEY TO CONTINUE");
	SelectBackColour( clBlack);
	SelectTextColour( clWhite);
	*/
	return (getKeyPress());	
} // end of anotherGo

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
	
		
	/*
	// the old manual input, just for testing
	int command;
	//read in the selected option
	command = _getch();  	// to read arrow keys
	while ( command == 224)	// to clear extra info from buffer
		command = _getch();
	*/

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

