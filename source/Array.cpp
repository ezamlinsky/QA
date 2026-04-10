/*                                                                     Array.cpp
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                           ARRAY LIBRARY TEST SUITE                           #
#                                                                              #
# Ordnung muss sein!                             Copyleft (Ɔ) Eugene Zamlinsky #
################################################################################
*/
# include	<cstring>
# include	<fenv.h>
# include	<Common.h>
# include	<RandomArray.h>
# include	<Array.h>

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
//      Key compare function                                                  //
//****************************************************************************//
sint64_t KeyCmp (const void *key1, const void *key2) {
	const sint64_t val1 = reinterpret_cast <const size_t> (key1);
	const sint64_t val2 = reinterpret_cast <const size_t> (key2);
	return (val1 > val2) - (val1 < val2);
}

//****************************************************************************//
//      Test a scalar function                                                //
//****************************************************************************//
template <typename type_t>
void TestArray (
	void (*func)(RandomArray <type_t>&, size_t, size_t, type_t)
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
		size_t count = array.Count (offset);

		// Create a random scalar value for the test
		type_t value = array.RandomValue();

		// Do many tries with the same offset and element count, but different data
		for (size_t j = 0; j < TRIES; j++) {

			// Populate the array with random data
			array.Populate ();

			// Apply the operation to the array data
			func (array, offset, count, value);
		}
	}
}

//****************************************************************************//
//      Test a vector function                                                //
//****************************************************************************//
template <typename type_t>
void TestArrays (
	void (*func)(RandomArray <type_t>&, RandomArray <type_t>&, size_t, size_t, size_t)
){
	// Print test info
	PrintInfo (typeid (type_t).name());

	// Create arrays of the target size
	RandomArray <type_t> target (SIZE, SEED - 1, MAX_VALUE);
	RandomArray <type_t> source (SIZE, SEED + 1, MAX_VALUE);

	// Run the test in many rounds with a random offset and element count
	for (size_t i = 0; i < ROUNDS; i++) {

		// Get a random offset inside the arrays and a random number of elements
		// to work with
		size_t toffset = target.Offset ();
		size_t soffset = source.Offset ();
		size_t tcount = target.Count (toffset);
		size_t scount = source.Count (soffset);
		size_t count = min (tcount, scount);

		// Do many tries with the same offset and element count, but different data
		for (size_t j = 0; j < TRIES; j++) {

			// Populate both arrays with random data
			target.Populate ();
			source.Populate ();

			// Apply the operation to the array data
			func (target, source, toffset, soffset, count);
		}
	}
}

//****************************************************************************//
//      Test a convert function                                               //
//****************************************************************************//
template <typename type1, typename type2>
void TestConvert (
	void (*func)(RandomArray <type1>&, RandomArray <type2>&, size_t, size_t, size_t)
){
	// Print test info
	PrintInfo (typeid (type1).name(), typeid (type2).name());

	// Create arrays of the target size
	RandomArray <type1> target (SIZE, SEED, MAX_VALUE);
	RandomArray <type2> source (SIZE, SEED, MAX_VALUE);

	// Run the test in many rounds with a random offset and element count
	for (size_t i = 0; i < ROUNDS; i++) {

		// Get a random offset inside the arrays and a random number of elements
		// to work with
		size_t toffset = target.Offset ();
		size_t soffset = source.Offset ();
		size_t tcount = target.Count (toffset);
		size_t scount = source.Count (soffset);
		size_t count = min (tcount, scount);

		// Do many tries with the same offset and element count, but different data
		for (size_t j = 0; j < TRIES; j++) {

			// Populate both arrays with random data
			target.Populate ();
			source.Populate ();

			// Apply the operation to the array data
			func (target, source, toffset, soffset, count);
		}
	}
}

//****************************************************************************//
//      Reference functions                                                   //
//****************************************************************************//

//============================================================================//
//      Init function                                                         //
//============================================================================//
template <typename type_t>
void Init (type_t array[], size_t size, type_t value)
{
	for (size_t i = 0; i < size; i++)
		array[i] = value;
}

//============================================================================//
//      Copy function                                                         //
//============================================================================//
template <typename type_t>
void Copy (type_t target[], const type_t source[], size_t size)
{
	memcpy (target, source, size * sizeof (type_t));
}

//============================================================================//
//      Move function                                                         //
//============================================================================//
template <typename type_t>
void Move (type_t target[], type_t source[], size_t size)
{
	memmove (target, source, size * sizeof (type_t));
}

//============================================================================//
//      Clone function                                                        //
//============================================================================//
template <typename type_t>
void Clone (type_t source[], size_t size, size_t psize) {
	if (size > psize) {
		size -= psize;
		type_t *target = source + psize;
		for (size_t i = 0; i < size; ++i)
			target[i] = source[i];
	}
}

