/*                                                                Statistics.cpp
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                         STATISTICS LIBRARY TEST SUITE                        #
#                                                                              #
# Ordnung muss sein!                             Copyleft (Ɔ) Eugene Zamlinsky #
################################################################################
*/
# include	<cstring>
# include	<Common.h>
# include	<RandomArray.h>
# include	<Statistics.h>

//****************************************************************************//
//      Test parameters                                                       //
//****************************************************************************//
# define	SCALE		12				// Scale factor
# define	SIZE		((1<<SCALE)-1)	// Generate 2 ^ SCALE elements-long arrays
# define	ROUNDS		100				// Rounds for random testing
# define	TRIES		8				// Tries in each round
# define	SEED		100				// Seed value for the random number generator
# define	MAX_VALUE	127				// Max generated random value

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
//      Reference functions                                                   //
//****************************************************************************//

//============================================================================//
//      Quantiles                                                             //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ranked array                                                          //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
template <typename type_t>
type_t Quantile (const type_t array[], size_t size, double quantile) {
	if (size == 0 || quantile < 0 || quantile > 1)
		return 0;
	mpf_class quant (quantile);
	quant *= mpf_class (size - 1);
	mpf_class lower = floor (quant);
	mpf_class upper = ceil (quant);
	mpf_class gain = quant - lower;
	type_t val1 = array [static_cast <size_t> (lower.get_d())];
	type_t val2 = array [static_cast <size_t> (upper.get_d())];
	if constexpr (is_signed_v <type_t>) {
		long signed int delta = static_cast <long signed int> (val2) - static_cast <long signed int> (val1);
		gain *= delta;
	}
	else {
		long unsigned int delta = static_cast <long unsigned int> (val2) - static_cast <long unsigned int> (val1);
		gain *= delta;
	}
	return val1 + gain.get_ui();
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	SQUANTILE_FLT(type_t)												\
type_t Quantile (const type_t array[], size_t size, type_t quantile) {			\
	if (size == 0 || quantile < 0 || quantile > 1)								\
		return NAN;																\
	mpf_class quant (quantile);													\
	quant *= mpf_class (size - 1);												\
	mpf_class lower = floor (quant);											\
	mpf_class upper = ceil (quant);												\
	mpf_class gain = quant - lower;												\
	type_t val1 = array [static_cast <size_t> (lower.get_d())];					\
	type_t val2 = array [static_cast <size_t> (upper.get_d())];					\
	type_t delta = val2 - val1;													\
	return val1 + delta * gain.get_d();											\
}
SQUANTILE_FLT (flt32_t)
SQUANTILE_FLT (flt64_t)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Unordered array                                                       //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
template <typename type_t>
type_t QuantileRaw (type_t array[], size_t size, double quantile) {
	sort (array, array + size);
	return Quantile (array, size, quantile);
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	RQUANTILE_FLT(type_t)												\
type_t QuantileRaw (type_t array[], size_t size, type_t quantile) {				\
	sort (array, array + size);													\
	return Quantile (array, size, quantile);									\
}
RQUANTILE_FLT (flt32_t)
RQUANTILE_FLT (flt64_t)

//============================================================================//
//      Percentiles                                                           //
//============================================================================//

// Ranked array
template <typename type_t>
type_t Percentile (const type_t array[], size_t size, size_t percentile) {
	return Quantile (array, size, percentile / double (100.0));
}

// Unordered array
template <typename type_t>
type_t PercentileRaw (type_t array[], size_t size, size_t percentile) {
	return QuantileRaw (array, size, percentile / double (100.0));
}

//============================================================================//
//      Deciles                                                               //
//============================================================================//

// Ranked array
template <typename type_t>
type_t Decile (const type_t array[], size_t size, size_t percentile) {
	return Quantile (array, size, percentile / double (10.0));
}

// Unordered array
template <typename type_t>
type_t DecileRaw (type_t array[], size_t size, size_t percentile) {
	return QuantileRaw (array, size, percentile / double (10.0));
}

//============================================================================//
//      Lower quartile                                                        //
//============================================================================//

// Ranked array
template <typename type_t>
type_t LowerQuartile (const type_t array[], size_t size) {
	if constexpr (is_integral_v <type_t>)
		return Quantile (array, size, 0.25);
	else
		return Quantile (array, size, static_cast <type_t> (0.25));
}

// Unordered array
template <typename type_t>
type_t LowerQuartileRaw (type_t array[], size_t size) {
	if constexpr (is_integral_v <type_t>)
		return QuantileRaw (array, size, 0.25);
	else
		return QuantileRaw (array, size, static_cast <type_t> (0.25));
}

//============================================================================//
//      Upper quartile                                                        //
//============================================================================//

// Ranked array
template <typename type_t>
type_t UpperQuartile (const type_t array[], size_t size) {
	if constexpr (is_integral_v <type_t>)
		return Quantile (array, size, 0.75);
	else
		return Quantile (array, size, static_cast <type_t> (0.75));
}

// Unordered array
template <typename type_t>
type_t UpperQuartileRaw (type_t array[], size_t size) {
	if constexpr (is_integral_v <type_t>)
		return QuantileRaw (array, size, 0.75);
	else
		return QuantileRaw (array, size, static_cast <type_t> (0.75));
}

//============================================================================//
//      Median                                                                //
//============================================================================//

// Ranked array
template <typename type_t>
type_t Median (const type_t array[], size_t size) {
	if constexpr (is_integral_v <type_t>)
		return Quantile (array, size, 0.5);
	else
		return Quantile (array, size, static_cast <type_t> (0.5));

}

// Unordered array
template <typename type_t>
type_t MedianRaw (type_t array[], size_t size) {
	if constexpr (is_integral_v <type_t>)
		return QuantileRaw (array, size, 0.5);
	else
		return QuantileRaw (array, size, static_cast <type_t> (0.5));
}

//============================================================================//
//      Midhinge                                                              //
//============================================================================//

// Ranked array
template <typename type_t>
type_t Midhinge (const type_t array[], size_t size) {
	if constexpr (is_integral_v <type_t>)
		return (UpperQuartile (array, size) + LowerQuartile (array, size)) >> 1;
	else
		return 0.5 * (UpperQuartile (array, size) + LowerQuartile (array, size));
}

// Unordered array
template <typename type_t>
type_t MidhingeRaw (type_t array[], size_t size) {
	if constexpr (is_integral_v <type_t>)
		return (UpperQuartileRaw (array, size) + LowerQuartileRaw (array, size)) >> 1;
	else
		return 0.5 * (UpperQuartileRaw (array, size) + LowerQuartileRaw (array, size));
}

//============================================================================//
//      Tukey's trimean                                                       //
//============================================================================//

// Ranked array
template <typename type_t>
type_t Trimean (const type_t array[], size_t size, type_t median) {
	if constexpr (is_integral_v <type_t>)
		return (median + Midhinge (array, size)) >> 1;
	else
		return 0.5 * (median + Midhinge (array, size));
}

// Unordered array
template <typename type_t>
type_t TrimeanRaw (type_t array[], size_t size, type_t median) {
	if constexpr (is_integral_v <type_t>)
		return (median + MidhingeRaw (array, size)) >> 1;
	else
		return 0.5 * (median + MidhingeRaw (array, size));
}

//============================================================================//
//      Mid-range                                                             //
//============================================================================//
template <typename type_t>
type_t MidRange (const type_t array[], size_t size) {
	using namespace std;
	if (size == 0)
		return NAN;
	auto bounds = minmax_element (array, array + size);
	return 0.5 * (*bounds.second + *bounds.first);
}

//============================================================================//
//      Mean                                                                  //
//============================================================================//
template <typename type_t>
type_t Mean (const type_t array[], size_t size) {
	if (size == 0)
		return NAN;
	mpf_class total_sum (0);
	for (size_t i = 0; i < size; ++i) {
		type_t temp = array[i];
		mpf_class val (temp);
		total_sum += val;
	}
	mpf_class res = total_sum / mpf_class (size);
	return res.get_d();
}

//============================================================================//
//      Interquartile range                                                   //
//============================================================================//

// Ranked array
template <typename type_t>
type_t InterQuartileRange (const type_t array[], size_t size) {
	return UpperQuartile (array, size) - LowerQuartile (array, size);
}

// Unordered array
template <typename type_t>
type_t InterQuartileRangeRaw (type_t array[], size_t size) {
	return UpperQuartileRaw (array, size) - LowerQuartileRaw (array, size);
}

//============================================================================//
//      Range                                                                 //
//============================================================================//
template <typename type_t>
type_t Range (const type_t array[], size_t size) {
	using namespace std;
	if (size == 0)
		return NAN;
	auto bounds = minmax_element (array, array + size);
	return *bounds.second - *bounds.first;
}

//============================================================================//
//      Variance                                                              //
//============================================================================//
template <typename type_t>
mpf_class Variance (const type_t array[], size_t size, type_t value) {
	mpf_class total_sum (0);
	for (size_t i = 0; i < size; ++i) {
		mpf_class val (array[i] - value);
		total_sum += val * val;
	}
	return total_sum;
}

// Population variance
template <typename type_t>
type_t PopulationVariance (const type_t array[], size_t size, type_t value) {
	if (size == 0)
		return NAN;
	mpf_class temp = mpf_class (size);
	mpf_class res = Variance (array, size, value) / temp;
	return res.get_d();
}

// Sample variance
template <typename type_t>
type_t SampleVariance (const type_t array[], size_t size, type_t value) {
	if (size <= 1)
		return NAN;
	mpf_class temp = mpf_class (size - 1);
	mpf_class res = Variance (array, size, value) / temp;
	return res.get_d();
}

//============================================================================//
//      Standard error                                                        //
//============================================================================//

// Population standard error
template <typename type_t>
type_t PopulationStdErr (const type_t array[], size_t size, type_t value) {
	if (size == 0)
		return NAN;
	mpf_class temp = mpf_class (size) * mpf_class (size);
	mpf_class var = Variance (array, size, value) / temp;
	mpf_class root = sqrt (var);
	return root.get_d();
}

// Sample standard error
template <typename type_t>
type_t SampleStdErr (const type_t array[], size_t size, type_t value) {
	if (size <= 1)
		return NAN;
	mpf_class temp = mpf_class (size) * mpf_class (size - 1);
	mpf_class var = Variance (array, size, value) / temp;
	mpf_class root = sqrt (var);
	return root.get_d();
}

//============================================================================//
//      Standard deviation                                                    //
//============================================================================//

// Population standard deviation
template <typename type_t>
type_t PopulationStdDev (const type_t array[], size_t size, type_t value) {
	if (size == 0)
		return NAN;
	mpf_class temp = mpf_class (size);
	mpf_class var = Variance (array, size, value) / temp;
	mpf_class root = sqrt (var);
	return root.get_d();
}

// Sample standard deviation
template <typename type_t>
type_t SampleStdDev (const type_t array[], size_t size, type_t value) {
	if (size <= 1)
		return NAN;
	mpf_class temp = mpf_class (size - 1);
	mpf_class var = Variance (array, size, value) / temp;
	mpf_class root = sqrt (var);
	return root.get_d();
}

//============================================================================//
//      Absolute deviation                                                    //
//============================================================================//
template <typename type_t>
mpf_class AbsDev (const type_t array[], size_t size, type_t value) {
	mpf_class total_sum (0);
	for (size_t i = 0; i < size; ++i) {
		type_t temp = abs (array[i] - value);
		mpf_class val (temp);
		total_sum += val;
	}
	return total_sum;
}

// Population absolute deviation
template <typename type_t>
type_t PopulationAbsDev (const type_t array[], size_t size, type_t value) {
	if (size == 0)
		return NAN;
	mpf_class temp = mpf_class (size);
	mpf_class res = AbsDev (array, size, value) / temp;
	return res.get_d();
}

// Sample absolute deviation
template <typename type_t>
type_t SampleAbsDev (const type_t array[], size_t size, type_t value) {
	if (size <= 1)
		return NAN;
	mpf_class temp = mpf_class (size - 1);
	mpf_class res = AbsDev (array, size, value) / temp;
	return res.get_d();
}

//============================================================================//
//      Signed deviation                                                      //
//============================================================================//
template <typename type_t>
mpf_class SignDev (const type_t array[], size_t size, type_t value) {
	mpf_class total_sum (0);
	for (size_t i = 0; i < size; ++i) {
		type_t temp = array[i] - value;
		mpf_class val (temp);
		total_sum += val;
	}
	return total_sum;
}

// Population signed deviation
template <typename type_t>
type_t PopulationSignDev (const type_t array[], size_t size, type_t value) {
	if (size == 0)
		return NAN;
	mpf_class temp = mpf_class (size);
	mpf_class res = SignDev (array, size, value) / size;
	return res.get_d();
}

// Sample signed deviation
template <typename type_t>
type_t SampleSignDev (const type_t array[], size_t size, type_t value) {
	if (size <= 1)
		return NAN;
	mpf_class temp = mpf_class (size - 1);
	mpf_class res = SignDev (array, size, value) / (size - 1);
	return res.get_d();
}

//============================================================================//
//      Median of squared deviations                                          //
//============================================================================//
template <typename type_t>
type_t MedianSqrDev (type_t array[], size_t size, type_t value) {
	if (size == 0)
		return NAN;
	for (size_t i = 0; i < size; ++i) {
		type_t temp = array[i] - value;
		array[i] = temp * temp;
	}
	return MedianRaw (array, size);
}

//============================================================================//
//      Median of absolute deviations                                         //
//============================================================================//
template <typename type_t>
type_t MedianAbsDev (type_t array[], size_t size, type_t value) {
	if (size == 0)
		return NAN;
	for (size_t i = 0; i < size; ++i) {
		type_t temp = array[i] - value;
		array[i] = fabs (temp);
	}
	return MedianRaw (array, size);
}

//============================================================================//
//      Median of signed deviations                                           //
//============================================================================//
template <typename type_t>
type_t MedianSignDev (type_t array[], size_t size, type_t value) {
	if (size == 0)
		return NAN;
	for (size_t i = 0; i < size; ++i) {
		type_t temp = array[i] - value;
		array[i] = temp;
	}
	return MedianRaw (array, size);
}

//============================================================================//
//      Quartile skewness                                                     //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ranked array                                                          //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
template <typename type_t>
double QuartileSkewness (const type_t array[], size_t size, type_t median) {
	type_t lower = LowerQuartile (array, size);
	type_t upper = UpperQuartile (array, size);
	double range = (size_t) upper - (size_t) lower;
	sint64_t temp1 = upper + lower;
	sint64_t temp2 = 2 * median;
	return range ? (temp1 - temp2) / range : NAN;
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	SQSKEWNESS_FLT(type_t)												\
type_t QuartileSkewness (const type_t array[], size_t size, type_t median) {	\
	type_t lower = LowerQuartile (array, size);									\
	type_t upper = UpperQuartile (array, size);									\
	type_t range = upper - lower;												\
	return range ? (upper + lower - 2 * median) / range : NAN;					\
}
SQSKEWNESS_FLT (flt32_t)
SQSKEWNESS_FLT (flt64_t)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Unordered array                                                       //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
template <typename type_t>
double QuartileSkewnessRaw (type_t array[], size_t size, type_t median) {
	type_t lower = LowerQuartileRaw (array, size);
	type_t upper = UpperQuartileRaw (array, size);
	double range = (size_t) upper - (size_t) lower;
	sint64_t temp1 = upper + lower;
	sint64_t temp2 = 2 * median;
	return range ? (temp1 - temp2) / range : NAN;
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	RQSKEWNESS_FLT(type_t)												\
type_t QuartileSkewnessRaw (type_t array[], size_t size, type_t median) {		\
	type_t lower = LowerQuartileRaw (array, size);								\
	type_t upper = UpperQuartileRaw (array, size);								\
	type_t range = upper - lower;												\
	return range ? (upper + lower - 2 * median) / range : NAN;					\
}
RQSKEWNESS_FLT (flt32_t)
RQSKEWNESS_FLT (flt64_t)

//============================================================================//
//      Meeden's skewness                                                     //
//============================================================================//

// Population Meeden's skewness
template <typename type_t>
type_t PopulationMeedenSkewness (const type_t array[], size_t size, type_t mean, type_t median) {
	if (size == 0)
		return NAN;
	mpf_class var = AbsDev (array, size, median);
	if (var != 0) {
		mpf_class temp = mpf_class (size);
		mpf_class delta = (mpf_class (mean) - mpf_class (median)) * temp;
		mpf_class res = delta / var;
		return res.get_d();
	}
	return NAN;
}

// Sample Meeden's skewness
template <typename type_t>
type_t SampleMeedenSkewness (const type_t array[], size_t size, type_t mean, type_t median) {
	if (size <= 1)
		return NAN;
	mpf_class var = AbsDev (array, size, median);
	if (var != 0) {
		mpf_class temp = mpf_class (size - 1);
		mpf_class delta = (mpf_class (mean) - mpf_class (median)) * temp;
		mpf_class res = delta / var;
		return res.get_d();
	}
	return NAN;
}

//============================================================================//
//      Pearson's skewness                                                    //
//============================================================================//

// Population Pearson's skewness
template <typename type_t>
type_t PopulationPearsonSkewness (const type_t array[], size_t size, type_t mean, type_t median) {
	if (size == 0)
		return NAN;
	mpf_class var = Variance (array, size, mean);
	if (var != 0) {
		mpf_class temp = mpf_class (size);
		mpf_class delta = 3 * (mpf_class (mean) - mpf_class (median)) * sqrt (temp);
		mpf_class res = delta / sqrt (var);
		return res.get_d();
	}
	return NAN;
}

// Sample Pearson's skewness
template <typename type_t>
type_t SamplePearsonSkewness (const type_t array[], size_t size, type_t mean, type_t median) {
	if (size <= 1)
		return NAN;
	mpf_class var = Variance (array, size, mean);
	if (var != 0) {
		mpf_class temp = mpf_class (size - 1);
		mpf_class delta = 3 * (mpf_class (mean) - mpf_class (median)) * sqrt (temp);
		mpf_class res = delta / sqrt (var);
		return res.get_d();
	}
	return NAN;
}

//============================================================================//
//      Skewness                                                              //
//============================================================================//
template <typename type_t>
void Skewness (const type_t array[], size_t size, type_t value, mpf_class &cube, mpf_class &sqr) {
	sqr = 0;
	cube = 0;
	for (size_t i = 0; i < size; ++i) {
		mpf_class val (array[i] - value);
		mpf_class temp = val * val;
		sqr += temp;
		cube += temp * val;
	}
}

// Population skewness
template <typename type_t>
type_t PopulationSkewness (const type_t array[], size_t size, type_t value) {
	if (size == 0)
		return NAN;
	mpf_class cube, sqr;
	Skewness (array, size, value, cube, sqr);
	if (sqr != 0) {
		mpf_class n (size);
		mpf_class coeff = sqrt (n);
		mpf_class p = coeff * cube;
		mpf_class q = sqr * sqrt (sqr);
		mpf_class res = p / q;
		return res.get_d();
	}
	return NAN;
}

// Sample skewness
template <typename type_t>
type_t SampleSkewness (const type_t array[], size_t size, type_t value) {
	if (size <= 2)
		return NAN;
	mpf_class cube, sqr;
	Skewness (array, size, value, cube, sqr);
	if (sqr != 0) {
		mpf_class n (size - 1);
		mpf_class coeff = sqrt (n) + 2 * sqrt (n) / (n - 1);
		mpf_class p = coeff * cube;
		mpf_class q = sqr * sqrt (sqr);
		mpf_class res = p / q;
		return res.get_d();
	}
	return NAN;
}

//============================================================================//
//      Kurtosis                                                              //
//============================================================================//
template <typename type_t>
void Kurtosis (const type_t array[], size_t size, type_t value, mpf_class &quad, mpf_class &sqr) {
	sqr = 0;
	quad = 0;
	for (size_t i = 0; i < size; ++i) {
		mpf_class val (array[i] - value);
		mpf_class temp = val * val;
		sqr += temp;
		quad += temp * temp;
	}
}

// Population kurtosis
template <typename type_t>
type_t PopulationKurtosis (const type_t array[], size_t size, type_t value) {
	if (size == 0)
		return NAN;
	mpf_class quad, sqr;
	Kurtosis (array, size, value, quad, sqr);
	if (sqr != 0) {
		mpf_class coeff (size);
		mpf_class res = coeff * quad / (sqr * sqr);
		return res.get_d();
	}
	return NAN;
}

// Sample kurtosis
template <typename type_t>
type_t SampleKurtosis (const type_t array[], size_t size, type_t value) {
	if (size <= 3)
		return NAN;
	mpf_class quad, sqr;
	Kurtosis (array, size, value, quad, sqr);
	if (sqr != 0) {
		mpf_class n (size);
		mpf_class coeff = n * (n - 1) * (n + 1);
		mpf_class correction = 15 - 9 * n;
		mpf_class num_main = coeff * quad / (sqr * sqr);
		mpf_class res = (num_main + correction) / ((n - 2) * (n - 3));
		return res.get_d();
	}
	return NAN;
}

//============================================================================//
//      Excess kurtosis                                                       //
//============================================================================//

// Population excess kurtosis
template <typename type_t>
type_t PopulationExcessKurtosis (const type_t array[], size_t size, type_t value) {
	if (size == 0)
		return NAN;
	mpf_class quad, sqr;
	Kurtosis (array, size, value, quad, sqr);
	if (sqr != 0) {
		mpf_class coeff (size);
		mpf_class res = coeff * quad / (sqr * sqr) - 3;
		return res.get_d();
	}
	return NAN;
}

// Sample excess kurtosis
template <typename type_t>
type_t SampleExcessKurtosis (const type_t array[], size_t size, type_t value) {
	if (size <= 3)
		return NAN;
	mpf_class quad, sqr;
	Kurtosis (array, size, value, quad, sqr);
	if (sqr != 0) {
		mpf_class n (size);
		mpf_class coeff = n * (n - 1) * (n + 1);
		mpf_class correction = 3 * (n - 1) * (n - 1);
		mpf_class num_main = coeff * quad / (sqr * sqr);
		mpf_class res = (num_main - correction) / ((n - 2) * (n - 3));
		return res.get_d();
	}
	return NAN;
}

//============================================================================//
//      Covariance                                                            //
//============================================================================//
template <typename type_t>
mpf_class Covariance (const type_t array1[], const type_t array2[], size_t size, type_t value1, type_t value2) {
	mpf_class total_sum (0);
	for (size_t i = 0; i < size; ++i) {
		mpf_class val1 (array1[i] - value1);
		mpf_class val2 (array2[i] - value2);
		total_sum += val1 * val2;
	}
	return total_sum;
}

// Population covariance
template <typename type_t>
type_t PopulationCovariance (const type_t array1[], const type_t array2[], size_t size, type_t value1, type_t value2) {
	if (size == 0)
		return NAN;
	mpf_class temp = mpf_class (size);
	mpf_class res = Covariance (array1, array2, size, value1, value2) / temp;
	return res.get_d();
}

// Sample covariance
template <typename type_t>
type_t SampleCovariance (const type_t array1[], const type_t array2[], size_t size, type_t value1, type_t value2) {
	if (size <= 1)
		return NAN;
	mpf_class temp = mpf_class (size - 1);
	mpf_class res = Covariance (array1, array2, size, value1, value2) / temp;
	return res.get_d();
}

//============================================================================//
//      Pearson correlation                                                   //
//============================================================================//
template <typename type_t>
type_t PearsonCorrelation (const type_t array1[], const type_t array2[], size_t size, type_t value1, type_t value2) {
	mpf_class covar_sum (0);
	mpf_class var1_sum (0);
	mpf_class var2_sum (0);
	for (size_t i = 0; i < size; ++i) {
		mpf_class val1 (array1[i] - value1);
		mpf_class val2 (array2[i] - value2);
		covar_sum += val1 * val2;
		var1_sum += val1 * val1;
		var2_sum += val2 * val2;
	}
	mpf_class mul = var1_sum * var2_sum;
	mpf_class temp = sqrt (mul);
	mpf_class res = covar_sum / temp;
	return res.get_d ();
}

//============================================================================//
//      Fechner correlation                                                   //
//============================================================================//
template <typename type_t>
static type_t FechnerCorrelation (const type_t array1[], const type_t array2[], size_t size, type_t mean1, type_t mean2)
{
	if (size == 0)
		return NAN;
	size_t psum = 0;
	size_t nsum = 0;
	for (size_t i = 0; i < size; i++) {
		type_t temp1 = array1[i] - mean1;
		type_t temp2 = array2[i] - mean2;
		if ((temp1 > 0 && temp2 > 0) || (temp1 < 0 && temp2 < 0))
			psum++;
		if ((temp1 > 0 && temp2 < 0) || (temp1 < 0 && temp2 > 0))
			nsum++;
	}
	sint64_t diff = psum - nsum;
	size_t total = psum + nsum;
	if (total)
		return type_t (diff) / total;
	else
		return NAN;
}

//============================================================================//
//      Spearman correlation                                                  //
//============================================================================//
template <typename type_t>
type_t SpearmanCorrelation (type_t array1[], type_t array2[], size_t size) {
	if (size < 1)
		return NAN;
	struct SortItem {
		type_t value;
		size_t original_index;
	};
	vector <SortItem> buffer(size);
	auto transform_to_ranks_inplace = [&](type_t* data) {
		for (size_t i = 0; i < size; ++i)
			buffer[i] = {data[i], i};
		sort (buffer.begin(), buffer.end(), [](const SortItem &a, const SortItem &b) {
			return a.value < b.value;
		});
		for (size_t i = 0; i < size;) {
			type_t val = buffer[i].value;
			size_t j = i + 1;
			while (j < size && buffer[j].value == val) j++;
			type_t mean = static_cast <type_t> (0.5) * (i + j - 1);
			for (size_t k = i; k < j; ++k)
				data[buffer[k].original_index] = mean;
			i = j;
		}
	};
	transform_to_ranks_inplace (array1);
	transform_to_ranks_inplace (array2);
	type_t mean = static_cast <type_t> (0.5) * (size - 1);
	return PearsonCorrelation (array1, array2, size, mean, mean);
}

//****************************************************************************//
//      Testing functions                                                     //
//****************************************************************************//

//============================================================================//
//      Quantiles                                                             //
//============================================================================//
template <typename type_t>
void Quantile (
	RandomArray <type_t> &array,
	size_t offset,
	size_t count,
	type_t value
){
	array.Sort (false);
	double fraq = value / static_cast <double> (MAX_VALUE);
	RandomArray <type_t> reference (array);
	if constexpr (is_integral_v <type_t>) {
		type_t computed_value = Statistics::Quantile (array.Data() + offset, count, fraq);
		type_t correct_value = Quantile <type_t> (reference.Data() + offset, count, fraq);
		CheckResult (computed_value, correct_value);
	} else {
		type_t computed_value = Statistics::Quantile (array.Data() + offset, count, static_cast <type_t> (fraq));
		type_t correct_value = Quantile (reference.Data() + offset, count, static_cast <type_t> (fraq));
		CheckResult (computed_value, correct_value);
	}
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template <typename type_t>
void QuantileRaw (
	RandomArray <type_t> &array,
	size_t offset,
	size_t count,
	type_t value
){
	double fraq = value / static_cast <double> (MAX_VALUE);
	RandomArray <type_t> reference (array);
	if constexpr (is_integral_v <type_t>) {
		type_t computed_value = Statistics::QuantileRaw (array.Data() + offset, count, fraq);
		type_t correct_value = QuantileRaw <type_t> (reference.Data() + offset, count, fraq);
		CheckResult (computed_value, correct_value);
	} else {
		type_t computed_value = Statistics::QuantileRaw (array.Data() + offset, count, static_cast <type_t> (fraq));
		type_t correct_value = QuantileRaw (reference.Data() + offset, count, static_cast <type_t> (fraq));
		CheckResult (computed_value, correct_value);
	}
}

//============================================================================//
//      Percentiles                                                           //
//============================================================================//
template <typename type_t>
void Percentile (
	RandomArray <type_t> &array,
	size_t offset,
	size_t count,
	type_t value
){
	array.Sort (false);
	size_t percentile = 100 * value / static_cast <double> (MAX_VALUE);
	RandomArray <type_t> reference (array);
	type_t computed_value = Statistics::Percentile (array.Data() + offset, count, percentile);
	type_t correct_value = Percentile <type_t> (reference.Data() + offset, count, percentile);
	CheckResult (computed_value, correct_value);
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template <typename type_t>
void PercentileRaw (
	RandomArray <type_t> &array,
	size_t offset,
	size_t count,
	type_t value
){
	size_t percentile = 100 * value / static_cast <double> (MAX_VALUE);
	RandomArray <type_t> reference (array);
	type_t computed_value = Statistics::PercentileRaw (array.Data() + offset, count, percentile);
	type_t correct_value = PercentileRaw <type_t> (reference.Data() + offset, count, percentile);
	CheckResult (computed_value, correct_value);
}

//============================================================================//
//      Deciles                                                               //
//============================================================================//
template <typename type_t>
void Decile (
	RandomArray <type_t> &array,
	size_t offset,
	size_t count,
	type_t value
){
	array.Sort (false);
	size_t decile = 10 * value / static_cast <double> (MAX_VALUE);
	RandomArray <type_t> reference (array);
	type_t computed_value = Statistics::Decile (array.Data() + offset, count, decile);
	type_t correct_value = Decile <type_t> (reference.Data() + offset, count, decile);
	CheckResult (computed_value, correct_value);
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template <typename type_t>
void DecileRaw (
	RandomArray <type_t> &array,
	size_t offset,
	size_t count,
	type_t value
){
	size_t decile = 10 * value / static_cast <double> (MAX_VALUE);
	RandomArray <type_t> reference (array);
	type_t computed_value = Statistics::DecileRaw (array.Data() + offset, count, decile);
	type_t correct_value = DecileRaw <type_t> (reference.Data() + offset, count, decile);
	CheckResult (computed_value, correct_value);
}

//============================================================================//
//      Quartile skewness                                                     //
//============================================================================//
template <typename type_t>
void QuartileSkewness (
	RandomArray <type_t> &array,
	size_t offset,
	size_t count,
	type_t value
){
	array.Sort (false);
	RandomArray <type_t> reference (array);
	if constexpr (is_integral_v <type_t>) {
		double computed_value = Statistics::QuartileSkewness (array.Data() + offset, count, value);
		double correct_value = QuartileSkewness <type_t> (reference.Data() + offset, count, value);
		CheckResult (computed_value, correct_value);
	} else {
		type_t computed_value = Statistics::QuartileSkewness (array.Data() + offset, count, value);
		type_t correct_value = QuartileSkewness (reference.Data() + offset, count, value);
		CheckResult (computed_value, correct_value);
	}
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template <typename type_t>
void QuartileSkewnessRaw (
	RandomArray <type_t> &array,
	size_t offset,
	size_t count,
	type_t value
){
	RandomArray <type_t> reference (array);
	if constexpr (is_integral_v <type_t>) {
		double computed_value = Statistics::QuartileSkewnessRaw (array.Data() + offset, count, value);
		double correct_value = QuartileSkewnessRaw <type_t> (reference.Data() + offset, count, value);
		CheckResult (computed_value, correct_value);
	} else {
		type_t computed_value = Statistics::QuartileSkewnessRaw (array.Data() + offset, count, value);
		type_t correct_value = QuartileSkewnessRaw (reference.Data() + offset, count, value);
		CheckResult (computed_value, correct_value);
	}
}

//============================================================================//
//      Quartiles                                                             //
//============================================================================//
# define	QUART(fname)														\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t count,																\
	type_t value																\
){																				\
	array.Sort (false);															\
	RandomArray <type_t> reference (array);										\
	type_t computed_value = Statistics::fname (array.Data() + offset, count);	\
	type_t correct_value = fname <type_t> (reference.Data() + offset, count);	\
	CheckResult (computed_value, correct_value);								\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	QUARTRAW(fname)														\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t count,																\
	type_t value																\
){																				\
	RandomArray <type_t> reference (array);										\
	type_t computed_value = Statistics::fname (array.Data() + offset, count);	\
	type_t correct_value = fname <type_t> (reference.Data() + offset, count);	\
	CheckResult (computed_value, correct_value);								\
}

//============================================================================//
//      Trimean                                                               //
//============================================================================//
# define	TRIMEAN(fname)														\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t count,																\
	type_t value																\
){																				\
	array.Sort (false);															\
	RandomArray <type_t> reference (array);										\
	type_t computed_value = Statistics::fname (array.Data() + offset, count, value);\
	type_t correct_value = fname <type_t> (reference.Data() + offset, count, value);\
	CheckResult (computed_value, correct_value);								\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	TRIMEANRAW(fname)													\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t count,																\
	type_t value																\
){																				\
	RandomArray <type_t> reference (array);										\
	type_t computed_value = Statistics::fname (array.Data() + offset, count, value);\
	type_t correct_value = fname <type_t> (reference.Data() + offset, count, value);\
	CheckResult (computed_value, correct_value);								\
}

//============================================================================//
//      Sums                                                                  //
//============================================================================//
# define	SUM1(fname)															\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t count,																\
	type_t value																\
){																				\
	RandomArray <type_t> reference (array);										\
	type_t computed_value = Statistics::fname (array.Data() + offset, count);	\
	type_t correct_value = fname (reference.Data() + offset, count);			\
	CheckResult (computed_value, correct_value);								\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	SUM2(fname)															\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t count,																\
	type_t value																\
){																				\
	RandomArray <type_t> reference (array);										\
	type_t computed_value = Statistics::fname (array.Data() + offset, count, value);\
	type_t correct_value = fname (reference.Data() + offset, count, value);		\
	CheckResult (computed_value, correct_value);								\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	SUM3(fname)															\
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
	type_t computed_value = Statistics::fname (target.Data() + toffset, source.Data() + soffset, count, value1, value2);\
	type_t correct_value = fname <type_t> (target.Data() + toffset, source.Data() + soffset, count, value1, value2);\
	CheckResult (computed_value, correct_value);								\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	SUM4(fname)															\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &target,												\
	RandomArray <type_t> &source,												\
	size_t toffset,																\
	size_t soffset,																\
	size_t count																\
){																				\
	RandomArray <type_t> tref (target);											\
	RandomArray <type_t> sref (source);											\
	RandomArray <type_t> temp (SIZE, SEED, MAX_VALUE);							\
	RandomArray <size_t> ranks (SIZE, SEED, MAX_VALUE);							\
	type_t computed_value = Statistics::fname (target.Data() + toffset, source.Data() + soffset, temp.Data(), ranks.Data(), count);\
	type_t correct_value = fname <type_t> (tref.Data() + toffset, sref.Data() + soffset, count);\
	CheckResult (computed_value, correct_value);								\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	SUM5(fname)															\
template <typename type_t>														\
void fname (																	\
	RandomArray <type_t> &array,												\
	size_t offset,																\
	size_t count,																\
	type_t value																\
){																				\
	type_t median = array.RandomValue();										\
	RandomArray <type_t> reference (array);										\
	type_t computed_value = Statistics::fname (array.Data() + offset, count, value, median);\
	type_t correct_value = fname (reference.Data() + offset, count, value, median);\
	CheckResult (computed_value, correct_value);								\
}

//============================================================================//
//      Scalar testing functions                                              //
//============================================================================//
# define	USCALAR(fname)														\
void Test##fname (void) {														\
	PrintCaption ("Statistics::"#fname);										\
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
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	USCALAR_FLT(fname)													\
void Test##fname (void) {														\
	PrintCaption ("Statistics::"#fname);										\
	TestArray (fname <flt32_t>);												\
	TestArray (fname <flt64_t>);												\
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# define	UVECTOR_FLT(fname)													\
void Test##fname (void) {														\
	PrintCaption ("Statistics::"#fname);										\
	TestArrays (fname <flt32_t>);												\
	TestArrays (fname <flt64_t>);												\
}

//============================================================================//
//      Quantiles                                                             //
//============================================================================//
USCALAR (Quantile)
USCALAR (QuantileRaw)

//============================================================================//
//      Percentiles                                                           //
//============================================================================//
USCALAR (Percentile)
USCALAR (PercentileRaw)

//============================================================================//
//      Deciles                                                               //
//============================================================================//
USCALAR (Decile)
USCALAR (DecileRaw)

//============================================================================//
//      Lower quartile                                                        //
//============================================================================//
QUART (LowerQuartile)
QUARTRAW (LowerQuartileRaw)
USCALAR (LowerQuartile)
USCALAR (LowerQuartileRaw)

//============================================================================//
//      Upper quartile                                                        //
//============================================================================//
QUART (UpperQuartile)
QUARTRAW (UpperQuartileRaw)
USCALAR (UpperQuartile)
USCALAR (UpperQuartileRaw)

//============================================================================//
//      Median                                                                //
//============================================================================//
QUART (Median)
QUARTRAW (MedianRaw)
USCALAR (Median)
USCALAR (MedianRaw)

//============================================================================//
//      Midhinge                                                              //
//============================================================================//
QUART (Midhinge)
QUARTRAW (MidhingeRaw)
USCALAR (Midhinge)
USCALAR (MidhingeRaw)

//============================================================================//
//      Tukey's trimean                                                       //
//============================================================================//
TRIMEAN (Trimean)
TRIMEANRAW (TrimeanRaw)
USCALAR (Trimean)
USCALAR (TrimeanRaw)

//============================================================================//
//      Mid-range                                                             //
//============================================================================//
SUM1 (MidRange)
USCALAR_FLT (MidRange)

//============================================================================//
//      Mean                                                                  //
//============================================================================//
SUM1 (Mean)
USCALAR_FLT (Mean)

//============================================================================//
//      Interquartile range                                                   //
//============================================================================//
QUART (InterQuartileRange)
QUARTRAW (InterQuartileRangeRaw)
USCALAR (InterQuartileRange)
USCALAR (InterQuartileRangeRaw)

//============================================================================//
//      Range                                                                 //
//============================================================================//
SUM1 (Range)
USCALAR_FLT (Range)

//============================================================================//
//      Variance                                                              //
//============================================================================//

// Population variance
SUM2 (PopulationVariance)
USCALAR_FLT (PopulationVariance)

// Sample variance
SUM2 (SampleVariance)
USCALAR_FLT (SampleVariance)

//============================================================================//
//      Standard error                                                        //
//============================================================================//

// Population standard error
SUM2 (PopulationStdErr)
USCALAR_FLT (PopulationStdErr)

// Sample standard error
SUM2 (SampleStdErr)
USCALAR_FLT (SampleStdErr)

//============================================================================//
//      Standard deviation                                                    //
//============================================================================//

// Population standard deviation
SUM2 (PopulationStdDev)
USCALAR_FLT (PopulationStdDev)

// Sample standard deviation
SUM2 (SampleStdDev)
USCALAR_FLT (SampleStdDev)

//============================================================================//
//      Absolute deviation                                                    //
//============================================================================//

// Population absolute deviation
SUM2 (PopulationAbsDev)
USCALAR_FLT (PopulationAbsDev)

// Sample absolute deviation
SUM2 (SampleAbsDev)
USCALAR_FLT (SampleAbsDev)

//============================================================================//
//      Signed deviation                                                      //
//============================================================================//

// Population signed deviation
SUM2 (PopulationSignDev)
USCALAR_FLT (PopulationSignDev)

// Sample signed deviation
SUM2 (SampleSignDev)
USCALAR_FLT (SampleSignDev)

//============================================================================//
//      Median of squared deviations                                          //
//============================================================================//
SUM2 (MedianSqrDev)
USCALAR_FLT (MedianSqrDev)

//============================================================================//
//      Median of absolute deviations                                         //
//============================================================================//
SUM2 (MedianAbsDev)
USCALAR_FLT (MedianAbsDev)

//============================================================================//
//      Median of signed deviations                                           //
//============================================================================//
SUM2 (MedianSignDev)
USCALAR_FLT (MedianSignDev)

//============================================================================//
//      Quartile skewness                                                     //
//============================================================================//
USCALAR (QuartileSkewness)
USCALAR (QuartileSkewnessRaw)

//============================================================================//
//      Meeden's skewness                                                     //
//============================================================================//

// Population Meeden's skewness
SUM5 (PopulationMeedenSkewness)
USCALAR_FLT (PopulationMeedenSkewness)

// Sample Meeden's skewness
SUM5 (SampleMeedenSkewness)
USCALAR_FLT (SampleMeedenSkewness)

//============================================================================//
//      Pearson's skewness                                                    //
//============================================================================//

// Population Pearson's skewness
SUM5 (PopulationPearsonSkewness)
USCALAR_FLT (PopulationPearsonSkewness)

// Sample Pearson's skewness
SUM5 (SamplePearsonSkewness)
USCALAR_FLT (SamplePearsonSkewness)

//============================================================================//
//      Skewness                                                              //
//============================================================================//

// Population skewness
SUM2 (PopulationSkewness)
USCALAR_FLT (PopulationSkewness)

// Sample skewness
SUM2 (SampleSkewness)
USCALAR_FLT (SampleSkewness)

//============================================================================//
//      Kurtosis                                                              //
//============================================================================//

// Population kurtosis
SUM2 (PopulationKurtosis)
USCALAR_FLT (PopulationKurtosis)

// Sample kurtosis
SUM2 (SampleKurtosis)
USCALAR_FLT (SampleKurtosis)

//============================================================================//
//      Excess kurtosis                                                       //
//============================================================================//

// Population excess kurtosis
SUM2 (PopulationExcessKurtosis)
USCALAR_FLT (PopulationExcessKurtosis)

// Sample excess kurtosis
SUM2 (SampleExcessKurtosis)
USCALAR_FLT (SampleExcessKurtosis)

//============================================================================//
//      Covariance                                                            //
//============================================================================//

// Population covariance
SUM3 (PopulationCovariance)
UVECTOR_FLT (PopulationCovariance)

// Sample covariance
SUM3 (SampleCovariance)
UVECTOR_FLT (SampleCovariance)

//============================================================================//
//      Pearson correlation                                                   //
//============================================================================//
SUM3 (PearsonCorrelation)
UVECTOR_FLT (PearsonCorrelation)

//============================================================================//
//      Fechner correlation                                                   //
//============================================================================//
SUM3 (FechnerCorrelation)
UVECTOR_FLT (FechnerCorrelation)

//============================================================================//
//      Spearman correlation                                                  //
//============================================================================//
SUM4 (SpearmanCorrelation)
UVECTOR_FLT (SpearmanCorrelation)

//****************************************************************************//
//      Main function                                                         //
//****************************************************************************//
int main (void)
try {

	// Print program header
	PrintHeader ("Statistics library test suite");
	cout << "This test operates with " << SIZE << "-long arrays in " << ROUNDS << " rounds with " << TRIES << " tries in each." << endl;

	// Set the precision of GMP operations
	mpf_set_default_prec (256);

	// Quantiles
	TestQuantile();
	TestQuantileRaw();

	// Percentiles
	TestPercentile();
	TestPercentileRaw();

	// Deciles
	TestDecile();
	TestDecileRaw();

	// Lower quartile
	TestLowerQuartile();
	TestLowerQuartileRaw();

	// Upper quartile
	TestUpperQuartile();
	TestUpperQuartileRaw();

	// Median
	TestMedian();
	TestMedianRaw();

	// Midhinge
	TestMidhinge();
	TestMidhingeRaw();

	// Tukey's trimean
	TestTrimean();
	TestTrimeanRaw();

	// Mid-range
	TestMidRange();

	// Mean
	TestMean();

	// Interquartile range
	TestInterQuartileRange();
	TestInterQuartileRangeRaw();

	// Range
	TestRange();

	// Variance
	TestPopulationVariance();
	TestSampleVariance();

	// Standard error
	TestPopulationStdErr();
	TestSampleStdErr();

	// Standard deviation
	TestPopulationStdDev();
	TestSampleStdDev();

	// Absolute deviation
	TestPopulationAbsDev();
	TestSampleAbsDev();

	// Signed deviation
	TestPopulationSignDev();
	TestSampleSignDev();

	// Median of squared deviations
	TestMedianSqrDev();

	// Median of absolute deviations
	TestMedianAbsDev();

	// Median of signed deviations
	TestMedianSignDev();

	// Quartile skewness
	TestQuartileSkewness();
	TestQuartileSkewnessRaw();

	// Meeden's skewness
	TestPopulationMeedenSkewness();
	TestSampleMeedenSkewness();

	// Pearson's skewness
	TestPopulationPearsonSkewness();
	TestSamplePearsonSkewness();

	// Skewness
	TestPopulationSkewness();
	TestSampleSkewness();

	// Kurtosis
	TestPopulationKurtosis();
	TestSampleKurtosis();

	// Excess kurtosis
	TestPopulationExcessKurtosis();
	TestSampleExcessKurtosis();

	// Covariance
	TestPopulationCovariance();
	TestSampleCovariance();

	// Pearson correlation
	TestPearsonCorrelation();

	// Fechner correlation
	TestFechnerCorrelation();

	// Spearman correlation
	TestSpearmanCorrelation();

	// Print success message
	PrintSuccessStatus ("Statistics library");
	return 0;
}
catch (const runtime_error &ex) {

	// Print fail message
	PrintFailStatus ("Statistics library");
	cerr << ex.what() << endl;
	return 1;
}
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
