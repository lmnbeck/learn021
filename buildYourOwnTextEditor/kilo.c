/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** defines ***/

//sets the upper 3 bits of the character to 0. This mirrors what the Ctrl key does in the terminal
#define CTRL_KEY(k) ((k) & 0x1f)

/*** data ***/

struct editorConfig {
  struct termios orig_termios;
};
struct editorConfig E;

/*** terminal ***/

/*
  prints an error message and exits the program
*/
void die(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  perror(s);
  exit(1);
}

void disableRawMode() {
   if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
    die("tcsetattr");
}

void enableRawMode() {
  //Terminal attributes can be read into a termios struct by tcgetattr(). 
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");
  
  //atexit() comes from <stdlib.h>. We use it to register our disableRawMode() function to be called automatically when the program exits
  atexit(disableRawMode);

  struct termios raw = E.orig_termios;

  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  //ICRNL: Terminal is helpfully translating any carriage returns (13, '\r') inputted by the user into newlines (10, '\n'). Let’s turn off this feature.
  //Enable XON/XOFF flow control on output.
  raw.c_iflag &= ~(ICRNL | IXON);
  //Turn off all output processing features by turning off the OPOST flag.
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  //The ECHO feature causes each key you type to be printed to the terminal
  //ICANON flag that allows us to turn off canonical mode.
  //ISIG   When any of the characters INTR, QUIT, SUSP, or DSUSP are received, generate the corresponding signal.
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

  //The VMIN value sets the minimum number of bytes of input needed before read() can return.
  raw.c_cc[VMIN] = 0;
  //The VTIME value sets the maximum amount of time to wait before read() returns. 
  raw.c_cc[VTIME] = 1;  //Stands for 1/10 second

  //apply them to the terminal using tcsetattr(). 
  //The TCSAFLUSH argument specifies when to apply the change
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

char editorReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  return c;
}

/*** output ***/

void editorDrawRows() {
  int y;
  for (y = 0; y < 24; y++) {
    write(STDOUT_FILENO, "~\r\n", 3);
  }
}

void editorRefreshScreen() {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  editorDrawRows();
  
  write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** input ***/

void editorProcessKeypress() {
  char c = editorReadKey();
  switch (c) {
    case CTRL_KEY('q'):
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      exit(0);
      break;
  }
}

/*** init ***/

int main() {
  enableRawMode();

  while ( 1 ){
    editorRefreshScreen();
    editorProcessKeypress();
  }
  
  return 0;
}