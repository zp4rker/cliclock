/*
 * Thanks to xorg62 for tty-clock
 *      and genie5 for 12clock
 *
 * A simple CLI clock that displays the time in 24 or 12 hour format.
 *
 * Modified by: clyde80
 * Website for this version: https://github.com/clyde80/cliclock
 * Date Created: 2017-04-04
 * Date Modified: 2017-04-09
 *
 * Any bugs? Please report them to https://github.com/clyde80/cliclock/issues
 * Want to contribute? Fork the repo at https://github.com/clyde80/cliclock
 */
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <signal.h>
#include <stdbool.h>
#include <getopt.h>

/* Available:
 * COLOR_BLACK
 * COLOR_RED
 * COLOR_GREEN
 * COLOR_YELLOW
 * COLOR_BLUE
 * COLOR_MAGENTA
 * COLOR_CYAN
 * COLOR_WHITE */

#define DEFAULT_FG_COLOR COLOR_BLUE
#define DEFAULT_FG_COLOR_NAME "blue"

typedef struct {
    bool running;
    int bg;
    struct {
        int color;
        long delay;
        bool twenty_four_hour;
        bool use_seconds;
    } option;
    struct {
        int x, y, w, h;
    } geo;
    struct {
        unsigned int hour[2];
        unsigned int minute[2];
        unsigned int second[2];
    } date;

    struct tm *tm;
    time_t lt;

    WINDOW *framewin;

} cliclock_t;

static void init(void);
static void update_hour(void);
static void draw_number(int n, int x, int y);
static void draw_clock(void);
static void clock_move(int x, int y);
static void signal_handler(int signal);
static void change_color(void);
static void toggle_seconds(void);
static void toggle_twenty_four_hour(void);
static void key_event(void);

cliclock_t *cliclock;

int c_color = 0;    // Current color.

static const bool number[][15] = {
    {1,1,1,1,0,1,1,0,1,1,0,1,1,1,1}, /* 0 */
    {0,0,1,0,0,1,0,0,1,0,0,1,0,0,1}, /* 1 */
    {1,1,1,0,0,1,1,1,1,1,0,0,1,1,1}, /* 2 */
    {1,1,1,0,0,1,1,1,1,0,0,1,1,1,1}, /* 3 */
    {1,0,1,1,0,1,1,1,1,0,0,1,0,0,1}, /* 4 */
    {1,1,1,1,0,0,1,1,1,0,0,1,1,1,1}, /* 5 */
    {1,1,1,1,0,0,1,1,1,1,0,1,1,1,1}, /* 6 */
    {1,1,1,0,0,1,0,0,1,0,0,1,0,0,1}, /* 7 */
    {1,1,1,1,0,1,1,1,1,1,0,1,1,1,1}, /* 8 */
    {1,1,1,1,0,1,1,1,1,0,0,1,1,1,1}, /* 9 */
};


static void init(void) 
{
    struct sigaction sig;

    cliclock->bg = 0;

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, true);
    start_color();
    curs_set(false);
    clear();

    if (use_default_colors() == OK) cliclock->bg = -1;

    // Init color pair.
    init_pair(0, cliclock->bg, cliclock->bg);
    init_pair(1, cliclock->bg, cliclock->option.color);
    init_pair(2, cliclock->option.color, cliclock->bg);
    init_pair(3, -1, DEFAULT_FG_COLOR);

    refresh();

    // Init signal handler.
    sig.sa_handler = signal_handler;
    sig.sa_flags   = 0;
    sigaction(SIGWINCH, &sig, NULL);
    sigaction(SIGTERM,  &sig, NULL);

    /* Init global struct */ 
    cliclock->running = true;

    if (!cliclock->geo.x) cliclock->geo.x = 0;
    if (!cliclock->geo.y) cliclock->geo.y = 0;

    // Determine the width of the window.
    if (!cliclock->option.use_seconds 
            && cliclock->option.twenty_four_hour) {
        cliclock->geo.w = 34;
    } else if (cliclock->option.twenty_four_hour
            || !cliclock->option.use_seconds) {
        cliclock->geo.w = 54;
    } else {
        cliclock->geo.w = 74;
    }

    cliclock->geo.h = 7;    // Height of window.

    cliclock->tm = localtime(&(cliclock->lt));
    cliclock->lt = time(NULL);
    update_hour();

    // Create clock window.
    cliclock->framewin = newwin(cliclock->geo.h,
            cliclock->geo.w,
            cliclock->geo.x,
            cliclock->geo.y);
    clock_move((LINES / 2 - (cliclock->geo.h / 2)),
            (COLS  / 2 - (cliclock->geo.w / 2)));
    wborder(cliclock->framewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    nodelay(stdscr, true);
    wrefresh(cliclock->framewin);

    return;
}


