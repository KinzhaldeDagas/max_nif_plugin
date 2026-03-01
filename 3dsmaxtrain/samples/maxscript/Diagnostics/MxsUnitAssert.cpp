
#include "MxsUnitReporter.h"
#include "MxsUnitAssert.h"
#include <maxscript/maxscript.h>
#include <maxscript/kernel/value.h>
#include <maxscript/foundation/numbers.h>
#include <maxscript/foundation/3dmath.h>
#include <maxscript/foundation/functions.h>

void InitMxsAssert()
{
	
}

// Declare C++ function and register it with MAXScript
#include <maxscript\macros\define_instantiation_functions.h>
	def_visible_primitive(assert_true     , "assert_true"     );
	def_visible_primitive(assert_false    , "assert_false"    );
	def_visible_primitive(assert_equal    , "assert_equal"    );
	def_visible_primitive(assert_not_equal, "assert_not_equal");
	def_visible_primitive(assert_defined  , "assert_defined"  );
	def_visible_primitive(assert_undefined, "assert_undefined");
	def_visible_primitive(assert_float    , "assert_float"    );
	def_visible_primitive(assert_float_equal,  "assert_float_equal");
	def_visible_primitive(assert_string_equal, "assert_string_equal");
	def_visible_primitive(assert_point3_equal, "assert_point3_equal");
	def_visible_primitive(assert_matrix_equal, "assert_matrix_equal");
	def_visible_primitive(assert_point2_equal, "assert_point2_equal");
	def_visible_primitive(assert_point4_equal, "assert_point4_equal");
	def_visible_primitive(assert_box3_equal,   "assert_box3_equal");
	def_visible_primitive(assert_eulerAngles_equal, "assert_eulerAngles_equal");
	def_visible_primitive(assert_quat_equal,   "assert_quat_equal");

	def_visible_primitive(assert_array_equal,     "assert_array_equal" );
	def_visible_primitive(assert_bitarray_equal,  "assert_bitarray_equal" );
	def_visible_primitive(assert_matchpattern,    "assert_matchpattern");

static double DEFAULT_FLOAT_TOLERANCE = 0.001;
void LogFailure( Value* message, Value* expected, Value* actual, bool testing_for_equivalence )
{
	const MCHAR*        messageS    = NULL;
	if (message)
		 messageS = message->to_string();
	
	Value*        file_name   = thread_local(source_file);
	const MCHAR*        filename = _M("No File");
	if (file_name)
		filename = file_name->to_string();
	    
	MAXScript_TLS* _tls = (MAXScript_TLS*)TlsGetValue(thread_locals_index);
	two_typed_value_locals_tls(StringStream* expected_stream, StringStream* got_stream);

	vl.expected_stream = new StringStream(_T(""));
	expected->sprin1(vl.expected_stream);
	const MCHAR* expectedString = vl.expected_stream->to_string();

	vl.got_stream = new StringStream(_T(""));
	actual->sprin1(vl.got_stream);
	const MCHAR* gotString = vl.got_stream->to_string();

	MSTR function_name = _M("<No Stack frame>");
	Value** frame = thread_local(current_frame);
	if ( (frame != NULL) && (frame[2]->_is_function()) )
	{
		MAXScriptFunction* cur_fn = (MAXScriptFunction*)frame[2];
		function_name = cur_fn->name;
	}
	else if ( (frame != NULL) && (is_codeblock(frame[2])) )
	{
		CodeBlock* block = (CodeBlock*)frame[2];
		if (block->name)
		{
			function_name = block->name->to_string();
		}
	}

	unsigned int  line_number = (unsigned int)thread_local(source_line);
	MSTR failure = MxsUnitReporter::CreateMessage(function_name.data(), expectedString, gotString, messageS, filename, line_number, testing_for_equivalence );
	MxsUnitReporter::GetInstance()->LogAssertFailure(failure);
	// Also report to the Scripting listener, which is considered "standard out" in maxscript parlance.
	thread_local(current_stdout)->printf(_T("%s\n"), failure.data());
}

