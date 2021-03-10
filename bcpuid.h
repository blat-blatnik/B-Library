/*
  bcpuid.h - v1.0 - public domain processor feature detector
  by Blat Blatnik

  NO WARRANTY IMPLIED - USE AT YOUR OWN RISK
  For licence information see end of file.

  Do this:
	#define B_CPUID_IMPLEMENTATION
  before you include this file in *ONE* C or C++ file to create the implementation.

  //i.e. it should look something like this:
  #include ...
  #include ...
  #include B_CPUID_IMPLEMENTATION
  #include "bcpuid.h"

  This file exposes one function: get_CPUID() which you can call to get 
  processor information at runtime using the CPUID instruction. It only
  detects features that I tought would be relevant to game programming.
  All detected features are shown in the example program below.

  The library was tested with: Ryzen 5 5600X, Ryzen 7 1700X, i5-7300HQ, i7-8550U

  ...........................
  ...   Example program   ...
  ...........................

  #define B_CPUID_IMPLEMENTATION
  #include "bcpuid.h"
  #include <stdio.h>
  
  int main(void)
  {
  	CPUID id = get_CPUID();
  
  	printf("--------------------------------------------------\n");
  	printf(" vendor:   %s\n", id.vendor);
  	printf(" name:     %s\n", id.name);
  	printf(" family:   %d\n", id.family);
  	printf(" model:    %d\n", id.model);
  	printf(" stepping: %d\n", id.stepping);
  	printf("--------------------------------------------------\n");
  	printf(" %2d physical cores\n", id.num_physical_cores);
  	printf(" %2d logical  cores\n", id.num_logical_cores);
  	printf("--------------------------------------------------\n");
  	printf(" L1i cache size:  %d kB (per core)\n", id.L1i_cache_size);
  	printf(" L1d cache size:  %d kB (per core)\n", id.L1d_cache_size);
  	printf(" L2  cache size:  %d kB (per core)\n", id.L2_cache_size);
  	printf(" L3  cache size:  %d kB (shared)\n", id.L3_cache_size);
  	printf(" cache line size: %d bytes\n", id.cache_line_size);
  	printf("--------------------------------------------------\n");
  	printf(" features: ");
  	if (id.feature_flags & CPUID_MMX)       printf("mmx ");
  	if (id.feature_flags & CPUID_SSE)       printf("sse ");
  	if (id.feature_flags & CPUID_SSE2)      printf("sse2 ");
  	if (id.feature_flags & CPUID_SSE3)      printf("sse3 ");
  	if (id.feature_flags & CPUID_SSSE3)     printf("ssse3 ");
  	if (id.feature_flags & CPUID_FMA)       printf("fma ");
  	if (id.feature_flags & CPUID_SSE41)     printf("sse41 ");
  	if (id.feature_flags & CPUID_SSE42)     printf("sse42 ");
  	if (id.feature_flags & CPUID_AVX)       printf("avx ");
  	if (id.feature_flags & CPUID_AVX2)      printf("avx2 ");
  	if (id.feature_flags & CPUID_AVX512_f)  printf("avx512_f ");
  	if (id.feature_flags & CPUID_AVX512_dq) printf("avx512_dq ");
  	if (id.feature_flags & CPUID_AVX512_bw) printf("avx512_bw ");
  	if (id.feature_flags & CPUID_AVX512_vl) printf("avx512_vl ");
  	printf("\n");
  	printf("--------------------------------------------------\n");
  
  	return 0;
  }

  ...................
  ...   Options   ...
  ...................

  #define B_CPUID_MEMSET(dest, val, count) your_memset_function(dest, val, count)
  #define B_CPUID_MEMCPY(dest, src, count) your_memcpy_function(dest, src, count)
  #define B_CPUID_STRSTR(str, substr) your_strstr_function(str, substr)
  - Avoid including string.h for memset, memcpy and strstr by defining ALL of these. 
    You have to either define ALL of them or NONE of them.
*/


#ifndef B_CPUID_DEFINITION
#define B_CPUID_DEFINITION

