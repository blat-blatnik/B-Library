#ifndef B_FILE_DEFINITION
#define B_FILE_DEFINITION

#include <time.h>

size_t getFileSize(const char *filename);

time_t getFileTime(const char *filename);

char *readWholeFile(const char *filename, size_t *outLength);

void trackFileChanges(const char *filename, void *userData, int(*callback)(const char *filename, void *userData));

void checkTrackedFiles(void);

void stopTrackingFiles(void);

#endif /* !B_FILE_DEFINITION */

#ifdef B_FILE_IMPLEMENTATION
#ifndef B_FILE_IMPLEMENTED
#define B_FILE_IMPLEMENTED

#include <sys/stat.h>
#include <string.h>

typedef struct b__FileTrackData {
	time_t lastChange;
	void *userData;
	int(*callback)(const char *filename, void *userData);
	char *filenamePtr;
	char filenameBuffer[64];
} b__FileTrackData;

/* tracked file data is stored in a DIY dynamic array */
static size_t b__trackedFilesCapacity;
static size_t b__numTrackedFiles;
static b__FileTrackData *b__trackedFiles;

size_t getFileSize(const char *filename) {
	FILE *f = fopen(filename, "rb");
	if (!f)
		return 0;

	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fclose(f);
	return (size_t)fsize;
}

time_t getFileTime(const char *filename) {
	struct stat s;
	int ret = stat(filename, &s);
	if (ret != 0)
		return (time_t)0;
	return s.st_mtime;
}

char *readWholeFile(const char *filename, size_t *outLength) {
	FILE *f = fopen(filename, "rb");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	if (outLength)
		*outLength = (size_t)fsize;

	char *string = (char *)malloc((size_t)fsize + 1);
	fread(string, 1, (size_t)fsize, f);
	fclose(f);

	string[fsize] = 0;
	return string;
}

void trackFileChanges(const char *filename, void *userData, int(*callback)(const char *filename, void *userData)) {
	FILE *f = fopen(filename, "rb");
	if (!f)
		return;
	fclose(f);

	size_t filenameSize = 1 + strlen(filename);

	b__FileTrackData data;
	data.lastChange = getFileTime(filename);
	data.callback = callback;
	data.userData = userData;
	if (filenameSize <= sizeof(data.filenameBuffer)) {
		data.filenamePtr = NULL;
		memcpy(data.filenameBuffer, filename, filenameSize);
	} else {
		data.filenamePtr = (char *)malloc(filenameSize);
		memcpy(data.filenamePtr, filename, filenameSize);
	}

	if (b__numTrackedFiles + 1 > b__trackedFilesCapacity) {
		if (b__trackedFilesCapacity == 0)
			b__trackedFilesCapacity = 2;
		b__trackedFilesCapacity *= 2;
		b__trackedFiles = (b__FileTrackData *)realloc(b__trackedFiles, b__trackedFilesCapacity * sizeof(b__FileTrackData));
	}

	b__trackedFiles[b__numTrackedFiles] = data;
	++b__numTrackedFiles;
}

void checkTrackedFiles(void) {
	for (size_t i = 0; i < b__numTrackedFiles; ++i) {
		b__FileTrackData *data = &b__trackedFiles[i];
		const char *filename = data->filenamePtr ? data->filenamePtr : &data->filenameBuffer[0];
		time_t changeTime = getFileTime(filename);
		if (difftime(changeTime, data->lastChange) > 0) {
			data->lastChange = changeTime;
			int continueTracking = data->callback(filename, data->userData);
			/* returning 0 means stop tracking */
			if (!continueTracking) {
				if (data->filenamePtr)
					free(data->filenamePtr);
				memmove(&b__trackedFiles[i], &b__trackedFiles[i + 1], (b__numTrackedFiles - i - 1) * sizeof(*data));
				--b__numTrackedFiles;
				--i; /* make sure to correct the loop */
			}
		}
	}
}

void stopTrackingFiles(void) {
	for (size_t i = 0; i < b__numTrackedFiles; ++i)
		if (b__trackedFiles[i].filenamePtr)
			free(b__trackedFiles[i].filenamePtr);
	free(b__trackedFiles);
	b__trackedFiles = NULL;
	b__trackedFilesCapacity = 0;
	b__numTrackedFiles = 0;
}

#endif /* !B_FILE_IMPLEMENTED */
#endif /* B_FILE_IMPLEMENTATION */

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