/*
  bdebug.h - v1.0 - public domain debuging utilities

  by Blat Blatnik
  
  NO WARRANTY IMPLIED - USE AT YOUR OWN RISK

  For licence information see end of file.
  
  Do this:
    #define B_DEBUG_IMPLEMENTATION
  before you include this file in *ONE* C or C++ file to create the implementation.
  
  //i.e. it should look something like this:
  #include ...
  #include ...
  #include B_DEBUG_IMPLEMENTATION
  #include "bdebug.h"
  
  This library includes a logging function and an assertion macro.

  debugLog(message, args...)
    Write a format message to the log file.
    If no log file is open, this will try to open the default log file (log.txt). 
    The special string "CLOSE" can be passed as a first argument to close the currently open log file.
    The special string "OPEN" can be passed as a first argument to open a new log file,
    whose filename must be passed as the second argument.
    All log messages are printed to stdout as well.
    
    //examples
    ...
    debugLog("hello")            // opens the default log file and appends "hello"
    debugLog("world")            // appends "world" to the open log file
    debugLog("CLOSE")            // close the log file
    debugLog("again")            // reopens the default log file and appends "again"
    debugLog("OPEN", "log2.txt") // closes the open log file and opens "log2.txt" as the new log file
    ...

  ASSERT(condition, [message, args...])
    Unlike the standard assert from assert.h, this assert will not call abort upon triggering.
    Also, you can pass in a string LITERAL (it has to be a literal) as a format message, and 
    arguments to format it in. If the passed in condition does not hold, the message and args
    are passed to debugLog, and then a debugger breakpoint is triggered.
    If you #define NDEBUG or #define B_DONT_ASSERT before #include -ing this file, 
    ASSERT will be defined as a NO-OP. But if you #define B_ALWAYS_ASSERT before #include -ing
    then NDEBUG and B_DONT_ASSERT are ignored, and ASSERT will NOT be defined as a NO-OP.

    //examples
    ...
    #line 1 "example.c"
    ASSERT(2 == 2);                                        // condition is true, so nothing happens
    ASSERT(2 == 4);                                        // output 1 is generated and a breakpoint is triggered
    ASSERT(2 == 4, "oh no");                               // output 2 is generated and a breakpoint is triggered
    ASSERT(2 == 4, "oh oh, %d != %d, %s", 2, 4, "panic!!") // generates output 3 and triggers a debugger breakpoint
    ...

    output 1:
      ERROR assert failed "2 == 4"
       in file example.c
       on line 2
    
    output 2:
      ERROR assert failed "2 == 4"
       in file example.c
       on line 3
       oh no
    
    output 3:
      ERROR assert failed "2 == 4"
       in file example.c
       on line 4
       oh oh, 2 != 4, panic!!
  
  -------------------
  ----- Options -----
  -------------------

  #define any of these before including this file for them to take effect:

  #define B_TRIGGER_BREAKPOINT() your-trigger-debugger-breakpoint()
  - Use this to cause ASSERT to trigger a breakpoint on failure. You have to define it
    to something like MSVC's __debugbreak().

  #define B_DONT_ASSERT
  #define NDEBUG
  - Both these will cause ASSERT to be disabled for use in release builds.

  #define B_ALWAYS_ASSERT
  - Ignore B_DONT_ASSERT and NDEBUG and have ASSERT work in release builds as well.
*/

#ifndef B_DEBUG_DEFINITION
#define B_DEBUG_DEFINITION

void debugLog(const char *message, ...);

#define B_STRINGIFY(identifier) #identifier
#define B_STRINGIFY2(identifier) B_STRINGIFY(identifier)

/* Triggers a debugger breakpoint. */
#ifndef B_TRIGGER_BREAKPOINT
#	ifdef _MSC_VER
#		define B_TRIGGER_BREAKPOINT() __debugbreak()
#	else
#		define B_TRIGGER_BREAKPOINT()
#	endif
#endif

#if defined(B_ALWAYS_ASSERT) || (!defined(B_DONT_ASSERT) && !defined(NDEBUG))
#	define ASSERT(condition, ...)\
		do {\
			if (!!!(condition)) {\
				debugLog(\
					"ERROR assert failed \"" #condition "\"\n"\
					" in file " __FILE__ "\n"\
					" on line " B_STRINGIFY2(__LINE__) "\n"\
					" " __VA_ARGS__);\
				B_TRIGGER_BREAKPOINT();\
			}\
		} while (0)
#else
#	define ASSERT(condition, ...) do {} while (0)
#endif

#endif /* !B_DEBUG_DEFINITION */

/*
 * |                |
 * v Implementation v
 * |                |
 */

#ifdef B_DEBUG_IMPLEMENTATION
#ifndef B_DEBUG_IMPLEMENTED
#define B_DEBUG_IMPLEMENTED

#include <stdio.h>  /* FILE, fopen, fclose, fprintf, vfprintf, stdout, fflush */
#include <stdarg.h> /* va_list, va_start, va_end */
#include <string.h> /* strcmp */
#include <time.h>   /* time_t, time, localtime, struct tm, strftime */

static FILE *b__logFile;

void debugLog(const char *message, ...) {

	if (strcmp(message, "CLOSE") == 0) {

		if (b__logFile) {
			debugLog("log closed");
			fclose(b__logFile);
			b__logFile = NULL;
		}
		return;

	} else if (strcmp(message, "OPEN") == 0) {
	
		va_list args;
		va_start(args, message);
		const char *filename = va_arg(args, const char *);
		va_end(args);
	
		if (b__logFile) {
			debugLog("log closed");
			fclose(b__logFile);
		}
		
		b__logFile = fopen(filename, "at");
		if (b__logFile)
			debugLog("log opened");
	
		return;
	}
	
	if (!b__logFile) {
		/* try to open the default log file */
		b__logFile = fopen("log.txt", "at");
		if (!b__logFile) {
			/* can't assert here because assert also calls
			   debugLog so we would enter an infinite loop.. */
			return;
		}
		debugLog("log opened");
	}

	char timestamp[64];
	time_t timeTicks = time(NULL);
	struct tm *t = localtime(&timeTicks);
	strftime(timestamp, sizeof(timestamp), "[%T] ", t);

	fprintf(stdout, timestamp);
	va_list args1;
	va_start(args1, message);
	vfprintf(stdout, message, args1);
	va_end(args1);
	fprintf(stdout, "\n");

	fprintf(b__logFile, timestamp);
	va_list args2;
	va_start(args2, message);
	vfprintf(b__logFile, message, args2);
	va_end(args2);
	fprintf(b__logFile, "\n");

	fflush(b__logFile);
}

#endif /* !B_DEBUG_IMPLEMENTED */
#endif /* B_DEBUG_IMPLEMENTATION */

/* 
  PUBLIC DOMAIN LICENCE
  
  This is free and unencumbered software released into the public domain.
  
  Anyone is free to copy, modify, publish, use, compile, sell, or
  distribute this software, either in source code form or as a compiled
  binary, for any purpose, commercial or non - commercial, and by any
  means.
  
  In jurisdictions that recognize copyright laws, the author or authors
  of this software dedicate any and all copyright interest in the
  software to the public domain. We make this dedication for the benefit
  of the public at large and to the detriment of our heirs and
  successors. We intend this dedication to be an overt act of
  relinquishment in perpetuity of all present and future rights to this
  software under copyright law.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE. 
*/