// common code for boolean assert methods
Value* BooleanAssert( Value** arg_list, int count, bool bExpected)
{
	Value* actual = arg_list[0];
	bool bActual = (actual->to_bool() == TRUE);

	Value* expected = bExpected ? &true_value : &false_value;

	Value* message = key_arg_or_default(message, NULL);
	Value* result = &true_value;
	if (bExpected != bActual)
	{
		// report error here
		result = &false_value;
		LogFailure(message, expected, actual, bExpected);
	}
	return result;
}

//! common code for assert equal maxscript methods
Value* EqualAssert(bool bExpected, Value** arg_list,int count)
{
	Value* expected = arg_list[0];
	Value* actual   = arg_list[1];

	Value* message = key_arg_or_default(message, NULL);
	Value* result = &true_value;

	// All this is equivalent to the maxscript form of "value == value"
	BOOL comparible = expected->comparable(actual);
	Value* temp[1]  = {actual};
	bool equal      = expected->eq_vf(temp,1) == &true_value;
	bool same       = (comparible && equal);
	
	if (same != bExpected)
	{
		// report error here
		result = &false_value;
		LogFailure(message, expected, actual, bExpected);
	}
	return result;
}

Value* FloatEqualAssert(bool bExpected, Value** arg_list,int count, double tolerance)
{
	Value* expected = arg_list[0];
	Value* actual   = arg_list[1];

	Value* message = key_arg_or_default(message, NULL);
	Value* result = &true_value;

	double d_expected = expected->to_double();
	double d_actual   = actual->to_double();
	double diff = d_expected - d_actual;
	if (diff < 0) // poor man's absolute value method. :)
	{
		diff = -diff;
	}

	bool good_enough = diff < tolerance;

	if (good_enough != bExpected)
	{
		// report error here
		result = &false_value;
		LogFailure(message, expected, actual, bExpected);
	}
	return result;
}

Value* StringEqualAssert(bool bExpected, Value** arg_list,int count)
{
	Value* expected = arg_list[0];
	Value* actual   = arg_list[1];

	const MCHAR* s_expected = expected->to_string();
	const MCHAR* s_actual   = actual->to_string();

	// The default is to not take case into account when comparing strings
	Value* ignoreCase = key_arg_or_default(ignoreCase, &true_value);
	BOOL ignore_case = ignoreCase->to_bool();
	Value* message = key_arg_or_default(message, NULL);
	
	bool same = false;
	if (ignore_case)
	{
		// case insensitive comparison
		same = _tcsicmp(s_expected, s_actual) == 0;
	} 
	else
	{
		same = _tcscmp(s_expected,s_actual) == 0;
	}

	Value* result = &true_value;
	if (same != bExpected)
	{
		// report error here
		result = &false_value;
		LogFailure(message, expected, actual, bExpected);
	}
	return result;
}

bool ComparePoint4( const Point4& expected, const Point4& actual, double tolerance )
{
	int failures = 0;

	float diff_x = abs(expected.x - actual.x);
	if (diff_x > tolerance)
		failures++;

	float diff_y = abs(expected.y - actual.y);
	if (diff_y > tolerance)
		failures++;

	float diff_z = abs(expected.z - actual.z);
	if (diff_z > tolerance)
		failures++;

	float diff_w = abs(expected.w - actual.w);
	if (diff_w > tolerance)
		failures++;

	bool result = true;
	if (failures > 0)
	{
		result = false;
	}
	return result;
}
static const bool expect_true = true;
static const bool expect_false = false;
Value* assert_true_cf(Value** arg_list, int count)
{
	//--------------------------------------------------------
	// Maxscript usage:
	// assert_true <bool> [message:<string>]
	//--------------------------------------------------------
	check_arg_count_with_keys(assert_true, 1, count);
	Value* result = BooleanAssert(arg_list, count, expect_true);
	return result;
}

Value* assert_false_cf(Value** arg_list, int count)
{
	//--------------------------------------------------------
	// Maxscript usage:
	// assert_false <bool> [message:<string>]
	//--------------------------------------------------------
	check_arg_count_with_keys(assert_false, 1, count);
	Value* result = BooleanAssert(arg_list, count, expect_false);
	return result;
}

Value* assert_equal_cf(Value** arg_list, int count)
{
	//--------------------------------------------------------
	// Maxscript usage:
	// assert_equal expected actual [message:<string>]
	//--------------------------------------------------------
	check_arg_count_with_keys(assert_equal, 2, count);
	Value* result = EqualAssert(expect_true, arg_list,count);
	return result;
}

