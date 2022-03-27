#ifndef PROCESSOR_H
#define PROCESSOR_H
#include <vector>
#include <string>

class Processor {
 public:
  float Utilization();  // TODO: See src/processor.cpp

  // Declare any necessary private members
 private:
    std::vector<std::string> v = {};
    long long pre_idle = 0;
    long long pre_non_idle = 0; 
    long long pre_total = 1; // avoid divide by 0
};

#endif