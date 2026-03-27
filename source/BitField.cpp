/*                                                                  BitField.cpp
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                         BIT FIELD LIBRARY TEST SUITE                         #
#                                                                              #
# Ordnung muss sein!                             Copyleft (Ɔ) Eugene Zamlinsky #
################################################################################
*/
# include	<cstring>
# include	<Common.h>
# include	<RandomArray.h>
# include	<BitField.h>

//****************************************************************************//
//      Test parameters                                                       //
//****************************************************************************//
# define	SCALE		12				// Scale factor
# define	SIZE		(1 << SCALE) 	// Generate 2 ^ SCALE elements-long arrays
# define	ROUNDS		100				// Rounds for random testing
# define	TRIES		8				// Tries in each round
# define	SEED		100				// Seed value for the random number generator
# define	MAX_VALUE	127				// Max generated random value

//****************************************************************************//
//      Test a scalar function                                                //
//****************************************************************************//
template <typename type_t>
void TestBitField (
	void (*func)(RandomArray <type_t>&, size_t, size_t, size_t)
){
	// Print test info
	PrintInfo (typeid (type_t).name());

	// Create an array of the target size
	RandomArray <type_t> array (SIZE, SEED, MAX_VALUE);

	// Run the test in many rounds with random offset and elements count
	for (size_t i = 0; i < ROUNDS; i++) {

		// Get a random offset inside the array and a random number of elements
		// to work with
		size_t offset = array.Offset ();
		size_t pos1 = array.Count (offset);
		size_t pos2 = array.Count (offset);

		// Do many tries with the same offset and element count, but different data
		for (size_t j = 0; j < TRIES; j++) {

			// Populate the array with random data
			array.Populate ();

			// Apply the operation to the array data
			func (array, offset, pos1, pos2);
		}
	}
}

//****************************************************************************//
//      Reference functions                                                   //
//****************************************************************************//

//============================================================================//
//      Get bit value from a bit field                                        //
//============================================================================//
template <typename type_t>
bool GetBit (const type_t array[], size_t index) {
	const unsigned char *ptr = reinterpret_cast <const unsigned char*> (array);
	size_t bit_index = index & 0x7;
	unsigned char mask = 1 << bit_index;
	index >>= 3;
	return ptr[index] & mask;
}

//============================================================================//
//      Set bit value in a bit field                                          //
//============================================================================//
template <typename type_t>
void SetBit (type_t array[], size_t index) {
	unsigned char *ptr = reinterpret_cast <unsigned char*> (array);
	size_t bit_index = index & 0x7;
	unsigned char mask = 1 << bit_index;
	index >>= 3;
	ptr[index] |= mask;
}

//============================================================================//
//      Set bits in the bit field range                                       //
//============================================================================//
template <typename type_t>
void SetBits (type_t array[], size_t spos, size_t epos) {
	for (size_t i = spos; i < epos; ++i)
		SetBit (array, i);
}

//============================================================================//
//      Reset bit value in a bit field                                        //
//============================================================================//
template <typename type_t>
void ResetBit (type_t array[], size_t index) {
	unsigned char *ptr = reinterpret_cast <unsigned char*> (array);
	size_t bit_index = index & 0x7;
	unsigned char mask = 1 << bit_index;
	index >>= 3;
	ptr[index] &= ~mask;
}

//============================================================================//
//      Reset bits in the bit field range                                     //
//============================================================================//
template <typename type_t>
void ResetBits (type_t array[], size_t spos, size_t epos) {
	for (size_t i = spos; i < epos; ++i)
		ResetBit (array, i);
}

//============================================================================//
//      Invert bit value in a bit field                                       //
//============================================================================//
template <typename type_t>
void InvertBit (type_t array[], size_t index) {
	unsigned char *ptr = reinterpret_cast <unsigned char*> (array);
	size_t bit_index = index & 0x7;
	unsigned char mask = 1 << bit_index;
	index >>= 3;
	ptr[index] ^= mask;
}

