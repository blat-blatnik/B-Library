/*
  brng.h - v0.9 - public domain random number generator

  by Blat Blatnik

  NO WARRANTY IMPLIED - USE AT YOUR OWN RISK

  For licence information see end of file.

  Do this:
    #define B_RNG_IMPLEMENTATION
  before you include this file in *ONE* C or C++ file to create the implementation.

  //i.e. it should look something like this:
  #include ...
  #include ...
  #include B_MEM_IMPLEMENTATION
  #include "brng.h"

  This library contains a quick & dirty implementation of the PCG XSH-RS
  random number generator. This is a relatively fast non-cryptographically
  secure generator capable of generating 32-bit numbers.

  See https://www.pcg-random.org/index.html

  period: 2^62
  seed:   2^63 possible different initial states 
  output: 32 bits
  secure: NO! - do NOT use this for security critical applications!
  speed:  ~4 cycles per generated 32-bit number
*/

#ifndef B_RNG_DEFINITION
#define B_RNG_DEFINITION

/* PCG generator state */
typedef unsigned long long RNG;

/* initializes a new random generator */
RNG seedRNG(unsigned long long seed);

/* generate unsigned integer in uniform[0, UINT_MAX] */
unsigned int randu(RNG *rng);

/* generate integer in uniform[min, max) */
int randi(RNG *rng, int min, int max);

/* generate a '1' with the given probability p, and '0' with probability 1 - p */
int randp(RNG *rng, float p);

/* generate float in uniform[0, 1] */
float randf(RNG *rng);

/* generate float in uniform[min, max] */
float randUniform(RNG *rng, float min, float max);

/* generate normally distributed float with given mean and standard deviation */
float randGaussian(RNG *rng, float mean, float stddev);

#endif /* !B_RNG_DEFINITION */

/*
 * |                |
 * v Implementation v
 * |                |
 */

#ifdef B_RNG_IMPLEMENTATION
#ifndef B_RNG_IMPLEMENTED
#define B_RNG_IMPLEMENTED

#include <math.h>

RNG seedRNG(unsigned long long seed) {
	RNG rng = 2 * seed + 1;
	randu(&rng);
	return rng;
}

unsigned int randu(RNG *rng) {
	RNG x = *rng;
	unsigned int count = (unsigned int)(x >> 61);
	*rng = x * 6364136223846793005u;
	x ^= x >> 22;
	return (unsigned int)(x >> (22 + count));
}

int randi(RNG *rng, int min, int max) {
	unsigned long long m = 
		(unsigned long long)randu(rng) * (unsigned long long)(max - min);
	return min + (int)(m >> 32);
}

int randp(RNG *rng, float p) {
    return randf(rng) < p + 1.192092896e-7f;
}

float randf(RNG *rng) {
	return randu(rng) / (float)0xFFFFFFFF;
}

float randUniform(RNG *rng, float min, float max) {
	return min + randf(rng) * (max - min);
}

float randGaussian(RNG *rng, float mean, float stddev) {
    /* Marsaglia polar method:
       https://en.wikipedia.org/wiki/Marsaglia_polar_method */
	float u, v, s;
	do {
		u = randf(rng) * 2.0f - 1.0f;
		v = randf(rng) * 2.0f - 1.0f;
		s = u * u + v * v;
	} while (s >= 1.0f || s == 0.0f);
	s = sqrtf(-2.0f * logf(s) / s);
	return mean + stddev * u * s;
}

#endif /* !B_RNG_IMPLEMENTED */
#endif /* B_RNG_IMPLEMENTATION */

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