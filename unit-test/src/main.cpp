#include <fstream>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/ui/text/TestRunner.h>

int main()
{
  CppUnit::Test *suite;                // test suite
  CppUnit::TextUi::TestRunner runner;
  std::ofstream outputFile("result.xml");

  // Specify XML output and inform the test runner of this format (optional)
  CppUnit::XmlOutputter xmlOutputter(&runner.result(), outputFile);

  // get the top-level suite from the registry
  suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

  // add the suite to the runner
  runner.addTest(suite);

  // run the tests
  bool succeeded = runner.run();
  xmlOutputter.write();
  outputFile.close();

  // return 0 if all tests were succeeded, 1 if any test failed
  return succeeded ? 0 : 1;
}
