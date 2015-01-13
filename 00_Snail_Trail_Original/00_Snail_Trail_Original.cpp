/*
The original, unaltered snail trail program.
*/

//THE SNAIL TRAIL GAME
// A.Oram 2013, based on the P.Vacher's skeletal program.
 /*
 A snail (the player) moves about the garden leaving a trail of slime that it doesn't like to cross, but this does dissolve in due course. 
 Some swine has scattered a number of hard-to-see slug pellets around and if the snail slithers over a number of these it dies. 
 The garden has frogs that leap about trying to make lunch of the snail, but may jump over it by accident. They may incidentally
 soak up a slug pellet, thus helping the snail stay alive.
 If they land on it or the snail runs into a frog, it's curtains for the snail. 
 Frogs also have a certain % chance of being carried off by a hungry eagle, so the snail may live to see another day. 
 In this version there are a number of lettuces to eat - eat them all before being eaten and the snail wins!

 This code is certainly snail-like, can you get the frame rate up?


 Screenshot of console during gameplay:
___________________________________________________________________________
...THE SNAIL TRAIL...
                                   DATE: 11/12/2012
++++++++++++++++++++++++++++++     TIME: 18:42:30
+              .             +
+      -       .           - +
+           -  .          -  +
+              .        -    +     FRAME RATE = 19.6 at 0.051s/frame
+              .             +
+-         @   .             +
+              .        -    +
+              .             +
+  -           .             +
+       -      .             +     TO MOVE USE ARROW KEYS - EAT ALL LETTUCES (@)
+         -    .             +     TO QUIT USE 'Q'
+              ....&         +
+   -            M           +     LETTUCE EATEN
+                            +
+ @                   M      +     SLITHERED OVER 1 PELLETS!
+                           -+
+                            +
+                            +
++++++++++++++++++++++++++++++
___________________________________________________________________________

x co-ordinates follow this pattern (y co-ordinates similar):
                      
0,1,2 ...    ...SIZEX-2, SIZEX-1

Garden interior extends 1..SIZEX-2, 1..SIZEY-2.
Walls are at 0 and SIZEX-1, 0 and SIZEY-1.

Annoying bleeps from the PC's speaker announce significant events, check the message shown to see what happened.

*/

//---------------------------------
//include libraries
//include standard libraries
#include <iostream >         //for output and input
#include <iomanip>           //for formatted output in 'cout'
#include <conio.h>           //for getch
#include <fstream>           //for files
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

//define a few global control constants
int	 snailStillAlive (true);				// snail starts alive!
int  pellets( 0);							// number of times snail slimes over a slug pullet
int  lettucesEaten ( 0);					// win when this reaches LETTUCE_QUOTA
bool fullOfLettuce (false);					// when full and alive snail has won!

// Start of the 'SNAIL TRAIL' listing
//---------------------------------
int main()
{
	//function prototypes

	void initialiseGame ( char [][SIZEX], char [][SIZEX], char [][SIZEX], int [], int [][2], int&, int&, bool&);
	void paintGame ( string message, int, const char [][SIZEX]);
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
		
	// Now start the game...
	
	Seed();									//seed the random number generator
	CStopWatch s;							// create a stopwatch for timing
		
	while ((newGame | 0x20) != QUIT)		// keep playing games
	{	Clrscr();

		//initialise garden (incl. walls, frogs, lettuces & snail)
		initialiseGame( garden, slimeTrail, lettucePatch, snail, frogs, pellets, lettucesEaten, fullOfLettuce );
		string message( "READY TO SLITHER!? PRESS A KEY...");
		paintGame( message, pellets, garden);			//display game info, garden & messages
		
		key = getKeyPress();							//get started or quit game

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

			paintGame  ( message, pellets, garden);		// display game info, garden & messages
			clearMessage ( message);					// reset message array

			// *************** end of timed section ******************************************

			s.stopTimer(); // not part of game
			showFrameRate ( s.getElapsedTime());		// display frame rate - not part of game
		
			key = getKeyPress();						// display menu & read in next option
		}
		//							If alive...								If dead...
		(snailStillAlive) ? message = "WELL DONE, YOU'VE SURVIVED" : message = "REST IN PEAS.";
		if (!snailStillAlive) garden[snail[0]][snail[1]] = DEADSNAIL;
		paintGame( message, pellets, garden);			//display final game info, garden & message
		
		newGame = anotherGo();

	} // finally done

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
					garden [y][x] = BLANK;									// remove slime from garden
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
void paintGame ( string msg, int pellets, const char garden[][SIZEX])
{ //display game title, messages, snail & other elements on screen

	void showTitle();
	void showDateAndTime();
	void paintGarden( const char [][SIZEX]);
	void showOptions();
	void showMessage( string);
	void showPelletCount( int);
	
	showTitle();				// display game title
	//showDateAndTime();			// display system clock
	paintGarden ( garden);		// display garden contents
	showOptions();				// display menu options available
	showPelletCount ( pellets);	// display poisonous moves made so far
	showMessage ( msg);			// display status message, if any

} //end of paintGame

