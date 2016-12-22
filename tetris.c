#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <curses.h>

#define ROW_NUM				11
#define COLUMN_NUM			10
#define BOUNDARY_LEFT 		( START_COLUMN + COL(0) )
#define BOUNDARY_RIGHT 		( START_COLUMN + COL(COLUMN_NUM-1) )
#define BOUNDARY_BOTTOM 	( START_ROW + ROW(ROW_NUM-1) )

#define DIR_LEFT			1
#define DIR_RIGHT			2
#define WEIYI_ADD			1
#define WEIYI_MINUS			2

#define COL(x)				((x) * DIST + 1)
#define ROW(y)				((y) * DIST + 1)

#define SQUARE_TYPE_NUM		6
#define SQUARE_NO_NUM		4
#define POINT_PER_SQUARE 	4
#define ROTATE_NUM			4
#define PER_POINT_DISTANCE	2
#define DIST 				PER_POINT_DISTANCE
#define ROTATE_POINT 		1

#define START_ROW			0
#define START_COLUMN		10
#define X0					( START_COLUMN + COL(COLUMN_NUM/2-1) )
#define Y0					( START_ROW + ROW(0) )
#define Y1 					( START_ROW + ROW(1) )
#define Y2 					( START_ROW + ROW(2) )
#define START_NEXT_X 		(BOUNDARY_RIGHT + 1 + 5)
#define START_NEXT_Y 		(START_ROW + 0)

#define CHAR_SQUARE			'O'
#define CHAR_BLANK 			' '
#define CHAR_NEXT_1 		"---------"
#define CHAR_NEXT_2 		"| | | | |"
#define CHAR_ROW_1 			"---------------------"
#define CHAR_ROW_2 			"| | | | | | | | | | |"

void speed_up();
void key_ctl();
int is_horizon(int flag, int idx);
int out_horizon(int dir_horizon);
void move_horizon(int flag);
int is_recovery(int idx);
int is_bottom(int idx);
void remove_row(int row);
int out_bottom();
int is_full_row(int row);
void update_background();
void move_down(int sig);
void weiyi_compute(int compute);
int out_change();
void change();
void generate_next();
void draw_next();
void draw_map();
void draw_square();
void clear_square();
void next_to_current();
void new_square();
int is_current_fail();
void init_square();
void init();
void game_over();
int set_ticker(int n_msec);

struct point {
	int x, y;
};

struct square {
	int type;
	int no;
	struct point p[POINT_PER_SQUARE];
};

struct point init_point[SQUARE_TYPE_NUM][ROTATE_NUM] = {
	{X0,Y0, 	X0,Y0, 		X0,Y0, 		X0,Y0},
	{X0,Y0,		X0,Y1,		X0,Y0,		X0,Y2},
	{X0,Y0,		X0,Y1,		X0,Y1,		X0,Y1},
	{X0,Y0,		X0,Y1,		X0,Y1,		X0,Y1},
	{X0,Y1,		X0,Y2,		X0,Y0,		X0,Y0},
	{X0,Y1,		X0,Y0,		X0,Y0,		X0,Y2}
};

struct point weiyi[SQUARE_TYPE_NUM][ROTATE_NUM][POINT_PER_SQUARE] = 
{
	{
		{-2,0,		0,0,		-2,2,		0,2},
		{-2,0,		0,0,		-2,2,		0,2},
		{-2,0,		0,0,		-2,2,		0,2},
		{-2,0,		0,0,		-2,2,		0,2},
	},
	{
		{-2,0,	 	0,0, 		2,0,	 	4,0},
		{0,-2,	 	0,0,	 	0,2,	 	0,4},
		{2,0,	 	0,0, 		-2,0,	 	-4,0},
		{0,2,	 	0,0,	 	0,-2,	 	0,-4}
	},
	{
		{-2,0,	 	0,0, 		0,2,	 	2,2},
		{0,-2,	 	0,0, 		-2,0,	 	-2,2},
		{2,0,	 	0,0, 		0,-2, 		-2,-2},
		{0,2,	 	0,0, 		2,0, 		2,-2}
	},
	{
		{2,0,	 	0,0, 		0,2, 		-2,2},
		{0,2,	 	0,0,	 	-2,0,	 	-2,-2},
		{-2,0,	 	0,0, 		0,-2,	 	2,-2},
		{0,-2,	 	0,0,	 	2,0, 		2,2}
	},
	{
		{0,-2,	 	0,0, 		-2,0,	 	-4,0},
		{2,0,	 	0,0,	 	0,-2,	 	0,-4},
		{0,2, 		0,0,	 	2,0,	 	4,0},
		{-2,0,	 	0,0, 		0,2,	 	0,4}
	},
	{
		{0,-2,	 	0,0,	 	2,0,	 	4,0},
		{2,0,	 	0,0,	 	0,2,	 	0,4},
		{0,2,	 	0,0,		-2,0,	 	-4,0},
		{-2,0,	 	0,0, 		0,-2,	 	0,-4}
	}
};

