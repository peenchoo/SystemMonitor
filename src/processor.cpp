#include "processor.h"
#include "linux_parser.h"

Processor::Processor() :
    _previousIdle(LinuxParser::IdleJiffies()),
    _previousNonIdle(LinuxParser::ActiveJiffies()),
    _previousTotal(_previousIdle + _previousNonIdle) {}

// TODO: Return the aggregate CPU utilization
float Processor::Utilization()
{
    const long currentIdle = LinuxParser::IdleJiffies(); 
    const long currentNonIdle = LinuxParser::ActiveJiffies(); 
    const long currentTotal = currentIdle + currentNonIdle;

    const long deltaIdle = currentIdle - _previousIdle;
    const long deltaTotal = currentTotal - _previousTotal;

    _previousIdle = currentIdle;
    _previousNonIdle = currentNonIdle;
    _previousTotal = currentTotal;
    
    const float cpuUsage = deltaTotal == 0 ? 0.0f : (deltaTotal - deltaIdle) * 1.0 / deltaTotal;
    return cpuUsage;
}