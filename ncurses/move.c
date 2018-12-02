#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>

#define DELAY 30000

int up_down = 0;

// Thread Functions
void * moveloop(void * arg);
void * getKey(void * arg);
void startThreads();

int main(int argc, char *argv[]) {

    initscr();
    noecho();
    curs_set(FALSE);
    raw();
    keypad(stdscr, TRUE);

    startThreads();
    endwin();
    return 0;
}

void * moveloop(void * arg)
{
    int x = 0, y = 0;
    int max_y = 0, max_x = 0;
    int next_x = 0;
    int next_y = 0;
    int direction = 1;

    // Global var `stdscr` is created by the call to `initscr()`


    while(1)
    {
      getmaxyx(stdscr, max_y, max_x);
        clear(); // Clear the screen of all
        // previously-printed characters
        mvprintw(y, x, "o"); // Print our "ball" at the current xy position
        refresh();

        usleep(DELAY); // Shorter delay between movements

        next_x = x + direction;

        y+=up_down;

        if (next_x >= max_x || next_x < 0){

            direction *= -1;

        }
        else{

            x+=direction;

        }
    }
    endwin();
}

void * getKey(void * arg){

    int ch;
    while((ch = getch()) != '#')
    {
        switch(ch)
        {
            case KEY_UP:
                up_down = -1;
                break;
            case KEY_DOWN:
                up_down = 1;
                break;
            default:

                break;
        }
    }
}

void startThreads()
{
    pthread_t tid[2];
    int status;

    // Start the two threads
    status = pthread_create(&tid[0], NULL, &moveloop, NULL);
    if (status)
    {
        fprintf(stderr, "ERROR: pthread_create %d\n", status);
    }
    status = pthread_create(&tid[1], NULL, &getKey, NULL);
    if (status)
    {
        fprintf(stderr, "ERROR: pthread_create %d\n", status);
    }

    pthread_exit(NULL);
}