Value* assert_not_equal_cf(Value** arg_list, int count)
{
	//--------------------------------------------------------
	// Maxscript usage:
	// assert_not_equal expected actual [message:<string>]
	//--------------------------------------------------------
	check_arg_count_with_keys(assert_not_equal, 2, count);
	Value* result = EqualAssert(expect_false, arg_list, count);
	return result;
}

Value* assert_float_cf(Value** arg_list, int count)
{
	//--------------------------------------------------------
	// Maxscript usage:
	// assert_float expected actual [tolerance: delta] [message:<string>]
	//--------------------------------------------------------
	check_arg_count_with_keys(assert_float, 2, count);
	Value* arg = NULL;
	double tolerance  = float_key_arg(tolerance, arg, DEFAULT_FLOAT_TOLERANCE);
	Value* result = FloatEqualAssert(expect_true, arg_list,count, tolerance);
	return result;
}

Value* assert_float_equal_cf(Value** arg_list, int count)
{
	//--------------------------------------------------------
	// Maxscript usage:
	// assert_float_equal expected actual [tolerance: delta] [message:<string>]
	//--------------------------------------------------------
	check_arg_count_with_keys(assert_float_equal, 2, count);
	Value* arg = NULL;
	double tolerance = float_key_arg(tolerance, arg, DEFAULT_FLOAT_TOLERANCE);
	Value* result = FloatEqualAssert(expect_true, arg_list, count, tolerance);
	return result;
}

Value* assert_point3_equal_cf(Value** arg_list, int count)
{
	//--------------------------------------------------------
	// Maxscript usage:
	// assert_point3_equal expected actual [tolerance: delta] [message:<string>]
	//--------------------------------------------------------
	check_arg_count_with_keys(assert_point3_equal, 2, count);

	Point4 expected = Point4(arg_list[0]->to_point3());
	Point4 actual   = Point4(arg_list[1]->to_point3());

	Value* arg = NULL;
	double tolerance  = float_key_arg(tolerance, arg, DEFAULT_FLOAT_TOLERANCE);

	bool same = ComparePoint4(expected, actual, tolerance);

	Value* result = &true_value;
	if (!same)
	{
		Value* message = key_arg_or_default(message, NULL);
		LogFailure(message, arg_list[0], arg_list[1], expect_true);
		result = &false_value;
	}
	return result;
}

Value* assert_point4_equal_cf(Value** arg_list, int count)
{
	//--------------------------------------------------------
	// Maxscript usage:
	// assert_point4_equal expected actual [tolerance: delta] [message:<string>]
	//--------------------------------------------------------
	check_arg_count_with_keys(assert_point4_equal, 2, count);

	Point4 expected = arg_list[0]->to_point4();
	Point4 actual = arg_list[1]->to_point4();

	Value* arg = NULL;
	double tolerance = float_key_arg(tolerance, arg, DEFAULT_FLOAT_TOLERANCE);

	bool same = ComparePoint4(expected, actual, tolerance);

	Value* result = &true_value;
	if (!same)
	{
		Value* message = key_arg_or_default(message, NULL);
		LogFailure(message, arg_list[0], arg_list[1], expect_true);
		result = &false_value;
	}
	return result;
}

Value* assert_point2_equal_cf(Value** arg_list, int count)
{
	//--------------------------------------------------------
	// Maxscript usage:
	// assert_point2_equal expected actual [tolerance: delta] [message:<string>]
	//--------------------------------------------------------
	check_arg_count_with_keys(assert_point2_equal, 2, count);

	Point2 expected_p2 = arg_list[0]->to_point2();
	Point2 actual_p2 = arg_list[1]->to_point2();
	Point4 expected (expected_p2.x, expected_p2.y, 0.f, 0.f);
	Point4 actual (actual_p2.x, actual_p2.y, 0.f, 0.f);

	Value* arg = NULL;
	double tolerance = float_key_arg(tolerance, arg, DEFAULT_FLOAT_TOLERANCE);

	bool same = ComparePoint4(expected, actual, tolerance);

	Value* result = &true_value;
	if (!same)
	{
		Value* message = key_arg_or_default(message, NULL);
		LogFailure(message, arg_list[0], arg_list[1], expect_true);
		result = &false_value;
	}
	return result;
}

