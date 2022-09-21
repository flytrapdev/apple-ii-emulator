#ifndef TEST_HPP
#define TEST_HPP

#include "cpu.hpp"
#include "testmem.hpp"
#include <string>

struct TestSuite {

    CPU* cpu;

    TestSuite(CPU* cpu);

    int run();

};


#endif