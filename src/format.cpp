#include <string>

#include "format.h"

using std::string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds) { 
  int hours = seconds / 3600;
  int minutes = (seconds % 3600) / 60;
  seconds = seconds % 60;
  char time[9];
  sprintf(time, "%02u:%02u:%02u", hours, minutes, seconds);
  return time; 
}