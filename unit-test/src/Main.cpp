#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

int main()
{
  CppUnit::Test *suite;                // test suite
  CppUnit::TextUi::TestRunner runner;  // runner

  // get the top-level suite from the registry
  suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

  // add the suite to the runner
  runner.addTest(suite);

  // run the tests
  bool succeeded = runner.run();

  // return 0 if all tests were succeeded, 1 if any test failed
  return succeeded ? 0 : 1;
}
