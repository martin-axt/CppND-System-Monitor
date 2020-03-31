#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <iomanip>
#include <unistd.h>

// Delete
#include<iostream>


#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;
using std::ifstream;
using std::istringstream;
using std::cout;
using std::getline;
using std::endl;
using std::replace;
using std::to_string;

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
  string os, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> kernel;
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
float LinuxParser::MemoryUtilization() { 
  string line, key;
  float memtotal = -1, memfree = -1;
  float value;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line) and !(memtotal >= 0 && memfree >= 0)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      line = line.substr(0, line.find("kB"));
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "MemTotal") {
        memtotal = value;
      } else if (key == "MemFree") {
        memfree = value;
      }
    }
  }
  return (memtotal - memfree) / memtotal; 
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
  long uptime;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    string line;
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime;
  }
  
  return uptime;
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  return ActiveJiffies() + IdleJiffies();
}

vector<string> LinuxParser::GetPidStat(int pid) {
  ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  vector<string> values;
  string value;
  string line;
  if (stream.is_open()) {
    getline(stream, line);
    istringstream linestream(line);
    while (linestream >> value) {
      values.push_back(value);
    }
  }
  return values;
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) { 
  vector<string> pid_stat = GetPidStat(pid);
  return stof(pid_stat[22]);
}

float LinuxParser::CpuUtilization(int pid) {
  vector<string> stats = GetPidStat(pid);
  long utime = stol(stats[14]), stime = stol(stats[15]), cutime = stol(stats[16]), cstime = stol(stats[17]), starttime = stol(stats[22]);
  long uptime = UpTime();
  long hertz = sysconf(_SC_CLK_TCK);
  long total_time = utime + stime;
  total_time = total_time + cutime + cstime;
  long seconds = uptime - (starttime / hertz);
  float cpu_usage = 100 * ((total_time / hertz) / seconds);
  return cpu_usage;
}

long LinuxParser::SumCpuData(vector<int> pos) {
  vector<long> cpu_data = GetStat("cpu");
  long res = 0;
  for (int p: pos) {
  	res += cpu_data[p];
  }
  return res;
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  vector<int> active_pos = {0, 1, 2, 5, 6, 7};
  return SumCpuData(active_pos);
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { 
  vector<int> idle_pos = {3, 4};
  return SumCpuData(idle_pos);
}

vector<long> LinuxParser::GetStat(string stat) {
  std::ifstream stream(kProcDirectory + kStatFilename);
  string line;
  string key;
  vector<long> values{};
  if (stream.is_open()) {
  	while(std::getline(stream, line)) {
      std::istringstream linestream(line);
      if (linestream >> key && key == stat) {
        long value;
      	while (linestream >> value) {
          values.push_back(value);
        }
        break;
      }
    }
  }
  return values;
}

// TODO: Read and return CPU utilization
float LinuxParser::CpuUtilization() { 
  long active = ActiveJiffies();
  long idle = IdleJiffies();
//   char buffer[50];
  float cpu_use = active / (active + idle);
//   sprintf(buffer, "%.1f", cpu_use);
  return cpu_use;
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  vector<long> stats = GetStat("processes");
  return stats[0];
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
  vector<long> stats = GetStat("procs_running");
  return stats[0];
}

string LinuxParser::GetPidStatus(int pid, string status) {
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  string key;
  string value;
  if (stream.is_open()) {
  	string line;
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      if (linestream >> key >> value && key == status) {
      	break;
      }
    }
  }
  return value;
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid[[maybe_unused]]) { 
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  string command;
  if (stream.is_open()) {
  	std::getline(stream, command);
  }
  return command;
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) { 
  std::istringstream stream(GetPidStatus(pid, "VmSize"));
  float ram;
  stream >> ram;
  ram = ram / 1000;
  std::ostringstream ss;
  ss << std::fixed << std::setprecision(2) << ram;
  return ss.str();
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) { 
  return GetPidStatus(pid, "Uid");
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) { 
  string user_uid = Uid(pid);
  string user, uid, line, x;
  bool found = false;
  ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (getline(stream, line) && !found) {
      replace(line.begin(), line.end(), ':', ' ');
      istringstream linestream(line);
      found = linestream >> user >> x >> uid && uid == user_uid;
    }
  }
  return user;
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) { 
  return ActiveJiffies(pid) / sysconf(_SC_CLK_TCK);
}