enum FeatureFlags
{
	CPUID_MMX         = (1 <<  0),
	CPUID_SSE         = (1 <<  1),
	CPUID_SSE2        = (1 <<  2),
	CPUID_SSE3        = (1 <<  3),
	CPUID_SSSE3       = (1 <<  4),
	CPUID_FMA         = (1 <<  5),
	CPUID_SSE41       = (1 <<  6),
	CPUID_SSE42       = (1 <<  7),
	CPUID_AVX         = (1 <<  8),
	CPUID_AVX2        = (1 <<  9),
	CPUID_AVX512_f    = (1 << 10),
	CPUID_AVX512_dq   = (1 << 11),
	CPUID_AVX512_bw   = (1 << 12),
	CPUID_AVX512_vl   = (1 << 13),
};

typedef struct CPUID
{
	char vendor[13];        // You can do strstr(vendor, "AMD") or strstr(vendor, "Intel") to determine the vendor.
	char name[49];          // You can determine the exact CPU model from this - or print it somewhere.

	int family;             // These fields give you an integer ID you can use to identify the exact processor if you need to.
	int model;
	int stepping;

	int cache_line_size;    // L1 cache line size in bytes.
	int L1i_cache_size;     // L1 instruction cache size *per core* in kilobytes.
	int L1d_cache_size;     // L1 data cache size *per core* in kilobytes.
	int L2_cache_size;      // L2 cache size *per core* in kilobytes.
	int L3_cache_size;      // L3 cache size (shared by all cores) in kilobytes.

	int num_physical_cores; // Number of physical cores on the chip. Some of them could be disabled - so this is only the maximum number possible for this CPU.
	int num_logical_cores;  // Number of logical cores. This will be 2 x num_physical_cores when the CPU *can* hyperthread, and 1 x num_physical_cores when it cannot.

	int feature_flags;      // Bitmask of relevant CPU features. See FeatureFlags.
} CPUID;

CPUID get_CPUID(void);

#endif // B_CPUID_DEFINITION

// 
// v-- Implementation --v
//

#if defined(B_CPUID_IMPLEMENTATION) && !defined(B_CPUID_IMPLEMENTED)
#define B_CPUID_IMPLEMENTED

#if !defined(B_MEMSET)
#include <string.h> // for memset, memcpy and strstr
#define B_MEMSET(dest, val, count) memset(dest, val, count)
#define B_MEMCPY(dest, src, count) memcpy(dest, src, count)
#define B_STRSTR(str, substr) strstr(str, substr)
#endif

#if defined(_MSC_VER)
#include <intrin.h>

static void b__cpuid(int leaf, int subleaf, int *eax, int *ebx, int *ecx, int *edx)
{
	int registers[4];
	if (subleaf == 0)
		__cpuid(registers, leaf);
	else
		__cpuidex(registers, leaf, subleaf);
	
	*eax = registers[0];
	*ebx = registers[1];
	*ecx = registers[2];
	*edx = registers[3];
}

static int b__cpuid_is_supported(void)
{
	// Try to set and clear bit 21 in the EFLAGS register. This indicates support for the CPUID instruction.
	// We bail out immediately if it's not supported.
	unsigned long long bit21 = 1llu << 21;

	__writeeflags(__readeflags() | bit21);
	if ((__readeflags() & bit21) == 0)
		return 0;

	__writeeflags(__readeflags() & ~bit21);
	if ((__readeflags() & bit21) == 1)
		return 0;

	return 1;
}
#elif defined(__GNUC__) || defined(__clang__)
#include <cpuid.h>

static void b__cpuid(int leaf, int subleaf, int *eax, int *ebx, int *ecx, int *edx)
{
	__cpuid_count(leaf, subleaf, *eax, *ebx, *ecx, *edx);
}

static int b__cpuid_is_supported(void)
{
	return __get_cpuid_max(0, 0) != 0;
}
#else
#error "This library only works with MSVC, clang, or GCC."
#endif // Compiler detecion.

static int b__extract_bits(int x, int highest, int lowest)
{
	unsigned int u = (unsigned int)x;
	u <<=  31 - highest;
	u >>= (31 - highest) + lowest;
	return (int)u;
}

static int b__extract_bit(int x, int index)
{
	return (int)((unsigned int)(x & (1 << index)) >> index);
}