static void signal_handler(int signal)
{
    switch (signal) {
        case SIGWINCH: // Window resize signal.
            endwin();
            init();
            break;
        case SIGTERM:
            cliclock->running = false; // Interruption signal.
            break;
    }

    return;
}


static void update_hour(void)
{
    int ihour;

    cliclock->tm = localtime(&(cliclock->lt));
    cliclock->lt = time(NULL);
    ihour = cliclock->tm->tm_hour;

    // Set the hour.
    if (cliclock->option.twenty_four_hour 
            && (ihour % 12 == 0)
            && (ihour > 12)) {
        cliclock->date.hour[0] = 0;
        cliclock->date.hour[1] = 0;
    } else if (cliclock->option.twenty_four_hour) {
        cliclock->date.hour[0] = ihour / 10;
        cliclock->date.hour[1] = ihour % 10;
    } else if (ihour % 12 == 0) {
        cliclock->date.hour[0] = 1;
        cliclock->date.hour[1] = 2;
    } else {
        cliclock->date.hour[0] = (ihour % 12) / 10;
        cliclock->date.hour[1] = (ihour % 12) % 10;
    }

    // Set minutes
    cliclock->date.minute[0] = cliclock->tm->tm_min / 10;
    cliclock->date.minute[1] = cliclock->tm->tm_min % 10;

    // Set seconds
    if (cliclock->option.use_seconds) {
        cliclock->date.second[0] = cliclock->tm->tm_sec / 10;
        cliclock->date.second[1] = cliclock->tm->tm_sec % 10;
    }

    return;
}


static void draw_number(int n, int x, int y)
{
    int i, sy = y;

    for (i = 0; i < 30; ++i, ++sy) {
        if (sy == y + 6) {
            sy = y;
            ++x;
        }
        wbkgdset(cliclock->framewin, COLOR_PAIR(number[n][i/2]));
        mvwaddch(cliclock->framewin, x, sy, ' ');
    }

    wrefresh(cliclock->framewin);

    return;
}


static void draw_clock(void)
{
    // These functions take coordinates in (y,x) order. Not (x,y). 

    // Calculate where the AM and PM should be drawn.
    int am_pm_start = 0;
    if (!cliclock->option.twenty_four_hour
            && cliclock->option.use_seconds) {
        am_pm_start = 58;
    } else if (!cliclock->option.twenty_four_hour
            && !cliclock->option.use_seconds) {
        am_pm_start = 39;
    }

    // Draw hours
    draw_number(cliclock->date.hour[0], 1, 1);
    draw_number(cliclock->date.hour[1], 1, 8);

    // Two dots separation
    wbkgdset(cliclock->framewin, COLOR_PAIR(1));
    mvwaddstr(cliclock->framewin, 2, 16, "  ");
    mvwaddstr(cliclock->framewin, 4, 16, "  ");

    // Draw minutes
    draw_number(cliclock->date.minute[0], 1, 20);
    draw_number(cliclock->date.minute[1], 1, 27);

    // Check if the seconds should be drawn
    if (cliclock->option.use_seconds) {

        // Two dots separation
        wbkgdset(cliclock->framewin, COLOR_PAIR(1));
        mvwaddstr(cliclock->framewin, 2, 35, "  ");
        mvwaddstr(cliclock->framewin, 4, 35, "  ");

        // Draw seconds
        draw_number(cliclock->date.second[0], 1, 39);
        draw_number(cliclock->date.second[1], 1, 46);
    }

    // Draw AM or PM
    if (!cliclock->option.twenty_four_hour) {
        wbkgdset(cliclock->framewin, COLOR_PAIR(1));
        mvwaddstr(cliclock->framewin, 1, am_pm_start, "      ");
        mvwaddstr(cliclock->framewin, 2, am_pm_start, "  ");
        mvwaddstr(cliclock->framewin, 2, am_pm_start + 4, "  ");
        mvwaddstr(cliclock->framewin, 3, am_pm_start, "      ");
        mvwaddstr(cliclock->framewin, 4, am_pm_start, "  ");
        mvwaddstr(cliclock->framewin, 5, am_pm_start, "  ");

        // Finish drawing the "A" in "AM" if the hour is < 12.
        if(cliclock->tm->tm_hour < 12)
        {   mvwaddstr(cliclock->framewin, 4, am_pm_start + 4, "  ");
            mvwaddstr(cliclock->framewin, 5, am_pm_start + 4, "  ");
        }
            
        // Draw the M
        mvwaddstr(cliclock->framewin, 1, am_pm_start + 7, "        ");
        mvwaddstr(cliclock->framewin, 2, am_pm_start + 7, "  ");
        mvwaddstr(cliclock->framewin, 3, am_pm_start + 7, "  ");
        mvwaddstr(cliclock->framewin, 4, am_pm_start + 7, "  ");
        mvwaddstr(cliclock->framewin, 5, am_pm_start + 7, "  ");
        mvwaddstr(cliclock->framewin, 2, am_pm_start + 10, "  ");
        mvwaddstr(cliclock->framewin, 3, am_pm_start + 10, "  ");
        mvwaddstr(cliclock->framewin, 4, am_pm_start + 10, "  ");
        mvwaddstr(cliclock->framewin, 5, am_pm_start + 10, "  ");
        mvwaddstr(cliclock->framewin, 2, am_pm_start + 13, "  ");
        mvwaddstr(cliclock->framewin, 3, am_pm_start + 13, "  ");
        mvwaddstr(cliclock->framewin, 4, am_pm_start + 13, "  ");
        mvwaddstr(cliclock->framewin, 5, am_pm_start + 13, "  ");
    }
}


