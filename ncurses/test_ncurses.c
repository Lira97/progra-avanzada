

#include<ncurses.h>
#include <unistd.h>
#define DELAY 30000
int main()
{
    int ch;
    int x = 0, y = 0;
    int max_y = 0, max_x = 0;
    int next_x = 0;
     int next_y = 0;
    int direction = 1;
    /* Curses Initialisations */
    initscr();
    raw();                  // Do not buffer inputs, but pass them immediately to be read. Do not react to signal inputs
    keypad(stdscr, TRUE);   // Allow reading special keys from the keyboard (Fx, and arrows)
    noecho();               // Do not show on the terminal the keys pressed

    printw("Welcome - Press # to Exit\n");
    while((ch = getch()) != '#')
    {
      getmaxyx(stdscr, max_y, max_x);
      clear();
      mvprintw(y, x, "o");
      refresh();

      usleep(DELAY);
      next_x = x + direction;
      if (next_x >= max_x || next_x < 0)
      {
        direction*= -1;
      }
      else
      {
        x+= direction;
      }
        switch(ch)
        {
            case KEY_UP:
                direction*= 1;
                y+= direction;
                break;
            case KEY_DOWN:
                direction*= -1;
                y+= direction;
                //printw("\nDown Arrow");
                break;
            case KEY_LEFT:
                printw("\nLeft Arrow");
                break;
            case KEY_RIGHT:
                printw("\nRight Arrow");
                break;
            default:
                printw("\nThe pressed key is ");
                attron(A_BOLD);             // Activate the attribute to when displaying characters
                printw("%c", ch);
                attroff(A_BOLD);
                break;
        }
  }

  endwin();

    return 0;
}
