#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <ncurses.h>

#include "my.xbm"


#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define MAX_W 256
#define MAX_H 256

bool		gamble(float);

void		render(void);
void		fill_str(const char *);
bool		corrupt(const char *, const char *);
void		draw_bmp(const char *, const int, const int);

void		setup(void);
static void	finish(int);

bool		foreword(void);
bool		my(void);
bool		my_to_nicolas(void);
bool		nicolas(void);
bool		afterword(void);


static int	actual_w, actual_h;
static char	screen[MAX_W][MAX_H];

static bool	(*drawers[5])(void) = {
	foreword,
	my,
	my_to_nicolas,
	nicolas,
	afterword,
};

static int	active_drawer = 0;
static int	t = 0;

bool
foreword(void)
{
	memset(screen, ' ', sizeof(screen));

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
	if (t == 20)
		return true;

	memset(screen, ' ', sizeof(screen));

	fill_str("little my ");

	if (t < 10)
		return false;

	draw_bmp(my_image_bits, my_image_height, my_image_width);

	return false;
}

bool
my_to_nicolas(void)
{
	if (corrupt("little my ", "little nicholas "))
		return false;
	
	memset(screen, ' ', sizeof(screen));

	return true;
}

bool
nicolas(void)
{
	if (t == 20)
		return true;
	fill_str("little nicolas ");

	if (t < 10)
		return false;

	return false;
}

bool
afterword(void)
{
	memset(screen, ' ', sizeof(screen));

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
	if (!drawers[active_drawer]())
		return;

	t = 0;
	active_drawer++;
}

void
little(int opt)
{
	active_drawer = opt == -1? active_drawer : opt;

	while (true) {
		getmaxyx(stdscr, actual_h, actual_w);

		switch (getch()) {
		case 'q':
			return;
		case 'r':
			t = 0;
			active_drawer = 0;
			break;
		default:
			/* do nothing */;
		}

		draw();
		render();
		usleep(400000);
		t++;
	}
}

int
main(int argc, char **argv)
{
	setup();

	if (argc == 1)
		little(-1);
	else
		little(atoi(argv[1]));

	finish(0);
}

bool
gamble(float p)
{
	if ((float)rand() / (float)RAND_MAX < p)
		return true;
	return false;
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

bool
corrupt(const char *old, const char *new)
{
	const int oldln = strlen(old);
	const int newln = strlen(new);

	int y, x;
	bool found = false;
	for (y = 0; y < actual_h; y++) {
		for (x = 0; x < actual_w; x++) {
			if (y * actual_w + x >= actual_h * actual_w - newln)
				return false;
			if (strncmp(&screen[y][x], old, oldln) != 0)
				continue;
			if (gamble(0.95))
				continue;
			found = true;
			break;
		}
		if (found)
			break;
	}

	char *start = &screen[y][x];
	for (int i = 0; i < newln; i++)
		*start++ = new[i];
	return true;
}

void
draw_bmp(const char *image_bits, const int image_height,
	 const int image_width)
{
	const char *map  = " .,-~:;=!*#$@";
	const int  mapln = strlen(map);

	int *counts = malloc(actual_h * actual_w * sizeof(int));
	memset((void*)counts, 0, actual_h * actual_w * sizeof(int));

	const int factor = max(image_height / actual_h,
			       image_width  / (actual_w / 2));
	const int y_factor = factor;
	const int x_factor = factor / 2;

	for (int y = 0; y < image_height; y++) {
		for (int x = 0; x < image_width; x++) {
			bool bit = (image_bits[y * (image_width / 8) + (x / 8)] >> (x % 8)) & 1;
			counts[(y / y_factor) * actual_w + (x / x_factor)] += bit;
		}
	}

	int max_ = 0;
	for (int i = 0; i < actual_h * actual_w; i++)
		max_ = max(max_, counts[i]);

	int start_y = 0;
	int end_y   = image_height / y_factor;
	int start_x = 0;
	int end_x   = image_width / x_factor;

	for (int y = start_y; y < end_y; y++)
		for (int x = start_x; x < end_x; x++){
			int uhh = (int)((float)mapln * counts[y * actual_w + x] / max_);
			screen[y][x] = map[uhh];
		}

	free(counts);
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

void
setup(void)
{
	signal(SIGINT, finish);

	initscr();
	noecho();
	curs_set(0);
	nodelay(stdscr, true);
}

static void
finish(int sig)
{
	endwin();
	exit(EXIT_SUCCESS);
}

