/*
  bfile.h - v0.1 - public domain file utilities

  by Blat Blatnik

  NO WARRANTY IMPLIED - USE AT YOUR OWN RISK

  For licence information see end of file.

  Do this:
    #define B_FILE_IMPLEMENTATION
  before you include this file in *ONE* C or C++ file to create the implementation.

  //i.e. it should look something like this:
  #include ...
  #include ...
  #include B_FILE_IMPLEMENTATION
  #include "bfile.h"

  NOTE: Currently, this file depends on the sys/stat.h header to get the time of last
        modification. Your compiler/OS does not provide this header then this will not
        compile, sorry.

  getFileSize
  - Return the full size of the given file in bytes.

  getFileTime
  - Return the time of last change of the given file as a time_t.

  readWholeFile
  - Reads the entire file and stores it in a malloce'd buffer. You
    need to call free on the return value.

  trackFileChanges
  - Register a given file to be tracked for changes.

  pollFileChanges
  - For all tracked files, check if they were modified and call
    a user passed function if they were.

  stopTrackingFiles
  - Stop tracking any files and free up used memory.

  -------------------
  ----- Options -----
  -------------------

  #define any of these before including this file for them to take effect:

  #define B_FILE_PREFIX(name) [your-name-prefix] ## name
  #define B_PREFIX(name) [your-name-prefix] ## name
  - Add a prefix to all functions/variables/types declared by this file.
    Useful for avoiding name-conflicts. By default no prefix is added.
  
  #define B_FILE_REALLOC(mem, size) [your-realloc(mem, size)]
  - Avoid using stdlib.h for realloc by definining your own realloc function
    that this library will use to manage memory.
*/

#ifndef B_FILE_DEFINITION
#define B_FILE_DEFINITION

#ifndef B_FILE_PREFIX
#   ifdef B_PREFIX
#       define B_FILE_PREFIX(name) B_PREFIX(name)
#   else
#       define B_FILE_PREFIX(name) name
#   endif
#endif

#include <time.h>

/* return '0' from callback to keep tracking file, or non-zero to stop tracking */
typedef int(*B_FILE_PREFIX(FileChangeCallback))(const char *filename, void *userData);

size_t  B_FILE_PREFIX(getFileSize)(const char *filename);
time_t  B_FILE_PREFIX(getFileTime)(const char *filename);
char *  B_FILE_PREFIX(readWholeFile)(const char *filename, size_t *outLength);
void    B_FILE_PREFIX(trackFileChanges)(const char *filename, void *userData, B_FILE_PREFIX(FileChangeCallback) callback);
void    B_FILE_PREFIX(pollFileChanges)(void);
void    B_FILE_PREFIX(stopTrackingFiles)(void);

#endif /* !B_FILE_DEFINITION */

/*
 * |                |
 * v Implementation v
 * |                |
 */

#ifdef B_FILE_IMPLEMENTATION
#ifndef B_FILE_IMPLEMENTED
#define B_FILE_IMPLEMENTED

#ifndef B_FILE_REALLOC
#include <stdlib.h>
#define B_FILE_REALLOC(mem, size) realloc(mem, size)
#endif

/* !!! platform dependant !!! */
#include <sys/stat.h>
/* !!! platform dependant !!! */

#include <string.h>

typedef struct b__FileTrackData {
	time_t lastChange;
	void *userData;
	B_FILE_PREFIX(FileChangeCallback) callback;
	char *filenamePtr;
	char filenameBuffer[64];
} b__FileTrackData;

/* tracked file data is stored in a DIY dynamic array */
static size_t b__trackedFilesCapacity;
static size_t b__numTrackedFiles;
static b__FileTrackData *b__trackedFiles;

size_t B_FILE_PREFIX(getFileSize)(const char *filename) {
	FILE *f = fopen(filename, "rb");
	if (!f)
		return 0;

	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fclose(f);
	return (size_t)fsize;
}

time_t B_FILE_PREFIX(getFileTime)(const char *filename) {
	struct stat s;
	int ret = stat(filename, &s);
	if (ret != 0)
		return (time_t)0;
	return s.st_mtime;
}

