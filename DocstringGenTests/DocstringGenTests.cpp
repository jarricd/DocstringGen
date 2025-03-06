#include "pch.h"
#include "CppUnitTest.h"
#include "../DocstringGen/Generator.h"
#include "../DocstringGen/Tools.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace DocstringGenTests
{
	TEST_CLASS(DocstringGenTests)
	{
	public:
		
		TEST_METHOD(test_not_empty_generator_single_function_file)
		{
			std::string response = generator::generate_docstring("tests\\test_with_docstring.py");
			Assert::AreNotEqual((int)response.length(), 0);
		}

		TEST_METHOD(test_not_empty_multiple_functions_file)
		{
			std::string response = generator::generate_docstring("tests\\test_with_multiple_docstrings.py");
			Assert::AreNotEqual((int)response.length(), 0);
		}

		TEST_METHOD(test_numpydoc_validation)
		{

		}
	};
}
 