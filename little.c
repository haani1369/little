#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <ncurses.h>


#define min(a, b) ((a) < (b) ? (a) : (b))

#define MAX_W 256
#define MAX_H 256


void		render(void);
static void	finish(int);
void		setup(void);
void		fill_str(const char *);

static bool	foreword(void);
static bool	my(void);
static bool	afterword(void);


static int	actual_w, actual_h;
static char	screen[MAX_W][MAX_H];

static bool	(*drawers[3])(void) = {
	foreword,
	my,
	afterword,
};

static int	active_drawer = 0;
static int	t = 0;

bool
foreword(void)
{
	const char *str  = "little my ";
	const int  strln = strlen(str);

	const int  num_flashes = 3 * 2;

	if (t >= strln + num_flashes)
		return true;

	if (t >= strln && t & 1)
		return false;

	const int maxidx = min(t, strln);
	for (int y = 0; y < actual_h; y++) {
		for (int x = 0; x < actual_w; x++) {
			if (y * actual_w + x >= maxidx)
				return false;
			screen[y][x] = str[x % strln];
		}
	}

	return false;
}

bool
my(void)
{
	if (t == 10)
		return true;
	fill_str("little my ");
	return false;
}

bool
afterword(void)
{
	const char *str  = "thanks for playing! ";
	const int  strln = strlen(str);

	const int y       = actual_h / 2;
	const int start_x = actual_w / 2 - strln / 2;
	const int end_x   = actual_w / 2 + strln / 2;

	int i = 0;
	for (int x = start_x; x < end_x; x++)
		screen[y][x] = str[i++]; 

	return false;
}

void
draw(void)
{
	memset(screen, ' ', sizeof(screen));

	if (!drawers[active_drawer]())
		return;

	t = 0;
	active_drawer++;
}

void
little(void)
{
	while (getch() != 'q') {
		draw();
		render();
		usleep(400000);
		t++;
	}
}

int
main(void)
{
	setup();

	little();

	finish(0);
}

void
fill_str(const char *str)
{
	const int strln = strlen(str);

	for (int y = 0; y < actual_h; y++) {
		int offs = t + (y & 1? strln / 2 : 0);
		for (int x = 0; x < actual_w; x++) {	
			screen[y][x] = str[(x + offs) % strln];
		}
	}
}

void
render(void)
{
	clear();

	for (int y = 0; y < actual_h; y++)
		for (int x = 0; x < actual_w; x++)
			mvaddch(y, x, screen[y][x]);

	refresh();
}

static void
finish(int sig)
{
	endwin();
	exit(EXIT_SUCCESS);
}

void
setup(void)
{
	signal(SIGINT, finish);

	initscr();
	noecho();
	curs_set(0);
	nodelay(stdscr, true);

	getmaxyx(stdscr, actual_h, actual_w);
}

