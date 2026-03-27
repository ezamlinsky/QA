/*                                                                     Angle.cpp
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                           ANGLE LIBRARY TEST SUITE                           #
#                                                                              #
# Ordnung muss sein!                             Copyleft (Ɔ) Eugene Zamlinsky #
################################################################################
*/
# include	<Common.h>
# include	<RandomArray.h>
# include	<Angle.h>

//****************************************************************************//
//      Test parameters                                                       //
//****************************************************************************//
# define	SCALE		12				// Scale factor
# define	SIZE		(1 << SCALE) 	// Generate 2 ^ SCALE elements-long arrays
# define	ROUNDS		100				// Rounds for random testing
# define	SEED		100				// Seed value for the random number generator
# define	MAX_VALUE	127				// Max generated random value

//****************************************************************************//
//      Test angle conversion functions                                       //
//****************************************************************************//
template <typename type_t>
void TestAngles (
	void (*func)(RandomArray <type_t>&)
){
	// Print test info
	PrintInfo (typeid (type_t).name());

	// Create an array of the target size
	RandomArray <type_t> array (SIZE, SEED, MAX_VALUE);

	// Run the test in many rounds with random offset and elements count
	for (size_t i = 0; i < ROUNDS; i++) {

		// Populate the array with random angles
		array.Populate ();

		// Apply the operation to the array data
		func (array);
	}
}

//****************************************************************************//
//      Reference functions                                                   //
//****************************************************************************//

//============================================================================//
//      Conversion radians to degrees function                                //
//============================================================================//
template <typename type_t>
type_t RadToDeg (type_t angle)
{
	return angle * (static_cast <type_t> (180.0) / static_cast <type_t> (M_PI));
}

//============================================================================//
//      Conversion radians to gradians function                               //
//============================================================================//
template <typename type_t>
type_t RadToGrad (type_t angle)
{
	return angle * (static_cast <type_t> (200.0) / static_cast <type_t> (M_PI));
}

//============================================================================//
//      Conversion degrees to radians function                                //
//============================================================================//
template <typename type_t>
type_t DegToRad (type_t angle)
{
	return angle * (static_cast <type_t> (M_PI) / static_cast <type_t> (180.0));
}

//============================================================================//
//      Conversion degrees to gradians function                               //
//============================================================================//
template <typename type_t>
type_t DegToGrad (type_t angle)
{
	return angle * (static_cast <type_t> (200.0) / static_cast <type_t> (180.0));
}

//============================================================================//
//      Conversion gradians to radians function                               //
//============================================================================//
template <typename type_t>
type_t GradToRad (type_t angle)
{
	return angle * (static_cast <type_t> (M_PI) / static_cast <type_t> (200.0));
}

//============================================================================//
//      Conversion gradians to degrees function                               //
//============================================================================//
template <typename type_t>
type_t GradToDeg (type_t angle)
{
	return angle * (static_cast <type_t> (180.0) / static_cast <type_t> (200.0));
}

//****************************************************************************//
//      Testing functions                                                     //
//****************************************************************************//
# define	TEST(fname)															\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array													\
){																				\
	size_t size = array.Size();													\
	RandomArray <type_t> out (size, SEED, MAX_VALUE);							\
	RandomArray <type_t> ref (size, SEED, MAX_VALUE);							\
	type_t *data_arr = array.Data();											\
	type_t *out_arr = out.Data();												\
	type_t *ref_arr = ref.Data();												\
	for (size_t i = 0; i < size; i++) {											\
		out_arr[i] = Angle::fname (data_arr [i]);								\
		ref_arr[i] = fname (data_arr [i]);										\
	}																			\
	out.Compare (ref);															\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	UTEST(fname)														\
void Test##fname (void) {														\
	PrintCaption ("Array::"#fname);												\
	TestAngles (fname <flt32_t>);												\
	TestAngles (fname <flt64_t>);												\
}
TEST (RadToDeg)
TEST (RadToGrad)
TEST (DegToRad)
TEST (DegToGrad)
TEST (GradToRad)
TEST (GradToDeg)
UTEST (RadToDeg)
UTEST (RadToGrad)
UTEST (DegToRad)
UTEST (DegToGrad)
UTEST (GradToRad)
UTEST (GradToDeg)

//****************************************************************************//
//      Main function                                                         //
//****************************************************************************//
int main (void)
try {

	// Print program header
	PrintHeader ("Angle library test suite");
	cout << "This test operates with " << SIZE << " random angle values in " << ROUNDS << " rounds." << endl;

	// Run all the tests
	TestRadToDeg();
	TestRadToGrad();
	TestDegToRad();
	TestDegToGrad();
	TestGradToRad();
	TestGradToDeg();

	// Print success message
	PrintSuccessStatus ("Angle library");
	return 0;
}
catch (const runtime_error &ex) {

	// Print fail message
	PrintFailStatus ("Angle library");
	cerr << ex.what() << endl;
	return 1;
}
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
