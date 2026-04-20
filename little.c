#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#include <ncurses.h>

#include "my.xbm"
#include "smoke.xbm"
#include "star.xbm"


#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define MAX_W 256
#define MAX_H 256


void		render		  (void);
void		center_print	  (const char *);
void		fill_str	  (const char *);
int		gamble		  (int, int);
bool		corrupt		  (const char *, const char *, const char *);
void		draw_bmp	  (const char *, const int, const int, bool);

void		setup		  (void);
static void	finish		  (int);

#define NUM_DRAWERS 7

bool		foreword	  (void);
bool		my		  (void);
bool		my_to_nicolas	  (void);
bool		nicolas		  (void);
bool		nicolas_to_prince (void);
bool		prince		  (void);
bool		afterword	  (void);


static int	actual_w, actual_h;
static char	screen[MAX_W][MAX_H];
static char	colors[MAX_W][MAX_H];

static bool	(*drawers[NUM_DRAWERS])(void) = {
			foreword,
			my,
			my_to_nicolas,
			nicolas,
			nicolas_to_prince,
			prince,
			afterword,
		};

static int	active_drawer = 0;
static int	t             = 0;
static int	speed         = 1;

void
blank(void)
{
	memset(screen, ' ', sizeof(screen));
	memset(colors, 0,   sizeof(colors));
}

bool
foreword(void)
{
	blank();

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

	blank();

	fill_str("little my ");

	if (t < 10)
		return false;

	draw_bmp(my_image_bits, my_image_height, my_image_width, false);

	return false;
}

bool
my_to_nicolas(void)
{
	if (t >= 20) {
		speed = 1;
		blank();
		return true;
	}

	speed = 4;

	const char *s_old = "my ";
	const char *s_new = "nicholas ";

	char c_new[9];
	for(int i = 0; i < 9; i++)
		c_new[i] = 1;

	corrupt(s_old, s_new, c_new);
	return false;
}

bool
nicolas(void)
{
	if (t == 20)
		return true;

	blank();

	fill_str("little nicolas ");

	if (t < 10)
		return false;

	draw_bmp(smoke_image_bits, smoke_image_height, smoke_image_width, true);

	return false;
}

bool
nicolas_to_prince(void)
{
	if (t >= 15)
		return true;

	blank();

	if (t < 5)
		return false;

	const char *ref_str = "loading      ";	

	char *str = malloc(sizeof(ref_str));
	strcpy(str, ref_str);

	for (int i = 0; i < (t & 3); i++)
		str[i + 7] = '.';
	
	center_print(str);

	return false;	
}

bool
prince(void)
{
	draw_bmp(star_image_bits, star_image_height, star_image_width, false);

	return false;
}

bool
afterword(void)
{
	blank();

	const char *str  = "thanks for playing!";
	center_print(str);

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
	blank();
	active_drawer = opt == -1? active_drawer : opt;

	while (true) {
		getmaxyx(stdscr, actual_h, actual_w);

		switch (getch()) {
		case 'q':
			return;
		case 'r':
			t = 0;
			active_drawer = 0;
			speed = 1;
			break;
		case 'a':
			active_drawer = NUM_DRAWERS - 1;
			break;
		default:
			/* do nothing */;
		}

		draw();
		render();
		usleep(400000 / speed);
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

void
center_print(const char *str)
{
	const int  strln = strlen(str);

	const int y       = actual_h / 2;
	const int start_x = actual_w / 2 - strln / 2;
	const int end_x   = actual_w / 2 + strln / 2 + ((actual_w ^ strln) & 1);

	int i = 0;
	for (int x = start_x; x < end_x; x++)
		screen[y][x] = str[i++]; 
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

int
gamble(int start, int end)
{
	return rand() % (end + start) + start;
}

bool
corrupt(const char *s_old, const char *s_new, const char* c_new)
{
	const int oldln = strlen(s_old);
	const int newln = strlen(s_new);

	bool found = false;

	int y = gamble(0, actual_h);
	int x = gamble(0, actual_w);
	for (; y < actual_h; y++) {
		for (; x < actual_w; x++){
			if (strncmp(&screen[y][x], s_old, oldln) != 0)
				continue;
			found = true;
			break;
		}
		if (found)
			break;
	}

	if (!found)
		return false;

	char *s_start = &screen[y][x];
	char *c_start = &colors[y][x];
	for (int i = 0; i < newln; i++){
		*s_start++ = s_new[i];
		*c_start++ = c_new[i];
	}

	return true;
}

void
draw_bmp(const char *image_bits, const int image_height,
	 const int image_width, bool inv)
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
		const char *src_row = &image_bits[y * (image_width / 8)];
		int        *dst_row = &counts[(y / y_factor) * actual_w];
		for (int x = 0; x < image_width; x++) {
			bool bit = (src_row[x / 8] >>  (x % 8)) & 1;
			if (inv)
				bit = (!bit) & 1;
			dst_row[x / x_factor] += bit;
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
			uhh = max(min(uhh, mapln-1), 0);
			screen[y][x] = map[uhh];
		}

	free(counts);
}

void
render(void)
{
	clear();

	for (int y = 0; y < actual_h; y++) {
		for (int x = 0; x < actual_w; x++) {
			char pixel = screen[y][x];
			char color = colors[y][x];

			attron(COLOR_PAIR(color));
			mvaddch(y, x, pixel);
			attroff(COLOR_PAIR(color));
		}
	}

	refresh();
}

void
setup(void)
{
	srand(time(NULL));
	signal(SIGINT, finish);

	initscr();
	noecho();
	curs_set(0);
	nodelay(stdscr, true);

	start_color();
	init_pair(0, COLOR_WHITE, COLOR_BLACK);
	init_pair(1, COLOR_RED, COLOR_BLACK);
}

static void
finish(int sig)
{
	endwin();
	exit(EXIT_SUCCESS);
}

