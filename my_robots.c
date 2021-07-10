#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <ncursesw/curses.h>
#include "my_robots.h"
#include "alloc_perm.h"

/* function declarations */
void init_plr_data(PLAYER_DATA *player);
void init_robots(int level);
void game_loop(PLAYER_DATA *player);
void DrawPerimeter(void);
void PlayerInput(PLAYER_DATA *player);
void RedrawScreen(PLAYER_DATA *player);
void move_ch(PLAYER_DATA *player);
int CheckWalls(PLAYER_DATA *player);
void robot_movement(PLAYER_DATA *player);
void locate(int x, int y);
int rnd(int range);

/* curses
int clear(void); //clears the screen
int move(int y, int x); //move the cursor
int getch(void); //gets a character
int refresh(void); //must be called to get output to the terminal
chtype winch(WINDOW *win); //get character at current cursor location
int addstr(const char *str); //curses printf
int addch(const ctype *ch); //curses putchar (dont move anything)
int insch(ctype *ch); //putchar (move everything right, use after delch)
int delch(void) //curses 'delete character' (moves everything left)

*/

/* global vars */
int game_over;
int num_robots;
WINDOW *robot_game;

/* Begin Program */
int main(void)
{
	PLAYER_DATA *player;
	game_over = FALSE;
	player = alloc_perm(sizeof(*player));
//	malloc(sizeof(*player));

	/* init curses */
	robot_game = initscr();
	cbreak();
	noecho();
	nonl();
	intrflush(robot_game, FALSE);
//    keypad(robot_game, TRUE);
	leaveok(robot_game, TRUE);
	
	/* GO TIME */	
	clear();
	refresh();
	DrawPerimeter();
	init_plr_data(player);
	init_robots(player->level);

	do
	{
		game_loop(player);
	}
	while(!game_over);
	locate(0,BOTTOM_ROW+1);
	addstr("GAME OVER press enter to exit the game.");
	getch();
	
	/* uninit curses */
	nocbreak();
	echo();
	nl();
	leaveok(robot_game, FALSE);
	clear();
	refresh();
	locate(0,0);
	endwin();
	
	return 0;
}

void init_plr_data(PLAYER_DATA *player)
{
	player->level = 1;
	player->prev_level = 1;
	player->restart_level = FALSE;
	player->lives = LIVES;
	player->current_x = 40;
	player->current_y = 12;
	player->NewX = player->current_x;
	player->NewY = player->current_y;
	locate(player->current_x, player->current_y);
	addch(PLAYER);
	return; //game_loop
}

void init_robots(int level)
{
    int rnd_x;
    int rnd_y;
    int robot = 0;


    seedrnd();
	
	switch(level)
	{
		case 1:
		case 2:
		case 3:
		case 4:
    		num_robots = rnd(6); //up to 6 robots at first.
			break;
		
		case 5:
		case 6:
		case 7:
		case 8:
			num_robots = rnd(10)+5; //level 5 - 10 robots
			break;
		
		case 9:
		case 10:
		case 11:
			num_robots = rnd(13)+9; //level 9 - 13 robots
			break;
		
		case 12:
		case 13:
		case 14:
			num_robots = rnd(18)+12; //level 12 - 18 robots
			break;

		case 15:
		default:
			num_robots = rnd(20)+15; //level 15 - 20 robots
			break;
	}
	
	/* make it possible to win */
	if(num_robots <= 1)
		num_robots = 2;

    for(robot = 0; robot < num_robots; robot++)
    {
		/* do not place a dead robot -- for use in RedrawScreen
		if(robot_table[robot].dead == 1)
			continue;
		*/
		
		for(;;)
		{
    		rnd_x = number_range(2, LAST_COLUMN-1);
    		rnd_y = number_range(2, BOTTOM_ROW-1);

			locate(rnd_x, rnd_y);
			if( (winch(robot_game) & A_CHARTEXT) == ROBOT
			||  (winch(robot_game) & A_CHARTEXT) == PLAYER)
				continue;

			break;
		}
		
		/* Place the robots. */
		robot_table[robot].x = rnd_x;
		robot_table[robot].y = rnd_y;
		robot_table[robot].dead = 0;
		addch(ROBOT);
		refresh();
    }
	return;
}

