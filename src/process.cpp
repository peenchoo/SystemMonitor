#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(const int pidId) : _pidId(pidId)
{
    _user = LinuxParser::User(_pidId);
    _command = LinuxParser::Command(_pidId);
}

// TODO: Return this process's ID
int Process::Pid() 
{ 
    return _pidId; 
}

// TODO: Return this process's CPU utilization
float Process::CpuUtilization() 
{ 
    const int systemUpTimeSeconds = LinuxParser::UpTime();
    const int totalTimeActiveSeconds = LinuxParser::ActiveJiffies(_pidId);
    const int processUpTimeSeconds = LinuxParser::UpTime(_pidId);
    
    const int totalTimeSiceStartUp = systemUpTimeSeconds - processUpTimeSeconds;

    _cpuUtilization = (1.0*totalTimeActiveSeconds)/totalTimeSiceStartUp;
    return (_cpuUtilization);
}

// TODO: Return the command that generated this process
string Process::Command() 
{
    return _command; 
}

// TODO: Return this process's memory utilization
string Process::Ram() 
{ 
    return LinuxParser::Ram(_pidId);
}

// TODO: Return the user (name) that generated this process
string Process::User() { return string(); }

// TODO: Return the age of this process (in seconds)
long int Process::UpTime() { return 0; }

// TODO: Overload the "less than" comparison operator for Process objects
// REMOVE: [[maybe_unused]] once you define the function
bool Process::operator<(Process const& a) const 
{
    return _cpuUtilization  < a._cpuUtilization;
}