static void clock_move(int x, int y)
{
    mvwin(cliclock->framewin, (cliclock->geo.x = x), (cliclock->geo.y = y));
    return;
}


static void change_color(void) 
{
    cliclock->running = false;

    c_color = (c_color < 7) ? c_color+1 : 0;
    cliclock->option.color = c_color;

    init();

    while (cliclock->running) {
        update_hour();
        draw_clock();
        key_event();
    }
}


static void toggle_seconds(void) 
{
    cliclock->running = false;

    cliclock->option.use_seconds = !cliclock->option.use_seconds;

    init();

    while (cliclock->running) {
        update_hour();
        draw_clock();
        key_event();
    }
}


static void toggle_twenty_four_hour(void) 
{
    cliclock->running = false;

    cliclock->option.twenty_four_hour = !cliclock->option.twenty_four_hour;

    init();

    while (cliclock->running) {
        update_hour();
        draw_clock();
        key_event();
    }
}


static void key_event(void)
{
    int c;  // The character the user typed.

    struct timespec length = { 0, cliclock->option.delay };

    switch (c = wgetch(stdscr)) {
        case 'q':
            cliclock->running = false;
            break;
        case 'c':
            change_color();
            break;
        case 's':
            toggle_seconds();
            break;
        case 't':
            toggle_twenty_four_hour();
            break;
        default:
            nanosleep(&length, NULL);
            break;
    }

    return;
}


void usage(void)
{
    printf(" Usage: cliclock [-c color] [-t]\n");
    printf("    -c [color]: Use this color for clock (default: %s)\n", DEFAULT_FG_COLOR_NAME);
    printf("                Or start the clock and press 'c' to change color.\n");
    printf("    -t: Display 24 Hour time format.\n");
    printf("        Press 't' to toggle 24 hour time.\n");
    printf("    -s: Do not show the seconds field.\n");
    printf("        Press 's' to toggle the seconds field.\n");
}


int main(int argc, char **argv)
{
     // Parses options
    int color = DEFAULT_FG_COLOR;   // The color.
    int c;                          // The option the user typed.
    int option_index = 0;
    bool twenty_four_hour = false;  // Use twenty-four hour format.
    bool use_seconds = true;        // Display the seconds field.

    static struct option long_options[] =
    {
        {"help", no_argument, 0, 'h'},
        {"ttime", no_argument, 0, 't'},
        {"color", required_argument, 0, 'c'},
        {"noseconds", no_argument, 0, 's'}
    };

    while ((c = getopt_long (argc, argv, "hc:ts", long_options, &option_index)) != -1) {
        switch (c) {
            case 'c':
                color = atoi(optarg);   // Convert the int (0-7) to a string.
                c_color = atoi(optarg);

                if (color < 0 || color > 7) {
                    printf("Invalid color number: %d\n\n", color);
                    printf("Valid colors are black (0), red (1),  green (2), yellow (3),\n");
                    printf("                 blue (4), magenta (5), cyan (6), white (7).\n");
                    return 1;
                }

                break;
            case 't':
                twenty_four_hour = true;    // Display time in 24 hour format.
                break;
            case 's':
                use_seconds = false;        // Do not display the seconds field.
                break;
            case 'h':
                usage();                    // Print the usage.
                return 0;
        }
    }

    cliclock = malloc(sizeof(cliclock_t));

    // Set the options in the clock.
    cliclock->option.color = color;
    cliclock->option.twenty_four_hour = twenty_four_hour;
    cliclock->option.use_seconds = use_seconds;
    cliclock->option.delay = 40000000;

    // Initialize the clock
    init();

    while (cliclock->running) {
        update_hour();
        draw_clock();
        key_event();
    }

    free(cliclock);
    endwin();
    return 0;
}