void DrawPerimeter(void)
{
    int DrawX;
    int DrawY;
   
    for(DrawX = 0; DrawX <= LAST_COLUMN; DrawX++)
    {
        locate(DrawX,0);
        addch(WALL);
        locate(DrawX,BOTTOM_ROW);
        addch(WALL);
    }

    for(DrawY = 1; DrawY < BOTTOM_ROW; DrawY++)
    {
        locate(0,DrawY);
        addch(WALL);
        locate(LAST_COLUMN,DrawY);
        addch(WALL);
    }
    return;

}

void game_loop(PLAYER_DATA *player)
{
	locate(player->current_x, player->current_y);
    PlayerInput(player);
    robot_movement(player);
	
	/* Level Up and Player Death */
	if(player->level > player->prev_level //level up
	|| player->restart_level == TRUE) //player death
	{
		player->prev_level = player->level;
		clear();
		refresh();
		DrawPerimeter();
		player->restart_level = FALSE;
		player->current_x = 40;
		player->current_y = 12;
		locate(player->current_x, player->current_y);
		addch(PLAYER);
		init_robots(player->level);
	}

    return;
}

/* Move all robots towards the player. */
void robot_movement(PLAYER_DATA *player)
{
	int robot = 0;
	int num_robots_left = 0;
	int dead_robot = 0;
	
	if(game_over)
		return;

	for(robot = 0; robot < num_robots; robot++)
	{
		/* if the robot is destroyed, skip it. */
		if(robot_table[robot].dead == TRUE)
			continue;

		num_robots_left++;
	}

	/* If all the robots are dead, we have passed the level.
	 * May give a BUG where robots will no longer move after
	 * one or more robots are destroyed */	
	if(num_robots_left == 0)
		return;

	for(robot = 0; robot < num_robots; robot++)
	{
		/* if the robot is destroyed, skip it. */
		if(robot_table[robot].dead == TRUE)
			continue;
		
		locate(robot_table[robot].x, robot_table[robot].y);
		delch();
		insch(' ');
		refresh();
		//printf("\b \b"); //erase robot character

		/* move robot left */
		if(robot_table[robot].x > player->current_x)
		{
			robot_table[robot].x -= 1;
		}
		
		/* move robot right */
		if(robot_table[robot].x < player->current_x)
		{
			robot_table[robot].x += 1;
		}

		/* move robot up */
		if(robot_table[robot].y > player->current_y)
		{
			robot_table[robot].y -=1;
		}

		/* move robot down */
		if(robot_table[robot].y < player->current_y)
		{
			robot_table[robot].y += 1;
		}

		locate(robot_table[robot].x, robot_table[robot].y);
/*		delch(); //erase whatever's there
		insch(ROBOT);
*/
		/* Player is DEAD!!! */
		if(robot_table[robot].x == player->current_x
		&& robot_table[robot].y == player->current_y)
		{
			player->lives--;
			player->restart_level = TRUE;

			if(player->lives <= 0) //Game Over
			{
				game_over = TRUE;
				return;
			}
		}

		/* Robot runs into another robot, destroy them. */
		if( (winch(robot_game) & A_CHARTEXT) == ROBOT
		||  (winch(robot_game) & A_CHARTEXT) == DEAD_ROBOT)
		{
			delch();
			insch(DEAD_ROBOT);
			refresh();
			robot_table[robot].dead = TRUE;
			for(dead_robot = 0; dead_robot < num_robots; dead_robot++)
			{	/* robot = already dead (above)   dead_robot = dying robot */
				if(robot_table[robot].number == robot_table[dead_robot].number)
					continue;
				
				if(robot_table[robot].x == robot_table[dead_robot].x
				&& robot_table[robot].y == robot_table[dead_robot].y)
				{
					robot_table[dead_robot].dead = TRUE; //We ran into this robot.
				}
			}	
		}
		
		/* Place the live robot*/
		if(!robot_table[robot].dead)
		{
			delch();
			insch(ROBOT);
			refresh();
		}

	} //end for

	/* end robot movement */
	return;
}