Value* assert_matrix_equal_cf(Value** arg_list, int count)
{
	//--------------------------------------------------------
	// Maxscript usage:
	// assert_matrix_equal expected actual [tolerance: delta] [message:<string>]
	//--------------------------------------------------------
	check_arg_count_with_keys(assert_matrix_equal, 2, count);

	Matrix3 expected = arg_list[0]->to_matrix3();
	Matrix3 actual   = arg_list[1]->to_matrix3();

	Value* arg = NULL;
	double tolerance  = float_key_arg(tolerance, arg, DEFAULT_FLOAT_TOLERANCE);

	int failures = 0;

	bool same = ComparePoint4(expected.GetRow(0), actual.GetRow(0), tolerance);
	if (!same)
		failures++;
	same = ComparePoint4(expected.GetRow(1), actual.GetRow(1), tolerance);
	if (!same)
		failures++;
	same = ComparePoint4(expected.GetRow(2), actual.GetRow(2), tolerance);
	if (!same)
		failures++;
	same = ComparePoint4(expected.GetRow(3), actual.GetRow(3), tolerance);
	if (!same)
		failures++;

	Value* result = &true_value;
	if (failures > 0)
	{
		Value* message = key_arg_or_default(message, NULL);
		LogFailure(message, arg_list[0], arg_list[1], expect_true);
		result = &false_value;
	}
	return result;
}

Value* assert_box3_equal_cf(Value** arg_list, int count)
{
	//--------------------------------------------------------
	// Maxscript usage:
	// assert_box3_equal expected actual [tolerance: delta] [message:<string>]
	//--------------------------------------------------------
	check_arg_count_with_keys(assert_box3_equal, 2, count);

	Box3 expected = arg_list[0]->to_box3();
	Box3 actual = arg_list[1]->to_box3();

	Value* arg = NULL;
	double tolerance = float_key_arg(tolerance, arg, DEFAULT_FLOAT_TOLERANCE);

	bool same = ComparePoint4(expected.Min(), actual.Min(), tolerance);
	same &= ComparePoint4(expected.Max(), actual.Max(), tolerance);

	Value* result = &true_value;
	if (!same)
	{
		Value* message = key_arg_or_default(message, NULL);
		LogFailure(message, arg_list[0], arg_list[1], expect_true);
		result = &false_value;
	}
	return result;
}

Value* assert_eulerAngles_equal_cf(Value** arg_list, int count)
{
	//--------------------------------------------------------
	// Maxscript usage:
	// assert_eulerAngles_equal expected actual [tolerance: delta] [message:<string>]
	//--------------------------------------------------------
	check_arg_count_with_keys(assert_eulerAngles_equal, 2, count);
	float* eulerAngles = arg_list[0]->to_eulerangles();
	Point4 expected = Point4(eulerAngles[0], eulerAngles[1], eulerAngles[2], 0.f);
	eulerAngles = arg_list[1]->to_eulerangles();
	Point4 actual = Point4(eulerAngles[0], eulerAngles[1], eulerAngles[2], 0.f);

	Value* arg = NULL;
	double tolerance = float_key_arg(tolerance, arg, DEFAULT_FLOAT_TOLERANCE);

	bool same = ComparePoint4(expected, actual, tolerance);

	Value* result = &true_value;
	if (!same)
	{
		Value* message = key_arg_or_default(message, NULL);
		LogFailure(message, arg_list[0], arg_list[1], expect_true);
		result = &false_value;
	}
	return result;
}

Value* assert_quat_equal_cf(Value** arg_list, int count)
{
	//--------------------------------------------------------
	// Maxscript usage:
	// assert_quat_equal expected actual [tolerance: delta] [message:<string>]
	//--------------------------------------------------------
	check_arg_count_with_keys(assert_quat_equal, 2, count);
	Quat expected = arg_list[0]->to_quat();
	Quat actual = arg_list[1]->to_quat();

	Value* arg = NULL;
	double tolerance = float_key_arg(tolerance, arg, DEFAULT_FLOAT_TOLERANCE);

	// following equivalent to Quat::operator== with specifiable tolerence
	expected.Normalize();
	actual.Normalize();
	Quat diff = expected - actual;

	Point4 _actual(diff.x, diff.y, diff.z, 0.f);
	Point4 _expected(0.f, 0.f, 0.f, 0.0f);
	bool same = ComparePoint4(_actual, _expected, tolerance);

	Value* result = &true_value;
	if (!same)
	{
		Value* message = key_arg_or_default(message, NULL);
		LogFailure(message, arg_list[0], arg_list[1], expect_true);
		result = &false_value;
	}
	return result;
}

