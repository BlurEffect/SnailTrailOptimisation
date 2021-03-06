/*
12_Snail_Trail_Final_Version
Kevin Meergans, 2013, based on the snail trail game by A. Oram
This is the final and probably fastest version. All output operations are included and the code for measuring the 
program has been removed for that version. The randomness is also back in there. Except for that it is identical to version 11 of the program.
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
const unsigned int SIZEY(20);						// vertical dimension
const unsigned int SIZEX(30);						// horizontal dimension

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

const unsigned int  SLIMELIFE (25);					// how long slime lasts (in keypresses)
const unsigned int  NUM_PELLETS (15);				// number of slug pellets scattered about
const unsigned int  PELLET_THRESHOLD (5);			// deadly threshold! Slither over this number and you die!
const unsigned int  LETTUCE_QUOTA (4);				// how many lettuces you need to eat before you win.
const unsigned int  NUM_FROGS (2);
const unsigned int  FROGLEAP (4);					// How many spaces do frogs jump when they move
const unsigned int  EagleStrike (32);				// There's a 1 in 'nn' chance of an eagle strike on a frog

// the keyboard arrow codes
const unsigned int UP    (72);						// up key
const unsigned int DOWN  (80);						// down key
const unsigned int RIGHT (77);						// right key
const unsigned int LEFT  (75);						// left key

// other command letters
const char QUIT('q');						//end the game

const unsigned int MLEFT (SIZEX+5);					//define left margin for messages (avoiding garden)

// needed for output with WriteConsoleOutput in paintgarden
const COORD OUTPUT_BUFFER_START = {0, 0}; 
const COORD OUTPUT_BUFFER_SIZE = {SIZEX, SIZEY};

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

// Start of the 'SNAIL TRAIL' listing
//---------------------------------
int main()
{
	//function prototypes
	void paintGame(int, int, const char [][SIZEX+2]);
	int getKeyPress();
	void showFrameRate(double);

	// holds frog positions
	// [0] - y coordinate of frog 1
	// [1] - x coordinate of frog 1
	// [2] - y coordinate of frog 2
	// [3] - x coordinate of frog 2
	int frogs[4];
	
	// the position of the snail
	// [0] - y coordinate
	// [1] - x coordinate
	int snail[2];

	int counters[4] = {0,0,0,0}; // hold message ID, slime counter, count pellets eaten and lettuces eaten

	// holds the position of each slime ball
	int slimeTrail[SLIMELIFE][2];

	// key can have the following values
	// 0 - Left
	// 1 - Right
	// 2 - Up
	// 3 - Down
	// 4 - Other
	// 5 - Quit
	int  key(4);

	char garden		  [SIZEY][SIZEX+2];		// the game 'world'

	bool isSnailAlive(true);

	// keeps track of whether frogs are currently sitting on lettuces or not
	bool lettucesBlocked[2] = {false, false};
		
	CStopWatch s;							// create a stopwatch for timing

	// Now start the game...
					
	srand( (unsigned)time( NULL ) );  //seed the random number generator

	while (key != 5)		// keep playing games
	{	
		Clrscr();

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

		int random = rand();
		snail[0] = (random - (SIZEY-2) * (random/(SIZEY-2))) + 1;		// vertical coordinate in range [1..(SIZEY - 2)]
		random = rand();
		snail[1] = (random - (SIZEX-2) * (random/(SIZEX-2))) + 1;		// horizontal coordinate in range [1..(SIZEX - 2)]

		garden[snail[0]][snail[1]] = SNAIL;

		//--------------------------------------------------------------------------------------
		// scatter pellets
		int	y = (0);
		int x = (0);

		for (int slugP=0; slugP < NUM_PELLETS; ++slugP)								// scatter some slug pellets...
		{	
			do
			{
				random = rand();
				x = (random - (SIZEX-2) * (random/(SIZEX-2))) + 1;
				random = rand();
				y = (random - (SIZEY-2) * (random/(SIZEY-2))) + 1;
			}while(garden [y][x] == PELLET || ((y == snail[0]) && (x == snail[1]))); // avoid snail and other pellets

			garden [y][x] = PELLET;
		}
	
		//---------------------------------------------------------------------------------
		// scatter lettuces

		do
		{
			random = rand();
			y = (random - (SIZEY-2) * (random/(SIZEY-2))) + 1;
			random = rand();
			x = (random - (SIZEX-2) * (random/(SIZEX-2))) + 1;
						// avoid snail, pellets and other lettucii
		}while(garden [y][x] == PELLET || ((y == snail[0]) && (x == snail[1]))); // no need to check for lettuces already
			 
		garden [y][x] = LETTUCE;

		do
		{
			random = rand();
			y = (random - (SIZEY-2) * (random/(SIZEY-2))) + 1;
			random = rand();
			x = (random - (SIZEX-2) * (random/(SIZEX-2))) + 1;
						// avoid snail, pellets and other lettucii
		}while(garden [y][x] == PELLET || garden [y][x] == LETTUCE || ((y == snail[0]) && (x == snail[1])));
			 
		garden [y][x] = LETTUCE;

		do
		{
			random = rand();
			y = (random - (SIZEY-2) * (random/(SIZEY-2))) + 1;
			random = rand();
			x = (random - (SIZEX-2) * (random/(SIZEX-2))) + 1;
						// avoid snail, pellets and other lettucii
		}while(garden [y][x] == PELLET || garden [y][x] == LETTUCE || ((y == snail[0]) && (x == snail[1])));
			 
		garden [y][x] = LETTUCE;

		do
		{
			random = rand();
			y = (random - (SIZEY-2) * (random/(SIZEY-2))) + 1;
			random = rand();
			x = (random - (SIZEX-2) * (random/(SIZEX-2))) + 1;
						// avoid snail, pellets and other lettucii
		}while(garden [y][x] == PELLET || garden [y][x] == LETTUCE || ((y == snail[0]) && (x == snail[1])));
			 
		garden [y][x] = LETTUCE;

		//-------------------------------------------------------------------------------
		//scatter frogs

		// unrolled for loop

		// frog 1
		do
		{
			random = rand();
			frogs[0] = (random - (SIZEY-2) * (random/(SIZEY-2))) + 1;
			random = rand();
			frogs[1] = (random - (SIZEX-2) * (random/(SIZEX-2))) + 1;
				
		}while((frogs[0] == snail[0]) && (frogs[1] == snail[1]));  // avoid snail (no need to avoid other frog yet)

		//frog 2
		do
		{
			random = rand();
			frogs[2] = (random - (SIZEY-2) * (random/(SIZEY-2))) + 1;
			random = rand();
			frogs[3] = (random - (SIZEX-2) * (random/(SIZEX-2))) + 1;


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

		paintGame( counters[0], counters[2], garden);			//display game info, garden & messages

		key = getKeyPress();							//get started or quit game

		/************************************************************************************************
		Game loop
		*************************************************************************************************/

		while (isSnailAlive && (key != 5) && counters[3] != LETTUCE_QUOTA)	//user not bored, and snail not dead or full (main game loop)
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
	
				switch( garden[snail[0] + moveDirections[key][0]][snail[1] + moveDirections[key][1]]) //depending on what is at target position
				{
					case BLANK:
						{
						garden[snail[0]][snail[1]] = SLIME;				//lay a trail of slime

						slimeTrail[counters[1]][0] = snail[0];
						slimeTrail[counters[1]][1] = snail[1];
							
						snail[0] += moveDirections[key][0];							//go in direction indicated by keyMove
						snail[1] += moveDirections[key][1];
						// place snail (move snail in garden)
						garden[snail[0]][snail[1]] = SNAIL;
						break;
						}
					case PELLET:		// increment pellet count and kill snail if > threshold
						{
						garden[snail[0]][snail[1]] = SLIME;				// lay a trail of slime

						slimeTrail[counters[1]][0] = snail[0];
						slimeTrail[counters[1]][1] = snail[1];
			
						snail[0] += moveDirections[key][0];							// go in direction indicated by keyMove
						snail[1] += moveDirections[key][1];
						// place snail (move snail in garden)
						garden[snail[0]][snail[1]] = SNAIL;
						printf("\a");									// produce a warning sound
						if (++counters[2] >= PELLET_THRESHOLD)				// aaaargh! poisoned!
						{	
							counters[0] = 1;
							printf("\a\a\a\a");							// produce a death knell
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

						// place snail (move snail in garden)
						garden[snail[0]][snail[1]] = SNAIL;

						if(++counters[3] != LETTUCE_QUOTA)
						{
							counters[0] = 3;
							printf("\a");	
						}
						else
						{
							counters[0] = 2;
							printf("\a\a\a\a\a\a\a");	
						}

						break;
						}
					case DEAD_FROG_BONES:		//its safe to move over dead/missing frogs too
						{
						garden[snail[0]][snail[1]] = SLIME;				//lay a trail of slime

						slimeTrail[counters[1]][0] = snail[0];
						slimeTrail[counters[1]][1] = snail[1];
			
						snail[0] += moveDirections[key][0];							//go in direction indicated by keyMove
						snail[1] += moveDirections[key][1];
						// place snail (move snail in garden)
						garden[snail[0]][snail[1]] = SNAIL;
						break;
						}
					case SLIME:
						{
							counters[0] = 4;
							break;
						}
					case WALL:				//oops, garden wall
						printf("\a");		//produce a warning sound
						counters[0] = 5;
						break;				//& stay put
					case FROG:			//	kill snail if it throws itself at a frog!
						{
						garden[snail[0]][snail[1]] = SLIME;				// lay a final trail of slime
						snail[0] += moveDirections[key][0];							// go in direction indicated by keyMove
						snail[1] += moveDirections[key][1];
							
						// place snail (move snail in garden)
						garden[snail[0]][snail[1]] = SNAIL;
							
						counters[0] = 6;
						printf("\a\a\a\a");								// produce a death knell
						isSnailAlive = false;
							
						break;
						}
				}
					
			}else
			{
				counters[0] = 7;
			}
				
			/*********************************************************************************
			Dissolve the slime
			**********************************************************************************/

			if(++counters[1] >= SLIMELIFE)
			{
				counters[1] = 0;
			}

			if(slimeTrail[counters[1]][0] >= 0)
			{
				garden[slimeTrail[counters[1]][0]][slimeTrail[counters[1]][1]] = BLANK;
				slimeTrail[counters[1]][0] = -1;
			}

			/*********************************************************************************
			Move the frogs
			**********************************************************************************/

			// frog 1
			if((frogs[0] >= 0) && isSnailAlive)  // if frog not been gotten by an eagle or GameOver
			{	
				// if frog was blocking a lettuce, restore the lettuce (lettuce is 0x40, blank is 0x20)
				garden [frogs[0]][frogs[1]] = BLANK << static_cast<int>(lettucesBlocked[0]);

				// work out where to jump to depending on where the snail is...
				// see which way to jump in the Y direction (up and down)
					
				// the assembly code looks a bit chaotic because I reordered the original code I had resolve some register dependencies
				__asm
				{
					align 4								    ; ensure code starts on a word boundary

					mov         eax, dword ptr [snail]		; get snail[0]
					
					mov			esi, 0x4					; positive frogleap, necessary to store in registers as conditional move only works with two registers as operands
					
					mov			ebx, dword ptr [frogs]		; get frogs[0]
					
					mov			edi, 0xFFFFFFFC				; negative frogleap
					xor			ecx, ecx					; make sure ecx is zero (necessary when frog and snail have the same position)

					cmp         eax, ebx					; subtract frog Y from snail Y -> sets flags, no need to store result of substraction
						
					cmovg		ecx, esi					; set ecx to 4 (frogleap to the right) when result of sub positive
					mov			eax, dword ptr [snail + 4]	; get snail[1]

					cmovl		ecx, edi					; set ecx to -4 (frogleap to the left) when result of sub negative
					mov			edx, dword ptr [frogs + 4]	; get frogs[1]
					add			ebx, ecx					; add frogleap to frog[0]

					xor			ecx, ecx					; make sure ecx is zero (necessary when frog and snail have the same position)

					cmp			eax, edx					; compare the x coordinates of snail and frog 1

					cmovg		ecx, esi					; set ecx to 4 (frogleap to the right) when result of sub positive
					cmovl		ecx, edi					; set ecx to -4 (frogleap to the left) when result of sub negative
					mov			eax, 0x1C					; move right garden bound (SIZEX 2) into eax
					add			edx, ecx					; add frogleap to frog[1]

					mov			esi, 0x12					; move lower garden bound (SIZEY -2) into esi

					cmp			ebx, 0x13					; check whether frog jumped over the lower wall
					mov			edi, 0x1					; move upper and left garden bound (1) into edi
					cmovge		ebx, esi					; frog jumped over lower wall -> set back
					
					cmp			ebx, edi					; check whether frog jumped over upper wall
					cmovl		ebx, edi					; frog jumped over upper wall -> set back

					cmp			edx, 0x1D					; check whether frog jumped over the right wall
					
					mov         dword ptr [frogs], ebx		; store the y coordinate in frog[0]
					cmovge		edx, eax					; frog jumped over right wall -> set back
					
					cmp			edx, edi					; check whether frog jumped over the left wall
					cmovl		edx, edi					; frog jumped over left wall -> set back

					mov         dword ptr [frogs+4], edx	; store the x coordinate in frog[1]
				};
					
			
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
						printf("\a\a\a\a");									// produce a death knell
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
						
					printf("\a");												//produce a warning sound
				}
			}

			// frog 2
			if ((frogs[2] >= 0) && isSnailAlive)		// if frog not been gotten by an eagle or GameOver
			{	
				// jump off garden (taking any slug pellet with it)... check it wasn't on a lettuce though...

				garden [frogs[2]][frogs[3]] = BLANK << static_cast<int>(lettucesBlocked[1]);
					
					
				// work out where to jump to depending on where the snail is...
				__asm
				{
					align 4								    ; ensure code starts on a word boundary

					; move in y direction 
					mov         eax, dword ptr [snail]		; get snail[0]
					
					mov			esi, 0x4					; positive frogleap, necessary to store in registers as conditional move only works with two registers as operands
					
					mov			ebx, dword ptr [frogs+8]	; get frogs[2]
					
					mov			edi, 0xFFFFFFFC				; negative frogleap
					xor			ecx, ecx					; make sure ecx is zero (necessary when frog and snail have the same position)

					cmp         eax, ebx					; subtract frog Y from snail Y -> sets flags, no need to store result of substraction
						
					cmovg		ecx, esi					; set ecx to 4 (frogleap to the right) when result of sub positive
					mov			eax, dword ptr [snail + 4]	; get snail[1]
					cmovl		ecx, edi					; set ecx to -4 (frogleap to the left) when result of sub negative
					mov			edx, dword ptr [frogs + 12]	; get frogs[3]
					add			ebx, ecx					; add frogleap to frog[2]

					; move in x direction
					xor			ecx, ecx					; make sure ecx is zero (necessary when frog and snail have the same position)
					
					cmp			eax, edx					; compare the x coordinates of snail and frog 1

					cmovg		ecx, esi					; set ecx to 4 (frogleap to the right) when result of sub positive
					cmovl		ecx, edi					; set ecx to -4 (frogleap to the left) when result of sub negative
					mov			eax, 0x1C					; move right garden bound (SIZEX 2) into eax
					add			edx, ecx					; add frogleap to frog[3]

					; check whether frog jumped off garden

					mov			esi, 0x12					; move lower garden bound (SIZEY -2) into esi

					cmp			ebx, 0x13					; check whether frog jumped over the lower wall
					mov			edi, 0x1					; move upper and left garden bound (1) into edi
					cmovge		ebx, esi					; frog jumped over lower wall -> set back
					
					cmp			ebx, edi					; check whether frog jumped over upper wall
					cmovl		ebx, edi					; frog jumped over upper wall -> set back

					cmp			edx, 0x1D					; check whether frog jumped over the right wall
					
					mov         dword ptr [frogs+8], ebx	; store the y coordinate in frog[2]
					cmovge		edx, eax					; frog jumped over right wall -> set back
					
					cmp			edx, edi					; check whether frog jumped over the left wall
					cmovl		edx, edi					; frog jumped over left wall -> set back

					mov         dword ptr [frogs+12], edx	; store the x coordinate in frog[3]
				};
					
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
						printf("\a\a\a\a");									// produce a death knell
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
					printf("\a");												//produce a warning sound
				}
			}

			/*************************************************************************************
			Paint the game
			**************************************************************************************/
			
			paintGame  ( counters[0], counters[2], garden);		// display game info, garden & messages
			counters[0] = 12; // reset message
				
			//*************** end of timed section ******************************************
			// now everything is timed
				
			s.stopTimer(); // not part of game
			showFrameRate ( s.getElapsedTime());		// display frame rate - not part of game

			key = getKeyPress();						// display menu & read in next option
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

		paintGame( counters[0], counters[2], garden);			//display final game info, garden & message
		
		// another go
		key = getKeyPress();
		
		SelectBackColour( clRed);
		SelectTextColour( clYellow);
		Gotoxy(MLEFT, 18);
		printf("PRESS 'Q' TO QUIT OR ANY KEY TO CONTINUE");
		SelectBackColour( clBlack);
		SelectTextColour( clWhite);
		
	} 
	// finally done

	return 0;
} //end main

/******************************************************************************************
Output the game
*******************************************************************************************/

void paintGame (int msgId, int pellets, const char garden[][SIZEX+2])
{

	void paintGarden( const char [][SIZEX+2]);
	void showInformation(int, int);

	
	Clrscr();
	paintGarden ( garden);		// display garden contents
	showInformation(pellets, msgId); // show title, options, pellet count, data and time and message

} //end of paintGame

void paintGarden( const char garden[][SIZEX+2])
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


int getKeyPress()
{
	
	// the old manual input, just for testing
	int command;
	//read in the selected option
	command = _getch();  	// to read arrow keys
	while ( command == 224)	// to clear extra info from buffer
		command = _getch();

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