void PlayerInput(PLAYER_DATA *player)
{
    char key;
    int OldX;
    int OldY;
    int rnd_x;
    int rnd_y;
	int robot = 0;
	int num_robots_left = 0;

    OldX = player->current_x;
    OldY = player->current_y;
    player->NewX = player->current_x;
    player->NewY = player->current_y;
    
    seedrnd();

	/* This is here as a cheap hack so you dont have to move again
	 * after the level is cleared of all robots */
	for(robot = 0; robot < num_robots; robot++)
	{
		/* if the robot is destroyed, skip it. */
		if(robot_table[robot].dead == TRUE)
			continue;

		num_robots_left++;
	}

	/* If there are no more robots left, win level. */		
	if(num_robots_left == 0)
	{
		player->level++;
		return;
	}

   	for(;;)
    {
    	rnd_x = number_range(2, LAST_COLUMN-1);
    	rnd_y = number_range(2, BOTTOM_ROW-1);

		locate(rnd_x, rnd_y);
		
		if(rnd_x == player->current_x
		&& rnd_y == player->current_y)
			continue;
		
		if( (winch(robot_game) & A_CHARTEXT) != ROBOT
		&&  (winch(robot_game) & A_CHARTEXT) != DEAD_ROBOT
		&&  (winch(robot_game) & A_CHARTEXT) != PLAYER
		&&  (winch(robot_game) & A_CHARTEXT) != WALL )
			break;
    }

    key = getch();
    if(key == ESC)
    {
        game_over = TRUE;
        return;
    }
    
    switch(toupper(key))
    {
		case ESC:
			game_over = TRUE;
			break;
		
        case 'W':
             player->NewY = OldY-1;
             if( !CheckWalls(player) ) move_ch(player);
             break;

        case 'A':
             player->NewX = OldX-1;
             if( !CheckWalls(player) ) move_ch(player);
             break;
             
        case 'S':
             player->NewY = OldY+1;
             if( !CheckWalls(player) ) move_ch(player);
             break;
             
        case 'D':
             player->NewX = OldX+1;
             if( !CheckWalls(player) ) move_ch(player);
             break;
             
		case 'Q':
		     player->NewX = OldX-1;
		     player->NewY = OldY-1;
             if( !CheckWalls(player) ) move_ch(player);
             break;
             
		case 'E':
	    	 player->NewX = OldX+1;
	    	 player->NewY = OldY-1;
             if( !CheckWalls(player) ) move_ch(player);
             break;
             
		case 'Z':
	    	 player->NewX = OldX-1;
	    	 player->NewY = OldY+1;
             if( !CheckWalls(player) ) move_ch(player);
             break;
             
		case 'X':
		case 'C':
		     player->NewX = OldX+1;
	    	 player->NewY = OldY+1;
             if( !CheckWalls(player) ) move_ch(player);
             break;
			 
 		case 'R':
			player->NewX = rnd_x;
			player->NewY = rnd_y;
			if( !CheckWalls(player) ) move_ch(player);
			break;
			
/*        case 'H':
             locate(0,1);
             printf("\n\r# -= HELP=-");
             printf("\n\r#  1. Controls:");
             printf("\n\r#    ESC KEY: Quit the game.");
             printf("\n\r#    W - Up; A - Left; S - Down; D - Right");
             printf("\n\r#\n\r# Press Enter to continue with the game.");
             getch();
             RedrawScreen(player);
             break;
*/             
        default:
/*           printf("\n\r# I don't know that command. Press any key to continue, then Type H for help.");
             getch();
             RedrawScreen(player); */
             break;
    }
    return;
}

void move_ch(PLAYER_DATA *player)
{
	locate(player->current_x, player->current_y);
	delch();
	insch(' ');
	refresh();
    player->current_x = player->NewX;
    player->current_y = player->NewY;
    locate(player->current_x, player->current_y);
    addch(PLAYER);
    return;
}

int CheckWalls(PLAYER_DATA *player)
{
    int CheckX;
    int CheckY;

    CheckX = player->NewX;
    CheckY = player->NewY;

	locate(CheckX, CheckY);
    if( (winch(robot_game) & A_CHARTEXT) == WALL
	||  (winch(robot_game) & A_CHARTEXT) == ROBOT
	||  (winch(robot_game) & A_CHARTEXT) == DEAD_ROBOT
    || CheckX < 0 || CheckX > LAST_COLUMN-1
    || CheckY < 0 || CheckY > BOTTOM_ROW-1 )
          return(TRUE);
          
    return(FALSE);
}
