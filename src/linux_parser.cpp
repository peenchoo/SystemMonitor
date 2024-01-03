#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <numeric>
#include <algorithm>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() 
{ 
  std::ifstream stream(kProcDirectory + kMeminfoFilename);

  string line;
  float memTotal = 0.0f;
  float memFree = 0.0f;

  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      if (line.find("MemTotal:") != std::string::npos) 
      {
        memTotal = std::stol(line.substr(line.find(":") + 1));
      } 
      else if (line.find("MemFree:") != std::string::npos) 
      {
        memFree = std::stol(line.substr(line.find(":") + 1));
      }
    }
  }

  return memTotal == 0.0f ? 0.0f : ((memTotal-memFree)/memTotal);
  
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime()
{
  long uptimeInSeconds = 0; 
  
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    filestream >> uptimeInSeconds;
  }

  return uptimeInSeconds;
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return 0; }

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) { 
  long totalTime = 0;

  std::stringstream path;
  path << kProcDirectory << pid << kStatFilename;
    
  std::ifstream filestream(path.str());
  if (filestream.is_open()) 
  {
    const int position = 13;
    std::string uselessValue;
    for (int i = 0; i < position; i++) 
    {
      if (!(filestream >> uselessValue)) 
      {
        return 10000;
      }
    }

    long userTimeTicks, kernelTimeTicks = 0, userChildrenTimeTicks = 0, kernelChildrenTimeTicks = 0;
    if (filestream >> userTimeTicks >> kernelTimeTicks >> userChildrenTimeTicks >> kernelChildrenTimeTicks) 
    {
        totalTime = (userTimeTicks + kernelTimeTicks + userChildrenTimeTicks + kernelChildrenTimeTicks);
    }
  }
  
  return totalTime/sysconf(_SC_CLK_TCK);
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() 
{ 
  std::vector<long> cpuJiffies = CpuUtilization();
  
  return std::accumulate(cpuJiffies.begin() + kUser_, cpuJiffies.begin() + kSteal_ + 1, 0);
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies()
{
  return std::accumulate(CpuUtilization().begin() + kIdle_, CpuUtilization().begin() + kIOwait_ + 1, 0);
}

// TODO: Read and return CPU utilization
std::vector<long> LinuxParser::CpuUtilization() 
{ 
    std::vector<long> cpuJiffies;
    std::ifstream filestream(kProcDirectory + kStatFilename);
  
    if (filestream.is_open()) 
    {
        std::string line;
        std::getline(filestream, line);

        if (line.substr(0, 3) == "cpu") 
        {
            std::istringstream linestream(line);
            std::string cpu;
            linestream >> cpu;

            // Utiliza un bucle while para leer los valores
            while (!linestream.eof()) 
            {
                long value;
                linestream >> value;
                cpuJiffies.push_back(value);
            }
        }
    }

    return cpuJiffies;
}

int LinuxParser::FindValueByKey(const std::string &key) 
{
    std::string line, currentKey, value;
    std::ifstream stream(kProcDirectory + kStatFilename);

    if (stream.is_open()) 
    {
        while (std::getline(stream, line)) 
        {
            std::istringstream linestream(line);
            linestream >> currentKey >> value;
            if (currentKey == key) 
            {
                return std::stoi(value);
            }
        }
    }
    
    // Si no se encuentra la clave, devolver 0
    return 0;
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() 
{
  return FindValueByKey("processes");
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses()
{
  return FindValueByKey("procs_running");
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) 
{ 
  std::string command = "No command";

  std::stringstream path;
  path << kProcDirectory << pid << kCmdlineFilename;

  std::ifstream filestream(path.str());

  if (filestream.is_open()) 
  {
    std::string line;
    if (std::getline(filestream, line)) 
    {
      command = line;
    }
  }

  return command;
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
std::string LinuxParser::Ram(int pid) { 
    std::string path = kProcDirectory + std::to_string(pid) + kStatusFilename;

    // Utilizar FindValueByKey con la clave "VmSize:"
    int ramInKilobytes = FindValueByKey(path, "VmSize:");
    int ramInMegabytes = ramInKilobytes / 1000;

    return std::to_string(ramInMegabytes);
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid[[maybe_unused]]) { return 0; }
