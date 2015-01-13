/*
07_Snail_Trail_Code_Optimization_II
Kevin Meergans, 2013, based on the snail trail game by A. Oram
This continues to improve the performance of the game.
Changes (copied from report):
•	Replaced the “slimeTrail” array with a queue of coordinates, from which an element is popped each frame, as soon 
	as the queue once reaches the size of “SLIMELIFE”. Therefore, it was no longer necessary to iterate through the 
	slime array each frame. However, at a later point of optimising this version, I got rid of the queue again as the 
	dynamic allocation of memory proved to be too expensive. Instead, I chose an array of size “SLIMELIFE” and obtain 
	a “slimeCounter” that is used to determine which slime ball to set or remove in the array.
•	Moved the code in “dissolveSlime” into the main function to get rid of function call overhead
•	Got rid of lettuce patch. Instead, there is an array of size two that stores the coordinates of lettuces being 
	blocked by the frogs (of which there can be at most two)
•	Used an array “gardenData” to store snail position, move direction, frog positions and the positions of lettuces 
	that frogs are sitting on to reduce the number of local variables and parameters. However, no speedup could be 
	measured, which implies that in this case accessing the elements of the array isn’t really more efficient than 
	having the elements as variables on their own. In the later version, I split this up again as it didn’t really 
	help.
•	Moved the code of all functions called in the initialisation function directly into that function to save further 
	execution time by reducing function call overheads.
•	Unrolled the for-loops in “scatterFrogs” and “moveFrogs” to avoid loop overhead. As both loops are very small 
	(only two iterations) the increase in code size is not very big.
•	As the assignment mainly focuses on the improvement of the non-output part of the program, the queue used to store 
	changes to the garden was removed because the dynamic putting of elements slowed down the timed part of the program. 
	Instead, the whole garden is now printed every frame.
•	Reduced jumps in the “moveFrogs” function (I reverted this in a later function as it didn’t bring any noticeable 
	speedup and I had some doubts that the frogs were still moving correctly at all times).
•	Moved all code from the remaining functions, except those related to I/O, to the main function to get rid of all 
	function call overheads
•	Moved the code from the “Random” function to all the places, where it is called

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
const char SNAIL('&');						// snail (player's icon)
const char DEADSNAIL ('o');					// just the shell left...
const char BLANK(' ');						// open space
const char WALL('+');                       // garden wall

const char SLIME ('.');						// snail produce
const int  SLIMELIFE (25);					// how long slime lasts (in keypresses)

const char PELLET ('-'); //(BLANK);			// should be blank) but test using a visible character.
const int  NUM_PELLETS (15);				// number of slug pellets scattered about
const int  PELLET_THRESHOLD (5);			// deadly threshold! Slither over this number and you die!

const char LETTUCE ('@');					// a lettuce
const char NO_LETTUCE (BLANK);				// guess!
const int  LETTUCE_QUOTA (4);				// how many lettuces you need to eat before you win.

const int  NUM_FROGS (2);
const char FROG ('M');
const char DEAD_FROG_BONES ('X');			// Dead frogs are marked as such in their 'y' coordinate
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
SMALL_RECT OUTPUT_BUFFER_RECT = {0, 2, SIZEX, SIZEY + 1};

// stuff for the test harness

// The streams used to read in keys and to record frame rates for each iteration of the main game loop
ifstream inKeys; // used to read in key presses from a file
ofstream outFramerates("Framerates.txt"); // used to store the time needed for an iteration of the main loop

// the count of the current main loop iteration within the current cycle
int iterationCount(0);
// run through the prerecorded 120 main loop iterations this many times
const int numberOfCycles(1000000);
// accumulates the framerates for each of the 120 main loop iterations if there are multiple cycles

double accumulatedFramerates(0.0);
long long frameCount(0LL);

// Start of the 'SNAIL TRAIL' listing
//---------------------------------
int main()
{
	
	//function prototypes
	void paintGame(string& message, int, const char [][SIZEX]);
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
	int gardenData[6][2];

	int slimeTrail[SLIMELIFE][2];
	
	//define a few global control constants
	int	 snailStillAlive (true);				// snail starts alive!
	int  pellets( 0);							// number of times snail slimes over a slug pullet
	int  lettucesEaten ( 0);					// win when this reaches LETTUCE_QUOTA
	bool fullOfLettuce (false);					// when full and alive snail has won!
	int slimeCounter = 0;
	int  key, newGame (!QUIT);				// start new game by not quitting initially!
		
	for(int i = 0; i < numberOfCycles; ++i)
	{
		// Now start the game...
		inKeys.open("Keys.txt");

		//Seed();									//seed the random number generator
		srand(256); 

		CStopWatch s;							// create a stopwatch for timing
		
		while ((newGame | 0x20) != QUIT)		// keep playing games
		{	
			//Clrscr();

			/**********************************************************************************************
			Initialisation 
			***********************************************************************************************/
			
			// initialise slime trail
			for(int i = 0; i < SLIMELIFE; ++i)
			{
				slimeTrail[i][0] = -1;
				slimeTrail[i][1] = -1;
			}

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

			// set snail initial coordinates
			gardenData[0][0] = (rand() % (SIZEY-2)) + 1;		// vertical coordinate in range [1..(SIZEY - 2)]
			gardenData[0][1] = (rand() % (SIZEX-2)) + 1;		// horizontal coordinate in range [1..(SIZEX - 2)]

			// place snail
			garden[gardenData[0][0]][gardenData[0][1]] = SNAIL;

			// scatter stuff
			for (int slugP=0; slugP < NUM_PELLETS; ++slugP)								// scatter some slug pellets...
			{	
				int x((rand() % (SIZEX-2)) + 1), y((rand() % (SIZEY-2)) + 1);
				while (( (y = (rand() % (SIZEY-2)) + 1) == gardenData[0][0]) && ((x=(rand() % (SIZEX-2)) + 1) == gardenData[0][1]) 
						|| garden [y][x] == PELLET) ;								// avoid snail and other pellets 
				garden [y][x] = PELLET;												// hide pellets around the garden
			}
	
			for (int food=0; food < LETTUCE_QUOTA; ++food)							// scatter lettuces for eating...
			{	
				int x((rand() % (SIZEX-2)) + 1), y((rand() % (SIZEY-2)) + 1);
				while (( (y = (rand() % (SIZEY-2)) + 1) == gardenData[0][0]) && ((x=(rand() % (SIZEX-2)) + 1) == gardenData[0][1]) 
						|| garden [y][x] == PELLET || garden [y][x] == LETTUCE) ;	// avoid snail, pellets and other lettucii 
				garden [y][x] = LETTUCE;
			}

			// scatter frogs

			// frog 1
			int x((rand() % (SIZEX-2)) + 1), y((rand() % (SIZEY-2)) + 1);	// prime coords before checking
			while (( (y = (rand() % (SIZEY-2)) + 1) == gardenData[0][0]) && ((x=(rand() % (SIZEX-2)) + 1) == gardenData[0][1])
					|| garden [y][x] == FROG) ;				// avoid snail and existing frogs 
		 
			gardenData[2][0] = y;								// store initial positions of frog
			gardenData[2][1] = x;

			if(garden[y][x] != LETTUCE)
			{
				gardenData[4][0] = -1; // frog is currently not blocking a lettuce
				gardenData[4][1] = -1;
			}else
			{
				gardenData[4][0] = y; // remember the lettuce that is blocked by the frog
				gardenData[4][1] = x;
			}

			garden [y][x] = FROG;		// put frogs on garden (this may overwrite a slug pellet)
	
			// frog 2
			x = (rand() % (SIZEX-2)) + 1;
			y = (rand() % (SIZEY-2)) + 1;	// prime coords before checking
			while (( (y = (rand() % (SIZEY-2)) + 1) == gardenData[0][0]) && ((x=(rand() % (SIZEX-2)) + 1) == gardenData[0][1])
					|| garden [y][x] == FROG) ;				// avoid snail and existing frogs 
		 
			gardenData[3][0] = y;								// store initial positions of frog
			gardenData[3][1] = x;

			if(garden[y][x] != LETTUCE)
			{
				gardenData[5][0] = -1; // frog is currently not blocking a lettuce
				gardenData[5][1] = -1;
			}else
			{
				gardenData[5][0] = y; // remember the lettuce that is blocked by the frog
				gardenData[5][1] = x;
			}

			garden [y][x] = FROG;		// put frogs on garden (this may overwrite a slug pellet)

			snailStillAlive = true;					// bring snail to life!
			pellets = 0;							// no slug pellets slithered over yet
			lettucesEaten = 0;								// reset number of lettuces eaten
			fullOfLettuce = false;							// snail is hungry again			

			slimeCounter = 0;

			string message( "READY TO SLITHER!? PRESS A KEY...");

			//paintGame( message, pellets, garden);			//display game info, garden & messages

			key = getKeyPress();							//get started or quit game

			/************************************************************************************************
			Game loop
			*************************************************************************************************/

			while (((key | 0x20) != QUIT) && snailStillAlive && !fullOfLettuce)	//user not bored, and snail not dead or full (main game loop)
			{
				s.startTimer(); // not part of game

				// ************** code to be timed ***********************************************
		
				/*********************************************************************************
				Analyse the user input
				**********************************************************************************/
				// No jumps alternative that was tested -> no measurable speedup
				// gardenData[1][0] = 0 + (key == DOWN) - (key == UP);
				// gardenData[1][1] = 0 + (key == RIGHT) - (key == LEFT);
				
				switch( key)		//...depending on the selected key...
				{
					case LEFT:	//prepare to move left
						gardenData[1][0] = 0; 
						gardenData[1][1] = -1;	// decrease the X coordinate
						break;
					case RIGHT: //prepare to move right
						gardenData[1][0] = 0; 
						gardenData[1][1] = +1;	// increase the X coordinate
						break;
					case UP: //prepare to move up
						gardenData[1][0] = -1; 
						gardenData[1][1] = 0;	// decrease the Y coordinate
						break;
					case DOWN: //prepare to move down
						gardenData[1][0] = +1; 
						gardenData[1][1] = 0;	// increase the Y coordinate
						break;
					default:  					// this shouldn't happen
						message = "INVALID KEY";	// prepare error message
						gardenData[1][0] = 0;			// move snail out of the garden
						gardenData[1][1] = 0;
				}

				/*********************************************************************************
				Move the snail
				**********************************************************************************/
				
				switch( garden[gardenData[0][0] + gardenData[1][0]][gardenData[0][1] + gardenData[1][1]]) //depending on what is at target position
				{
					case BLANK:
						{
						garden[gardenData[0][0]][gardenData[0][1]] = SLIME;				//lay a trail of slime

						slimeTrail[slimeCounter][0] = gardenData[0][0];
						slimeTrail[slimeCounter][1] = gardenData[0][1];
			
						gardenData[0][0] += gardenData[1][0];							//go in direction indicated by keyMove
						gardenData[0][1] += gardenData[1][1];
						break;
						}
					case PELLET:		// increment pellet count and kill snail if > threshold
						{
						garden[gardenData[0][0]][gardenData[0][1]] = SLIME;				// lay a trail of slime

						slimeTrail[slimeCounter][0] = gardenData[0][0];
						slimeTrail[slimeCounter][1] = gardenData[0][1];
			
						gardenData[0][0] += gardenData[1][0];							// go in direction indicated by keyMove
						gardenData[0][1] += gardenData[1][1];
						++pellets;
						//printf("\a");									// produce a warning sound
						if (pellets >= PELLET_THRESHOLD)				// aaaargh! poisoned!
						{	message = "TOO MANY PELLETS SLITHERED OVER!";
							//printf("\a\a\a\a");							// produce a death knell
							snailStillAlive = false;					// game over
						}
						break;
						}
					case LETTUCE:		// increment lettuce count and win if snail is full
						{
						garden[gardenData[0][0]][gardenData[0][1]] = SLIME;				//lay a trail of slime

						slimeTrail[slimeCounter][0] = gardenData[0][0];
						slimeTrail[slimeCounter][1] = gardenData[0][1];
			
						gardenData[0][0] += gardenData[1][0];							//go in direction indicated by keyMove
						gardenData[0][1] += gardenData[1][1];
						++lettucesEaten;								// keep a count
						fullOfLettuce = (lettucesEaten == LETTUCE_QUOTA); // if full, stop the game as snail wins!
						fullOfLettuce ? message = "LAST LETTUCE EATEN" : message = "LETTUCE EATEN";
						//fullOfLettuce ? printf("\a\a\a\a\a\a\a") : printf("\a");		
												//WIN! WIN! WIN!
						break;
						}
					case SLIME:
						{
							message = "TRY A DIFFERENT DIRECTION";
						}
					case WALL:				//oops, garden wall
						//printf("\a");		//produce a warning sound
						message = "THAT'S A WALL!";	
						break;				//& stay put
					case FROG:			//	kill snail if it throws itself at a frog!
						{
						garden[gardenData[0][0]][gardenData[0][1]] = SLIME;				// lay a final trail of slime
						gardenData[0][0] += gardenData[1][0];							// go in direction indicated by keyMove
						gardenData[0][1] += gardenData[1][1];
						message = "OOPS! ENCOUNTERED A FROG!";
						//printf("\a\a\a\a");								// produce a death knell
						snailStillAlive = false;						// game over
						break;
						}
					case DEAD_FROG_BONES:		//its safe to move over dead/missing frogs too
						{
						garden[gardenData[0][0]][gardenData[0][1]] = SLIME;				//lay a trail of slime

						slimeTrail[slimeCounter][0] = gardenData[0][0];
						slimeTrail[slimeCounter][1] = gardenData[0][1];
			
						gardenData[0][0] += gardenData[1][0];							//go in direction indicated by keyMove
						gardenData[0][1] += gardenData[1][1];
						break;
						}
				}

				// place snail (move snail in garden)
				garden[gardenData[0][0]][gardenData[0][1]] = SNAIL;

				/*********************************************************************************
				Dissolve the slime
				**********************************************************************************/
				
				slimeCounter = (slimeCounter + 1)%SLIMELIFE;
				if(slimeTrail[slimeCounter][0] != -1)
				{
					garden[slimeTrail[slimeCounter][0]][slimeTrail[slimeCounter][1]] = BLANK;
					slimeTrail[slimeCounter][0] = -1;
				}

				/*********************************************************************************
				Move the frogs
				**********************************************************************************/

				//moveFrogs  ( message, garden, gardenData);	// frogs attempt to home in on snail
				int distance = 0;
				int sign = 0;

				// frog 1
				if ((gardenData[2][0] != -1) && snailStillAlive)		// if frog not been gotten by an eagle or GameOver
				{	
					// jump off garden (taking any slug pellet with it)... check it wasn't on a lettuce though...

					if (gardenData[4][0] == -1)
					{
						garden [gardenData[2][0]][gardenData[2][1]] = BLANK;
					}else  
					{
						garden [gardenData[2][0]][gardenData[2][1]] = LETTUCE;
						gardenData[4][0] = -1; // no longer sitting on lettuce
					}
			
					// work out where to jump to depending on where the snail is...
		
					// see which way to jump in the Y direction (up and down)
		
					distance = gardenData[0][0] - gardenData[2][0];

					// sign is 0, -1 or 1
					sign = (distance > 0) - (distance < 0);

					gardenData[2][0] += FROGLEAP * sign;
					if((gardenData[2][0] >= SIZEY - 1) || (gardenData[2][0] < 1))
					{
						gardenData[2][0] = 1 + (SIZEY - 3) * (sign > 0);
					}

					// see which way to jump in the X direction (left and right)
					distance = gardenData[0][1] - gardenData[2][1];
					sign = (distance > 0) - (distance < 0);

					gardenData[2][1] += FROGLEAP * sign;
					if((gardenData[2][1] >= SIZEX - 1) || (gardenData[2][1] < 1))
					{
						gardenData[2][1] = 1 + (SIZEX - 3) * (sign > 0);
					}
		
					if (((rand() % EagleStrike) + 1) != EagleStrike)  // not gotten by eagle?
					{
						if (gardenData[2][0] != gardenData[0][0] || gardenData[2][1] != gardenData[0][1])	// landed on snail? - grub up!
						{
							// if frog jumps onto a lettuce, remember the lettuce to restore it later
							if(garden [gardenData[2][0]][gardenData[2][1]] == LETTUCE)
							{
								gardenData[4][0] = gardenData[2][0];
								gardenData[4][1] = gardenData[2][1];
							}
							garden [gardenData[2][0]][gardenData[2][1]] = FROG;				// display frog on garden (thus destroying any pellet that might be there).
						}
						else 
						{
							message = "FROG GOT YOU!";
							//printf("\a\a\a\a");									// produce a death knell
							snailStillAlive = false;								// snail is dead!
						}
					}
					else 
					{
						if(gardenData[4][0] == -1)
						{
							garden [gardenData[2][0]][gardenData[2][1]] = DEAD_FROG_BONES;				// show remnants of frog in garden
						}else
						{
							// if the frog was sitting on a lettuce as he was killed, restore the lettuce
							garden [gardenData[2][0]][gardenData[2][1]] = LETTUCE;				// show remnants of frog in garden
						}
			
						gardenData[2][0] = -1;									// and mark frog as deceased
						message = "EAGLE GOT A FROG";
						//printf("\a");												//produce a warning sound
					}
				}

				// frog 2,
				if ((gardenData[3][0] != -1) && snailStillAlive)		// if frog not been gotten by an eagle or GameOver
				{	
					// jump off garden (taking any slug pellet with it)... check it wasn't on a lettuce though...

					if (gardenData[5][0] == -1)
					{
						garden [gardenData[3][0]][gardenData[3][1]] = BLANK;
					}else  
					{
						garden [gardenData[3][0]][gardenData[3][1]] = LETTUCE; 
						gardenData[5][0] = -1; // no longer sitting on lettuce
					}
			
					// work out where to jump to depending on where the snail is...
					// see which way to jump in the Y direction (up and down)

		
					distance = gardenData[0][0] - gardenData[3][0];
					// sign is 0, -1 or 1
					sign = (distance > 0) - (distance < 0);

					gardenData[3][0] += FROGLEAP * sign;
					if((gardenData[3][0] >= SIZEY - 1) || (gardenData[3][0] < 1))
					{
						gardenData[3][0] = 1 + (SIZEY - 3) * (sign > 0);
					}

					// see which way to jump in the X direction (left and right)
					distance = gardenData[0][1] - gardenData[3][1];
					sign = (distance > 0) - (distance < 0);

					gardenData[3][1] += FROGLEAP * sign;
					if((gardenData[3][1] >= SIZEX - 1) || (gardenData[3][1] < 1))
					{
						gardenData[3][1] = 1 + (SIZEX - 3) * (sign > 0);
					}
		
					if (((rand() % EagleStrike) + 1) != EagleStrike)  // not gotten by eagle?						
					{
						if (gardenData[3][0] != gardenData[0][0] || gardenData[3][1] != gardenData[0][1])	// landed on snail? - grub up!
						{
							// if frog jumps onto a lettuce, remember the lettuce to restore it later
							if(garden [gardenData[3][0]][gardenData[3][1]] == LETTUCE)
							{
								gardenData[5][0] = gardenData[3][0];
								gardenData[5][1] = gardenData[3][1];
							}
				
							garden [gardenData[3][0]][gardenData[3][1]] = FROG;				// display frog on garden (thus destroying any pellet that might be there).
						}
						else 
						{
							message = "FROG GOT YOU!";
							//printf("\a\a\a\a");									// produce a death knell
							snailStillAlive = false;								// snail is dead!
						}
					}
					else 
					{

						if(gardenData[5][0] == -1)
						{
							garden [gardenData[3][0]][gardenData[3][1]] = DEAD_FROG_BONES;				// show remnants of frog in garden
				
						}else
						{
							// if the frog was sitting on a lettuce as he was killed, restore the lettuce
							garden [gardenData[3][0]][gardenData[3][1]] = LETTUCE;				// show remnants of frog in garden
				
						}

						gardenData[3][0] = -1;									// and mark frog as deceased
						message = "EAGLE GOT A FROG";
						//printf("\a");												//produce a warning sound
					}
				}

				/*************************************************************************************
				Paint the game
				**************************************************************************************/

				//paintGame  ( message, pellets, garden);		// display game info, garden & messages
				

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
			(snailStillAlive) ? message = "WELL DONE, YOU'VE SURVIVED" : message = "REST IN PEAS.";
			if (!snailStillAlive)
			{
				garden[gardenData[0][0]][gardenData[0][1]] = DEADSNAIL;
			}
			//paintGame( message, pellets, garden);			//display final game info, garden & message
		
			newGame = getKeyPress();
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
		pellets = 0;						
		lettucesEaten = 0;					
		fullOfLettuce = false;	
		newGame = !QUIT;
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

void paintGame ( string& msg, int pellets, const char garden[][SIZEX])
{

	void paintGarden( const char [][SIZEX]);
	void showInformation(int, string&);

	Clrscr();
	paintGarden ( garden);		// display garden contents
	showInformation(pellets, msg); // show title, options, pellet count, data and time and message

} //end of paintGame

void paintGarden( const char garden[][SIZEX])
{ 
	//display garden content on screen

	// use native windows function WriteConsoleOutput, faster than cout/printf/putch
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms687404%28v=vs.85%29.aspx
	// store garden in buffer and output everything together in one call
	// still filling buffer every time :/
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

void showInformation(int pellets, string& message)
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
	printf(message.c_str());	

	// show pellet count
	Gotoxy(MLEFT, 17);
	printf("SLITHERED OVER %d PELLETS SO FAR!", pellets);

	//reset message to blank
	message = "";
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

	return command;
	
	/*
	// the old manual input, just for testing
	int command;
	//read in the selected option
	command = _getch();  	// to read arrow keys
	while ( command == 224)	// to clear extra info from buffer
		command = _getch();
	return( command);
	*/
} //end of getKeyPress

// End of the 'SNAIL TRAIL' listing

