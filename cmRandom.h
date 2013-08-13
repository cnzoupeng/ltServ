
/*
 * @file	CmRandomNumberGenerator.h
 * @brief	Random Number Generator H file
 * @author  Takayuki HARUKI (University of Toyama, Japan)
 * @since	Nov. 2005
 */


#ifndef _CM_RANDOM_NUMBER_GENERATOR_H_INCLUDED_
#define _CM_RANDOM_NUMBER_GENERATOR_H_INCLUDED_

typedef unsigned char		u8;
typedef	unsigned short		u16;
typedef unsigned int		u32;
typedef unsigned long long	u64;

#include <stdio.h>

//!< Period Parameter for Mersenne Twister
#define	N (624)
#define	M (397)


/*
 * @class	CmRandomNumberGenerator
 * @brief	Class (of mathematical module) for generating random number
 *			by Mersenne Twister
 * @author  Takayuki HARUKI (University of Toyama, Japan)
 * @data	Nov. 2005
 * @par	how to use
 *			-# get singleton object by using getInstance
 *			-# change seed by changeSeed if you want
 *			-# get random number by calling getFloat or getDouble functions
 *			-# call release finally only at once
 *
*/

class CmRandom
{
private:
	
	CmRandom();
	virtual ~CmRandom();
	
public:
	static void		changeSeed(u32 a_ulSeed);
	static float	getFloat();
	static double	getDouble();

	static u32 getUint(){
		return (u32)genrand_int32();
	}

	static int getInt()
	{
		return (int)genrand_int32();
	}

	static u64 getU64()
	{
		double x = getDouble();
		
		return *((u64*)(&x));
	}
	
private:	
	static void init_genrand(unsigned long s);	// initialize mt[N] with a seed
	static unsigned long genrand_int32(void);	// [0, 0xffffffff]
	static long genrand_int31(void);			// [0, 0x7fffffff]
	static double genrand_real1(void);			// [0, 1]
	static double genrand_real2(void);			// [0, 1)
	static double genrand_real3(void);			// (0, 1)
	
private:
	
	static CmRandom* s_pInstance;	
	static unsigned long	mt[N];	//!< array for the state vector
	static int				mti;	//!< mti==N+1 means mt[N] is not initialized
};

#endif // _CM_RANDOM_NUMBER_GENERATOR_H_INCLUDED_

