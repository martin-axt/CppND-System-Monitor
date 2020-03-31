#include "ncurses_display.h"
#include "system.h"
#include "linux_parser.h"
#include "format.h"
#include "process.h"
#include <iostream>
#include <unistd.h>

using std::cout;
using std::endl;

int main() {
  System system;
  NCursesDisplay::Display(system);
}