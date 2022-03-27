#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;


// optional: template function 
template <typename T>
T findValueByKey(std::string const &keyFilter, std::string const &filename) {
  std::string line, key;
  T value;

  std::ifstream stream(LinuxParser::kProcDirectory + filename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == keyFilter) {
          return value;
        }
      }
    }
  }
  // is the second return to avoid compile warnings?
  return value;
};

template <typename T>
T getValueOfFile(std::string const &filename) {
  std::string line;
  T value;

  std::ifstream stream(LinuxParser::kProcDirectory + filename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> value;
  }
  return value;
};


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

/**
 * Use https://stackoverflow.com/a/41251290 answer to calculate the memory utilization.
 * utilization rate = (total - used) / total
 */
float LinuxParser::MemoryUtilization() { 
  string line, key;
  double num, total, free;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()){
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> num) {
        if (key == "MemTotal:") {
          total = num;
        }
        if (key == "MemFree:") {
          free = num;
          break;
        }
      }
    }
  }
  return (total - free) / total; 
}

// Read and return the uptime of a process
long LinuxParser::UpTime() {
  long up_time, idle_time;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> up_time >> idle_time;
  }
  return up_time;
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  vector<string> stat_vec;
  string line;
  string value;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while(linestream >> value) {
      if (value != "cpu"){
        stat_vec.push_back(value);
      }
    }    
  }
  return stat_vec;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  string key;
  int value;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      stream >> key >> value;
      if (key == "processes") {
        break;
      }        
    }
  }
  return value;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string key;
  int value;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      stream >> key >> value;
      if (key == "procs_running") {
        break;
      }        
    }
  }
  return value;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) { 
  std::ifstream stream(kProcDirectory + "/" + std::to_string(pid) + kCmdlineFilename);
  std::string line;
  if (stream.is_open()) {
    std::getline(stream, line);
  }

  // truncate the long command into reasonable and readable short string
  auto found = line.find_last_of("/");
  if (found == std::string::npos) {
    return line;
  }
  else {
    return line.substr(found+1);
  }
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) { 
  std::ifstream stream(kProcDirectory + "/" + std::to_string(pid) + kStatusFilename);
  std::string line, key;
  long value;
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "VmRss:") { // use physical memory insteam of virtual memory
        break;
      }
    }
  } 
  // conver KB to MB, and round off
  return std::to_string(value/1000);
}

// Read and return the user associated with a process
string LinuxParser::User(std::string uid) { 
  std::ifstream stream(kPasswordPath);
  std::string line, name, x, id;
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> name >> x >> id;
      if (id == uid) {
        break;
      }
    }
  } 
  return name;
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) { 
  std::ifstream stream(kProcDirectory + "/" + std::to_string(pid) + kStatusFilename);
  std::string line, key, value;
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      stream >> key >> value;
      if (key == "Uid:") {
        break;
      }
    }
  } 
  return value;
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  std::ifstream stream(kProcDirectory + "/" + std::to_string(pid) + kStatFilename);
  std::string line, s;
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    int counter = 1;
    while (linestream >> s) {
      if (counter == 22) {
        break;
      } 
      else {
        counter++;
      }
      
    }
  }
  return UpTime() - std::stol(s)/100;
}

// return the processor utilization
float LinuxParser::ProcUtilization(int pid) {
  std::ifstream stream(kProcDirectory + "/" + std::to_string(pid) + kStatFilename);
  std::string line, s;
  std::vector<std::string> v;
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> s) {
      v.push_back(s);
    }
  } 

  float total = (std::stoi(v[13]) + std::stoi(v[14]))/sysconf(_SC_CLK_TCK);
  float seconds = (float) UpTime(pid);
  return total / seconds;
}
