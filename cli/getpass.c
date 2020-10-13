#include <stdint.h>
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

static char BACKSPACE = 8;
static char RETURN = 13;
static char LINEFEED = 10;

#ifdef _WIN32

void cli_getpass(const char* prompt, uint8_t* pass, size_t max, size_t* pass_len) {
  printf("%s\n", prompt);

  HANDLE hConsole = GetStdHandle(STD_INPUT_HANDLE);
  DWORD con_mode;
  GetConsoleMode(hConsole, &con_mode);
  SetConsoleMode(hConsole, con_mode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT));

  size_t len = 0;

  for (;;) {
    WCHAR utf16_mb[2];
    CHAR utf8_mb[4];
    DWORD utf16_read;
    DWORD utf8_read;

    if (!ReadConsoleW(hConsole, &utf16_mb, 2, &utf16_read, NULL)) {
      exit(1);
    }

    utf8_read = WideCharToMultiByte(CP_UTF8, 0, utf16_mb, utf16_read, utf8_mb, sizeof(utf8_mb), NULL, NULL);

    if (utf8_mb[0] == BACKSPACE) {
      len = 0;
    }
    else if (utf8_mb[0] == RETURN) {
      break;
    }
    else if ((len + utf8_read - 1) < max) {
      for (size_t i = 0; i < utf8_read; i += 1, len += 1) {
        pass[len] = utf8_mb[i];
      }
    }
  }

  SetConsoleMode(hConsole, con_mode);
  *pass_len = len;
}

#else // _WIN32

void cli_getpass(const char* prompt, uint8_t* pass, size_t max, size_t* pass_len) {
  printf("%s\n", prompt);

  struct termios old_terminal;
  struct termios new_terminal;
  tcgetattr(STDIN_FILENO, &old_terminal);
  new_terminal = old_terminal;
  new_terminal.c_lflag &= ~(ECHO);          
  tcsetattr(STDIN_FILENO, TCSANOW, &new_terminal);

  size_t len = 0;

  for (;;) {
    int utf8_mb = getchar();
    if (utf8_mb == EOF) {
      break;
    }

    if (utf8_mb == BACKSPACE) {
      len = 0;
    }
    else if (utf8_mb == LINEFEED) {
      break;
    }
    else if (len < max) {
      pass[len] = utf8_mb;
      len += 1;
    }
  }

  tcsetattr(STDIN_FILENO, TCSANOW, &old_terminal);
  *pass_len = len;
}

#endif
