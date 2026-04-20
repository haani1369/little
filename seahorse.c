#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#include <ncurses.h>

#include "img/seahorse.xbm"

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define MAX_W 2048
#define MAX_H 2048

static int	actual_w, actual_h;
static char	screen[MAX_W][MAX_H];

static char	*code;
static int	codeln;

void
draw_bmp(void)
{
	bool *idk = calloc(actual_h * actual_w, sizeof(bool));

	const int factor = max(image_height / actual_h,
			       image_width  / (actual_w / 2));
	const int y_factor = factor;
	const int x_factor = factor / 2;

	for (int y = 0; y < image_height; y++) {
		const char *src_row = &image_bits[y * (image_width / 8)];
		bool	   *dst_row = &idk[(y / y_factor) * actual_w];
		for (int x = 0; x < image_width; x++) {
			bool bit = (src_row[x / 8] >>  (x % 8)) & 1;
			dst_row[x / x_factor] |= bit;
		}
	}

	int i = 0;
	for (int y = 0; y < actual_h; y++) {
		for (int x = 0; x < actual_w; x++) {
			if (!idk[y * actual_w + x]) {
				mvaddch(y, x, ' ');
				continue;
			}
			char c = code[i];
			if (c == '\t')
				c = ' ';
			mvaddch(y, x, c);

			i = (i + 1) % codeln; 
		}
	}

	free(idk);
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
	srand(time(NULL));
	signal(SIGINT, finish);

	initscr();
	noecho();
	curs_set(0);
	nodelay(stdscr, true);

	start_color();
}

int
main(int argc, char **argv)
{
	if (argc != 2)
		return -1;

	FILE *f = fopen(argv[1], "r");

	if (f == NULL)
		exit(EXIT_FAILURE);

	fseek(f, 0, SEEK_END);
	codeln = ftell(f);
	fseek(f, 0, SEEK_SET);

	code = malloc(codeln + 1);

	fread(code, codeln, 1, f);	

	fclose(f);


	setup();

	while(getch() != 'q') {
		getmaxyx(stdscr, actual_h, actual_w);
		draw_bmp();
		usleep(400000);
	}
	
	free(code);

	finish(0);
}