//============================================================================//
//      Convert functions                                                     //
//============================================================================//
# define	REF1(fname, ttype, stype)											\
void fname (ttype target[], const stype source[], size_t size){					\
	for (size_t i = 0; i < size; ++i)											\
		target[i] = source[i];													\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	REF2(fname, ttype, stype)											\
void fname (ttype target[], const stype source[], size_t size){					\
	for (size_t i = 0; i < size; ++i)											\
		target[i] = rint (source[i]);											\
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Extension to wider signed integer types                               //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Extend sint8_t to sint16_t
REF1 (ExtendToSint16, sint16_t, sint8_t)

// Extend uint8_t to sint16_t
REF1 (ExtendToSint16, sint16_t, uint8_t)

// Extend sint8_t to sint32_t
REF1 (ExtendToSint32, sint32_t, sint8_t)

// Extend uint8_t to sint32_t
REF1 (ExtendToSint32, sint32_t, uint8_t)

// Extend sint16_t to sint32_t
REF1 (ExtendToSint32, sint32_t, sint16_t)

// Extend uint16_t to sint32_t
REF1 (ExtendToSint32, sint32_t, uint16_t)

// Extend sint8_t to sint64_t
REF1 (ExtendToSint64, sint64_t, sint8_t)

// Extend uint8_t to sint64_t
REF1 (ExtendToSint64, sint64_t, uint8_t)

// Extend sint16_t to sint64_t
REF1 (ExtendToSint64, sint64_t, sint16_t)

// Extend uint16_t to sint64_t
REF1 (ExtendToSint64, sint64_t, uint16_t)

// Extend sint32_t to sint64_t
REF1 (ExtendToSint64, sint64_t, sint32_t)

// Extend uint32_t to sint64_t
REF1 (ExtendToSint64, sint64_t, uint32_t)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Conversion between floating-point types                               //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Convert flt64_t to flt32_t
REF1 (ConvertToFlt32, flt32_t, flt64_t)

// Convert flt32_t to flt64_t
REF1 (ConvertToFlt64, flt64_t, flt32_t)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Conversion from signed integer types to floating-point types          //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Convert sint32_t to flt32_t
REF1 (ConvertToFlt32, flt32_t, sint32_t)

// Convert sint64_t to flt32_t
REF1 (ConvertToFlt32, flt32_t, sint64_t)

// Convert sint32_t to flt64_t
REF1 (ConvertToFlt64, flt64_t, sint32_t)

// Convert sint64_t to flt64_t
REF1 (ConvertToFlt64, flt64_t, sint64_t)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Conversion from floating-point types to signed integer types          //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Convert flt32_t to sint32_t
REF2 (ConvertToSint32, sint32_t, flt32_t)

// Truncate flt64_t to sint32_t
REF2 (ConvertToSint32, sint32_t, flt64_t)

// Truncate flt32_t to sint64_t
REF2 (ConvertToSint64, sint64_t, flt32_t)

// Truncate flt64_t to sint64_t
REF2 (ConvertToSint64, sint64_t, flt64_t)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Truncating from floating-point types to signed integer types          //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Truncate flt32_t to sint32_t
REF1 (TruncateToSint32, sint32_t, flt32_t)

// Truncate flt64_t to sint32_t
REF1 (TruncateToSint32, sint32_t, flt64_t)

// Truncate flt32_t to sint64_t
REF1 (TruncateToSint64, sint64_t, flt32_t)

// Truncate flt64_t to sint64_t
REF1 (TruncateToSint64, sint64_t, flt64_t)


//============================================================================//
//      Byte swap function                                                    //
//============================================================================//
template <typename type_t>
void ByteSwap (type_t array[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		array[i] = byteswap (array[i]);
}

//============================================================================//
//      Bit reversal permutation function                                     //
//============================================================================//
template <typename type_t>
type_t reverse_bits (type_t value) {
	type_t result = 0;
	constexpr int num_bits = numeric_limits <type_t>::digits;
	for (int i = 0; i < num_bits; ++i) {
		result |= (value & 1) << (num_bits - 1 - i);
		value >>= 1;
	}
	return result;
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template <typename type_t>
void BitReverse (type_t array[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		array[i] = reverse_bits (array[i]);
}

//============================================================================//
//      Population count function                                             //
//============================================================================//
template <typename type_t>
type_t bits_count (type_t value) {
	type_t result = 0;
	constexpr int num_bits = numeric_limits <type_t>::digits;
	for (int i = 0; i < num_bits; ++i) {
		if (value & 1) result++;
		value >>= 1;
	}
	return result;
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template <typename type_t>
void PopCount (type_t array[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		array[i] = bits_count (array[i]);
}

//============================================================================//
//      Bitwise NOT function                                                  //
//============================================================================//
template <typename type_t>
void Not (type_t array[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		array[i] = ~array[i];
}

//============================================================================//
//      Scalar bitwise AND function                                           //
//============================================================================//
template <typename type_t>
void AndScalar (type_t array[], size_t size, type_t value)
{
	for (size_t i = 0; i < size; i++)
		array[i] &= value;
}

//============================================================================//
//      Vector bitwise AND function                                           //
//============================================================================//
template <typename type_t>
void AndVector (type_t target[], const type_t source[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		target[i] &= source[i];
}

//============================================================================//
//      Scalar bitwise OR function                                            //
//============================================================================//
template <typename type_t>
void OrScalar (type_t array[], size_t size, type_t value)
{
	for (size_t i = 0; i < size; i++)
		array[i] |= value;
}

//============================================================================//
//      Vector bitwise OR function                                            //
//============================================================================//
template <typename type_t>
void OrVector (type_t target[], const type_t source[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		target[i] |= source[i];
}

//============================================================================//
//      Scalar bitwise XOR function                                           //
//============================================================================//
template <typename type_t>
void XorScalar (type_t array[], size_t size, type_t value)
{
	for (size_t i = 0; i < size; i++)
		array[i] ^= value;
}

//============================================================================//
//      Vector bitwise XOR function                                           //
//============================================================================//
template <typename type_t>
void XorVector (type_t target[], const type_t source[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		target[i] ^= source[i];
}

//============================================================================//
//      Negative value function                                               //
//============================================================================//
template <typename type_t>
void Neg (type_t array[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		array[i] = -array[i];
}

//============================================================================//
//      Absolute value function                                               //
//============================================================================//
template <typename type_t>
void Abs (type_t array[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		array[i] = array[i] < 0 ? -array[i] : array[i];
}

//============================================================================//
//      Negative absolute value function                                      //
//============================================================================//
template <typename type_t>
void NegAbs (type_t array[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		array[i] = array[i] > 0 ? -array[i] : array[i];
}

//============================================================================//
//      Number sign function                                                  //
//============================================================================//
template <typename type_t>
void Sign (type_t array[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		array[i] = (array[i] > type_t(0)) - (array[i] < type_t(0));
}

//============================================================================//
//      Square function                                                       //
//============================================================================//
template <typename type_t>
void Sqr (type_t array[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		array[i] = array[i] * array[i];
}

//============================================================================//
//      Square root function                                                  //
//============================================================================//
template <typename type_t>
void Sqrt (type_t array[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		array[i] = sqrt (array[i]);
}

//============================================================================//
//      Scalar Add function                                                   //
//============================================================================//
template <typename type_t>
void AddScalar (type_t array[], size_t size, type_t value)
{
	for (size_t i = 0; i < size; i++)
		array[i] += value;
}

//============================================================================//
//      Vector Add function                                                   //
//============================================================================//
template <typename type_t>
void AddVector (type_t target[], const type_t source[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		target[i] += source[i];
}

//============================================================================//
//      Scalar Sub function                                                   //
//============================================================================//
template <typename type_t>
void SubScalar (type_t array[], size_t size, type_t value)
{
	for (size_t i = 0; i < size; i++)
		array[i] -= value;
}

//============================================================================//
//      Vector Sub function                                                   //
//============================================================================//
template <typename type_t>
void SubVector (type_t target[], const type_t source[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		target[i] -= source[i];
}

//============================================================================//
//      Scalar Reverse Sub function                                           //
//============================================================================//
template <typename type_t>
void ReverseSubScalar (type_t array[], size_t size, type_t value)
{
	for (size_t i = 0; i < size; i++)
		array[i] = value - array[i];
}

//============================================================================//
//      Vector Reverse Sub function                                           //
//============================================================================//
template <typename type_t>
void ReverseSubVector (type_t target[], const type_t source[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		target[i] = source[i] - target[i];
}

//============================================================================//
//      Scalar Mul function                                                   //
//============================================================================//
template <typename type_t>
void MulScalar (type_t array[], size_t size, type_t value)
{
	for (size_t i = 0; i < size; i++)
		array[i] *= value;
}

//============================================================================//
//      Vector Mul function                                                   //
//============================================================================//
template <typename type_t>
void MulVector (type_t target[], const type_t source[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		target[i] *= source[i];
}

//============================================================================//
//      Scalar Div function                                                   //
//============================================================================//
template <typename type_t>
void DivScalar (type_t array[], size_t size, type_t value)
{
	for (size_t i = 0; i < size; i++)
		array[i] /= value;
}

//============================================================================//
//      Vector Div function                                                   //
//============================================================================//
template <typename type_t>
void DivVector (type_t target[], const type_t source[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		target[i] /= source[i];
}

//============================================================================//
//      Scalar Reverse Div function                                           //
//============================================================================//
template <typename type_t>
void ReverseDivScalar (type_t array[], size_t size, type_t value)
{
	for (size_t i = 0; i < size; i++)
		array[i] = value / array[i];
}

//============================================================================//
//      Vector Reverse Div function                                           //
//============================================================================//
template <typename type_t>
void ReverseDivVector (type_t target[], const type_t source[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		target[i] = source[i] / target[i];
}

//============================================================================//
//      Fused arithmetic operations                                           //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Fused addition                                                        //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
template <typename type_t>
void FusedAdd (type_t target[], const type_t source[], size_t size, type_t value) {
	for (size_t i = 0; i < size; ++i)
		target[i] += source[i] * value;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Fused subtraction                                                     //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
template <typename type_t>
void FusedSub (type_t target[], const type_t source[], size_t size, type_t value) {
	for (size_t i = 0; i < size; ++i)
		target[i] -= source[i] * value;
}

//============================================================================//
//      Round down (floor)                                                    //
//============================================================================//
template <typename type_t>
void RoundDown (type_t array[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		array[i] = floor (array[i]);
}

//============================================================================//
//      Round up (ceil)                                                       //
//============================================================================//
template <typename type_t>
void RoundUp (type_t array[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		array[i] = ceil (array[i]);
}

//============================================================================//
//      Round to the nearest integer using the current rounding mode          //
//============================================================================//
template <typename type_t>
void RoundInt (type_t array[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		array[i] = rint (array[i]);
}

//============================================================================//
//      Round to the nearest even integer                                     //
//============================================================================//
template <typename type_t>
void RoundEven (type_t array[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		array[i] = roundeven (array[i]);
}

//============================================================================//
//      Round to the nearest integer away from zero                           //
//============================================================================//
template <typename type_t>
void Round (type_t array[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		array[i] = round (array[i]);
}

//============================================================================//
//      Round to the nearest integer toward zero (truncation)                 //
//============================================================================//
template <typename type_t>
void Truncate (type_t array[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		array[i] = trunc (array[i]);
}

//============================================================================//
//      Fractional part                                                       //
//============================================================================//
template <typename type_t>
void Frac (type_t array[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		array[i] = array[i] - trunc (array[i]);
}

//============================================================================//
//      Sum of values                                                         //
//============================================================================//
template <typename type_t>
type_t Sum (const type_t array[], size_t size) {
	mpf_class total_sum (0);
	for (size_t i = 0; i < size; ++i) {
		type_t temp = array[i];
		mpf_class val (temp);
		total_sum += val;
	}
	return total_sum.get_d ();
}

//============================================================================//
//      Sum of absolute values                                                //
//============================================================================//
template <typename type_t>
type_t SumAbs (const type_t array[], size_t size) {
	mpf_class total_sum (0);
	for (size_t i = 0; i < size; ++i) {
		type_t temp = abs (array[i]);
		mpf_class val (temp);
		total_sum += val;
	}
	return total_sum.get_d ();
}

//============================================================================//
//      Sum of squared values                                                 //
//============================================================================//
template <typename type_t>
type_t SumSqr (const type_t array[], size_t size) {
	mpf_class total_sum (0);
	for (size_t i = 0; i < size; ++i) {
		mpf_class val (array[i]);
		total_sum += val * val;
	}
	return total_sum.get_d ();
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template <typename type_t>
void SumSqr2 (const type_t array[], size_t size, mpf_class &total_sum) {
	total_sum = 0;
	for (size_t i = 0; i < size; ++i) {
		mpf_class val (array[i]);
		total_sum += val * val;
	}
}

//============================================================================//
//      Sum of multiplied values                                              //
//============================================================================//
template <typename type_t>
type_t SumMul (const type_t array1[], const type_t array2[], size_t size) {
	mpf_class total_sum (0);
	for (size_t i = 0; i < size; ++i) {
		mpf_class val1 (array1[i]);
		mpf_class val2 (array2[i]);
		total_sum += val1 * val2;
	}
	return total_sum.get_d ();
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template <typename type_t>
void SumMul2 (const type_t array1[], const type_t array2[], size_t size, mpf_class &total_sum) {
	total_sum = 0;
	for (size_t i = 0; i < size; ++i) {
		mpf_class val1 (array1[i]);
		mpf_class val2 (array2[i]);
		total_sum += val1 * val2;
	}
}

//============================================================================//
//      Sum of signed differences                                             //
//============================================================================//
template <typename type_t>
type_t SumDiff (const type_t array[], size_t size, type_t value) {
	mpf_class total_sum (0);
	for (size_t i = 0; i < size; ++i) {
		type_t temp = array[i] - value;
		mpf_class val (temp);
		total_sum += val;
	}
	return total_sum.get_d ();
}

//============================================================================//
//      Sum of absolute differences                                           //
//============================================================================//
template <typename type_t>
type_t SumAbsDiff (const type_t array[], size_t size, type_t value) {
	mpf_class total_sum (0);
	for (size_t i = 0; i < size; ++i) {
		type_t temp = abs (array[i] - value);
		mpf_class val (temp);
		total_sum += val;
	}
	return total_sum.get_d ();
}

//============================================================================//
//      Sum of squared differences                                            //
//============================================================================//
template <typename type_t>
type_t SumSqrDiff (const type_t array[], size_t size, type_t value) {
	mpf_class total_sum (0);
	for (size_t i = 0; i < size; ++i) {
		mpf_class val (array[i] - value);
		total_sum += val * val;
	}
	return total_sum.get_d ();
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template <typename type_t>
void SumSqrDiff2 (const type_t array[], size_t size, type_t value, mpf_class &total_sum) {
	total_sum = 0;
	for (size_t i = 0; i < size; ++i) {
		mpf_class val (array[i] - value);
		total_sum += val * val;
	}
}

//============================================================================//
//      Sum of multiplied differences                                         //
//============================================================================//
template <typename type_t>
type_t SumMulDiff (const type_t array1[], const type_t array2[], size_t size, type_t value1, type_t value2) {
	mpf_class total_sum (0);
	for (size_t i = 0; i < size; ++i) {
		mpf_class val1 (array1[i] - value1);
		mpf_class val2 (array2[i] - value2);
		total_sum += val1 * val2;
	}
	return total_sum.get_d ();
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template <typename type_t>
void SumMulDiff2 (const type_t array1[], const type_t array2[], size_t size, type_t value1, type_t value2, mpf_class &total_sum) {
	total_sum = 0;
	for (size_t i = 0; i < size; ++i) {
		mpf_class val1 (array1[i] - value1);
		mpf_class val2 (array2[i] - value2);
		total_sum += val1 * val2;
	}
}

//============================================================================//
//      Sum of signed distances                                               //
//============================================================================//
template <typename type_t>
type_t SumDist (const type_t array1[], const type_t array2[], size_t size) {
	mpf_class total_sum (0);
	for (size_t i = 0; i < size; ++i) {
		type_t temp = array1[i] - array2[i];
		mpf_class val (temp);
		total_sum += val;
	}
	return total_sum.get_d ();
}

//============================================================================//
//      Sum of absolute distances                                             //
//============================================================================//
template <typename type_t>
type_t SumAbsDist (const type_t array1[], const type_t array2[], size_t size) {
	mpf_class total_sum (0);
	for (size_t i = 0; i < size; ++i) {
		type_t temp = abs (array1[i] - array2[i]);
		mpf_class val (temp);
		total_sum += val;
	}
	return total_sum.get_d ();
}

//============================================================================//
//      Sum of squared distances                                              //
//============================================================================//
template <typename type_t>
type_t SumSqrDist (const type_t array1[], const type_t array2[], size_t size) {
	mpf_class total_sum (0);
	for (size_t i = 0; i < size; ++i) {
		mpf_class val (array1[i] - array2[i]);
		total_sum += val * val;
	}
	return total_sum.get_d ();
}

//============================================================================//
//      Minimum absolute value                                                //
//============================================================================//
template <typename type_t>
inline bool cmp_abs (
	const type_t& a,
	const type_t& b
){
	return fabs (a) < fabs (b);
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template <typename res_t, typename type_t>
res_t MinAbs (const type_t array[], size_t size){
	if (size)
		return fabs (*min_element (array, array + size, cmp_abs <type_t>));
	else {
		if constexpr (is_integral_v <type_t>)
			return 0;
		else
			return NAN;
	}
}

//============================================================================//
//      Maximum absolute value                                                //
//============================================================================//
template <typename res_t, typename type_t>
res_t MaxAbs (const type_t array[], size_t size){
	if (size)
		return fabs (*max_element (array, array + size, cmp_abs <type_t>));
	else {
		if constexpr (is_integral_v <type_t>)
			return 0;
		else
			return NAN;
	}
}

//============================================================================//
//      Both minimum and maximum absolute values                              //
//============================================================================//
template <typename res_t, typename type_t>
void MinMaxAbs (
	const type_t array[], size_t size, res_t &min, res_t &max
){
	using namespace std;
	if (size) {
		auto bounds = minmax_element (array, array + size, cmp_abs <type_t>);
		min = fabs (*bounds.first);
		max = fabs (*bounds.second);
	}
	else {
		if constexpr (is_integral_v <type_t>) {
			min = 0;
			max = 0;
		}
		else {
			min = NAN;
			max = NAN;
		}
	}
}

//============================================================================//
//      Minimum value                                                         //
//============================================================================//
template <typename res_t, typename type_t>
res_t Min (const type_t array[], size_t size){
	if (size)
		return *min_element (array, array + size);
	else {
		if constexpr (is_integral_v <type_t>)
			return 0;
		else
			return NAN;
	}
}

//============================================================================//
//      Maximum value                                                         //
//============================================================================//
template <typename res_t, typename type_t>
res_t Max (const type_t array[], size_t size){
	if (size)
		return *max_element (array, array + size);
	else {
		if constexpr (is_integral_v <type_t>)
			return 0;
		else
			return NAN;
	}
}

//============================================================================//
//      Both minimum and maximum values                                       //
//============================================================================//
template <typename res_t, typename type_t>
void MinMax (
	const type_t array[], size_t size, res_t &min, res_t &max
){
	using namespace std;
	if (size) {
		auto bounds = minmax_element (array, array + size);
		min = *bounds.first;
		max = *bounds.second;
	}
	else {
		if constexpr (is_integral_v <type_t>) {
			min = 0;
			max = 0;
		}
		else {
			min = NAN;
			max = NAN;
		}
	}
}

//============================================================================//
//      Find first key position                                               //
//============================================================================//
template <typename type_t>
size_t FindFwd (const type_t array[], size_t size, type_t value){
	for (size_t i = 0; i < size; i++)
		if (array[i] == value)
			return i;
	return static_cast <size_t> (-1);
}

//============================================================================//
//      Find last key position                                                //
//============================================================================//
template <typename type_t>
size_t FindBwd (const type_t array[], size_t size, type_t value){
	for (size_t i = 0; i < size; i++)
		if (array[size - 1 - i] == value)
			return size - 1 - i;
	return static_cast <size_t> (-1);
}

//============================================================================//
//      Searching for the first equal element                                 //
//============================================================================//
template <typename type_t>
size_t FindFirstEqual (const type_t array[], size_t size, type_t value){
	for (size_t i = 0; i < size; i++) {
		const size_t index = i;
		if (array[index] == value)
			return index;
	}
	return static_cast <size_t> (-1);
}

//============================================================================//
//      Searching for the last equal element                                  //
//============================================================================//
template <typename type_t>
size_t FindLastEqual (const type_t array[], size_t size, type_t value){
	for (size_t i = 0; i < size; i++) {
		const size_t index = size - 1 - i;
		if (array[index] == value)
			return index;
	}
	return static_cast <size_t> (-1);
}

//============================================================================//
//      Searching for a greater element                                       //
//============================================================================//
template <typename type_t>
size_t FindGreatAsc (const type_t array[], size_t size, type_t value){
	for (size_t i = 0; i < size; i++) {
		const size_t index = i;
		if (array[index] > value)
			return index;
	}
	return static_cast <size_t> (-1);
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template <typename type_t>
size_t FindGreatDsc (const type_t array[], size_t size, type_t value){
	for (size_t i = 0; i < size; i++) {
		const size_t index = size - 1 - i;
		if (array[index] > value)
			return index;
	}
	return static_cast <size_t> (-1);
}

//============================================================================//
//      Searching for a greater or equal element                              //
//============================================================================//
template <typename type_t>
size_t FindGreatOrEqualAsc (const type_t array[], size_t size, type_t value){
	for (size_t i = 0; i < size; i++) {
		const size_t index = i;
		if (array[index] >= value)
			return index;
	}
	return static_cast <size_t> (-1);
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template <typename type_t>
size_t FindGreatOrEqualDsc (const type_t array[], size_t size, type_t value){
	for (size_t i = 0; i < size; i++) {
		const size_t index = size - 1 - i;
		if (array[index] >= value)
			return index;
	}
	return static_cast <size_t> (-1);
}

//============================================================================//
//      Searching for a less element                                          //
//============================================================================//
template <typename type_t>
size_t FindLessAsc (const type_t array[], size_t size, type_t value){
	for (size_t i = 0; i < size; i++) {
		const size_t index = size - 1 - i;
		if (array[index] < value)
			return index;
	}
	return static_cast <size_t> (-1);
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template <typename type_t>
size_t FindLessDsc (const type_t array[], size_t size, type_t value){
	for (size_t i = 0; i < size; i++) {
		const size_t index = i;
		if (array[index] < value)
			return index;
	}
	return static_cast <size_t> (-1);
}

//============================================================================//
//      Searching for a less or equal element                                 //
//============================================================================//
template <typename type_t>
size_t FindLessOrEqualAsc (const type_t array[], size_t size, type_t value){
	for (size_t i = 0; i < size; i++) {
		const size_t index = size - 1 - i;
		if (array[index] <= value)
			return index;
	}
	return static_cast <size_t> (-1);
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template <typename type_t>
size_t FindLessOrEqualDsc (const type_t array[], size_t size, type_t value){
	for (size_t i = 0; i < size; i++) {
		const size_t index = i;
		if (array[index] <= value)
			return index;
	}
	return static_cast <size_t> (-1);
}


//============================================================================//
//      Elements counting                                                     //
//============================================================================//
template <typename type_t>
size_t Count (const type_t array[], size_t size, type_t value)
{
	size_t count = 0;
	for (size_t i = 0; i < size; i++)
		if (array[i] == value)
			count++;
	return count;
}

//============================================================================//
//      Elements replacing                                                    //
//============================================================================//
template <typename type_t>
void Replace (type_t array[], size_t size, type_t pattern, type_t value)
{
	for (size_t i = 0; i < size; i++)
		if (array[i] == pattern)
			array[i] = value;
}

//============================================================================//
//      Order reversing                                                       //
//============================================================================//
template <typename type_t>
void Reverse (type_t array[], size_t size) {
	for (size_t i = 0; i < size / 2; i++) {
		type_t t1 = array[i];
		type_t t2 = array[size - 1 - i];
		array[i] = t2;
		array[size - 1 - i] = t1;
	}
}

//============================================================================//
//      Unique elements                                                       //
//============================================================================//
template <typename type_t>
size_t Unique (
	type_t unique[],
	const type_t array[],
	size_t size
){
	if (size) {
		type_t value = array[0];
		size_t usize = 0;
		for (size_t i = 1; i < size; i++) {
			if (array[i] != value) {
				unique[0] = value;
				unique++;
				value = array[i];
				usize++;
			}
		}
		unique[0] = value;
		usize++;
		return usize;
	}
	else
		return 0;
}

//============================================================================//
//      Duplicate elements                                                    //
//============================================================================//
template <typename type_t>
size_t Duplicates (
	type_t unique[],
	size_t count[],
	const type_t array[],
	size_t size
){
	if (size) {
		type_t value = array[0];
		size_t total = 1;
		size_t usize = 0;
		for (size_t i = 1; i < size; i++) {
			if (array[i] != value) {
				unique[0] = value;
				unique++;
				count[0] = total;
				count++;
				value = array[i];
				total = 1;
				usize++;
			}
			else
				total++;
		}
		unique[0] = value;
		count[0] = total;
		usize++;
		return usize;
	}
	else
		return 0;
}

//============================================================================//
//      N-th order statistic                                                  //
//============================================================================//
template <typename type_t>
void QuickSelectAsc (type_t array[], size_t size, size_t index){
	if (index < size)
		sort (array, array + size, CmpAsc <type_t>);
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template <typename type_t>
void QuickSelectDsc (type_t array[], size_t size, size_t index){
	if (index < size)
		sort (array, array + size, CmpDsc <type_t>);
}

//============================================================================//
//      Quick sort                                                            //
//============================================================================//
template <typename type_t>
void QuickSortAsc (type_t array[], size_t size){
	sort (array, array + size, CmpAsc <type_t>);
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template <typename type_t>
void QuickSortDsc (type_t array[], size_t size){
	sort (array, array + size, CmpDsc <type_t>);
}

//============================================================================//
//      Merge sort                                                            //
//============================================================================//
template <typename type_t>
void MergeSortAsc (type_t array[], size_t size){
	sort (array, array + size, CmpAsc <type_t>);
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template <typename type_t>
void MergeSortDsc (type_t array[], size_t size){
	sort (array, array + size, CmpDsc <type_t>);
}

//============================================================================//
//      Radix sort                                                            //
//============================================================================//
template <typename type_t>
void RadixSortAsc (type_t array[], size_t size){
	sort (array, array + size, CmpAsc <type_t>);
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template <typename type_t>
void RadixSortDsc (type_t array[], size_t size){
	sort (array, array + size, CmpDsc <type_t>);
}
//============================================================================//
//      Compare functions                                                     //
//============================================================================//
template <typename type_t>
sint64_t Compare (const type_t array1[], const type_t array2[], size_t size){
	for (size_t i = 0; i < size; i++)
		if (array1[i] != array2[i])
			return (array1[i] > array2[i]) - (array1[i] < array2[i]);
	return static_cast <sint64_t> (0);
}

//============================================================================//
//      Check for differences                                                 //
//============================================================================//
template <typename type_t>
sint64_t CheckDiff (const type_t array1[], const type_t array2[], size_t size){
	for (size_t i = 0; i < size; i++)
		if (array1[i] != array2[i])
			return i;
	return static_cast <sint64_t> (-1);
}

//============================================================================//
//      Check for duplicate values                                            //
//============================================================================//
template <typename type_t>
sint64_t CheckDup (const type_t array[], size_t size){
	const type_t *ptr = array + 1;
	if (size > 1)
		for (size_t i = 0; i < size - 1; i++)
			if (array[i] == ptr[i])
				return i;
	return static_cast <sint64_t> (-1);
}

//============================================================================//
//      Check for ascending sort order                                        //
//============================================================================//
template <typename type_t>
sint64_t CheckSortAsc (const type_t array[], size_t size){
	const type_t *ptr = array + 1;
	if (size > 1)
		for (size_t i = 0; i < size - 1; i++)
			if (array[i] > ptr[i])
				return i;
	return static_cast <sint64_t> (-1);
}

//============================================================================//
//      Check for descending sort order                                       //
//============================================================================//
template <typename type_t>
sint64_t CheckSortDsc (const type_t array[], size_t size){
	const type_t *ptr = array + 1;
	if (size > 1)
		for (size_t i = 0; i < size - 1; i++)
			if (array[i] < ptr[i])
				return i;
	return static_cast <sint64_t> (-1);
}

//============================================================================//
//      Check for infinite values                                             //
//============================================================================//
template <typename type_t>
sint64_t CheckInf (const type_t array[], size_t size){
	for (size_t i = 0; i < size; i++)
		if (isinf(array[i]))
			return i;
	return static_cast <sint64_t> (-1);
}

//============================================================================//
//      Check for NaN values                                                  //
//============================================================================//
template <typename type_t>
sint64_t CheckNaN (const type_t array[], size_t size){
	for (size_t i = 0; i < size; i++)
		if (isnan(array[i]))
			return i;
	return static_cast <sint64_t> (-1);
}

//****************************************************************************//
//      Testing functions                                                     //
//****************************************************************************//

//============================================================================//
//      Scalar testing functions                                              //
//============================================================================//
# define	SCALAR(fname)														\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t count,																\
	type_t value																\
){																				\
	RandomArray <type_t> reference (array);										\
	Array::fname (array.Data() + offset, count, value);							\
	fname (reference.Data() + offset, count, value);							\
	array.Compare (reference);													\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	USCALAR_INT(fname)													\
void Test##fname (void) {														\
	PrintCaption ("Array::"#fname);												\
	TestArray (fname <uint8_t>);												\
	TestArray (fname <uint16_t>);												\
	TestArray (fname <uint32_t>);												\
	TestArray (fname <uint64_t>);												\
	TestArray (fname <sint8_t>);												\
	TestArray (fname <sint16_t>);												\
	TestArray (fname <sint32_t>);												\
	TestArray (fname <sint64_t>);												\
	TestArray (fname <size_t>);													\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	USCALAR_SIGN(fname)													\
void Test##fname (void) {														\
	PrintCaption ("Array::"#fname);												\
	TestArray (fname <sint8_t>);												\
	TestArray (fname <sint16_t>);												\
	TestArray (fname <sint32_t>);												\
	TestArray (fname <sint64_t>);												\
	TestArray (fname <flt32_t>);												\
	TestArray (fname <flt64_t>);												\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	USCALAR_FLT(fname)													\
void Test##fname (void) {														\
	PrintCaption ("Array::"#fname);												\
	TestArray (fname <flt32_t>);												\
	TestArray (fname <flt64_t>);												\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	USCALAR_OBJ(fname)													\
void Test##fname (void) {														\
	PrintCaption ("Array::"#fname);												\
	TestArray (fname);															\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	USCALAR(fname)														\
void Test##fname (void) {														\
	PrintCaption ("Array::"#fname);												\
	TestArray (fname <uint8_t>);												\
	TestArray (fname <uint16_t>);												\
	TestArray (fname <uint32_t>);												\
	TestArray (fname <uint64_t>);												\
	TestArray (fname <sint8_t>);												\
	TestArray (fname <sint16_t>);												\
	TestArray (fname <sint32_t>);												\
	TestArray (fname <sint64_t>);												\
	TestArray (fname <flt32_t>);												\
	TestArray (fname <flt64_t>);												\
	TestArray (fname <size_t>);													\
}
SCALAR (Init)

//============================================================================//
//      Vector testing functions                                              //
//============================================================================//
# define	VECTOR(fname)														\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &target,												\
	RandomArray <type_t> &source,												\
	size_t toffset,																\
	size_t soffset,																\
	size_t count																\
){																				\
	RandomArray <type_t> treference (target);									\
	RandomArray <type_t> sreference (source);									\
	Array::fname (target.Data() + toffset, source.Data() + soffset, count);		\
	fname (treference.Data() + toffset, sreference.Data() + soffset, count);	\
	target.Compare (treference);												\
	source.Compare (sreference);												\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	UVECTOR_INT(fname)													\
void Test##fname (void) {														\
	PrintCaption ("Array::"#fname);												\
	TestArrays (fname <uint8_t>);												\
	TestArrays (fname <uint16_t>);												\
	TestArrays (fname <uint32_t>);												\
	TestArrays (fname <uint64_t>);												\
	TestArrays (fname <sint8_t>);												\
	TestArrays (fname <sint16_t>);												\
	TestArrays (fname <sint32_t>);												\
	TestArrays (fname <sint64_t>);												\
	TestArrays (fname <size_t>);												\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	UVECTOR_FLT(fname)													\
void Test##fname (void) {														\
	PrintCaption ("Array::"#fname);												\
	TestArrays (fname <flt32_t>);												\
	TestArrays (fname <flt64_t>);												\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	UVECTOR_OBJ(fname)													\
void Test##fname (void) {														\
	PrintCaption ("Array::"#fname);												\
	TestArrays (fname);															\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	UVECTOR(fname)														\
void Test##fname (void) {														\
	PrintCaption ("Array::"#fname);												\
	TestArrays (fname <uint8_t>);												\
	TestArrays (fname <uint16_t>);												\
	TestArrays (fname <uint32_t>);												\
	TestArrays (fname <uint64_t>);												\
	TestArrays (fname <sint8_t>);												\
	TestArrays (fname <sint16_t>);												\
	TestArrays (fname <sint32_t>);												\
	TestArrays (fname <sint64_t>);												\
	TestArrays (fname <flt32_t>);												\
	TestArrays (fname <flt64_t>);												\
	TestArrays (fname <size_t>);												\
}
VECTOR (Copy)
VECTOR (Move)

//============================================================================//
//      Initialization                                                        //
//============================================================================//
USCALAR (Init)

//============================================================================//
//      Copying arrays                                                        //
//============================================================================//
UVECTOR (Copy)

//============================================================================//
//      Moving arrays                                                         //
//============================================================================//
UVECTOR (Move)

//============================================================================//
//      Pattern cloning                                                       //
//============================================================================//
template <typename type_t>
void Clone (
	RandomArray <type_t> &array,
	size_t offset,
	size_t count,
	type_t value
){
	// Make a copy for the reference implementation of the function
	RandomArray <type_t> reference (array);

	// Apply the operation to the array data. Both the testing and the reference
	Array::Clone (array.Data(), count, offset);
	Clone (reference.Data(), count, offset);

	// Compare arrays for different elements
	array.Compare (reference);
}
USCALAR (Clone)

//============================================================================//
//      Convert function test                                                 //
//============================================================================//
# define	CONVERT(fname, T1, T2)												\
void fname (																	\
	RandomArray <T1> &target,													\
	RandomArray <T2> &source,													\
	size_t toffset,																\
	size_t soffset,																\
	size_t count																\
){																				\
	RandomArray <T1> treference (target);										\
	RandomArray <T2> sreference (source);										\
	Array::fname (target.Data() + toffset, source.Data() + soffset, count);		\
	fname (treference.Data() + toffset, sreference.Data() + soffset, count);	\
	target.Compare (treference);												\
	source.Compare (sreference);												\
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Extension to wider signed integer types                               //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
CONVERT (ExtendToSint16, sint16_t, sint8_t)
CONVERT (ExtendToSint16, sint16_t, uint8_t)
CONVERT (ExtendToSint32, sint32_t, sint8_t)
CONVERT (ExtendToSint32, sint32_t, uint8_t)
CONVERT (ExtendToSint32, sint32_t, sint16_t)
CONVERT (ExtendToSint32, sint32_t, uint16_t)
CONVERT (ExtendToSint64, sint64_t, sint8_t)
CONVERT (ExtendToSint64, sint64_t, uint8_t)
CONVERT (ExtendToSint64, sint64_t, sint16_t)
CONVERT (ExtendToSint64, sint64_t, uint16_t)
CONVERT (ExtendToSint64, sint64_t, sint32_t)
CONVERT (ExtendToSint64, sint64_t, uint32_t)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Conversion between floating-point types                               //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
CONVERT (ConvertToFlt32, flt32_t, flt64_t)
CONVERT (ConvertToFlt64, flt64_t, flt32_t)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Conversion from signed integer types to floating-point types          //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
CONVERT (ConvertToFlt32, flt32_t, sint32_t)
CONVERT (ConvertToFlt32, flt32_t, sint64_t)
CONVERT (ConvertToFlt64, flt64_t, sint32_t)
CONVERT (ConvertToFlt64, flt64_t, sint64_t)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Conversion from floating-point types to signed integer types          //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
CONVERT (ConvertToSint32, sint32_t, flt32_t)
CONVERT (ConvertToSint32, sint32_t, flt64_t)
CONVERT (ConvertToSint64, sint64_t, flt32_t)
CONVERT (ConvertToSint64, sint64_t, flt64_t)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Truncating from floating-point types to signed integer types          //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
CONVERT (TruncateToSint32, sint32_t, flt32_t)
CONVERT (TruncateToSint32, sint32_t, flt64_t)
CONVERT (TruncateToSint64, sint64_t, flt32_t)
CONVERT (TruncateToSint64, sint64_t, flt64_t)
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void TestConvert (void) {

	// Print test caption
	PrintCaption ("Array::Convert");

	// Extension to wider signed integer types
	TestConvert <sint16_t, sint8_t> (ExtendToSint16);
	TestConvert <sint16_t, uint8_t> (ExtendToSint16);
	TestConvert <sint32_t, sint8_t> (ExtendToSint32);
	TestConvert <sint32_t, uint8_t> (ExtendToSint32);
	TestConvert <sint32_t, sint16_t> (ExtendToSint32);
	TestConvert <sint32_t, uint16_t> (ExtendToSint32);
	TestConvert <sint64_t, sint8_t> (ExtendToSint64);
	TestConvert <sint64_t, uint8_t> (ExtendToSint64);
	TestConvert <sint64_t, sint16_t> (ExtendToSint64);
	TestConvert <sint64_t, uint16_t> (ExtendToSint64);
	TestConvert <sint64_t, sint32_t> (ExtendToSint64);
	TestConvert <sint64_t, uint32_t> (ExtendToSint64);

	// Conversion between floating-point types
	TestConvert <flt32_t, flt64_t> (ConvertToFlt32);
	TestConvert <flt64_t, flt32_t> (ConvertToFlt64);

	// Conversion from signed integer types to floating-point types
	TestConvert <flt32_t, sint32_t> (ConvertToFlt32);
//	TestConvert <flt32_t, sint64_t> (ConvertToFlt32);
	TestConvert <flt64_t, sint32_t> (ConvertToFlt64);
//	TestConvert <flt64_t, sint64_t> (ConvertToFlt64);

	// Conversion from floating-point types to signed integer types
	TestConvert <sint32_t, flt32_t> (ConvertToSint32);
	TestConvert <sint32_t, flt64_t> (ConvertToSint32);
//	TestConvert <sint64_t, flt32_t> (ConvertToSint64);
//	TestConvert <sint64_t, flt64_t> (ConvertToSint64);

	// Truncating from floating-point types to signed integer types
	TestConvert <sint32_t, flt32_t> (TruncateToSint32);
	TestConvert <sint32_t, flt64_t> (TruncateToSint32);
//	TestConvert <sint64_t, flt32_t> (TruncateToSint64);
//	TestConvert <sint64_t, flt64_t> (TruncateToSint64);
}

//============================================================================//
//      Bitwise operations                                                    //
//============================================================================//
# define	UNARY1(fname)														\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t count,																\
	type_t value																\
){																				\
	RandomArray <type_t> reference (array);										\
	Array::fname (array.Data() + offset, count);								\
	fname (reference.Data() + offset, count);									\
	array.Compare (reference);													\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	UNARY2(fname)														\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t count,																\
	type_t value																\
){																				\
	RandomArray <type_t> temp (SIZE, SEED, MAX_VALUE);							\
	RandomArray <type_t> reference (array);										\
	Array::fname (array.Data() + offset, temp.Data() + offset, count);			\
	fname (reference.Data() + offset, count);									\
	array.Compare (reference);													\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	UNARY_OBJ(fname,ref)												\
void fname (																	\
	RandomArray <sint64_t> &array,												\
	size_t offset,																\
	size_t count,																\
	sint64_t value																\
){																				\
	const void **arr = reinterpret_cast <const void**> (array.Data() + offset);	\
	RandomArray <sint64_t> reference (array);									\
	Array::fname (arr, count);													\
	ref (reference.Data() + offset, count);										\
	array.Compare (reference);													\
}
UNARY1 (ByteSwap)
UNARY1 (BitReverse)
UNARY1 (PopCount)
UNARY1 (Not)
UNARY1 (Neg)
UNARY1 (Abs)
UNARY1 (NegAbs)
UNARY1 (Sign)
UNARY1 (Sqr)
UNARY1 (Sqrt)
UNARY1 (RoundDown)
UNARY1 (RoundUp)
UNARY1 (RoundInt)
UNARY1 (RoundEven)
UNARY1 (Round)
UNARY1 (Truncate)
UNARY1 (Frac)
UNARY1 (Reverse)
UNARY1 (QuickSortAsc)
UNARY1 (QuickSortDsc)
UNARY2 (MergeSortAsc)
UNARY2 (MergeSortDsc)
UNARY2 (RadixSortAsc)
UNARY2 (RadixSortDsc)
UNARY_OBJ (ReverseObj, Reverse)
SCALAR (AndScalar)
SCALAR (OrScalar)
SCALAR (XorScalar)
SCALAR (AddScalar)
SCALAR (SubScalar)
SCALAR (ReverseSubScalar)
SCALAR (MulScalar)
SCALAR (DivScalar)
SCALAR (ReverseDivScalar)
VECTOR (AndVector)
VECTOR (OrVector)
VECTOR (XorVector)
VECTOR (AddVector)
VECTOR (SubVector)
VECTOR (ReverseSubVector)
VECTOR (MulVector)
VECTOR (DivVector)
VECTOR (ReverseDivVector)

//============================================================================//
//      Byte swap                                                             //
//============================================================================//
USCALAR_INT (ByteSwap)

//============================================================================//
//      Bit reversal permutation                                              //
//============================================================================//
void TestBitReverse (void) {

	// Print test caption
	PrintCaption ("Array::BitReverse");

	// Unsigned integer types
	TestArray (BitReverse <uint8_t>);
	TestArray (BitReverse <uint16_t>);
	TestArray (BitReverse <uint32_t>);
	TestArray (BitReverse <uint64_t>);

	// Signed integer types
//	TestArray (BitReverse <sint8_t>);
//	TestArray (BitReverse <sint16_t>);
//	TestArray (BitReverse <sint32_t>);
//	TestArray (BitReverse <sint64_t>);

	// Other types
	TestArray (BitReverse <size_t>);
}

//============================================================================//
//      Population count                                                      //
//============================================================================//
void TestPopCount (void) {

	// Print test caption
	PrintCaption ("Array::PopCount");

	// Unsigned integer types
	TestArray (PopCount <uint8_t>);
	TestArray (PopCount <uint16_t>);
	TestArray (PopCount <uint32_t>);
	TestArray (PopCount <uint64_t>);

	// Signed integer types
//	TestArray (PopCount <sint8_t>);
//	TestArray (PopCount <sint16_t>);
//	TestArray (PopCount <sint32_t>);
//	TestArray (PopCount <sint64_t>);

	// Other types
	TestArray (PopCount <size_t>);
}

//============================================================================//
//      Bitwise NOT                                                           //
//============================================================================//
USCALAR_INT (Not)

//============================================================================//
//      Bitwise AND                                                           //
//============================================================================//
USCALAR_INT (AndScalar)
UVECTOR_INT (AndVector)

//============================================================================//
//      Bitwise OR                                                            //
//============================================================================//
USCALAR_INT (OrScalar)
UVECTOR_INT (OrVector)

//============================================================================//
//      Bitwise XOR                                                           //
//============================================================================//
USCALAR_INT (XorScalar)
UVECTOR_INT (XorVector)

//============================================================================//
//      Negative value                                                        //
//============================================================================//
USCALAR_SIGN (Neg)

//============================================================================//
//      Absolute value                                                        //
//============================================================================//
void TestAbs (void) {

	// Print test caption
	PrintCaption ("Array::Abs");

	// Signed integer types
	TestArray (Abs <sint8_t>);
	TestArray (Abs <sint16_t>);
	TestArray (Abs <sint32_t>);
	//TestArray (Abs <sint64_t>);

	// Floating-point types
	TestArray (Abs <flt32_t>);
	TestArray (Abs <flt64_t>);
}

//============================================================================//
//      Negative absolute value                                               //
//============================================================================//
void TestNegAbs (void) {

	// Print test caption
	PrintCaption ("Array::NegAbs");

	// Signed integer types
	TestArray (NegAbs <sint8_t>);
	TestArray (NegAbs <sint16_t>);
	TestArray (NegAbs <sint32_t>);
	//TestArray (NegAbs <sint64_t>);

	// Floating-point types
	TestArray (NegAbs <flt32_t>);
	TestArray (NegAbs <flt64_t>);
}

//============================================================================//
//      Number sign                                                           //
//============================================================================//
USCALAR_SIGN (Sign)

//============================================================================//
//      Square                                                                //
//============================================================================//
USCALAR_FLT (Sqr)

//============================================================================//
//      Square root                                                           //
//============================================================================//
USCALAR_FLT (Sqrt)

//============================================================================//
//      Scalar Add function test                                              //
//============================================================================//
USCALAR (AddScalar)

//============================================================================//
//      Vector Add function test                                              //
//============================================================================//
UVECTOR (AddVector)

//============================================================================//
//      Scalar Sub function test                                              //
//============================================================================//
USCALAR (SubScalar)

//============================================================================//
//      Vector Sub function test                                              //
//============================================================================//
UVECTOR (SubVector)

//============================================================================//
//      Scalar Reverse Sub function test                                      //
//============================================================================//
USCALAR (ReverseSubScalar)

//============================================================================//
//      Vector Reverse Sub function test                                      //
//============================================================================//
UVECTOR (ReverseSubVector)

//============================================================================//
//      Scalar Mul function test                                              //
//============================================================================//
USCALAR_FLT (MulScalar)

//============================================================================//
//      Vector Mul function test                                              //
//============================================================================//
UVECTOR_FLT (MulVector)

//============================================================================//
//      Scalar Div function test                                              //
//============================================================================//
USCALAR_FLT (DivScalar)

//============================================================================//
//      Vector Div function test                                              //
//============================================================================//
UVECTOR_FLT (DivVector)

//============================================================================//
//      Scalar Reverse Div function test                                      //
//============================================================================//
USCALAR_FLT (ReverseDivScalar)

//============================================================================//
//      Vector Reverse Div function test                                      //
//============================================================================//
UVECTOR_FLT (ReverseDivVector)

//============================================================================//
//      Fused arithmetic operations                                           //
//============================================================================//
# define	FUSED(fname)														\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &target,												\
	RandomArray <type_t> &source,												\
	size_t toffset,																\
	size_t soffset,																\
	size_t count																\
){																				\
	type_t value = source.RandomValue();										\
	RandomArray <type_t> treference (target);									\
	RandomArray <type_t> sreference (source);									\
	Array::fname (target.Data() + toffset, source.Data() + soffset, count, value);\
	fname (treference.Data() + toffset, sreference.Data() + soffset, count, value);\
	target.Compare (treference);												\
	source.Compare (sreference);												\
}
FUSED (FusedAdd)
FUSED (FusedSub)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Fused addition                                                        //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
UVECTOR_FLT (FusedAdd)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Fused subtraction                                                     //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
UVECTOR_FLT (FusedSub)

//============================================================================//
//      Round down (floor)                                                    //
//============================================================================//
USCALAR_FLT (RoundDown)

//============================================================================//
//      Round up (ceil)                                                       //
//============================================================================//
USCALAR_FLT (RoundUp)

//============================================================================//
//      Round to the nearest integer using the current rounding mode          //
//============================================================================//
USCALAR_FLT (RoundInt)

//============================================================================//
//      Round to the nearest even integer                                     //
//============================================================================//
USCALAR_FLT (RoundEven)

//============================================================================//
//      Round to the nearest integer away from zero                           //
//============================================================================//
USCALAR_FLT (Round)

//============================================================================//
//      Round to the nearest integer toward zero (truncation)                 //
//============================================================================//
USCALAR_FLT (Truncate)

//============================================================================//
//      Fractional part                                                       //
//============================================================================//
USCALAR_FLT (Frac)

//============================================================================//
//      Numerical integration                                                 //
//============================================================================//
# define	SUMS(fname)															\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t count,																\
	type_t value																\
){																				\
	type_t computed_value = Array::fname (array.Data() + offset, count);		\
	type_t correct_value = fname <type_t> (array.Data() + offset, count);		\
	CheckResult (computed_value, correct_value);								\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	SUMDIFF(fname)														\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t count,																\
	type_t value																\
){																				\
	type_t computed_value = Array::fname (array.Data() + offset, count, value);	\
	type_t correct_value = fname <type_t> (array.Data() + offset, count, value);\
	CheckResult (computed_value, correct_value);								\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	SUMDIST(fname)														\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &target,												\
	RandomArray <type_t> &source,												\
	size_t toffset,																\
	size_t soffset,																\
	size_t count																\
){																				\
	type_t computed_value = Array::fname (target.Data() + toffset, source.Data() + soffset, count);\
	type_t correct_value = fname <type_t> (target.Data() + toffset, source.Data() + soffset, count);\
	CheckResult (computed_value, correct_value);								\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	SUMV(fname)															\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &target,												\
	RandomArray <type_t> &source,												\
	size_t toffset,																\
	size_t soffset,																\
	size_t count																\
){																				\
	type_t value1 = target.RandomValue();										\
	type_t value2 = source.RandomValue();										\
	type_t computed_value = Array::fname (target.Data() + toffset, source.Data() + soffset, count, value1, value2);\
	type_t correct_value = fname <type_t> (target.Data() + toffset, source.Data() + soffset, count, value1, value2);\
	CheckResult (computed_value, correct_value);								\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	PSUM1(fname)														\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t count,																\
	type_t value																\
){																				\
	mpf_class correct_sum;														\
	type_t computed_high, computed_low;											\
	Array::fname (array.Data() + offset, count, computed_high, computed_low);	\
	fname <type_t> (array.Data() + offset, count, correct_sum);					\
	CheckResult2 (computed_high, computed_low, correct_sum, type_t (4));		\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	PSUM2(fname)														\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t count,																\
	type_t value																\
){																				\
	mpf_class correct_sum;														\
	type_t computed_high, computed_low;											\
	Array::fname (array.Data() + offset, count, value, computed_high, computed_low);\
	fname <type_t> (array.Data() + offset, count, value, correct_sum);			\
	CheckResult2 (computed_high, computed_low, correct_sum, type_t (4));		\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	PSUM3(fname)														\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &target,												\
	RandomArray <type_t> &source,												\
	size_t toffset,																\
	size_t soffset,																\
	size_t count																\
){																				\
	mpf_class correct_sum;														\
	type_t computed_high, computed_low;											\
	Array::fname (target.Data() + toffset, source.Data() + soffset, count, computed_high, computed_low);\
	fname <type_t> (target.Data() + toffset, source.Data() + soffset, count, correct_sum);\
	CheckResult2 (computed_high, computed_low, correct_sum, type_t (4));		\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	PSUM4(fname)														\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &target,												\
	RandomArray <type_t> &source,												\
	size_t toffset,																\
	size_t soffset,																\
	size_t count																\
){																				\
	type_t value1 = target.RandomValue();										\
	type_t value2 = source.RandomValue();										\
	mpf_class correct_sum;														\
	type_t computed_high, computed_low;											\
	Array::fname (target.Data() + toffset, source.Data() + soffset, count, value1, value2, computed_high, computed_low);\
	fname <type_t> (target.Data() + toffset, source.Data() + soffset, count, value1, value2, correct_sum);\
	CheckResult2 (computed_high, computed_low, correct_sum, type_t (8));		\
}
SUMS (Sum)
SUMS (SumAbs)
SUMS (SumSqr)
SUMDIST (SumMul)
PSUM1 (SumSqr2)
PSUM3 (SumMul2)
SUMDIFF (SumDiff)
SUMDIFF (SumAbsDiff)
SUMDIFF (SumSqrDiff)
SUMV (SumMulDiff)
PSUM2 (SumSqrDiff2)
PSUM4 (SumMulDiff2)
SUMDIST (SumDist)
SUMDIST (SumAbsDist)
SUMDIST (SumSqrDist)

//============================================================================//
//      Sum of values                                                         //
//============================================================================//
USCALAR_FLT (Sum)

//============================================================================//
//      Sum of absolute values                                                //
//============================================================================//
USCALAR_FLT (SumAbs)

//============================================================================//
//      Sum of squared values                                                 //
//============================================================================//
USCALAR_FLT (SumSqr)

//============================================================================//
//      Sum of multiplied values                                              //
//============================================================================//
UVECTOR_FLT (SumMul)

//============================================================================//
//      Precise sum of squared values                                         //
//============================================================================//
USCALAR_FLT (SumSqr2)

//============================================================================//
//      Precise sum of multiplied values                                      //
//============================================================================//
UVECTOR_FLT (SumMul2)

//============================================================================//
//      Sum of signed differences                                             //
//============================================================================//
USCALAR_FLT (SumDiff)

//============================================================================//
//      Sum of absolute differences                                           //
//============================================================================//
USCALAR_FLT (SumAbsDiff)

//============================================================================//
//      Sum of squared differences                                            //
//============================================================================//
USCALAR_FLT (SumSqrDiff)

//============================================================================//
//      Sum of multiplied differences                                         //
//============================================================================//
UVECTOR_FLT (SumMulDiff)

//============================================================================//
//      Precise sum of squared differences                                    //
//============================================================================//
USCALAR_FLT (SumSqrDiff2)

//============================================================================//
//      Precise sum of multiplied differences                                 //
//============================================================================//
UVECTOR_FLT (SumMulDiff2)

//============================================================================//
//      Sum of signed distances                                               //
//============================================================================//
UVECTOR_FLT (SumDist)

//============================================================================//
//      Sum of absolute distances                                             //
//============================================================================//
UVECTOR_FLT (SumAbsDist)

//============================================================================//
//      Sum of squared distances                                              //
//============================================================================//
UVECTOR_FLT (SumSqrDist)

//============================================================================//
//      Minimum and maximum values                                            //
//============================================================================//
# define	MINMAX1(fname)														\
template <typename res_t, typename type_t>										\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t count,																\
	type_t value																\
){																				\
	res_t computed_value = Array::fname (array.Data() + offset, count);			\
	res_t correct_value = fname <res_t,type_t> (array.Data() + offset, count);	\
	CheckResult (computed_value, correct_value);								\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	MINMAX2(fname)														\
template <typename res_t, typename type_t>										\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t count,																\
	type_t value																\
){																				\
	res_t computed_val1, computed_val2;											\
	res_t correct_val1, correct_val2;											\
	Array::fname (array.Data() + offset, count, computed_val1, computed_val2);	\
	fname <res_t,type_t> (array.Data() + offset, count, correct_val1, correct_val2);\
	CheckResult (computed_val1, correct_val1);									\
	CheckResult (computed_val2, correct_val2);									\
}
MINMAX1 (Min)
MINMAX1 (Max)
MINMAX2 (MinMax)
MINMAX1 (MinAbs)
MINMAX1 (MaxAbs)
MINMAX2 (MinMaxAbs)

//============================================================================//
//      Minimum absolute value                                                //
//============================================================================//
void TestMinAbs (void) {

	// Print test caption
	PrintCaption ("Array::MinAbs");

	// Signed integer types
	TestArray (MinAbs <uint8_t, sint8_t>);
	TestArray (MinAbs <uint16_t, sint16_t>);
	TestArray (MinAbs <uint32_t, sint32_t>);
	//TestArray (MinAbs <uint64_t, sint64_t>);

	// Floating-point types
	TestArray (MinAbs <flt32_t, flt32_t>);
	TestArray (MinAbs <flt64_t, flt64_t>);
}

//============================================================================//
//      Maximum absolute value                                                //
//============================================================================//
void TestMaxAbs (void) {

	// Print test caption
	PrintCaption ("Array::MaxAbs");

	// Signed integer types
	TestArray (MaxAbs <uint8_t, sint8_t>);
	TestArray (MaxAbs <uint16_t, sint16_t>);
	TestArray (MaxAbs <uint32_t, sint32_t>);
	//TestArray (MaxAbs <uint64_t, sint64_t>);

	// Floating-point types
	TestArray (MaxAbs <flt32_t, flt32_t>);
	TestArray (MaxAbs <flt64_t, flt64_t>);
}

//============================================================================//
//      Both minimum and maximum values                                       //
//============================================================================//
void TestMinMaxAbs (void) {

	// Print test caption
	PrintCaption ("Array::MinMaxAbs");

	// Signed integer types
	TestArray (MinMaxAbs <uint8_t, sint8_t>);
	TestArray (MinMaxAbs <uint16_t, sint16_t>);
	TestArray (MinMaxAbs <uint32_t, sint32_t>);
	//TestArray (MinMaxAbs <uint64_t, sint64_t>);

	// Floating-point types
	TestArray (MinMaxAbs <flt32_t, flt32_t>);
	TestArray (MinMaxAbs <flt64_t, flt64_t>);
}

//============================================================================//
//      Minimum value                                                         //
//============================================================================//
void TestMin (void) {

	// Print test caption
	PrintCaption ("Array::Min");

	// Unsigned integer types
	TestArray (Min <uint8_t, uint8_t>);
	TestArray (Min <uint16_t, uint16_t>);
	TestArray (Min <uint32_t, uint32_t>);
	//TestArray (Min <uint64_t, uint64_t>);

	// Signed integer types
	TestArray (Min <sint8_t, sint8_t>);
	TestArray (Min <sint16_t, sint16_t>);
	TestArray (Min <sint32_t, sint32_t>);
	//TestArray (Min <sint64_t, sint64_t>);

	// Floating-point types
	TestArray (Min <flt32_t, flt32_t>);
	TestArray (Min <flt64_t, flt64_t>);
}

//============================================================================//
//      Maximum value                                                         //
//============================================================================//
void TestMax (void) {

	// Print test caption
	PrintCaption ("Array::Max");

	// Unsigned integer types
	TestArray (Max <uint8_t, uint8_t>);
	TestArray (Max <uint16_t, uint16_t>);
	TestArray (Max <uint32_t, uint32_t>);
	//TestArray (Max <uint64_t, uint64_t>);

	// Signed integer types
	TestArray (Max <sint8_t, sint8_t>);
	TestArray (Max <sint16_t, sint16_t>);
	TestArray (Max <sint32_t, sint32_t>);
	//TestArray (Max <sint64_t, sint64_t>);

	// Floating-point types
	TestArray (Max <flt32_t, flt32_t>);
	TestArray (Max <flt64_t, flt64_t>);
}

//============================================================================//
//      Both minimum and maximum values                                       //
//============================================================================//
void TestMinMax (void) {

	// Print test caption
	PrintCaption ("Array::MinMax");

	// Unsigned integer types
	TestArray (MinMax <uint8_t, uint8_t>);
	TestArray (MinMax <uint16_t, uint16_t>);
	TestArray (MinMax <uint32_t, uint32_t>);
	//TestArray (MinMax <uint64_t, uint64_t>);

	// Signed integer types
	TestArray (MinMax <sint8_t, sint8_t>);
	TestArray (MinMax <sint16_t, sint16_t>);
	TestArray (MinMax <sint32_t, sint32_t>);
	//TestArray (MinMax <sint64_t, sint64_t>);

	// Floating-point types
	TestArray (MinMax <flt32_t, flt32_t>);
	TestArray (MinMax <flt64_t, flt64_t>);
}

//============================================================================//
//      Linear search                                                         //
//============================================================================//
# define	FIND_LIN1(fname)													\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t count,																\
	type_t value																\
){																				\
	size_t computed_pos = Array::fname (array.Data() + offset, count, value);	\
	size_t correct_pos = fname (array.Data() + offset, count, value);			\
	CheckResult (computed_pos, correct_pos);									\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	FIND_LIN2(fname, ref)												\
void fname (																	\
	RandomArray <sint64_t> &array,												\
	size_t offset,																\
	size_t count,																\
	sint64_t value																\
){																				\
	const void **arr = reinterpret_cast <const void**> (array.Data() + offset);	\
	const void *val = reinterpret_cast <const void*> (value);					\
	size_t computed_pos = Array::fname (arr, count, val, KeyCmp);				\
	size_t correct_pos = ref (array.Data() + offset, count, value);				\
	CheckResult (computed_pos, correct_pos);									\
}
FIND_LIN1 (FindFwd)
FIND_LIN1 (FindBwd)
FIND_LIN1 (Count)
FIND_LIN2 (FindObjFwd, FindFwd)
FIND_LIN2 (FindObjBwd, FindBwd)
FIND_LIN2 (CountObj, Count)
USCALAR_INT (FindFwd)
USCALAR_INT (FindBwd)
USCALAR_OBJ (FindObjFwd)
USCALAR_OBJ (FindObjBwd)

//============================================================================//
//      Binary search                                                         //
//============================================================================//
# define	FIND_ASC1(fname, ref)												\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t count,																\
	type_t value																\
){																				\
	array.Sort (false);															\
	size_t computed_pos = Array::fname (array.Data() + offset, count, value);	\
	size_t correct_pos = ref (array.Data() + offset, count, value);				\
	CheckResult (computed_pos, correct_pos);									\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	FIND_DSC1(fname, ref)												\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t count,																\
	type_t value																\
){																				\
	array.Sort (true);															\
	size_t computed_pos = Array::fname (array.Data() + offset, count, value);	\
	size_t correct_pos = ref (array.Data() + offset, count, value);				\
	CheckResult (computed_pos, correct_pos);									\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	FIND_ASC2(fname, ref)												\
void fname (																	\
	RandomArray <sint64_t> &array,												\
	size_t offset,																\
	size_t count,																\
	sint64_t value																\
){																				\
	array.Sort (false);															\
	const void **arr = reinterpret_cast <const void**> (array.Data() + offset);	\
	const void *val = reinterpret_cast <const void*> (value);					\
	size_t computed_pos = Array::fname (arr, count, val, KeyCmp);				\
	size_t correct_pos = ref (array.Data() + offset, count, value);				\
	CheckResult (computed_pos, correct_pos);									\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	FIND_DSC2(fname, ref)												\
void fname (																	\
	RandomArray <sint64_t> &array,												\
	size_t offset,																\
	size_t count,																\
	sint64_t value																\
){																				\
	array.Sort (true);															\
	const void **arr = reinterpret_cast <const void**> (array.Data() + offset);	\
	const void *val = reinterpret_cast <const void*> (value);					\
	size_t computed_pos = Array::fname (arr, count, val, KeyCmp);				\
	size_t correct_pos = ref (array.Data() + offset, count, value);				\
	CheckResult (computed_pos, correct_pos);									\
}
FIND_ASC1 (FindFirstEqualAsc, FindFirstEqual)
FIND_ASC1 (FindLastEqualAsc, FindLastEqual)
FIND_ASC1 (FindGreatAsc, FindGreatAsc)
FIND_ASC1 (FindGreatOrEqualAsc, FindGreatOrEqualAsc)
FIND_ASC1 (FindLessAsc, FindLessAsc)
FIND_ASC1 (FindLessOrEqualAsc, FindLessOrEqualAsc)
FIND_ASC1 (CountAsc, Count)
FIND_DSC1 (FindFirstEqualDsc, FindFirstEqual)
FIND_DSC1 (FindLastEqualDsc, FindLastEqual)
FIND_DSC1 (FindGreatDsc, FindGreatDsc)
FIND_DSC1 (FindGreatOrEqualDsc, FindGreatOrEqualDsc)
FIND_DSC1 (FindLessDsc, FindLessDsc)
FIND_DSC1 (FindLessOrEqualDsc, FindLessOrEqualDsc)
FIND_DSC1 (CountDsc, Count)
FIND_ASC2 (FindFirstEqualObjAsc, FindFirstEqual)
FIND_ASC2 (FindLastEqualObjAsc, FindLastEqual)
FIND_ASC2 (FindGreatObjAsc, FindGreatAsc)
FIND_ASC2 (FindGreatOrEqualObjAsc, FindGreatOrEqualAsc)
FIND_ASC2 (FindLessObjAsc, FindLessAsc)
FIND_ASC2 (FindLessOrEqualObjAsc, FindLessOrEqualAsc)
FIND_ASC2 (CountObjAsc, Count)
FIND_DSC2 (FindFirstEqualObjDsc, FindFirstEqual)
FIND_DSC2 (FindLastEqualObjDsc, FindLastEqual)
FIND_DSC2 (FindGreatObjDsc, FindGreatDsc)
FIND_DSC2 (FindGreatOrEqualObjDsc, FindGreatOrEqualDsc)
FIND_DSC2 (FindLessObjDsc, FindLessDsc)
FIND_DSC2 (FindLessOrEqualObjDsc, FindLessOrEqualDsc)
FIND_DSC2 (CountObjDsc, Count)
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
USCALAR_INT (FindFirstEqualAsc)
USCALAR_INT (FindFirstEqualDsc)
USCALAR_INT (FindLastEqualAsc)
USCALAR_INT (FindLastEqualDsc)
USCALAR_INT (FindGreatAsc)
USCALAR_INT (FindGreatDsc)
USCALAR_INT (FindGreatOrEqualAsc)
USCALAR_INT (FindGreatOrEqualDsc)
USCALAR_INT (FindLessAsc)
USCALAR_INT (FindLessDsc)
USCALAR_INT (FindLessOrEqualAsc)
USCALAR_INT (FindLessOrEqualDsc)
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
USCALAR_OBJ (FindFirstEqualObjAsc)
USCALAR_OBJ (FindFirstEqualObjDsc)
USCALAR_OBJ (FindLastEqualObjAsc)
USCALAR_OBJ (FindLastEqualObjDsc)
USCALAR_OBJ (FindGreatObjAsc)
USCALAR_OBJ (FindGreatObjDsc)
USCALAR_OBJ (FindGreatOrEqualObjAsc)
USCALAR_OBJ (FindGreatOrEqualObjDsc)
USCALAR_OBJ (FindLessObjAsc)
USCALAR_OBJ (FindLessObjDsc)
USCALAR_OBJ (FindLessOrEqualObjAsc)
USCALAR_OBJ (FindLessOrEqualObjDsc)

//============================================================================//
//      Linear counting                                                       //
//============================================================================//
USCALAR_INT (Count)
USCALAR_OBJ (CountObj)

//============================================================================//
//      Binary counting                                                       //
//============================================================================//
USCALAR_INT (CountAsc)
USCALAR_INT (CountDsc)
USCALAR_OBJ (CountObjAsc)
USCALAR_OBJ (CountObjDsc)

//============================================================================//
//      Element replacing                                                     //
//============================================================================//
template <typename type_t>
void Replace (
	RandomArray <type_t> &array,
	size_t offset,
	size_t count,
	type_t value
){
	type_t replacement = array.RandomValue();

	// Make a copy for the reference implementation of the function
	RandomArray <type_t> reference (array);

	// Apply the operation to the array data. Both the testing and the reference
	Array::Replace (array.Data() + offset, count, value, replacement);
	Replace (reference.Data() + offset, count, value, replacement);

	// Compare arrays for different elements
	array.Compare (reference);
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void ReplaceObj (
	RandomArray <sint64_t> &array,
	size_t offset,
	size_t count,
	sint64_t value
){
	sint64_t replacement = array.RandomValue();
	const void **arr = reinterpret_cast <const void**> (array.Data() + offset);
	const void *val = reinterpret_cast <const void*> (value);
	const void *repl = reinterpret_cast <const void*> (replacement);

	// Make a copy for the reference implementation of the function
	RandomArray <sint64_t> reference (array);

	// Apply the operation to the array data. Both the testing and the reference
	Array::ReplaceObj (arr, count, val, repl, KeyCmp);
	Replace (reference.Data() + offset, count, value, replacement);

	// Compare arrays for different elements
	array.Compare (reference);
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
USCALAR_INT (Replace)
USCALAR_OBJ (ReplaceObj)

//============================================================================//
//      Order reversing                                                       //
//============================================================================//
USCALAR (Reverse)
USCALAR_OBJ (ReverseObj)

//============================================================================//
//      Unique elements                                                       //
//============================================================================//
template <typename type_t>
void Unique (
	RandomArray <type_t> &array,
	size_t offset,
	size_t count,
	type_t value
){
	// Create arrays for unique values
	RandomArray <type_t> func_output (SIZE, SEED, MAX_VALUE);
	RandomArray <type_t> ref_output (SIZE, SEED, MAX_VALUE);

	// Apply the operation to the array data. Both the testing and the reference
	size_t computed_count = Array::Unique (func_output.Data(), array.Data() + offset, count);
	size_t correct_count = Unique (ref_output.Data(), array.Data() + offset, count);

	// Compare the result values
	CheckResult (computed_count, correct_count);

	// Compare arrays for different elements
	func_output.Compare (ref_output, correct_count);
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void UniqueObj (
	RandomArray <sint64_t> &array,
	size_t offset,
	size_t count,
	sint64_t value
){
	// Create arrays for unique values
	RandomArray <sint64_t> func_output (SIZE, SEED, MAX_VALUE);
	RandomArray <sint64_t> ref_output (SIZE, SEED, MAX_VALUE);

	// Apply the operation to the array data. Both the testing and the reference
	const void **res = reinterpret_cast <const void**> (func_output.Data());
	const void **arr = reinterpret_cast <const void**> (array.Data() + offset);
	size_t computed_count = Array::UniqueObj (res, arr, count, KeyCmp);
	size_t correct_count = Unique (ref_output.Data(), array.Data() + offset, count);

	// Compare the result values
	CheckResult (computed_count, correct_count);

	// Compare arrays for different elements
	func_output.Compare (ref_output, correct_count);
}
USCALAR_INT (Unique)
USCALAR_OBJ (UniqueObj)

//============================================================================//
//      Duplicate elements                                                    //
//============================================================================//
template <typename type_t>
void Duplicates (
	RandomArray <type_t> &array,
	size_t offset,
	size_t count,
	type_t value
){
	// Create arrays for unique values
	RandomArray <type_t> func_output (SIZE, SEED, MAX_VALUE);
	RandomArray <type_t> ref_output (SIZE, SEED, MAX_VALUE);

	// Create arrays for counters
	RandomArray <size_t> func_counter (SIZE, SEED, MAX_VALUE);
	RandomArray <size_t> ref_counter (SIZE, SEED, MAX_VALUE);

	// Apply the operation to the array data. Both the testing and the reference
	size_t computed_count = Array::Duplicates (func_output.Data(), func_counter.Data(), array.Data() + offset, count);
	size_t correct_count = Duplicates (ref_output.Data(), ref_counter.Data(), array.Data() + offset, count);

	// Compare the result values
	CheckResult (computed_count, correct_count);

	// Compare arrays for different elements
	func_output.Compare (ref_output, correct_count);
	func_counter.Compare (ref_counter, correct_count);
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void DuplicatesObj (
	RandomArray <sint64_t> &array,
	size_t offset,
	size_t count,
	sint64_t value
){
	// Create arrays for unique values
	RandomArray <sint64_t> func_output (SIZE, SEED, MAX_VALUE);
	RandomArray <sint64_t> ref_output (SIZE, SEED, MAX_VALUE);

	// Create arrays for counters
	RandomArray <size_t> func_counter (SIZE, SEED, MAX_VALUE);
	RandomArray <size_t> ref_counter (SIZE, SEED, MAX_VALUE);

	// Apply the operation to the array data. Both the testing and the reference
	const void **res = reinterpret_cast <const void**> (func_output.Data());
	const void **arr = reinterpret_cast <const void**> (array.Data() + offset);
	size_t computed_count = Array::DuplicatesObj (res, func_counter.Data(), arr, count, KeyCmp);
	size_t correct_count = Duplicates (ref_output.Data(), ref_counter.Data(), array.Data() + offset, count);

	// Compare the result values
	CheckResult (computed_count, correct_count);

	// Compare arrays for different elements
	func_output.Compare (ref_output, correct_count);
	func_counter.Compare (ref_counter, correct_count);
}
USCALAR_INT (Duplicates)
USCALAR_OBJ (DuplicatesObj)

//============================================================================//
//      N-th order statistic                                                  //
//============================================================================//
# define	STATISTIC1(fname)													\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t count,																\
	type_t value																\
){																				\
	size_t index = array.Count (offset);										\
	RandomArray <type_t> reference (array);										\
	Array::fname (array.Data() + offset, count, index);							\
	fname <type_t> (reference.Data() + offset, count, index);					\
	if (index < count) {														\
		type_t computed_value = *(array.Data() + offset + index);				\
		type_t correct_value = *(reference.Data() + offset + index);			\
		CheckResult (computed_value, correct_value);							\
	}																			\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	STATISTIC2(fname, ref)												\
void fname (																	\
	RandomArray <sint64_t> &array,												\
	size_t offset,																\
	size_t count,																\
	sint64_t value																\
){																				\
	size_t index = array.Count (offset);										\
	RandomArray <sint64_t> reference (array);									\
	const void **arr = reinterpret_cast <const void**> (array.Data() + offset);	\
	Array::fname (arr, count, index, KeyCmp);									\
	ref <sint64_t> (reference.Data() + offset, count, index);					\
	if (index < count) {														\
		sint64_t computed_value = *(array.Data() + offset + index);				\
		sint64_t correct_value = *(reference.Data() + offset + index);			\
		CheckResult (computed_value, correct_value);							\
	}																			\
}
STATISTIC1 (QuickSelectAsc)
STATISTIC1 (QuickSelectDsc)
STATISTIC2 (QuickSelectObjAsc, QuickSelectAsc)
STATISTIC2 (QuickSelectObjDsc, QuickSelectDsc)
USCALAR (QuickSelectAsc)
USCALAR (QuickSelectDsc)
USCALAR_OBJ (QuickSelectObjAsc)
USCALAR_OBJ (QuickSelectObjDsc)

//============================================================================//
//      Quick sort                                                            //
//============================================================================//
# define	QSORT_KEY(fname,ref)												\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t count,																\
	type_t value																\
){																				\
	RandomArray <size_t> data (array);											\
	const void **ptr = reinterpret_cast <const void**> (data.Data() + offset);	\
	RandomArray <type_t> reference (array);										\
	Array::fname (array.Data() + offset, ptr, count);							\
	ref (reference.Data() + offset, count);										\
	array.Compare (reference);													\
	data.CheckValues (array);													\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	QSORT_OBJ(fname,ref)												\
void fname (																	\
	RandomArray <sint64_t> &array,												\
	size_t offset,																\
	size_t count,																\
	sint64_t value																\
){																				\
	const void **arr = reinterpret_cast <const void**> (array.Data() + offset);	\
	RandomArray <sint64_t> reference (array);									\
	Array::fname (arr, count, KeyCmp);											\
	ref (reference.Data() + offset, count);										\
	array.Compare (reference);													\
}
QSORT_KEY (QuickSortKeyAsc, QuickSortAsc)
QSORT_KEY (QuickSortKeyDsc, QuickSortDsc)
QSORT_OBJ (QuickSortObjAsc, QuickSortAsc)
QSORT_OBJ (QuickSortObjDsc, QuickSortDsc)
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void TestQuickSortAsc (void) {

	// Print test caption
	PrintCaption ("Array::QuickSortAsc");

	// Unsigned integer types
	TestArray (QuickSortAsc <uint8_t>);
	TestArray (QuickSortAsc <uint16_t>);
	TestArray (QuickSortAsc <uint32_t>);
	//TestArray (QuickSortAsc <uint64_t>);

	// Signed integer types
	TestArray (QuickSortAsc <sint8_t>);
	TestArray (QuickSortAsc <sint16_t>);
	TestArray (QuickSortAsc <sint32_t>);
	//TestArray (QuickSortAsc <sint64_t>);

	// Floating-point types
	TestArray (QuickSortAsc <flt32_t>);
	//TestArray (QuickSortAsc <flt64_t>);

	// Other types
	//TestArray (QuickSortAsc <size_t>);
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void TestQuickSortDsc (void) {

	// Print test caption
	PrintCaption ("Array::QuickSortDsc");

	// Unsigned integer types
	TestArray (QuickSortDsc <uint8_t>);
	TestArray (QuickSortDsc <uint16_t>);
	TestArray (QuickSortDsc <uint32_t>);
	//TestArray (QuickSortDsc <uint64_t>);

	// Signed integer types
	TestArray (QuickSortDsc <sint8_t>);
	TestArray (QuickSortDsc <sint16_t>);
	TestArray (QuickSortDsc <sint32_t>);
	//TestArray (QuickSortDsc <sint64_t>);

	// Floating-point types
	TestArray (QuickSortDsc <flt32_t>);
	//TestArray (QuickSortDsc <flt64_t>);

	// Other types
	//TestArray (QuickSortDsc <size_t>);
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
USCALAR (QuickSortKeyAsc)
USCALAR (QuickSortKeyDsc)
USCALAR_OBJ (QuickSortObjAsc)
USCALAR_OBJ (QuickSortObjDsc)

//============================================================================//
//      Merge sort                                                            //
//============================================================================//
# define	SORT_KEY(fname,ref)													\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t count,																\
	type_t value																\
){																				\
	RandomArray <type_t> temp (SIZE, SEED, MAX_VALUE);							\
	RandomArray <size_t> temp_ptr (SIZE, SEED, MAX_VALUE);						\
	RandomArray <size_t> data (array);											\
	const void **ptr = reinterpret_cast <const void**> (data.Data() + offset);	\
	const void **tptr = reinterpret_cast <const void**> (temp_ptr.Data());		\
	RandomArray <type_t> reference (array);										\
	Array::fname (array.Data() + offset, ptr, temp.Data(), tptr, count);		\
	ref (reference.Data() + offset, count);										\
	array.Compare (reference);													\
	data.CheckValues (array);													\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	SORT_OBJ(fname,ref)													\
void fname (																	\
	RandomArray <sint64_t> &array,												\
	size_t offset,																\
	size_t count,																\
	sint64_t value																\
){																				\
	RandomArray <sint64_t> temp (SIZE, SEED, MAX_VALUE);						\
	const void **arr = reinterpret_cast <const void**> (array.Data() + offset);	\
	const void **tptr = reinterpret_cast <const void**> (temp.Data() + offset);	\
	RandomArray <sint64_t> reference (array);									\
	Array::fname (arr, tptr, count, KeyCmp);									\
	ref (reference.Data() + offset, count);										\
	array.Compare (reference);													\
}
SORT_KEY (MergeSortKeyAsc, MergeSortAsc)
SORT_KEY (MergeSortKeyDsc, MergeSortDsc)
SORT_KEY (RadixSortKeyAsc, RadixSortAsc)
SORT_KEY (RadixSortKeyDsc, RadixSortDsc)
SORT_OBJ (MergeSortObjAsc, MergeSortAsc)
SORT_OBJ (MergeSortObjDsc, MergeSortDsc)
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void TestMergeSortAsc (void) {

	// Print test caption
	PrintCaption ("Array::MergeSortAsc");

	// Unsigned integer types
	TestArray (MergeSortAsc <uint8_t>);
	TestArray (MergeSortAsc <uint16_t>);
	TestArray (MergeSortAsc <uint32_t>);
	//TestArray (MergeSortAsc <uint64_t>);

	// Signed integer types
	TestArray (MergeSortAsc <sint8_t>);
	TestArray (MergeSortAsc <sint16_t>);
	TestArray (MergeSortAsc <sint32_t>);
	//TestArray (MergeSortAsc <sint64_t>);

	// Floating-point types
	TestArray (MergeSortAsc <flt32_t>);
	//TestArray (MergeSortAsc <flt64_t>);

	// Other types
	//TestArray (MergeSortAsc <size_t>);
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void TestMergeSortDsc (void) {

	// Print test caption
	PrintCaption ("Array::MergeSortDsc");

	// Unsigned integer types
	TestArray (MergeSortDsc <uint8_t>);
	TestArray (MergeSortDsc <uint16_t>);
	TestArray (MergeSortDsc <uint32_t>);
	//TestArray (MergeSortDsc <uint64_t>);

	// Signed integer types
	TestArray (MergeSortDsc <sint8_t>);
	TestArray (MergeSortDsc <sint16_t>);
	TestArray (MergeSortDsc <sint32_t>);
	//TestArray (MergeSortDsc <sint64_t>);

	// Floating-point types
	TestArray (MergeSortDsc <flt32_t>);
	//TestArray (MergeSortDsc <flt64_t>);

	// Other types
	//TestArray (MergeSortDsc <size_t>);
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
USCALAR (MergeSortKeyAsc)
USCALAR (MergeSortKeyDsc)
USCALAR_OBJ (MergeSortObjAsc)
USCALAR_OBJ (MergeSortObjDsc)

//============================================================================//
//      Radix sort                                                            //
//============================================================================//
void TestRadixSortAsc (void) {

	// Print test caption
	PrintCaption ("Array::RadixSortAsc");

	// Unsigned integer types
	TestArray (RadixSortAsc <uint8_t>);
	TestArray (RadixSortAsc <uint16_t>);
	TestArray (RadixSortAsc <uint32_t>);
	//TestArray (RadixSortAsc <uint64_t>);

	// Signed integer types
	TestArray (RadixSortAsc <sint8_t>);
	TestArray (RadixSortAsc <sint16_t>);
	TestArray (RadixSortAsc <sint32_t>);
	//TestArray (RadixSortAsc <sint64_t>);

	// Floating-point types
	TestArray (RadixSortAsc <flt32_t>);
	//TestArray (RadixSortAsc <flt64_t>);

	// Other types
	//TestArray (RadixSortAsc <size_t>);
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void TestRadixSortDsc (void) {

	// Print test caption
	PrintCaption ("Array::RadixSortDsc");

	// Unsigned integer types
	TestArray (RadixSortDsc <uint8_t>);
	TestArray (RadixSortDsc <uint16_t>);
	TestArray (RadixSortDsc <uint32_t>);
	//TestArray (RadixSortDsc <uint64_t>);

	// Signed integer types
	TestArray (RadixSortDsc <sint8_t>);
	TestArray (RadixSortDsc <sint16_t>);
	TestArray (RadixSortDsc <sint32_t>);
	//TestArray (RadixSortDsc <sint64_t>);

	// Floating-point types
	TestArray (RadixSortDsc <flt32_t>);
	//TestArray (RadixSortDsc <flt64_t>);

	// Other types
	//TestArray (RadixSortDsc <size_t>);
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
USCALAR (RadixSortKeyAsc)
USCALAR (RadixSortKeyDsc)

//============================================================================//
//      Compare functions                                                     //
//============================================================================//
# define	CMP(fname)															\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t count,																\
	type_t value																\
){																				\
	array.Sort (count % 2);														\
	sint64_t computed_value = Array::fname (array.Data() + offset, count);		\
	sint64_t correct_value = fname (array.Data() + offset, count);				\
	CheckResult (computed_value, correct_value);								\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	DIFF(fname)															\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &target,												\
	RandomArray <type_t> &source,												\
	size_t toffset,																\
	size_t soffset,																\
	size_t count																\
){																				\
	sint64_t computed_value = Array::fname (target.Data() + toffset, source.Data() + soffset, count);\
	sint64_t correct_value = fname (target.Data() + toffset, source.Data() + soffset, count);\
	CheckResult (computed_value, correct_value);								\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	CMP_OBJ(fname, ref)													\
void fname (																	\
	RandomArray <sint64_t> &array,												\
	size_t offset,																\
	size_t count,																\
	sint64_t value																\
){																				\
	array.Sort (count % 2);														\
	const void **arr = reinterpret_cast <const void**> (array.Data() + offset);	\
	sint64_t computed_value = Array::fname (arr, count, KeyCmp);				\
	sint64_t correct_value = ref (array.Data() + offset, count);				\
	CheckResult (computed_value, correct_value);								\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	DIFF_OBJ(fname, ref)												\
void fname (																	\
	RandomArray <sint64_t> &target,												\
	RandomArray <sint64_t> &source,												\
	size_t toffset,																\
	size_t soffset,																\
	size_t count																\
){																				\
	const void **tarr = reinterpret_cast <const void**> (target.Data() + toffset);\
	const void **sarr = reinterpret_cast <const void**> (source.Data() + soffset);\
	sint64_t computed_value = Array::fname (tarr, sarr, count, KeyCmp);			\
	sint64_t correct_value = ref (target.Data() + toffset, source.Data() + soffset, count);\
	CheckResult (computed_value, correct_value);								\
}
DIFF (Compare)
DIFF (CheckDiff)
CMP (CheckDup)
CMP (CheckSortAsc)
CMP (CheckSortDsc)
CMP (CheckInf)
CMP (CheckNaN)
DIFF_OBJ (CompareObj, Compare)
DIFF_OBJ (CheckDiffObj, CheckDiff)
CMP_OBJ (CheckDupObj, CheckDup)
CMP_OBJ (CheckSortObjAsc, CheckSortAsc)
CMP_OBJ (CheckSortObjDsc, CheckSortDsc)
UVECTOR (Compare)
UVECTOR_OBJ (CompareObj)

//============================================================================//
//      Check for differences                                                 //
//============================================================================//
UVECTOR (CheckDiff)
UVECTOR_OBJ (CheckDiffObj)

//============================================================================//
//      Check for duplicate values                                            //
//============================================================================//
USCALAR (CheckDup)
USCALAR_OBJ (CheckDupObj)

//============================================================================//
//      Check for ascending sort order                                        //
//============================================================================//
USCALAR (CheckSortAsc)
USCALAR_OBJ (CheckSortObjAsc)

//============================================================================//
//      Check for descending sort order                                       //
//============================================================================//
USCALAR (CheckSortDsc)
USCALAR_OBJ (CheckSortObjDsc)

//============================================================================//
//      Check for infinite values                                             //
//============================================================================//
USCALAR_FLT (CheckInf)

//============================================================================//
//      Check for NaN values                                                  //
//============================================================================//
USCALAR_FLT (CheckNaN)

//****************************************************************************//
//      Main function                                                         //
//****************************************************************************//
int main (void)
try {

	// Print program header
	PrintHeader ("Array library test suite");
	cout << "This test operates with " << SIZE << "-long arrays in " << ROUNDS << " rounds with " << TRIES << " tries in each." << endl;

	// Set the precision of GMP operations
	mpf_set_default_prec (256);

	// Initialization
	TestInit();

	// Copying arrays
	TestCopy();

	// Moving arrays
	TestMove();

	// Pattern cloning
	TestClone();

	// Data conversion
	TestConvert();

	// Bitwise operations
	TestByteSwap();
	TestBitReverse();
	TestPopCount();
	TestNot();
	TestAndScalar();
	TestAndVector();
	TestOrScalar();
	TestOrVector();
	TestXorScalar();
	TestXorVector();

	// Arithmetic operations
	TestNeg();
	TestAbs();
	TestNegAbs();
	TestSign();
	TestSqr();
	TestSqrt();

	TestAddScalar();
	TestAddVector();
	TestSubScalar();
	TestSubVector();
	TestReverseSubScalar();
	TestReverseSubVector();
	TestMulScalar();
	TestMulVector();
	TestDivScalar();
	TestDivVector();
	TestReverseDivScalar();
	TestReverseDivVector();

	// Fused arithmetic operations
	TestFusedAdd();
	TestFusedSub();

	// Rounding
	TestRoundDown();
	TestRoundUp();
	TestRoundInt();
	TestRoundEven();
	TestRound();
	TestTruncate();
	TestFrac();

	// Numerical integration
	TestSum();
	TestSumAbs();
	TestSumSqr();
	TestSumMul();
	TestSumSqr2();
	TestSumMul2();
	TestSumDiff();
	TestSumAbsDiff();
	TestSumSqrDiff();
	TestSumMulDiff();
	TestSumSqrDiff2();
	TestSumMulDiff2();
	TestSumDist();
	TestSumAbsDist();
	TestSumSqrDist();

	// Minimum and maximum absolute value
	TestMinAbs();
	TestMaxAbs();
	TestMinMaxAbs();

	// Minimum and maximum value
	TestMin();
	TestMax();
	TestMinMax();

	// Linear array search
	TestFindFwd();
	TestFindBwd();
	TestFindObjFwd();
	TestFindObjBwd();

	// Binary array search
	TestFindFirstEqualAsc();
	TestFindLastEqualAsc();
	TestFindGreatAsc();
	TestFindGreatOrEqualAsc();
	TestFindLessAsc();
	TestFindLessOrEqualAsc();
	TestFindFirstEqualDsc();
	TestFindLastEqualDsc();
	TestFindLessDsc();
	TestFindLessOrEqualDsc();
	TestFindGreatDsc();
	TestFindGreatOrEqualDsc();
	TestFindFirstEqualObjAsc();
	TestFindLastEqualObjAsc();
	TestFindGreatObjAsc();
	TestFindGreatOrEqualObjAsc();
	TestFindLessObjAsc();
	TestFindLessOrEqualObjAsc();
	TestFindFirstEqualObjDsc();
	TestFindLastEqualObjDsc();
	TestFindLessObjDsc();
	TestFindLessOrEqualObjDsc();
	TestFindGreatObjDsc();
	TestFindGreatOrEqualObjDsc();

	// Counting
	TestCount();
	TestCountObj();
	TestCountAsc();
	TestCountDsc();
	TestCountObjAsc();
	TestCountObjDsc();

	// Replacing
	TestReplace();
	TestReplaceObj();

	// Order reversing
	TestReverse();
	TestReverseObj();

	// Unique values
	TestUnique();
	TestUniqueObj();

	// Duplicate elements
	TestDuplicates();
	TestDuplicatesObj();

	// N-th order statistic
	TestQuickSelectAsc();
	TestQuickSelectDsc();
	TestQuickSelectObjAsc();
	TestQuickSelectObjDsc();

	// Quick sort
	TestQuickSortAsc();
	TestQuickSortDsc();
	TestQuickSortKeyAsc();
	TestQuickSortKeyDsc();
	TestQuickSortObjAsc();
	TestQuickSortObjDsc();

	// Merge sort
	TestMergeSortAsc();
	TestMergeSortDsc();
	TestMergeSortKeyAsc();
	TestMergeSortKeyDsc();
	TestMergeSortObjAsc();
	TestMergeSortObjDsc();

	// Radix sort
	TestRadixSortAsc();
	TestRadixSortDsc();
	TestRadixSortKeyAsc();
	TestRadixSortKeyDsc();

	// Comparison of arrays
	TestCompare();
	TestCompareObj();

	// Checks
	TestCheckDiff();
	TestCheckDiffObj();
	TestCheckDup();
	TestCheckDupObj();
	TestCheckSortAsc();
	TestCheckSortObjAsc();
	TestCheckSortDsc();
	TestCheckSortObjDsc();
	TestCheckInf();
	TestCheckNaN();

	// Print success message
	PrintSuccessStatus ("Array library");
	return 0;
}
catch (const runtime_error &ex) {

	// Print fail message
	PrintFailStatus ("Array library");
	cerr << ex.what() << endl;
	return 1;
}
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
