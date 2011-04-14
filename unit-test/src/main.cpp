// Copyright (c) 2010 Vector Fabrics B.V. All rights reserved.
//
// This file contains proprietary and confidential information of Vector
// Fabrics and all use (including distribution) is subject to the conditions of
// the license agreement between you and Vector Fabrics. Without such a license
// agreement in place, no usage or distribution rights are granted by Vector
// Fabrics.
//
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