void paintGarden( const char garden[][SIZEX])
{ //display garden content on screen
	
	SelectBackColour( clBlack);
	SelectTextColour( clWhite);
	Gotoxy(0, 2);
	for (int y( 0); y < (SIZEY); ++y)		//for each row
	{	for (int x( 0); x < (SIZEX); ++x)	//for each col
		{	
			cout << garden[y][x];			// display current garden contents
		} 
		cout << endl;
	}
	
} //end of paintGarden

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
			msg = "INVALID KEY";	// prepare error message
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
				  garden [frogs[f][0]][frogs[f][1]] = LETTUCE;
			else  garden [frogs[f][0]][frogs[f][1]] = BLANK;
			
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
					{msg = "FROG GOT YOU!";
					 cout << "\a\a\a\a";									// produce a death knell
					 snailStillAlive = false;								// snail is dead!
					}
				else garden [frogs[f][0]][frogs[f][1]] = FROG;				// display frog on garden (thus destroying any pellet that might be there).
				}
			else {
				msg = "EAGLE GOT A FROG";
				cout << '\a';												//produce a warning sound
				}
		}
	}// end of FOR loop
}

bool eatenByEagle (char garden [][SIZEX], int frog[])
{ //There's a 1 in 'EagleStrike' chance of being eaten

	if (Random (EagleStrike) == EagleStrike)
	{	garden [frog[0]][frog[1]] = DEAD_FROG_BONES;				// show remnants of frog in garden
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
			garden[snail[0]][snail[1]] = SLIME;				//lay a trail of slime
			slimeTrail[snail[0]][snail[1]] = SLIMELIFE;		//set slime lifespan
			snail[0] += keyMove[0];							//go in direction indicated by keyMove
			snail[1] += keyMove[1];
			lettucePatch [snail[0]][snail[1]] = NO_LETTUCE;	// eat the lettuce
			lettucesEaten++;								// keep a count
			fullOfLettuce = (lettucesEaten == LETTUCE_QUOTA); // if full, stop the game as snail wins!
			fullOfLettuce ? msg = "LAST LETTUCE EATEN" : msg = "LETTUCE EATEN";
			fullOfLettuce ? cout << "\a\a\a\a\a\a\a" : cout << "\a";		
									//WIN! WIN! WIN!
			break;

		case PELLET:		// increment pellet count and kill snail if > threshold
			garden[snail[0]][snail[1]] = SLIME;				// lay a trail of slime
			slimeTrail[snail[0]][snail[1]] = SLIMELIFE;		// set slime lifespan
			snail[0] += keyMove[0];							// go in direction indicated by keyMove
			snail[1] += keyMove[1];
			pellets++;
			cout << '\a';									// produce a warning sound
			if (pellets >= PELLET_THRESHOLD)				// aaaargh! poisoned!
			{	msg = "TOO MANY PELLETS SLITHERED OVER!";
				cout << "\a\a\a\a";							// produce a death knell
				snailStillAlive = false;					// game over
			}
			break;
		
		case FROG:			//	kill snail if it throws itself at a frog!
			garden[snail[0]][snail[1]] = SLIME;				// lay a final trail of slime
			snail[0] += keyMove[0];							// go in direction indicated by keyMove
			snail[1] += keyMove[1];
			msg = "OOPS! ENCOUNTERED A FROG!";
			cout << "\a\a\a\a";								// produce a death knell
			snailStillAlive = false;						// game over
			break;

		case WALL:				//oops, garden wall
			cout << '\a';		//produce a warning sound
			msg = "THAT'S A WALL!";	
			break;				//& stay put

		case BLANK:
		case DEAD_FROG_BONES:		//its safe to move over dead/missing frogs too
			garden[snail[0]][snail[1]] = SLIME;				//lay a trail of slime
			slimeTrail[snail[0]][snail[1]] = SLIMELIFE;		//set slime lifespan
			snail[0] += keyMove[0];							//go in direction indicated by keyMove
			snail[1] += keyMove[1];
			break;

		default: msg = "TRY A DIFFERENT DIRECTION";
	}
} //end of MoveSnail

