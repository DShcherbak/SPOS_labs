#ifndef LAB_1_FUNCTIONS_H
#define LAB_1_FUNCTIONS_H
#include <iostream>
#include <thread>
#include <future>
#include <termios.h>
#include <unistd.h>
#include <vector>
#include <ncurses.h>
#include "cmake-build-debug/demofuncs"

void f_function(int x);

void g_function(int x);

void main_thread(int fx, int gx, bool demo_mode);


#endif //LAB_1_FUNCTIONS_H
