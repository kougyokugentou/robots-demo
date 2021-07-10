//#define PLAYER 0x01
#define PLAYER 'X'
#define ROBOT 'o'
#define DEAD_ROBOT '%'
#define WALL '#'
#define BOTTOM_ROW 22
#define LAST_COLUMN 79
#define ESC 0x1b //Escape Key
#define TRUE 1
#define FALSE 0
#define LIVES 3

/* globals */
typedef struct player_data PLAYER_DATA;

struct player_data
{
	int current_x;
	int current_y;
	int NewX;
	int NewY;
	int level;
	int restart_level;
	int prev_level;
	int lives;
};

struct robot_data
{
	int number;
	int x;
	int y;
	int dead;
};

struct robot_data robot_table[21] = //20 max robots for now
{
	/* { number, x, y, dead } */
	{ 1, 0, 0, 0 },
	{ 2, 0, 0, 0 },
	{ 3, 0, 0, 0 },
	{ 4, 0, 0, 0 },
	{ 5, 0, 0, 0 },
	{ 6, 0, 0, 0 },
	{ 7, 0, 0, 0 },
	{ 8, 0, 0, 0 },
	{ 9, 0, 0, 0 },
	{ 10, 0, 0, 0 },
	{ 11, 0, 0, 0 },
	{ 12, 0, 0, 0 },
	{ 13, 0, 0, 0 },
	{ 14, 0, 0, 0 },
	{ 15, 0, 0, 0 },
	{ 16, 0, 0, 0 },
	{ 17, 0, 0, 0 },
	{ 18, 0, 0, 0 },
	{ 19, 0, 0, 0 },
	{ 20, 0, 0, 0 },
	{ 0, 0, 0, 0} //all 0's is the end
};

/* Randomizer. */
void seedrnd(void) //Seed the randomizer
{
 srand((unsigned)time(NULL));
}

int rnd(int range)
{
 return(rand()%range); //spit up random num 0-range
}

long number_mm(void)
{
	return random() >> 6;
}

int number_range(int from, int to)
{
	int power;
	int number;

	if (from == 0 && to == 0)
		return 0;

	if ( ( to = to - from + 1 ) <= 1 )
		return from;

	for ( power = 2; power < to; power <<= 1)
		;

	while ( ( number = number_mm() & (power -1 ) ) >= to )
		;

	return from+number;
}


void locate(int x, int y)
{
	move(y, x);
	refresh();
	return;
}
