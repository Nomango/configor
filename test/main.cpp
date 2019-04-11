#include "nomango/json.hpp"
#include "test_basic_json.h"
#include "test_parser.h"
#include "test_serializer.h"
#include <cassert>

using namespace nomango;

int main()
{
	try
	{
		test_basic_json();
		test_parser();
		test_serializer();
	}
	catch (const json_exception&)
	{
		// testing must not fail
		assert(false);
	}
	return 0;
}
