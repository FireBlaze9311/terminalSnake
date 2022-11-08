#include <string>
#include <ncurses.h>
#include <unistd.h>
#include <deque>
#include <stdlib.h>
#include <locale.h>
#include <ctime>

#include "game.h"

#define AREA_X1 5
#define AREA_Y1 2
#define AREA_X2 50
#define AREA_Y2 20

#define BACKGROUND_PAIR 1
#define SNAKE_BODY_PAIR 2

typedef struct
{
    uint_fast8_t x;
    uint_fast8_t y;
} vec2ui;

typedef struct
{
    int_fast8_t x;
    int_fast8_t y;
} vec2i;

WINDOW *wnd;
vec2ui applePos;
std::deque<vec2ui> snake;
u_int8_t score = 0;

void rectangle(int y1, int x1, int y2, int x2)
{
    mvhline(y1, x1, 0, x2 - x1);
    mvhline(y2, x1, 0, x2 - x1);
    mvvline(y1, x1, 0, y2 - y1);
    mvvline(y1, x2, 0, y2 - y1);
    mvaddch(y1, x1, ACS_ULCORNER);
    mvaddch(y2, x1, ACS_LLCORNER);
    mvaddch(y1, x2, ACS_URCORNER);
    mvaddch(y2, x2, ACS_LRCORNER);
}

int init()
{
    setlocale(LC_ALL, "");
    wnd = initscr();
    cbreak();
    noecho();
    clear();
    refresh();
    keypad(wnd, true);
    nodelay(wnd, true);
    curs_set(0);

    if (!has_colors())
    {
        endwin();
        printf("ERROR: Terminal does not support color.\n");
        exit(1);
    }
    start_color();

    init_pair(BACKGROUND_PAIR, COLOR_BLACK, COLOR_BLUE);
    init_pair(SNAKE_BODY_PAIR, COLOR_GREEN, COLOR_GREEN);

    srand(time(NULL));

    return 0;
}

void setScore(u_int8_t score)
{
    mvprintw(AREA_Y1, AREA_X2 - 9, "Score: %d", score);
}

bool checkCollisionWalls()
{
    if (snake.back().x >= AREA_X2 || snake.back().x <= AREA_X1 ||
        snake.back().y >= AREA_Y2 || snake.back().y <= AREA_Y1)
    {
        return true;
    }
    return false;
}

void lost()
{
    mvaddstr(AREA_Y2 + 2, AREA_X1+1, "You lost! Press q to exit.");
    refresh();
    int in_char;
    while(1){
        in_char = wgetch(wnd);
        if(in_char == 'q'){
            close();
            break;
        }
        usleep(100000);
    }
}

void spawnApple()
{
    uint8_t randX = rand() % (AREA_X2 - AREA_X1 - 1) + AREA_X1 + 1;
    uint8_t randY = rand() % (AREA_Y2 - AREA_Y1 - 1) + AREA_Y1 + 1;
    const cchar_t apple = {A_NORMAL, L"🍎"};
    mvadd_wch(randY, randX, &apple);
    applePos.x = randX;
    applePos.y = randY;
}

bool checkOwnCollision()
{
    for (size_t i = 0; i < snake.size() - 1; i++)
    {
        if (snake.back().x == snake[i].x && snake.back().y == snake[i].y)
        {
            return true;
        }
    }
    return false;
}

void checkApple(vec2i direction)
{
    if (snake.back().x == applePos.x && snake.back().y == applePos.y)
    {
        score++;
        setScore(score);

        vec2i lastDirection = {0, 0};
        if (snake.size() > 1 && snake[0].y == snake[1].y)
        {
            lastDirection.x = snake[0].x > snake[1].x ? 1 : -1;
        }
        else
        {
            lastDirection.y = snake[0].y > snake[1].y ? 1 : -1;
        }
        snake.push_front((vec2ui){snake.front().x + lastDirection.x, snake.front().y + lastDirection.y});
        spawnApple();
    }
}

void moveSnake(vec2i direction)
{
    attron(COLOR_PAIR(BACKGROUND_PAIR));
    vec2ui last = snake.front();
    snake.pop_front();
    mvaddch(last.y, last.x, ' ');
    attroff(COLOR_PAIR(BACKGROUND_PAIR));

    vec2ui newHead = {snake.back().x + direction.x, snake.back().y + direction.y};
    snake.push_back(newHead);
    attron(COLOR_PAIR(SNAKE_BODY_PAIR));
    mvaddch(newHead.y, newHead.x, ' ');
    attroff(COLOR_PAIR(SNAKE_BODY_PAIR));
}

void run()
{
    wbkgd(wnd, COLOR_PAIR(BACKGROUND_PAIR));

    rectangle(AREA_Y1, AREA_X1, AREA_Y2, AREA_X2);
    mvaddstr(AREA_Y1, AREA_X1, "Snake Game");

    setScore(0);
    snake.push_back({((AREA_X1 + AREA_X2) / 2) + 1, (AREA_Y1 + AREA_Y2) / 2});
    snake.push_back({(AREA_X1 + AREA_X2) / 2, (AREA_Y1 + AREA_Y2) / 2});

    int in_char;
    bool exit_requested = false;

    vec2i direction;
    direction.x = 1;
    direction.y = 0;

    useconds_t speed = 10;

    spawnApple();

    while (1)
    {
        in_char = wgetch(wnd);

        switch (in_char)
        {
        case 'q':
            exit_requested = true;
            break;
        case KEY_UP:
        case 'w':
            direction.x = 0;
            direction.y = -1;
            break;
        case KEY_DOWN:
        case 's':
            direction.x = 0;
            direction.y = 1;
            break;
        case KEY_LEFT:
        case 'a':
            direction.x = -1;
            direction.y = 0;
            break;
        case KEY_RIGHT:
        case 'd':
            direction.x = 1;
            direction.y = 0;
            break;
        default:
            break;
        }

        flushinp();

        moveSnake(direction);

        if (checkCollisionWalls() || checkOwnCollision())
        {
            lost();
            exit_requested = true;
        }

        checkApple(direction);

        usleep(speed * 10000);

        refresh();

        if (exit_requested)
            break;
    }
}

void close()
{
    endwin();
}