Value* assert_string_equal_cf(Value** arg_list, int count)
{
	//--------------------------------------------------------
	// Maxscript usage:
	// assert_string_equal expected actual [ignorecase: val] [message:<string>]
	// NOTE: The default for ignore case is true, since maxscript itself ignores the case for
	// strings. So if you want a case sensitive comparison pass in false for val.
	//--------------------------------------------------------
	check_arg_count_with_keys(assert_equal, 2, count);
	Value* result = StringEqualAssert(expect_true, arg_list,count);
	return result;
}

Value* assert_defined_cf(Value** arg_list, int count)
{
	//--------------------------------------------------------
	// Maxscript usage:
	// assert_defined <val> [message:<string>]
	//--------------------------------------------------------
	check_arg_count_with_keys(assert_defined, 1, count);
	Value* actual   = arg_list[0]->eval();
	Value* message = key_arg_or_default(message, NULL);
	Value* result = &true_value;
	if (actual == &undefined)
	{
		// There is no global 'Defined' Value class, so I have to
		// use a string to stand in for the expected Value.
		MAXScript_TLS* _tls = (MAXScript_TLS*)TlsGetValue(thread_locals_index);
		one_value_local_tls(expected);
		vl.expected = new String(_T("defined"));
		LogFailure(message, vl.expected, actual, expect_true);
		result = &false_value;
	}
	
	return result;
}

Value* assert_undefined_cf(Value** arg_list, int count)
{
	//--------------------------------------------------------
	// Maxscript usage:
	// assert_undefined <val> [message:<string>]
	//--------------------------------------------------------
	check_arg_count_with_keys(assert_undefined, 1, count);
	Value* actual   = arg_list[0]->eval();
	Value* message = key_arg_or_default(message, NULL);
	Value* result = &true_value;
	if (actual != &undefined)
	{
		Value* expected = &undefined;
		LogFailure(message, expected, actual, expect_true);
		result = &false_value;
	}
	return result;
}

