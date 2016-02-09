/*
 * all_tests.cpp
 *
 *  Created on: 09.02.2016
 *      Author: badi
 */
#include <stdio.h>
#include "CppUTest/TestHarness.h"
#include "CppUTest/CommandLineTestRunner.h"


int main(int ac, char** av)
{
    return CommandLineTestRunner::RunAllTests(ac, av);
}
