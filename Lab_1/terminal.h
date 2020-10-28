#ifndef LAB_1_TERMINAL_H
#define LAB_1_TERMINAL_H

#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

termios replace_terminal();

void set_terminal(const termios &terminal);

#endif //LAB_1_TERMINAL_H