//**************************************************************************
//											 get control key from player
int getKeyPress()
{ //get command from user

	int command;
	//read in the selected option
	command = _getch();  	// to read arrow keys
	while ( command == 224)	// to clear extra info from buffer
		command = _getch();
	return( command);

} //end of getKeyPress

//**************************************************************************
//											display info on screen
void clearMessage( string& msg)
{ //reset message to blank
	msg = "";
} //end of clearMessage

//**************************************************************************

void showTitle()
{ //display game title

	Clrscr();
	SelectBackColour( clBlack);
	SelectTextColour( clYellow);
	Gotoxy(0, 0);
	cout << "...THE SNAIL TRAIL..." << endl;
	SelectBackColour( clWhite);
	SelectTextColour( clRed);

} //end of showTitle

void showDateAndTime()
{ //show current date and time

	SelectBackColour( clWhite);
	SelectTextColour( clBlack);
	Gotoxy(MLEFT, 1);
	cout << "DATE: " << GetDate();
	Gotoxy(MLEFT, 2);
	cout << "TIME: " << GetTime();
} //end of showDateAndTime

void showOptions()
{ //show game options

	SelectBackColour( clRed);
	SelectTextColour( clYellow);
	Gotoxy(MLEFT, 12);
	cout << "TO MOVE USE ARROW KEYS - EAT ALL LETTUCES (" <<LETTUCE << ')';
	Gotoxy(MLEFT, 13);
	cout << "TO QUIT USE 'Q'";
} //end of showOptions

void showPelletCount( int pellets)
{ //display number of pellets slimed over

	SelectBackColour( clBlack);
	SelectTextColour( clWhite);
	Gotoxy(MLEFT, 17);
	cout << "SLITHERED OVER " << pellets << " PELLETS SO FAR!";
} //end of showPelletCount

void showMessage( string msg)
{ //display auxiliary messages if any

	SelectBackColour( clBlack);
	SelectTextColour( clWhite);
	Gotoxy(MLEFT, 15);
	cout << msg;	//display current message
} //end of showMessage

int anotherGo (void)
{ //show end message and hold output screen

	SelectBackColour( clRed);
	SelectTextColour( clYellow);
	Gotoxy(MLEFT, 18);
	cout << "PRESS 'Q' TO QUIT OR ANY KEY TO CONTINUE";
	SelectBackColour( clBlack);
	SelectTextColour( clWhite);
	return (getKeyPress());	
} // end of anotherGo

void showFrameRate ( double timeSecs)
{ // show time for one iteration of main game loop

	SelectBackColour( clBlack);
	SelectTextColour( clWhite);
	Gotoxy(MLEFT, 6);
	cout << setprecision(3)<<"FRAME RATE = " << (double) 1.0/timeSecs  << " at " << timeSecs << "s/frame" ;
} // end of showFrameRate

// End of the 'SNAIL TRAIL' listing

