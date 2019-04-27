/**
 * COMP30023 Project 1
 * This header file imageTaggerConstant.h stores constants and is used in image_taggerServer.c 
 * Name: Yuqiang Zhu. ID: 853912
 * Email: yuqiangz@student.unimelb.edu.au
 */
 
static char const * const HTTP_200_FORMAT = "HTTP/1.1 200 OK\r\n\
Content-Type: text/html\r\n\
Content-Length: %ld\r\n\r\n";
static char const * const HTTP_200_FORMAT_NEED_COOKIE = "HTTP/1.1 200 OK\r\n\
Content-Type: text/html\r\n\
Content-Length: %ld\r\n";
static char const * const HTTP_400 = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
static int const HTTP_400_LENGTH = 47;
static char const * const HTTP_404 = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
static int const HTTP_404_LENGTH = 45;

#define WELCOME_PAGE "1_intro.html"
#define MAIN_MENU_PAGE "2_start.html"
#define GAME_PLAYING_PAGE "3_first_turn.html"
#define KEYWORD_ACCEPTED_PAGE "4_accepted.html"
#define KEYWORD_DISCARDED_PAGE "5_discarded.html"
#define GAME_COMPLETED_PAGE "6_endgame.html"
#define GAME_OVER_PAGE "7_gameover.html"
#define NUM_PLAYERS 2