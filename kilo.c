/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** data ***/

struct termios orig_termios; // we store original attributes of terminal to set them back later

/*** terminal ***/

void die(const char *s){
    // prints an error message and exits the program
    perror(s);
    exit(1);
}

void disableRawMode(){
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) die("tcsetattr");
}

void enableRawMode(){
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr"); // read terminal attributes
    atexit(disableRawMode); // we need to turn off raw mode after exit from program to provide normal using of terminal

    struct termios raw = orig_termios;
    /* ICRNL hepls to read ctrl + M
     * IXON disables ctrl + S and ctrl + Q
     */
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST); // turn off all output processing features
    raw.c_cflag |= (CS8);
    /* following line
     * forces the fourth bit in the flags field to become 0,
     * and causes every other bit to retain its current value
     * ICANON helps to read text byte-by-byte but not line-by-line
     * IEXTEN disables ctrl + V (and ctrl + O on macOS)
     * ISIG turns off ctrl + C and ctrl + Z (and ctrl + Y on macOS)
     */
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    // BRKINT, INPCK, ISTRIP, and CS8 may be already turned off but we carry on a tradition
    raw.c_cc[VMIN] = 0; // sets the minimum number of bytes of input needed before read() can return
    raw.c_cc[VTIME] = 1; // sets the maximum amount of time to wait before read() returns


    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

/*** init ***/

int main(){
    enableRawMode();

    while (1) // read chars from terminal and write them to variable c
    {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
        if (iscntrl(c)) { //check whether symbol is control character and print it if not
            printf("%d\r\n", c);
        } else {
            printf("%d ('%c')\r\n", c, c);
        }
        if (c == 'q') break; // character 'q' is a command  to exit from program
    }
    return 0;
};