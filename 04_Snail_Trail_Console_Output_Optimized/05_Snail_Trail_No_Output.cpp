/*
05_Snail_Trail_No_Output
Kevin Meergans, 2013, based on the snail trail game by A. Oram
For this version, all output operations have been commented out, in order to allow proper timing of the rest of the 
game. That is, all calls to paintGame and all calls to printf (including the beeps) have been commented out. Also,
input is no longer read in from a file but received by accessing an array holding the recorded keycodes. 
In later versions the measurement method proved to be inadequate (as some frames were timed to last no time at all).
As a consequence, the measurement was changed retrospectively from this version onwards.
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

// needed for output with WriteConsoleOutput in paintgarden
COORD OUTPUT_BUFFER_START = {0, 0}; 
COORD OUTPUT_BUFFER_SIZE = {SIZEX, SIZEY};
SMALL_RECT OUTPUT_BUFFER_RECT = {0, 2, SIZEX, SIZEY + 1};

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

//define a few global control constants
int	 snailStillAlive (true);				// snail starts alive!
int  pellets( 0);							// number of times snail slimes over a slug pullet
int  lettucesEaten ( 0);					// win when this reaches LETTUCE_QUOTA
bool fullOfLettuce (false);					// when full and alive snail has won!

// The streams used to read in keys and to record frame rates for each iteration of the main game loop
ifstream inKeys; // used to read in key presses from a file
ofstream outFramerates("Framerates.txt"); // used to store the time needed for an iteration of the main loop

// the count of the current main loop iteration within the current cycle
int iterationCount = 0;
// run through the prerecorded 120 main loop iterations this many times
int numberOfCycles = 100000;

// from this version on, a new measurement method was used, as the old one proved to be problematic in later versions;
// accumulated times of all frames that were measured to be greater than zero
double accumulatedFramerates(0.0);
// counts the frames with times greater than zero
int frameCount(0);

// required for faster output;
queue<pair<COORD, char>> changeQueue;
int pelletCount = 0;
string lastMessage = " ";

// the user input (keycodes) that will be used to control the snail
int keys[133] = {75,75,80,80,75,75,75,75,80,75,75,75,75,75,75,75,75,75,75,75,75,75,75,75,72,75,72,72,72,75,75,72,77,72,77,72,72,72,77,72,72,77,77,77,77,77,75,75,77,77,77,72,77,77,77,77,100,100,100,77,77,72,80,80,75,75,72,72,72,75,80,77,77,77,77,77,77,77,75,75,75,75,75,75,80,77,77,77,77,77,72,72,72,77,77,77,77,77,80,75,80,80,80,80,80,75,75,75,75,75,75,75,72,80,80,75,72,102,75,72,72,72,77,77,80,75,75,80,77,80,80,113,113};
int keyCount = 0;

// Start of the 'SNAIL TRAIL' listing
//---------------------------------
int main()
{
	//function prototypes

	void initialiseGame ( char [][SIZEX], char [][SIZEX], char [][SIZEX], int [], int [][2], int&, int&, bool&);
	void paintGame ( string message, int, const char [][SIZEX], bool isInit);
	void clearMessage ( string& message);

	int getKeyPress ();
	void analyseKey ( string& message, int move[2], int);
	void moveSnail ( string&, int&, char [][SIZEX], char [][SIZEX], char [][SIZEX], int [], int []);
	void moveFrogs ( string&, int [], char [][SIZEX], char [][SIZEX],  int [][2]);
	void placeSnail( char [][SIZEX], int []);
	void dissolveSlime ( char [][SIZEX],char [][SIZEX]);
	void showLettuces  ( char [][SIZEX],char [][SIZEX]);

	int anotherGo (void);

	// Timing info
	void showFrameRate ( double);

	//local variables
	//arrays that store ...
	char garden		  [SIZEY][SIZEX];		// the game 'world'

	char slimeTrail	  [SIZEY][SIZEX];		// lifetime of slime counters overlay
	char lettucePatch [SIZEY][SIZEX];		// remember where the lettuces are planted

	int  snail[2];							// the snail's current position
	int  frogs [NUM_FROGS][2];				// coordinates of the frog contingent
	int  move[2];							// the requested move direction
	
	int  key, newGame (!QUIT);				// start new game by not quitting initially!
		
	CStopWatch s;							// create a stopwatch for timing

	for(int i = 0; i < numberOfCycles; ++i)
	{
		// Now start the game...

		Seed();									//seed the random number generator
		
		while ((newGame | 0x20) != QUIT)		// keep playing games
		{	
			//Clrscr();

			//initialise garden (incl. walls, frogs, lettuces & snail)
			initialiseGame( garden, slimeTrail, lettucePatch, snail, frogs, pellets, lettucesEaten, fullOfLettuce );
			string message( "READY TO SLITHER!? PRESS A KEY...");
			//paintGame( message, pellets, garden, true);			//display game info, garden & messages
		
			// read keys from array instead of calling the function
			key = keys[keyCount++];//getKeyPress();							//get started or quit game

			while (((key | 0x20) != QUIT) && snailStillAlive && !fullOfLettuce)	//user not bored, and snail not dead or full (main game loop)
			{
				s.startTimer(); // not part of game

				// ************** code to be timed ***********************************************
		
				analyseKey ( message, move, key);			// get next move from keyboard
				moveSnail ( message, pellets, garden, slimeTrail, lettucePatch, snail, move);
				dissolveSlime ( garden, slimeTrail);		// remove slime over time from garden
				showLettuces  ( garden, lettucePatch);		// show remaining lettuces on ground
				placeSnail ( garden, snail);				// move snail in garden
				moveFrogs  ( message, snail, garden, lettucePatch, frogs);	// frogs attempt to home in on snail
				
				//paintGame  ( message, pellets, garden, false);		// display game info, garden & messages
				clearMessage ( message);					// reset message array

				// *************** end of timed section ******************************************

				s.stopTimer(); // not part of game
				
				// check whether the frame was timed greater than zero
				if(s.getElapsedTime() > 0.0)
				{
					accumulatedFramerates += s.getElapsedTime();
					++frameCount;
				}

				//showFrameRate ( s.getElapsedTime());		// display frame rate - not part of game


				++iterationCount; 

				key = keys[keyCount++];//getKeyPress();						// display menu & read in next option
			}
			//							If alive...								If dead...
			(snailStillAlive) ? message = "WELL DONE, YOU'VE SURVIVED       " : message = "REST IN PEAS.                    ";
			if (!snailStillAlive)
			{
				garden[snail[0]][snail[1]] = DEADSNAIL;
				// update the change queue
				COORD coord = {snail[1], snail[0] + 2};
				changeQueue.push(pair<COORD, char>(coord, DEADSNAIL)); 
			}
			//paintGame( message, pellets, garden, false);			//display final game info, garden & message
		
			newGame = anotherGo();

		} 
		// finally done

		// close the stream to be able to read from anew in the next cycle
		inKeys.close();
		// reset the iteration count
		iterationCount = 0;

		// Reset game state
		snailStillAlive = true;		
		pellets = 0;						
		lettucesEaten = 0;					
		fullOfLettuce = false;	
		newGame = !QUIT;
		keyCount = 0;
	}


	ofstream outFramerates;
	outFramerates.open("Framerates.txt");
	outFramerates << accumulatedFramerates <<"\n" << frameCount;
	outFramerates.close();


	return 0;
} //end main
 
//**************************************************************************
//													set game configuration

void initialiseGame( char garden[][SIZEX], char slimeTrail[][SIZEX], char lettucePatch[][SIZEX],
					 int snail[], int frogs [][2], int& pellets, int& Eaten, bool& fullUp)
{ //initialise garden & place snail somewhere

	void setGarden( char [][SIZEX]);
	void setSnailInitialCoordinates( int []);
	void placeSnail( char [][SIZEX], int []);
	void initialiseSlimeTrail (char [][SIZEX]);
	void initialiseLettucePatch (char [][SIZEX]);
	void showLettuces (char [][SIZEX],char [][SIZEX]);
	void scatterStuff ( char [][SIZEX], char [][SIZEX], int []);
	void scatterFrogs   ( char [][SIZEX], int [], int [][2]);

	snailStillAlive = true;					// bring snail to life!
	setSnailInitialCoordinates( snail);		// initialise snail position
	setGarden( garden);						// reset the garden
	placeSnail( garden, snail);				// place snail at a random position in garden
	initialiseSlimeTrail (slimeTrail);		// no slime until snail moves
	initialiseLettucePatch (lettucePatch);	// lettuces not been planted yet
	scatterStuff ( garden, lettucePatch, snail);	// randomly scatter stuff about the garden (see function for details)
	showLettuces ( garden, lettucePatch);	// show lettuces on ground
	scatterFrogs ( garden, snail, frogs);	// randomly place a few frogs around

	pellets = 0;							// no slug pellets slithered over yet
	Eaten = 0;								// reset number of lettuces eaten
	fullUp = false;							// snail is hungry again
} 

//**************************************************************************
//												randomly drop snail in garden
void setSnailInitialCoordinates( int snail[])
{ //set snail's coordinates inside the garden at random at beginning of game

	snail[0] = Random( SIZEY-2);		// vertical coordinate in range [1..(SIZEY - 2)]
	snail[1] = Random( SIZEX-2);		// horizontal coordinate in range [1..(SIZEX - 2)]
} 

//**************************************************************************
//						set up garden array to represent blank garden and walls

void setGarden( char garden[][SIZEX])
{ //reset to empty garden configuration

	for ( int row( 0); row < SIZEY; ++row)			//for each row
	{	for ( int col( 0); col < SIZEX; ++col)		//for each col

		{	if (( row == 0) || (row == SIZEY-1))	//top & bottom walls
				garden[row][col] = WALL;			//draw a garden wall symbol
			else
				if (( col == 0) || (col == SIZEX-1))//left & right walls
					garden[row][col] = WALL;		//draw a garden wall symbol
				else
					garden[row][col] = BLANK;		//otherwise draw a space
		} 
	}
} //end of setGarden

//**************************************************************************
//														place snail in garden
void placeSnail( char garden[][SIZEX], int snail[])
{ //place snail at its new position in garden

	garden[snail[0]][snail[1]] = SNAIL;
	// update change queue
	COORD coord = {snail[1], snail[0] + 2};
	changeQueue.push(pair<COORD, char>(coord, SNAIL)); 
} //end of placeSnail

//**************************************************************************
//												slowly dissolve slime trail

void dissolveSlime ( char garden[][SIZEX],char slimeTrail[][SIZEX])
{// go through entire slime trail and decrement each item of slime in order

	for (int x=1; x < SIZEX-1; x++)
		for (int y=1; y < SIZEY-1; y++)
		{	if (slimeTrail [y][x] <= SLIMELIFE && slimeTrail [y][x] > 0)	// if this bit of slime exists
			{	slimeTrail [y][x] --;										// dissolve slime a little.
				if (slimeTrail [y][x] == 0)									// if totally dissolved then
				{
					garden [y][x] = BLANK;									// remove slime from garden
					// update change queue
					COORD coord = {x, y + 2};
					changeQueue.push(pair<COORD, char>(coord, BLANK)); 
				}
			}
		}
}

//**************************************************************************
//													show lettuces on garden

void showLettuces (char garden[][SIZEX],char lettucePatch[][SIZEX])
{	for (int x=1; x < SIZEX-1; x++)
		for (int y=1; y < SIZEY-1; y++)
			if (lettucePatch[y][x] == LETTUCE) garden[y][x] = LETTUCE;
}

//**************************************************************************
//													paint game on screen
void paintGame ( string msg, int pellets, const char garden[][SIZEX], bool isInit)
{ //display game title, messages, snail & other elements on screen

	void showTitle();
	void showDateAndTime();
	void paintGarden( const char [][SIZEX]);
	void showOptions();
	void showMessage( string);
	void showPelletCount( int);
	void paintGardenChanges( const char [][SIZEX]);

	if(isInit)
	{
		// a new game is being initialised, repaint the whole garden
		paintGarden ( garden);		// display garden contents
		showTitle();				// display game title, ok to paint once per game (won't be changed)
		showOptions();				// display menu options available (options stay the same for the whole time)
		showPelletCount ( pellets);	// display poisonous moves made so far
	}else
	{
		paintGardenChanges(garden); // update garden contents
		if(pellets > pelletCount)
		{
			showPelletCount ( pellets);	// display poisonous moves made so far  (count changes) 
			++pelletCount;
		}
	}

	//showDateAndTime();			// display system clock
	if(lastMessage.compare(msg) != 0)
	{
		showMessage ( msg);			// display status message, if any		(message changes)
		lastMessage = msg;
	}
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
			buffer[x + y * 30].Char.UnicodeChar = garden[y][x];
			// set bright white colour for every character
			buffer[x + y * 30].Attributes = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY;
		}
	}

	WriteConsoleOutput(GetStdHandle(STD_OUTPUT_HANDLE), buffer, OUTPUT_BUFFER_SIZE, OUTPUT_BUFFER_START, &OUTPUT_BUFFER_RECT);
	
} //end of paintGarden

// only paint changes to the garden, rest remains unaltered
void paintGardenChanges( const char garden[][SIZEX])
{
	while(!changeQueue.empty())
	{
		FillConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE), changeQueue.front().second, 1, changeQueue.front().first, NULL);
		changeQueue.pop();
	}
}

//**************************************************************************
//															no slime yet!
void initialiseSlimeTrail (char slimeTrail[][SIZEX])
{ // set the whole array to 0

	for (int x=1; x < SIZEX-1; x++)			// can't slime the walls
		for (int y=1; y < SIZEY-1; y++)
			slimeTrail [y][x] = 0;
}

//**************************************************************************
//															no lettuces yet!
void initialiseLettucePatch (char lettucePatch[][SIZEX])
{ // set the whole array to 0

	for (int x=1; x < SIZEX-1; x++)			// can't plant lettuces in walls!
		for (int y=1; y < SIZEY-1; y++)
			lettucePatch [y][x] = NO_LETTUCE;
}

//**************************************************************************
//												implement arrow key move
void analyseKey( string& msg, int move[2], int key)
{ //calculate snail movement required depending on the arrow key pressed

	switch( key)		//...depending on the selected key...
	{
		case LEFT:	//prepare to move left
			move[0] = 0; move[1] = -1;	// decrease the X coordinate
			break;
		case RIGHT: //prepare to move right
			move[0] = 0; move[1] = +1;	// increase the X coordinate
			break;
		case UP: //prepare to move up
			move[0] = -1; move[1] = 0;	// decrease the Y coordinate
			break;
		case DOWN: //prepare to move down
			move[0] = +1; move[1] = 0;	// increase the Y coordinate
			break;
		default:  					// this shouldn't happen
			msg = "INVALID KEY                      ";	// prepare error message
			move[0] = 0;			// move snail out of the garden
			move[1] = 0;
	}
}

//**************************************************************************
//			scatter some stuff around the garden (slug pellets and lettuces)

void scatterStuff ( char garden [][SIZEX], char lettucePatch [][SIZEX], int snail [])
{ 
	// ensure stuff doesn't land on the snail, or each other.
	// prime x,y coords with initial random numbers before checking

	for (int slugP=0; slugP < NUM_PELLETS; slugP++)								// scatter some slug pellets...
	{	int x(Random( SIZEX-2)), y(Random( SIZEY-2));
		while (( (y = Random( SIZEY-2)) == snail [0]) && ((x=Random( SIZEX-2)) == snail [1]) 
			    || garden [y][x] == PELLET) ;								// avoid snail and other pellets 
		garden [y][x] = PELLET;												// hide pellets around the garden
	}
	
	for (int food=0; food < LETTUCE_QUOTA; food++)							// scatter lettuces for eating...
	{	int x(Random( SIZEX-2)), y(Random( SIZEY-2));
		while (( (y = Random( SIZEY-2)) == snail [0]) && ((x=Random( SIZEX-2)) == snail [1]) 
			    || garden [y][x] == PELLET || lettucePatch [y][x] == LETTUCE) ;	// avoid snail, pellets and other lettucii 
		lettucePatch [y][x] = LETTUCE;										// plant a lettuce in the lettucePatch
	}
}

//**************************************************************************
//									some frogs have arrived looking for lunch

void scatterFrogs ( char garden [][SIZEX], int snail [], int frogs [][2])
{
	// need to avoid the snail initially (seems a bit unfair otherwise!). Frogs aren't affected by
	// slug pellets, btw, and will absorb them, and they may land on lettuces.

	for (int f=0; f < NUM_FROGS; f++)					// for each frog passing by ...
	{	int x(Random( SIZEX-2)), y(Random( SIZEY-2));	// prime coords before checking
		while (( (y = Random( SIZEY-2)) == snail [0]) && ((x=Random( SIZEX-2)) == snail [1])
				|| garden [y][x] == FROG) ;				// avoid snail and existing frogs 
		 
		frogs[f][0] = y;								// store initial positions of frog
		frogs[f][1] = x;
		garden [frogs[f][0]][frogs[f][1]] = FROG;		// put frogs on garden (this may overwrite a slug pellet)
	}
}

//**************************************************************************
//							move the Frogs toward the snail - watch for eagles!

void moveFrogs (string& msg, int snail [], char garden [][SIZEX], char lettuces [][SIZEX],  int frogs [][2])
{
//	Frogs move toward the snail. They jump 'n' positions at a time in either or both x and y
//	directions. If they land on the snail then it's dead meat. They might jump over it by accident. 
//	They can land on lettuces and slug pellets - in the latter case the pellet is
//  absorbed harmlessly by the frog (thus inadvertently helping the snail!).
//	Frogs may also be randomly eaten by an eagle, with only the bones left behind.
	
	bool eatenByEagle (char [][SIZEX], int []);
	
	for (int f=0; f<NUM_FROGS; f++)
	{	if ((frogs [f][0] != DEAD_FROG_BONES) && snailStillAlive)		// if frog not been gotten by an eagle or GameOver
		{	
			// jump off garden (taking any slug pellet with it)... check it wasn't on a lettuce though...

			if (lettuces [frogs[f][0]][frogs[f][1]] == LETTUCE)
			{
				garden [frogs[f][0]][frogs[f][1]] = LETTUCE;
				// update change queue
				COORD coord = {frogs[f][1], frogs[f][0] + 2};
				changeQueue.push(pair<COORD, char>(coord, LETTUCE)); 
			}else  
			{
				garden [frogs[f][0]][frogs[f][1]] = BLANK;
				// update change queue
				COORD coord = {frogs[f][1], frogs[f][0] + 2};
				changeQueue.push(pair<COORD, char>(coord, BLANK)); 
			}
			
			
			// work out where to jump to depending on where the snail is...
			// see which way to jump in the Y direction (up and down)

			if (snail[0] - frogs[f][0] > 0) 
				{frogs[f][0] += FROGLEAP;  if (frogs[f][0] >= SIZEY-1) frogs[f][0]=SIZEY-2;} // don't go over the garden walls!
			else if (snail[0] - frogs[f][0] < 0) 
				{frogs[f][0] -= FROGLEAP;  if (frogs[f][0] < 1) frogs[f][0]=1;		 };
			
			// see which way to jump in the X direction (left and right)

			if (snail[1] - frogs[f][1] > 0) 
				{frogs[f][1] += FROGLEAP;  if (frogs[f][1] >= SIZEX-1) frogs[f][1]=SIZEX-2;}
			else if (snail[1] - frogs[f][1] < 0) 
				{frogs[f][1] -= FROGLEAP;  if (frogs[f][1] < 1)	frogs[f][1]=1;		 };
		
			if (!eatenByEagle (garden, frogs[f]))							// not gotten by eagle?
				{if (frogs[f][0] == snail[0] && frogs[f][1] == snail[1])	// landed on snail? - grub up!
					{msg = "FROG GOT YOU!                    ";
					 //printf("\a\a\a\a");									// produce a death knell
					 snailStillAlive = false;								// snail is dead!
					}
				else 
				{
					garden [frogs[f][0]][frogs[f][1]] = FROG;				// display frog on garden (thus destroying any pellet that might be there).
					// update change queue
					COORD coord = {frogs[f][1], frogs[f][0] + 2};
					changeQueue.push(pair<COORD, char>(coord, FROG)); 
			}
				}
			else {
				msg = "EAGLE GOT A FROG                 ";
				//printf("\a");												//produce a warning sound
				}
		}
	}// end of FOR loop
}

bool eatenByEagle (char garden [][SIZEX], int frog[])
{ //There's a 1 in 'EagleStrike' chance of being eaten

	if (Random (EagleStrike) == EagleStrike)
	{	
		garden [frog[0]][frog[1]] = DEAD_FROG_BONES;				// show remnants of frog in garden
		// update change queue
		COORD coord = {frog[1], frog[0] + 2};
		changeQueue.push(pair<COORD, char>(coord, DEAD_FROG_BONES)); 

		frog [0] = DEAD_FROG_BONES;									// and mark frog as deceased
		return true;
	}
	else return false;
}

// end of moveFrogs

//**************************************************************************
//											implement player's move command

void moveSnail ( string& msg, int& pellets, char garden[][SIZEX], char slimeTrail[][SIZEX], char lettucePatch[][SIZEX], int snail[], int keyMove[] )
{ 
// move snail on the garden when possible.
// check intended new position & move if possible...
// ...depending on what's on the intended next position in garden.
	
	int targetY( snail[0] + keyMove[0]);
	int targetX( snail[1] + keyMove[1]);
	switch( garden[targetY][targetX]) //depending on what is at target position
	{
		case LETTUCE:		// increment lettuce count and win if snail is full
			{
			garden[snail[0]][snail[1]] = SLIME;				//lay a trail of slime
			// update change queue
			COORD coord = {snail[1], snail[0] + 2};
			changeQueue.push(pair<COORD, char>(coord, SLIME)); 
			slimeTrail[snail[0]][snail[1]] = SLIMELIFE;		//set slime lifespan
			snail[0] += keyMove[0];							//go in direction indicated by keyMove
			snail[1] += keyMove[1];
			lettucePatch [snail[0]][snail[1]] = NO_LETTUCE;	// eat the lettuce
			lettucesEaten++;								// keep a count
			fullOfLettuce = (lettucesEaten == LETTUCE_QUOTA); // if full, stop the game as snail wins!
			fullOfLettuce ? msg = "LAST LETTUCE EATEN               " : msg = "LETTUCE EATEN                    ";
			//fullOfLettuce ? printf("\a\a\a\a\a\a\a") : printf("\a");		
									//WIN! WIN! WIN!
			break;
			}
		case PELLET:		// increment pellet count and kill snail if > threshold
			{
			garden[snail[0]][snail[1]] = SLIME;				// lay a trail of slime
			// update change queue
			COORD coord = {snail[1], snail[0] + 2};
			changeQueue.push(pair<COORD, char>(coord, SLIME)); 
			slimeTrail[snail[0]][snail[1]] = SLIMELIFE;		// set slime lifespan
			snail[0] += keyMove[0];							// go in direction indicated by keyMove
			snail[1] += keyMove[1];
			pellets++;
			//printf("\a");									// produce a warning sound
			if (pellets >= PELLET_THRESHOLD)				// aaaargh! poisoned!
			{	msg = "TOO MANY PELLETS SLITHERED OVER! ";
				//printf("\a\a\a\a");							// produce a death knell
				snailStillAlive = false;					// game over
			}
			break;
			}
		case FROG:			//	kill snail if it throws itself at a frog!
			{
			garden[snail[0]][snail[1]] = SLIME;				// lay a final trail of slime
			// update change queue
			COORD coord = {snail[1], snail[0] + 2};
			changeQueue.push(pair<COORD, char>(coord, SLIME)); 
			snail[0] += keyMove[0];							// go in direction indicated by keyMove
			snail[1] += keyMove[1];
			msg = "OOPS! ENCOUNTERED A FROG!        ";
			//printf("\a\a\a\a");								// produce a death knell
			snailStillAlive = false;						// game over
			break;
			}
		case WALL:				//oops, garden wall
			//printf("\a");		//produce a warning sound
			msg = "THAT'S A WALL!                   ";	
			break;				//& stay put

		case BLANK:
		case DEAD_FROG_BONES:		//its safe to move over dead/missing frogs too
			{
			garden[snail[0]][snail[1]] = SLIME;				//lay a trail of slime
			// update change queue
			COORD coord = {snail[1], snail[0] + 2};
			changeQueue.push(pair<COORD, char>(coord, SLIME)); 
			slimeTrail[snail[0]][snail[1]] = SLIMELIFE;		//set slime lifespan
			snail[0] += keyMove[0];							//go in direction indicated by keyMove
			snail[1] += keyMove[1];
			break;
			}
		default: msg = "TRY A DIFFERENT DIRECTION        ";
	}
} //end of MoveSnail

//**************************************************************************
//											 get control key from player
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

//**************************************************************************
//											display info on screen
void clearMessage( string& msg)
{ //reset message to blank
	msg = "                                 ";
} //end of clearMessage

//**************************************************************************

void showTitle()
{ //display game title

	//Clrscr(); not required anymore, contents are just updated
	
	SelectBackColour( clBlack);
	SelectTextColour( clYellow);
	Gotoxy(0, 0);
	printf("...THE SNAIL TRAIL...\n");
	SelectBackColour( clWhite);
	SelectTextColour( clRed);

} //end of showTitle

void showDateAndTime()
{ //show current date and time

	SelectBackColour( clWhite);
	SelectTextColour( clBlack);
	Gotoxy(MLEFT, 1);
	printf("DATE: %s", GetDate());
	Gotoxy(MLEFT, 2);
	printf("TIME: %s", GetTime());
} //end of showDateAndTime

void showOptions()
{ //show game options

	SelectBackColour( clRed);
	SelectTextColour( clYellow);
	Gotoxy(MLEFT, 12);
	printf("TO MOVE USE ARROW KEYS - EAT ALL LETTUCES (%c)", LETTUCE);
	Gotoxy(MLEFT, 13);
	printf("TO QUIT USE 'Q'");
} //end of showOptions

void showPelletCount( int pellets)
{ //display number of pellets slimed over

	SelectBackColour( clBlack);
	SelectTextColour( clWhite);
	Gotoxy(MLEFT, 17);
	printf("SLITHERED OVER %d PELLETS SO FAR!", pellets);
} //end of showPelletCount

void showMessage( string msg)
{ //display auxiliary messages if any

	SelectBackColour( clBlack);
	SelectTextColour( clWhite);
	Gotoxy(MLEFT, 15);
	printf(msg.c_str());	//display current message
} //end of showMessage

int anotherGo (void)
{ //show end message and hold output screen
	/*
	SelectBackColour( clRed);
	SelectTextColour( clYellow);
	Gotoxy(MLEFT, 18);
	printf("PRESS 'Q' TO QUIT OR ANY KEY TO CONTINUE");
	SelectBackColour( clBlack);
	SelectTextColour( clWhite);
	*/
	return (keys[keyCount++]);//getKeyPress(););	
} // end of anotherGo

void showFrameRate ( double timeSecs)
{ // show time for one iteration of main game loop

	SelectBackColour( clBlack);
	SelectTextColour( clWhite);
	Gotoxy(MLEFT, 6);
	printf("FRAME RATE = %.3f at %.3f s/frame", (double) 1.0/timeSecs, timeSecs);
	//cout << setprecision(3)<<"FRAME RATE = " << (double) 1.0/timeSecs  << " at " << timeSecs << "s/frame" ;
} // end of showFrameRate

// End of the 'SNAIL TRAIL' listing