char *B_FILE_PREFIX(readWholeFile)(const char *filename, size_t *outLength) {
	FILE *f = fopen(filename, "rb");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	if (outLength)
		*outLength = (size_t)fsize;

	char *string = (char *)B_FILE_REALLOC(NULL, (size_t)fsize + 1);
	fread(string, 1, (size_t)fsize, f);
	fclose(f);

	string[fsize] = 0;
	return string;
}

void B_FILE_PREFIX(trackFileChanges)(const char *filename, void *userData, B_FILE_PREFIX(FileChangeCallback) callback) {
	FILE *f = fopen(filename, "rb");
	if (!f)
		return;
	fclose(f);

	size_t filenameSize = 1 + strlen(filename);

	b__FileTrackData data;
	data.lastChange = B_FILE_PREFIX(getFileTime)(filename);
	data.callback = callback;
	data.userData = userData;
	if (filenameSize <= sizeof(data.filenameBuffer)) {
		data.filenamePtr = NULL;
		memcpy(data.filenameBuffer, filename, filenameSize);
	} else {
		data.filenamePtr = (char *)B_FILE_REALLOC(NULL, filenameSize);
		memcpy(data.filenamePtr, filename, filenameSize);
	}

	if (b__numTrackedFiles + 1 > b__trackedFilesCapacity) {
		if (b__trackedFilesCapacity == 0)
			b__trackedFilesCapacity = 2;
		b__trackedFilesCapacity *= 2;
		b__trackedFiles = (b__FileTrackData *)B_FILE_REALLOC(b__trackedFiles, b__trackedFilesCapacity * sizeof(b__FileTrackData));
	}

	b__trackedFiles[b__numTrackedFiles] = data;
	++b__numTrackedFiles;
}

void B_FILE_PREFIX(pollFileChanges)(void) {
	for (size_t i = 0; i < b__numTrackedFiles; ++i) {
		b__FileTrackData *data = &b__trackedFiles[i];
		const char *filename = data->filenamePtr ? data->filenamePtr : &data->filenameBuffer[0];
		time_t changeTime = B_FILE_PREFIX(getFileTime)(filename);
		if (difftime(changeTime, data->lastChange) > 0) {
			data->lastChange = changeTime;
			int stopTracking = data->callback(filename, data->userData);
			/* returning non-zero means stop tracking */
			if (stopTracking) {
				if (data->filenamePtr)
					B_FILE_REALLOC(data->filenamePtr, 0);
				memmove(&b__trackedFiles[i], &b__trackedFiles[i + 1], (b__numTrackedFiles - i - 1) * sizeof(*data));
				--b__numTrackedFiles;
				--i; /* make sure to correct the loop */
			}
		}
	}
}

void B_FILE_PREFIX(stopTrackingFiles)(void) {
	for (size_t i = 0; i < b__numTrackedFiles; ++i)
		if (b__trackedFiles[i].filenamePtr)
			B_FILE_REALLOC(b__trackedFiles[i].filenamePtr, 0);
	B_FILE_REALLOC(b__trackedFiles, 0);
	b__trackedFiles = NULL;
	b__trackedFilesCapacity = 0;
	b__numTrackedFiles = 0;
}

#endif /* !B_FILE_IMPLEMENTED */
#endif /* B_FILE_IMPLEMENTATION */

/*
  ------------------------------------------------------------------------------
  This software is available under 2 licenses - choose whichever you prefer.
  ------------------------------------------------------------------------------
  ALTERNATIVE A - MIT License
  Copyright (c) 2020 Blat Blatnik
  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
  of the Software, and to permit persons to whom the Software is furnished to do
  so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
  ------------------------------------------------------------------------------
  ALTERNATIVE B - Public Domain (www.unlicense.org)
  This is free and unencumbered software released into the public domain.
  Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
  software, either in source code form or as a compiled binary, for any purpose,
  commercial or non-commercial, and by any means.
  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright interest in the software to the public
  domain. We make this dedication for the benefit of the public at large and to
  the detriment of our heirs and successors. We intend this dedication to be an
  overt act of relinquishment in perpetuity of all present and future rights to
  this software under copyright law.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  ------------------------------------------------------------------------------
*/