struct square current, next;
int on_speed = 0;

int main()
{
	init();
	while(1) {
		key_ctl();
	}
	return 0;
}

void speed_up()
{
	on_speed = 1;
	set_ticker(50);
	while(on_speed);
	set_ticker(500);
}

void key_ctl()
{
	int ch = getch();
	switch (ch) {
		case 'W':
		case 'w':
			change();
			break;
		case 'A':
		case 'a':
			move_horizon(DIR_LEFT);
			break;
		case 'D':
		case 'd':
			move_horizon(DIR_RIGHT);
			break;
		case 'S':
		case 's':
			speed_up();
			break;
		case ' ':
			set_ticker(0);
			while( (ch = getch()) != ' ');
			set_ticker(500);
			break;
		case 'q':
		case 'Q':
			game_over();
			break;
		default:
			break;
	}
}


int is_horizon(int flag, int idx)
{
	int i;

	if (DIR_LEFT == flag) {
		if (current.p[idx].x < BOUNDARY_LEFT)
			return 1;
	}
	else if (DIR_RIGHT == flag) {
		if (current.p[idx].x > BOUNDARY_RIGHT)
			return 1;
	}
	return 0;
}


int out_horizon(int dir_horizon)
{
	int i;

	for (i = 0; i < POINT_PER_SQUARE; i++) {
		if (is_horizon(dir_horizon, i) || is_recovery(i))
			return 1;
	}
	return 0;
}

void move_horizon(int flag)
{
	int i;

	if (DIR_LEFT == flag) {
		clear_square();
		for (i = 0; i < POINT_PER_SQUARE; i ++) {
			current.p[i].x -= DIST;
		}

		if (!out_horizon(DIR_LEFT)) {
			draw_square();
		}
		else {
			for (i = 0; i < POINT_PER_SQUARE; i ++) {
				current.p[i].x += DIST;
			}
			draw_square();
		}
	}
	else if (DIR_RIGHT == flag) {
		clear_square();
		for (i = 0; i < POINT_PER_SQUARE; i ++) {
			current.p[i].x += DIST;
		}

		if (!out_horizon(DIR_RIGHT)) {
			draw_square();
		}
		else {
			for (i = 0; i < POINT_PER_SQUARE; i ++) {
				current.p[i].x -= DIST;
			}
			draw_square();
		}
	}
}

int is_recovery(int idx)
{
	move(current.p[idx].y, current.p[idx].x);
	if ((char)inch() == CHAR_SQUARE) 
		return 1;
	else 
		return 0;
}

int is_bottom(int idx)
{
	if (current.p[idx].y > BOUNDARY_BOTTOM) 
		return 1;
	else 
		return 0;
}

int out_bottom()
{
	int i;

	for (i = 0; i < POINT_PER_SQUARE; i++) {
		if (is_bottom(i) || is_recovery(i)) {
			return 1;
		}
	}
	return 0;
}

void remove_row(int row)
{
	int x, y;
	char c;

	for (y = row; y > START_ROW; y -= DIST) {
		for (x = BOUNDARY_LEFT; x <= BOUNDARY_RIGHT; x += DIST) {
			move(y-DIST, x);
			c = (char)inch();
			move(y, x);
			addch(c);
		}
	}
	for (x = BOUNDARY_LEFT; x <= BOUNDARY_RIGHT; x += DIST) {
		move(START_ROW, x);
		addch(CHAR_BLANK);
	}
	refresh();
}

int is_full_row(int row)
{
	int x;

	for (x = BOUNDARY_LEFT; x <= BOUNDARY_RIGHT; x += DIST) {
		move(row, x);
		if ((char)inch() != CHAR_SQUARE)
			return 0;
	}
	return 1;
}

void update_background()
{
	int i, j, x, y;
	int max_row = -1;

	for (i = 0; i < POINT_PER_SQUARE; i ++) {
		if (is_full_row(current.p[i].y)) {
			if (current.p[i].y > max_row)
				max_row = current.p[i].y;
		}
	}
	if (max_row != -1)
		remove_row(max_row);
}

void move_down(int sig)
{
	int i;

	clear_square();
	for (i = 0; i < POINT_PER_SQUARE; i ++) {
		current.p[i].y += DIST;
	}

	if (!out_bottom()) {
		draw_square();
	}
	else {
		for (i = 0; i < POINT_PER_SQUARE; i ++) {
			current.p[i].y -= DIST;
		}
		draw_square();

		if (on_speed)
			on_speed = 0;
		update_background();
		new_square();
		draw_square();
	}
}

