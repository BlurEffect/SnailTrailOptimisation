/* Use the following procedures/functions for the following results:

Clrscr() to clear the screen, and reset cursor to (0, 0)
Gotoxy(x, y) to move the text cursor to position (x, y)
SelectBackColour(colour) to select a background colour from the colour constants list
SelectTextColour(colour) to select a text colour from the colour constants list

It is recommended to use the SelectAttributes() and Clrscr() calls to prepare the screen
for coloured text output.

Be advised that you may experience apparent weirdness in the colouring of text. The
easiest way to view this conceptually is to assume that whenever you write text to
the screen the attributes will apply to all text sent to the screen from that point
until the next point at which you select different text or background colour, and
output text with the new attributes.

If you are familiar with using ANSI escape codes to output colour, this effect is
the same in principle as an effect applying until another escape code is used to
select another or the default effect, but it may appear confusing occuring when the
text cursor is moved around to output text to different locations on the screen.

A code example to illustrate this effect is provided after the uncommented
main() function. It might be expected that the coloured text output would only
colour the output text, but the selection of text colour or background attribute
is carried forwards from the last point on the screen where text was output with
it explicitly selected (which is the first character of any output using a
selected set of attributes).
*/

#include <windows.h>

WORD backColour( 0);
WORD textColour( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
WORD textAttributes( backColour | textColour);

//colour constants for translation
const int clBlack( 0);
const int clDarkRed( 1);
const int clDarkGreen( 2);
const int clDarkBlue( 3);
const int clDarkCyan( 4);
const int clDarkMagenta( 5);
const int clDarkYellow( 6);
const int clDarkGrey( 7);
const int clGrey( 8);
const int clRed( 9);
const int clGreen( 10);
const int clBlue( 11);
const int clCyan( 12);
const int clMagenta( 13);
const int clYellow( 14);
const int clWhite( 15);


void Clrscr(void)
{
	//from web-site
	COORD coordScreen = { 0, 0 }; 
	DWORD cCharsWritten; 
	CONSOLE_SCREEN_BUFFER_INFO csbi; 
	DWORD dwConSize; 
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); 
	GetConsoleScreenBufferInfo(hConsole, &csbi); 
	dwConSize = csbi.dwSize.X * csbi.dwSize.Y; 
	FillConsoleOutputCharacter(hConsole, TEXT(' '), dwConSize, 
	coordScreen, &cCharsWritten); 
	GetConsoleScreenBufferInfo(hConsole, &csbi); 
	FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, 
	coordScreen, &cCharsWritten); 
	SetConsoleCursorPosition(hConsole, coordScreen); 
}


void Gotoxy(int x, int y)
{
	//from web-site
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}


void SelectAttributes(void)
{
	textAttributes = backColour | textColour;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), textAttributes);
}

void SelectBackColour(int colour)
{
	void SelectAttributes(void);
	switch (colour)
	{
		case clBlack: backColour = 0; break;
		case clDarkRed: backColour = BACKGROUND_RED; break;
		case clDarkGreen: backColour = BACKGROUND_GREEN; break;
		case clDarkBlue: backColour = BACKGROUND_BLUE; break;
		case clDarkCyan: backColour = BACKGROUND_GREEN | BACKGROUND_BLUE; break;
		case clDarkMagenta: backColour = BACKGROUND_RED | BACKGROUND_BLUE; break;
		case clDarkYellow: backColour = BACKGROUND_RED | BACKGROUND_GREEN; break;
		case clDarkGrey: backColour = BACKGROUND_INTENSITY; break;
		case clGrey: backColour = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE; break;
		case clRed: backColour = BACKGROUND_INTENSITY | BACKGROUND_RED; break;
		case clGreen: backColour = BACKGROUND_INTENSITY | BACKGROUND_GREEN; break;
		case clBlue: backColour = BACKGROUND_INTENSITY | BACKGROUND_BLUE; break;
		case clCyan: backColour = BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_BLUE; break;
		case clMagenta: backColour = BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE; break;
		case clYellow: backColour = BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN; break;
		default: backColour = BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
	}
	SelectAttributes();
}

void SelectTextColour(int colour)
{
	void SelectAttributes(void);
	switch (colour)
	{
		case clBlack: textColour = 0; break;
		case clDarkRed: textColour = FOREGROUND_RED; break;
		case clDarkGreen: textColour = FOREGROUND_GREEN; break;
		case clDarkBlue: textColour = FOREGROUND_BLUE; break;
		case clDarkCyan: textColour = FOREGROUND_GREEN | FOREGROUND_BLUE; break;
		case clDarkMagenta: textColour = FOREGROUND_RED | FOREGROUND_BLUE; break;
		case clDarkYellow: textColour = FOREGROUND_RED | FOREGROUND_GREEN; break;
		case clDarkGrey: textColour = FOREGROUND_INTENSITY; break;
		case clGrey: textColour = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; break;
		case clRed: textColour = FOREGROUND_INTENSITY | FOREGROUND_RED; break;
		case clGreen: textColour = FOREGROUND_INTENSITY | FOREGROUND_GREEN; break;
		case clBlue: textColour = FOREGROUND_INTENSITY | FOREGROUND_BLUE; break;
		case clCyan: textColour = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE; break;
		case clMagenta: textColour = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE; break;
		case clYellow: textColour = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN; break;
		default: textColour = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
	}
	SelectAttributes();
}


int screenHeight(void)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	return csbi.srWindow.Bottom + 1;
}

int screenWidth(void)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	return csbi.srWindow.Right + 1;
}