CPUID get_CPUID(void)
{
	CPUID id;
	B_MEMSET(&id, 0, sizeof id);

	if (!b__cpuid_is_supported())
		return id;

	int eax, ebx, ecx, edx;

	b__cpuid(0, 0, &eax, &ebx, &ecx, &edx);
	int max_cpuid = eax;

	B_MEMCPY(id.vendor + 0, &ebx, 4);
	B_MEMCPY(id.vendor + 4, &edx, 4); // Note that the string is in ebx:e**D**x:e**C**x.
	B_MEMCPY(id.vendor + 8, &ecx, 4);

	if (max_cpuid < 1)
		return id;

	b__cpuid(1, 0, &eax, &ebx, &ecx, &edx);
	
	int stepping_id   = b__extract_bits(eax,  3, 0);
	int model_id      = b__extract_bits(eax,  7, 4);
	int family_id     = b__extract_bits(eax, 11, 8);
	int ext_model_id  = b__extract_bits(eax, 19, 16);
	int ext_family_id = b__extract_bits(eax, 27, 20);

	id.family = family_id;
	if (family_id == 15)
		id.family += ext_family_id;

	id.model = model_id;
	if (family_id == 6 || family_id == 15)
		id.model += (ext_model_id << 4);

	id.stepping = stepping_id;

	int has_clflush = b__extract_bit(edx, 19);
	if (has_clflush)
		id.cache_line_size = b__extract_bits(ebx, 15, 8);

	// Note that the value reported here is not actually accurate. On an i5-7300HQ it reports hyperthreading
	// even though that CPU doesn't do that. I've also read that the value reported on AMD is also not always
	// accurate. It basically seems like this is always on, regardless of whether the CPU actually does HT or not.
	int is_hyperthreaded = b__extract_bit(edx, 28);

	int features = 0;

	if (b__extract_bit(edx, 23)) features |= CPUID_MMX;
	if (b__extract_bit(edx, 25)) features |= CPUID_SSE;
	if (b__extract_bit(edx, 26)) features |= CPUID_SSE2;
	if (b__extract_bit(ecx,  0)) features |= CPUID_SSE3;
	if (b__extract_bit(ecx,  9)) features |= CPUID_SSSE3;
	if (b__extract_bit(ecx, 12)) features |= CPUID_FMA;
	if (b__extract_bit(ecx, 19)) features |= CPUID_SSE41;
	if (b__extract_bit(ecx, 20)) features |= CPUID_SSE42;
	if (b__extract_bit(ecx, 28)) features |= CPUID_AVX;
	
	if (max_cpuid >= 7)
	{
		b__cpuid(7, 0, &eax, &ebx, &ecx, &edx);
		if (b__extract_bit(ebx,  5)) features |= CPUID_AVX2;
		if (b__extract_bit(ebx, 16)) features |= CPUID_AVX512_f;
		if (b__extract_bit(ebx, 17)) features |= CPUID_AVX512_dq;
		if (b__extract_bit(ebx, 30)) features |= CPUID_AVX512_bw;
		if (b__extract_bit(ebx, 31)) features |= CPUID_AVX512_vl;
		// There are quite a few more AVX512 features - all of them are reported separately - but these are the "important" ones.
	}

	id.feature_flags = features;

	b__cpuid(0x80000000, 0, &eax, &ebx, &ecx, &edx);
	unsigned int max_cpuid_ex = (unsigned int)eax;

	if (max_cpuid_ex >= 0x80000004)
	{
		// We can just copy the name string directly, since it seems to be the correct endianness on both AMD and Intel.
		// It's also null terminated unlike the vendor string.
		b__cpuid(0x80000002, 0, (int *)id.name + 0, (int *)id.name + 1, (int *)id.name +  2, (int *)id.name +  3);
		b__cpuid(0x80000003, 0, (int *)id.name + 4, (int *)id.name + 5, (int *)id.name +  6, (int *)id.name +  7);
		b__cpuid(0x80000004, 0, (int *)id.name + 8, (int *)id.name + 9, (int *)id.name + 10, (int *)id.name + 11);
	}

	if (B_STRSTR(id.vendor, "AMD"))
	{
		if (max_cpuid_ex >= 0x80000008)
		{
			b__cpuid(0x80000008, 0, &eax, &ebx, &ecx, &edx);
			id.num_logical_cores = 1 + b__extract_bits(ecx, 7, 0);
		}
		else
		{
			b__cpuid(1, 0, &eax, &ebx, &ecx, &edx);
			id.num_logical_cores = b__extract_bits(ebx, 23, 16);
		}

		// This really isn't a great indication. Many sources say that CPUID reports hyperthreading even when the processor doesn't actually support it.
		// But I can't test this right now since I don't have a non-hyperthreaded AMD chip..
		if (is_hyperthreaded)
			id.num_physical_cores = id.num_logical_cores / 2;
		else
			id.num_physical_cores = id.num_logical_cores;

		if (max_cpuid_ex >= 0x80000005)
		{
			b__cpuid(0x80000005, 0, &eax, &ebx, &ecx, &edx);
			id.L1d_cache_size = b__extract_bits(ecx, 31, 24);
			id.L1i_cache_size = b__extract_bits(edx, 31, 24);
		}

		if (max_cpuid_ex >= 0x80000006)
		{
			b__cpuid(0x80000006, 0, &eax, &ebx, &ecx, &edx);
			id.L2_cache_size = b__extract_bits(ecx, 31, 16);
			id.L3_cache_size = 512 * b__extract_bits(edx, 31, 18); // This is reported in units of 512kB.
		}
	}
	else if (B_STRSTR(id.vendor, "Intel"))
	{
		if (max_cpuid >= 4)
		{
			b__cpuid(4, 0, &eax, &ebx, &ecx, &edx);

			// The value reported here is not accurate (I'm not sure if that's always the case).
			// On an i5-7300HQ it reports 8 logical cores with hyperthreading, even though that CPU
			// doesn't have hyperthreading.. Still this is a decent approximation at least.
			id.num_logical_cores  = 1 + b__extract_bits(eax, 31, 26);
			id.num_physical_cores = id.num_logical_cores;
			if (is_hyperthreaded)
				id.num_physical_cores /= 2;

			// Enumerate all caches to find out sizes.
			for (int index = 0;; ++index)
			{
				b__cpuid(4, index, &eax, &ebx, &ecx, &edx);
				int type = b__extract_bits(eax, 4, 0); // 0 - invalid, 1 - data cache, 2 - instruction cache, 3 - unified cache.
				if (type == 0)
					break;

				int level = b__extract_bits(eax, 7, 5);
				int ways       = 1 + b__extract_bits(ebx, 31, 22);
				int partitions = 1 + b__extract_bits(ebx, 21, 12);
				int line_size  = 1 + b__extract_bits(ebx, 11, 0);
				int sets       = 1 + b__extract_bits(ecx, 31, 0);
				int cache_size = ways * partitions * line_size * sets / 1024;

				if (level == 1)
				{
					if (type == 1)
						id.L1d_cache_size = cache_size;
					else if (type == 2)
						id.L1i_cache_size = cache_size;
					else // type == 3
					{
						// For unified L1 caches, set instruction cache size to 0 and set data cache size to the actual cache size.
						id.L1i_cache_size = 0;
						id.L1d_cache_size = cache_size;
					}
				}
				else if (level == 2)
					id.L2_cache_size = cache_size;
				else if (level == 3)
					id.L3_cache_size = cache_size;
			}
		}

		if (max_cpuid >= 0xB)
		{
			// This is a much better way of checking the number of cores than with cpuid(4) above.
			// At least this one is accurate on a i5-7300HQ and i7-8550U.
			b__cpuid(0xB, 0, &eax, &ebx, &ecx, &edx);
			int num_logical_processors_per_physical_core = b__extract_bits(ebx, 15, 0);
			is_hyperthreaded = num_logical_processors_per_physical_core == 2;

			b__cpuid(0xB, 1, &eax, &ebx, &ecx, &edx);
			id.num_logical_cores  = b__extract_bits(ebx, 15, 0);
			id.num_physical_cores = id.num_logical_cores / num_logical_processors_per_physical_core;
		}
	}

	return id;
}
#endif // B_CPUID_IMPLEMENTATION

/*
  ------------------------------------------------------------------------------
  This software is available under 2 licenses - choose whichever you prefer.
  ------------------------------------------------------------------------------
  ALTERNATIVE A - MIT License
  Copyright (c) 2021 Blat Blatnik
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