void weiyi_compute(int flag)
{
	int i;

	if (WEIYI_ADD == flag) {
		current.no = (current.no+1) % ROTATE_NUM;
		for (i = 0; i < POINT_PER_SQUARE; i ++) {
			current.p[i].x = current.p[ROTATE_POINT].x + 
				weiyi[current.type][current.no][i].x;
			current.p[i].y = current.p[ROTATE_POINT].y + 
				weiyi[current.type][current.no][i].y;
		}
	}
	else if (WEIYI_MINUS == flag) {
		current.no = (current.no-1) % ROTATE_NUM;
		for (i = 0; i < POINT_PER_SQUARE; i ++) {
			current.p[i].x = current.p[ROTATE_POINT].x + 
				weiyi[current.type][current.no][i].x;
			current.p[i].y = current.p[ROTATE_POINT].y + 
				weiyi[current.type][current.no][i].y;
		}
	}
}

int out_change() 
{
	int i;

	for (i = 0; i < POINT_PER_SQUARE; i ++) {
		if (out_bottom() || out_horizon(DIR_LEFT) || 
				out_horizon(DIR_RIGHT))
			return 1;
	}
	return 0;
}

void change()
{
	int i;

	clear_square();
	weiyi_compute(WEIYI_ADD);
	if (out_change()) {
		weiyi_compute(WEIYI_MINUS);
	}
	draw_square();
}

void game_over()
{
	set_ticker(0);
	sleep(1);
	endwin();
	exit(0);
}

void generate_next()
{
	int i, j, k;

	i = rand() % SQUARE_TYPE_NUM;
	j = rand() % SQUARE_NO_NUM;
	next = (struct square){i, j, 0,0, init_point[i][j], 0,0, 0,0};

	for (k = 0; k < POINT_PER_SQUARE; k++) {
		next.p[k].x = next.p[ROTATE_POINT].x + 
			weiyi[next.type][next.no][k].x;
		next.p[k].y = next.p[ROTATE_POINT].y + 
			weiyi[next.type][next.no][k].y;

	}
}

void draw_map()
{
	int i;

	move(START_ROW, START_COLUMN);
	printw(CHAR_ROW_1);
	for (i = START_ROW+1; i <= (START_ROW+ROW_NUM*DIST); i += 2) {
		move(START_ROW+i, START_COLUMN);
		printw(CHAR_ROW_2);
		move(START_ROW+i+1, START_COLUMN);
		printw(CHAR_ROW_1);
	}
	refresh();
}

void draw_next()
{
	int i, min_x, offset_x;

	move(START_NEXT_Y, START_NEXT_X);
	printw(CHAR_NEXT_1);
	for (i = START_NEXT_Y+1; i <= (START_NEXT_Y+4*DIST); i += 2) {
		move(i, START_NEXT_X);
		printw(CHAR_NEXT_2);
		move(i+1, START_NEXT_X);
		printw(CHAR_NEXT_1);
	}

	min_x = next.p[0].x;
	for (i = 1; i < POINT_PER_SQUARE; i ++) {
		if (min_x > next.p[i].x)
			min_x = next.p[i].x;
	}
	offset_x = START_NEXT_X + 1 - min_x;

	for (i = 0; i < POINT_PER_SQUARE; i ++) {
		move(next.p[i].y, next.p[i].x+offset_x);
		addch(CHAR_SQUARE);
	}

	refresh();
}

void draw_square()
{
	int i;
	for (i = 0; i < POINT_PER_SQUARE; i++) {
		move(current.p[i].y, current.p[i].x);
		addch(CHAR_SQUARE);
	}
	refresh();
}

void clear_square()
{
	int i;
	for (i = 0; i < POINT_PER_SQUARE; i++) {
		move(current.p[i].y, current.p[i].x);
		addch(CHAR_BLANK);
	}
	refresh();
}

void next_to_current()
{
	memcpy(&current, &next, sizeof(struct square));
}

int is_current_fail()
{
	int i;

	for (i = 0; i < POINT_PER_SQUARE; i ++) {
		move(current.p[i].y, current.p[i].x);
		if ((char)inch() == CHAR_SQUARE) {
			return 1;
		}
	}
	return 0;
}

void new_square()
{
	next_to_current();
	if (is_current_fail())
		game_over();
	generate_next();
	draw_next();
}

void init_square()
{
	generate_next();
	new_square();
}

void init()
{
	initscr();
	clear();
	cbreak();
	noecho();
	curs_set(0);
	srand(time(0));
	
	draw_map();
	init_square();
	draw_square();
	signal(SIGALRM, move_down);
	set_ticker(500);
}

int set_ticker(int n_msec)
{
	struct itimerval timeset;
	long n_sec, n_usec;

	n_sec = n_msec / 1000;
	n_usec = (n_msec % 1000) * 1000L;

	timeset.it_interval.tv_sec = n_sec;
	timeset.it_interval.tv_usec = n_usec;

	timeset.it_value.tv_sec = n_sec;
	timeset.it_value.tv_usec = n_usec;

	return setitimer(ITIMER_REAL, &timeset, NULL);
}
