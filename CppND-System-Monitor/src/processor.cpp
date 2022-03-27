#include "processor.h"
#include "linux_parser.h"
 
/**
 * Return the aggregate CPU utilization.
 * Reference: https://stackoverflow.com/a/23376195
 */ 
float Processor::Utilization() { 
    v = LinuxParser::CpuUtilization();
    long long idle = std::stoi(v[3]) + std::stoi(v[4]);
    long long non_idle = std::stoi(v[0]) + std::stoi(v[1]) + std::stoi(v[2]) + std::stoi(v[5]) + std::stoi(v[6]) + std::stoi(v[7]);
    long long total = idle + non_idle;
    float precentage = (float)((total - pre_total) - (idle - pre_idle)) / (total - pre_total);

    // store the current values as previous values before new calling 
    pre_idle = idle;
    pre_non_idle = non_idle;
    pre_total = total;
    return precentage;
}