Value* assert_array_equal_cf(Value** arg_list, int count)
{
	//--------------------------------------------------------
	// Maxscript usage:
	// assert_array_equal expected_array actual_array [message:<string>] [tolerance:<float>] [ignoreCase:<bool>]
	//--------------------------------------------------------
	check_arg_count_with_keys(assert_array_equal, 2, count);

	type_check(arg_list[0], Array, _T("assert_array_equal"));
	type_check(arg_list[1], Array, _T("assert_array_equal"));
	Array* expected = (Array*)arg_list[0];
	Array* actual = (Array*)arg_list[1];
	Value* message = key_arg_or_default(message, NULL);
	Value* arg = NULL;
	double tolerance = float_key_arg(tolerance, arg, DEFAULT_FLOAT_TOLERANCE);
	// The default is to not take case into account when comparing strings
	Value* ignoreCase = key_arg_or_default(ignoreCase, &true_value);
	BOOL ignore_case = ignoreCase->to_bool();

	Value* result = &true_value;
	if (expected->size != actual->size)
	{
		TSTR errMsg;
		if (message)
			errMsg.printf(_T("%s: array sizes not equal"), message->to_string());
		else
			errMsg.printf(_T("%s"), _T("array sizes not equal"));
		MAXScript_TLS* _tls = (MAXScript_TLS*)TlsGetValue(thread_locals_index);
		three_value_locals_tls(error_message, expected_size, actual_size);
		vl.error_message = new String(errMsg);
		vl.expected_size = Integer::intern(expected->size);
		vl.actual_size = Integer::intern(actual->size);
		LogFailure(vl.error_message, vl.expected_size, vl.actual_size, expect_true);
		result = &false_value;
	}
	else
	{
		MAXScript_TLS* _tls = (MAXScript_TLS*)TlsGetValue(thread_locals_index);
		one_value_local_tls(error_message);
		for (int i = 0; i < expected->size; ++i)
		{
			Value* expected_ele = expected->data[i];
			Value* actual_ele = actual->data[i];
			if (expected_ele->tag != actual_ele->tag)
			{
				TSTR errMsg;
				if (message)
					errMsg.printf(_T("%s: element %d not same class"), message->to_string(), i + 1);
				else
					errMsg.printf(_T("element %d not same class"), i + 1);
				vl.error_message = new String(errMsg);
				LogFailure(vl.error_message, expected_ele->tag, actual_ele->tag, expect_true);
				result = &false_value;
				break;
			}
			if (expected_ele == actual_ele)
			{
				// same Value pointer
			}
			else if (expected_ele->tag == class_tag(Float)) 
			{
				TSTR errMsg;
				if (message)
					errMsg.printf(_T("%s: element %d not equal"), message->to_string(), i + 1);
				else
					errMsg.printf(_T("element %d not not equal"), i + 1);
				vl.error_message = new String(errMsg);
				Value* args[5] = { expected_ele, actual_ele, &keyarg_marker, n_message, vl.error_message};
				result = FloatEqualAssert(expect_true, args, 5, tolerance);
				if (result != &true_value)
					break;
			}
			else if (expected_ele->tag == class_tag(Point2Value) || expected_ele->tag == class_tag(Point3Value) || expected_ele->tag == class_tag(Point4Value))
			{
				TSTR errMsg;
				if (message)
					errMsg.printf(_T("%s: element %d not equal"), message->to_string(), i + 1);
				else
					errMsg.printf(_T("element %d not not equal"), i + 1);
				vl.error_message = new String(errMsg);

				Point4 expectedP4;
				Point4 actualP4;
				if (expected_ele->tag == class_tag(Point2Value))
				{
					Point2 expected_p2 = expected_ele->to_point2();
					Point2 actual_p2 = actual_ele->to_point2();
					expectedP4  = Point4(expected_p2.x, expected_p2.y, 0.f, 0.f);
					actualP4 = Point4(actual_p2.x, actual_p2.y, 0.f, 0.f);
				}
				else if (expected_ele->tag == class_tag(Point3Value))
				{
					expectedP4 = expected_ele->to_point3();
					actualP4 = actual_ele->to_point3();
				}
				else
				{
					expectedP4  = expected_ele->to_point4();
					actualP4 = actual_ele->to_point4();
				}
				bool same = ComparePoint4(expectedP4, actualP4, tolerance);
				if (!same)
				{
					if (message)
						errMsg.printf(_T("%s: element %d not equal"), message->to_string(), i + 1);
					else
						errMsg.printf(_T("element %d not not equal"), i + 1);
					vl.error_message = new String(errMsg);
					LogFailure(vl.error_message, expected_ele, actual_ele, expect_true);
					result = &false_value;
					break;
				}
			}
			else if (expected_ele->tag == class_tag(String))
			{
				const MCHAR* s_expected_ele = expected_ele->to_string();
				const MCHAR* s_actual_ele = actual_ele->to_string();

				bool same = false;
				if (ignore_case)
				{
					// case insensitive comparison
					same = _tcsicmp(s_expected_ele, s_actual_ele) == 0;
				}
				else
				{
					same = _tcscmp(s_expected_ele, s_actual_ele) == 0;
				}

				if (!same)
				{
					TSTR errMsg;
					if (message)
						errMsg.printf(_T("%s: element %d not equal"), message->to_string(), i + 1);
					else
						errMsg.printf(_T("element %d not not equal"), i + 1);
					vl.error_message = new String(errMsg);
					LogFailure(vl.error_message, expected_ele, actual_ele, expect_true);
					result = &false_value;
					break;
				}
			}
			else if (expected_ele->tag == class_tag(Matrix3Value))
			{
				Matrix3 expected = expected_ele->to_matrix3();
				Matrix3 actual = actual_ele->to_matrix3();

				bool same = ComparePoint4(expected.GetRow(0), actual.GetRow(0), tolerance);
				same &= ComparePoint4(expected.GetRow(1), actual.GetRow(1), tolerance);
				same &= ComparePoint4(expected.GetRow(2), actual.GetRow(2), tolerance);
				same &= ComparePoint4(expected.GetRow(3), actual.GetRow(3), tolerance);

				if (!same)
				{
					TSTR errMsg;
					if (message)
						errMsg.printf(_T("%s: element %d not equal"), message->to_string(), i + 1);
					else
						errMsg.printf(_T("element %d not not equal"), i + 1);
					vl.error_message = new String(errMsg);
					LogFailure(vl.error_message, expected_ele, actual_ele, expect_true);
					result = &false_value;
					break;
				}
			}
			else if (expected_ele->tag == class_tag(Box3Value))
			{
				Box3 expected = expected_ele->to_box3();
				Box3 actual = actual_ele->to_box3();

				bool same = ComparePoint4(expected.Min(), actual.Min(), tolerance);
				same &= ComparePoint4(expected.Max(), actual.Max(), tolerance);

				if (!same)
				{
					TSTR errMsg;
					if (message)
						errMsg.printf(_T("%s: element %d not equal"), message->to_string(), i + 1);
					else
						errMsg.printf(_T("element %d not not equal"), i + 1);
					vl.error_message = new String(errMsg);
					LogFailure(vl.error_message, expected_ele, actual_ele, expect_true);
					result = &false_value;
					break;
				}
			}
			else
			{
				TSTR errMsg;
				if (message)
					errMsg.printf(_T("%s: element %d not equal"), message->to_string(), i + 1);
				else
					errMsg.printf(_T("element %d not not equal"), i + 1);
				vl.error_message = new String(errMsg);
				Value* args[5] = { expected_ele, actual_ele, &keyarg_marker, n_message, vl.error_message};
				result = EqualAssert(expect_true, args, 5);
				if (result != &true_value)
					break;
			}
		}
	}
	return result;
}

