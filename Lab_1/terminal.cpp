
#include "terminal.h"

int oldfl = 0;

termios replace_terminal(){

    termios old_tio{};      // for storing settings from old terminal
    tcgetattr(STDIN_FILENO, &old_tio);  // save old terminal

    termios new_tio{old_tio};
    new_tio.c_lflag &= (~ICANON & ~ECHO);// disable canonical mode (buffered i/o) and local echo
    oldfl = fcntl(0, F_GETFL);
    fcntl(0, F_SETFL, O_NONBLOCK);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
    /* set the new settings */

    return old_tio;
}

void set_terminal(const termios &terminal){

    if (oldfl == -1) {
        return;
    }
    fcntl(0, F_SETFL, oldfl & ~O_NONBLOCK);
    tcsetattr(STDIN_FILENO, TCSANOW, &terminal);
}