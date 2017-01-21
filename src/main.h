/**
 * Program: 4981 Assignment 1
 *
 * Source File: main.cpp
 *
 * Functions: 
 *      int main(void)
 *      void input(void)
 *      void translate(void)
 *      void output(void)
 *
 * Date: 2017/01/16
 *
 * Designer: Isaac Morneau; A00958405
 *
 * Programmer: Isaac Morneau; A00958405
 *
 * Notes: This program is an echo program that captures input in raw mode then translates it and 
 * outputs it in three seperate processes using pipes to transfer the data. it will accept
 * characters and store them in a buffer until a control character is received.
 * The control characters are:
 *  X  - backspace in buffer
 *  K  - clear line
 *  E  - enter, send line
 *  T  - exit safely
 *  ^k - exit immediately
 */
#ifndef MAIN_H
#define MAIN_H

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>
#include <vector>
#include <string>
#include <algorithm>

#define MSG_SIZE 256

/**
 * Function: output
 *
 * Date: 2017/01/16 
 *
 * Designer: Isaac Morneau; A00958405
 *
 * Programmer: Isaac Morneau; A00958405
 *
 * Interface: void output(void)
 *
 * Return: void
 *
 * Notes: Output reads from two pipes non-blocking and upon reading a message it prints it to
 * standard output.
 */
void output(void);

/**
 * Function: input
 *
 * Date: 2017/01/16 
 *
 * Designer: Isaac Morneau; A00958405
 *
 * Programmer: Isaac Morneau; A00958405
 *
 * Interface: void input(void)
 *
 * Return: void
 *
 * Notes: Input disables the default terminal standard input handling and then reads for characters.
 * It also handles the control characters
 *      When an 'E' is received it writes the message to its output pipes.
 *      When a 'T' is received it writes the message to its output pipes and closes the program
 *          killing both other processes.
 *      When a ^K is received the program shuts down without cleanup
 */
void input(void);

/**
 * Function: translate
 *
 * Date: 2017/01/16 
 *
 * Designer: Isaac Morneau; A00958405
 *
 * Programmer: Isaac Morneau; A00958405
 *
 * Interface: void translate(void)
 *
 * Return: void
 *
 * Notes: Translate reads from its input pipe and replaces all 'a's with 'z's then writes the new
 * message to the output pipe. It also handles the control characters:
 *      When a 'X' is received the last character before it is deleted from the buffer.
 *      When a 'K' is received the buffer is cleared.
 */
void translate(void);
#endif