//============================================================================//
//      Invert bits in the bit field range                                    //
//============================================================================//
template <typename type_t>
void InvertBits (type_t array[], size_t spos, size_t epos) {
	for (size_t i = spos; i < epos; ++i)
		InvertBit (array, i);
}

//============================================================================//
//      Searching for a set bit                                               //
//============================================================================//
template <typename type_t>
size_t FindSetBitFwd (const type_t array[], size_t spos, size_t epos) {
	for (size_t i = spos; i < epos; ++i) {
		const size_t pos = i;
		const size_t index = pos / (sizeof (type_t) * 8);
		const int bit = pos % (sizeof (type_t) * 8);
		if (array [index] & (1ULL << bit))
			return pos;
	}
	return static_cast <size_t> (-1);
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template <typename type_t>
size_t FindSetBitBwd (const type_t array[], size_t spos, size_t epos) {
	for (size_t i = epos; i > spos; --i) {
		const size_t pos = i - 1;
		const size_t index = pos / (sizeof (type_t) * 8);
		const int bit = pos % (sizeof (type_t) * 8);
		if (array [index] & (1ULL << bit))
			return pos;
	}
	return static_cast <size_t> (-1);
}

//============================================================================//
//      Searching for a reset bit                                             //
//============================================================================//
template <typename type_t>
size_t FindResetBitFwd (const type_t array[], size_t spos, size_t epos) {
	for (size_t i = spos; i < epos; ++i) {
		const size_t pos = i;
		const size_t index = pos / (sizeof (type_t) * 8);
		const int bit = pos % (sizeof (type_t) * 8);
		if (~(array [index]) & (1ULL << bit))
			return pos;
	}
	return static_cast <size_t> (-1);
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template <typename type_t>
size_t FindResetBitBwd (const type_t array[], size_t spos, size_t epos) {
	for (size_t i = epos; i > spos; --i) {
		const size_t pos = i - 1;
		const size_t index = pos / (sizeof (type_t) * 8);
		const int bit = pos % (sizeof (type_t) * 8);
		if (~array [index] & (1ULL << bit))
			return pos;
	}
	return static_cast <size_t> (-1);
}

//============================================================================//
//      Bit counting                                                          //
//============================================================================//
template <typename type_t>
size_t CountBits (const type_t array[], size_t spos, size_t epos) {
	size_t count = 0;
	for (size_t i = spos; i < epos; ++i) {
		const size_t pos = i;
		const size_t index = pos / (sizeof (type_t) * 8);
		const int bit = pos % (sizeof (type_t) * 8);
		if (array [index] & (1ULL << bit))
			count++;
	}
	return count;
}

//============================================================================//
//      Check for a set bit in the bit field range                            //
//============================================================================//
template <typename type_t>
bool CheckSetBit (const type_t array[], size_t spos, size_t epos){
	return FindSetBitFwd (array, spos, epos) != static_cast <size_t> (-1);
}

//============================================================================//
//      Check for a reset bit in the bit field range                          //
//============================================================================//
template <typename type_t>
bool CheckResetBit (const type_t array[], size_t spos, size_t epos){
	return FindResetBitFwd (array, spos, epos) != static_cast <size_t> (-1);
}

//****************************************************************************//
//      Testing functions                                                     //
//****************************************************************************//
# define	GET1(fname)															\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t pos1,																\
	size_t pos2																	\
){																				\
	bool computed_value = BitField::fname (array.Data() + offset, pos1);		\
	bool correct_value = fname (array.Data() + offset, pos1);					\
	CheckResult (computed_value, correct_value);								\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	GET2(fname)															\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t pos1,																\
	size_t pos2																	\
){																				\
	bool computed_value = BitField::fname (array.Data() + offset, pos1, pos2);	\
	bool correct_value = fname (array.Data() + offset, pos1, pos2);				\
	CheckResult (computed_value, correct_value);								\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	GET3(fname)															\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t pos1,																\
	size_t pos2																	\
){																				\
	size_t computed_value = BitField::fname (array.Data() + offset, pos1, pos2);	\
	size_t correct_value = fname (array.Data() + offset, pos1, pos2);				\
	CheckResult (computed_value, correct_value);								\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	SET1(fname)															\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t pos1,																\
	size_t pos2																	\
){																				\
	RandomArray <type_t> reference (array);										\
	BitField::fname (array.Data() + offset, pos1);								\
	fname (reference.Data() + offset, pos1);									\
	array.Compare (reference);													\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	SET2(fname)															\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t pos1,																\
	size_t pos2																	\
){																				\
	RandomArray <type_t> reference (array);										\
	BitField::fname (array.Data() + offset, pos1, pos2);						\
	fname (reference.Data() + offset, pos1, pos2);								\
	array.Compare (reference);													\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	UTEST(fname)														\
void Test##fname (void) {														\
	PrintCaption ("BitField::"#fname);											\
	TestBitField (fname <uint8_t>);												\
	TestBitField (fname <uint16_t>);											\
	TestBitField (fname <uint32_t>);											\
	TestBitField (fname <uint64_t>);											\
	TestBitField (fname <sint8_t>);												\
	TestBitField (fname <sint16_t>);											\
	TestBitField (fname <sint32_t>);											\
	TestBitField (fname <sint64_t>);											\
	TestBitField (fname <size_t>);												\
}

//============================================================================//
//      Get bit value from a bit field                                        //
//============================================================================//
GET1 (GetBit)
UTEST (GetBit)

//============================================================================//
//      Set bit value in a bit field                                          //
//============================================================================//
SET1 (SetBit)
UTEST (SetBit)

//============================================================================//
//      Set bits in the bit field range                                       //
//============================================================================//
SET2 (SetBits)
UTEST (SetBits)

//============================================================================//
//      Reset bit value in a bit field                                        //
//============================================================================//
SET1 (ResetBit)
UTEST (ResetBit)

//============================================================================//
//      Reset bits in the bit field range                                     //
//============================================================================//
SET2 (ResetBits)
UTEST (ResetBits)

//============================================================================//
//      Invert bit value in a bit field                                       //
//============================================================================//
SET1 (InvertBit)
UTEST (InvertBit)

//============================================================================//
//      Invert bits in the bit field range                                    //
//============================================================================//
SET2 (InvertBits)
UTEST (InvertBits)

//============================================================================//
//      Searching for a set bit                                               //
//============================================================================//
GET2 (FindSetBitFwd)
GET2 (FindSetBitBwd)
UTEST (FindSetBitFwd)
UTEST (FindSetBitBwd)

//============================================================================//
//      Searching for a reset bit                                             //
//============================================================================//
GET2 (FindResetBitFwd)
GET2 (FindResetBitBwd)
UTEST (FindResetBitFwd)
UTEST (FindResetBitBwd)

//============================================================================//
//      Bit counting                                                          //
//============================================================================//
GET3 (CountBits)
UTEST (CountBits)

//============================================================================//
//      Check for a set bit in the bit field range                            //
//============================================================================//
GET2 (CheckSetBit)
UTEST (CheckSetBit)

//============================================================================//
//      Check for a reset bit in the bit field range                          //
//============================================================================//
GET2 (CheckResetBit)
UTEST (CheckResetBit)

//****************************************************************************//
//      Main function                                                         //
//****************************************************************************//
int main (void)
try {

	// Print program header
	PrintHeader ("Bit field library test suite");
	cout << "This test operates with " << SIZE << "-long arrays in " << ROUNDS << " rounds with " << TRIES << " tries in each." << endl;

	// Bit field operations
	TestGetBit();
	TestSetBit();
	TestSetBits();
	TestResetBit();
	TestResetBits();
	TestInvertBit();
	TestInvertBits();

	// Search operations
	TestFindSetBitFwd();
	TestFindResetBitFwd();
	TestFindSetBitBwd();
	TestFindResetBitBwd();
	TestCountBits();
	TestCheckSetBit();
	TestCheckResetBit();

	// Print success message
	PrintSuccessStatus ("BitField library");
	return 0;
}
catch (const runtime_error &ex) {

	// Print fail message
	PrintFailStatus ("BitField library");
	cerr << ex.what() << endl;
	return 1;
}
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