Value* assert_bitarray_equal_cf(Value** arg_list, int count)
{
	//--------------------------------------------------------
	// Maxscript usage:
	// assert_bitarray_equal expected_bitarray actual_bitarray [message:<string>]
	//--------------------------------------------------------
	check_arg_count_with_keys(assert_bitarray_equal, 2, count);

	type_check(arg_list[0], BitArrayValue, _T("assert_bitarray_equal"));
	type_check(arg_list[1], BitArrayValue, _T("assert_bitarray_equal"));
	Value* message = key_arg_or_default(message, NULL);
	Value* result = &true_value;

	BitArrayValue* expected = (BitArrayValue*)arg_list[0];
	BitArrayValue* actual = (BitArrayValue*)arg_list[1];
	if (actual->eq_vf(arg_list, 1) == &false_value)
	{
		// report error here
		result = &false_value;
		LogFailure(message, expected, actual, true);
	}
	return result;
}

Value* assert_matchpattern_cf(Value** arg_list, int count)
{
	//--------------------------------------------------------
	// Maxscript usage:
	// assert_matchpattern expected actual [message:<string>] [ignoreCase:<bool>]
	//--------------------------------------------------------
	check_arg_count_with_keys(assert_matchpattern, 2, count);
	Value* expected = arg_list[0];
	Value* actual = arg_list[1];
	const MCHAR* s_expected = arg_list[0]->to_string();
	const MCHAR* s_actual = arg_list[1]->to_string();
	// The default is to not take case into account when comparing strings
	Value* ignoreCase = key_arg_or_default(ignoreCase, &true_value);
	BOOL ignore_case = ignoreCase->to_bool();

	// this is to test for keyword arg that was used when assert_matchpattern was a scripted function
	Value* n_ignore_case = Name::intern(_T("ignore_case"));
	Value* ignore_case_v = key_arg_or_default(ignore_case, NULL);
	if (ignore_case_v)
		throw RuntimeError(_T("ignore_case keyword arg specified for assert_matchpattern"));

	Value* message = key_arg_or_default(message, NULL);
	Value* result = &true_value;
	BOOL same = MatchPattern(s_actual, s_expected, ignore_case);
	if (!same) 
	{
		LogFailure(message, expected, actual, expect_true);
		result = &false_value;
	}

	return result;
}
/////////////////