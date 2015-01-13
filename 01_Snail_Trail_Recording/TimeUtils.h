#include <time.h>		//for time routines
#include <sys/types.h>
#include <sys/timeb.h>


void GetSystemTime( int& hrs, int& mins, int& secs)
{ //get time from system
	time_t ltime;
	struct tm *now;
    // Set time zone from TZ environment variable. If TZ is not set, the operating
    // system is queried to obtain the default value for the variable. 
    _tzset();
    // Get UNIX-style time. 
    time( &ltime );
    // Convert to time structure. 
	now =  localtime( &ltime );
    // Set Time objects members. 
	hrs = now->tm_hour ;
	mins = now->tm_min ;
	secs = now->tm_sec ;
}

string TimeToString( int hours, int minutes, int seconds) 
{ //convert the time to a string in 24-h digital clock format (00:00:00)
    char h[25], m[25], s[25];
    _itoa( hours, h, 10);
    _itoa( minutes, m, 10);
    _itoa( seconds, s, 10);
    string sH = h;
	string sM = m;
	string sS = s;
    if ( hours < 10) sH = "0"+sH;
	if ( minutes < 10) sM = "0"+sM;
	if ( seconds < 10) sS = "0"+sS;
    return (sH + ":" + sM + ":" + sS);
}

string GetTime() 
{ //return the current time in a string format
	int hrs, mins, secs;		//hold the current time
	GetSystemTime( hrs, mins, secs);
	return ( TimeToString( hrs, mins, secs));
}


void GetSystemDate( int& day, int& month, int& year)
{ //get date from system
	time_t ltime;
	struct tm *now;
    // Set time zone from TZ environment variable. If TZ is not set, the operating
    // system is queried to obtain the default value for the variable. 
    _tzset();
    // Get UNIX-style time. 
    time( &ltime );
    // Convert to time structure. 
	now =  localtime( &ltime );
    // Set Time objects members. 
	day = now->tm_mday;
	month = now->tm_mon + 1;
	year = now->tm_year + 1900;
}

string DateToString( int day, int month, int year) 
{ //convert the date to a string in format (dd/mm/yyyy)
    char d[25], m[25], y[25];
    _itoa( day, d, 10);
    _itoa( month, m, 10);
    _itoa( year, y, 10);
    string sD = d;
	string sM = m;
	string sY = y;
    if ( day < 10) sD = "0"+sD;
	if ( month < 10) sM = "0"+sM;
	if ( year < 10) sY = "0"+sY;
    return (sD + "/" + sM + "/" + sY);
}

string GetDate() 
{ //return the current date in a string format
	int day, month, year;		//hold the current date
	GetSystemDate( day, month, year);
	return ( DateToString( day, month, year));
}
