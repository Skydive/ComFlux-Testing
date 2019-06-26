/*
 * ncurses_block_screen.c
 *
 *	Created on: 26 Jun 2018
 *		Author: K. Aleem
 */


#include <endpoint.h>
#include <middleware.h>
#include <load_mw_config.h>

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include <stdlib.h>

#include <signal.h>


// NCURSES
#include <curses.h>
#include <sys/time.h>
#include <math.h>

#define TIMESTEP 100000


int oldcur;
int x=0;
int y=0;
int dir;
int mx, my;
WINDOW* mainwin;


void do_movement() {
	switch(dir){
		case 'w':
		case 'W':
		case KEY_UP:
			y--;
			break;
		case 's':
		case 'S':
		case KEY_DOWN:
			y++;
			break;
		case 'a':
		case 'A':
		case KEY_LEFT:
			x--;
			break;
		case 'd':
		case 'D':
		case KEY_RIGHT:
			x++;
			break;
		default:
			dir=0;
			break;
	}
}

int tick() {
	do_movement();	

	getmaxyx(stdscr, my, mx);
	if(y>=my)y=0;
	if(x>=mx)x=0;
	if(x<0)x=mx-1;
	if(y<0)y=my-1;

	clear();

	//mvprintw(y, x, "o");
	attron(COLOR_PAIR(1));
	mvaddch(y, x, ' ');
	attroff(COLOR_PAIR(1));

	//wbkgd(mainwin, COLOR_PAIR(0));

 	refresh();
}


void handler(int signum) {
	switch(signum) {
		case SIGALRM:
			tick();
			break;
		case SIGTERM:
		case SIGINT:
			if(mainwin != NULL) {
				delwin(mainwin);
    			endwin();
    			refresh();
			}
			printf("Terminating process...\n");

			exit(EXIT_SUCCESS);
			break;
	}

}

void movement_callback(MESSAGE *msg) {
	ENDPOINT *ep = msg->ep;

	JSON* elem_msg = msg->_msg_json;
	int value = json_get_int(elem_msg, "dir");
	dir=value;
	do_movement();
}

int main(int argc, char *argv[]) {
	char *mw_cfg_path = NULL;
	char *src_addr = NULL;
	if(argc < 3) {
		printf("Usage: ./ncurses_block_screen.out [mw_cfg_path] sink_addr\n"
				"\tmw_cfg_path		is the path to the config file for the middleware;\n"
				"\t                 default improved_sink.json\n"
				"\tsource_addr      is the address of the source we want to map to\n");

		mw_cfg_path = "improved_sink.json";
		src_addr=argv[1];
	} else {
		src_addr=argv[1];
		mw_cfg_path = argv[2];
	}

	int load_cfg_result = load_mw_config(mw_cfg_path);
	printf("Loading configuration: %s\n", load_cfg_result==0?"ok":"error");
	printf("\tApp log level: %d\n", config_get_app_log_lvl());

	char* app_name = mw_init("ncurses_block_screen", config_get_core_log_lvl(), 1);
	printf("Initialising core: %s\n", app_name!=NULL?"ok":"error");
	printf("\tApp name: %s\n", app_name);

	load_cfg_result = config_load_com_libs();
	printf("Load coms module result: %s\n", load_cfg_result==0?"ok":"error");



	// JSON* manifest = json_new(NULL);
	// json_set_str(manifest, "app_name", app_name);
	// json_set_str(manifest, "author", "khalid");
	// mw_add_manifest(json_to_str(manifest));
	// mw_add_rdc("comtcp", "127.0.0.1:1508");
	// mw_register_rdcs();



	ENDPOINT *ep_snk = endpoint_new_snk_file(
		"ep_dir_sink", /* name */
		"example snk endpoint", /* description */
		"example_schemata/ncurses_dir_value.json", /* message schemata */
		&movement_callback); /* handler for incoming messages */


	Array *ep_query_array = array_new(ELEM_TYPE_STR);
	array_add(ep_query_array, "ep_name = 'ep_dir_src'");
	JSON *ep_query_json = json_new(NULL);
	json_set_array(ep_query_json, NULL, ep_query_array);
	char* ep_query_str = json_to_str(ep_query_json);
	char* cpt_query_str = "";

	/* map according to the query */
	int map_result = endpoint_map_to(ep_snk, src_addr, ep_query_str, cpt_query_str);
	printf("Map result: %d \n", map_result);

	printf("Press any key to continue...\n");
	getch();

	struct itimerval it;
    timerclear(&it.it_interval);
    timerclear(&it.it_value);
    it.it_interval.tv_usec = TIMESTEP;
    it.it_value.tv_usec    = TIMESTEP;
    setitimer(ITIMER_REAL, &it, NULL);

	struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_flags   = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);
    sa.sa_handler = SIG_IGN;
    sigaction(SIGTSTP, &sa, NULL);

	if((mainwin = initscr()) == NULL) {
		fprintf(stderr, "Error initialising ncurses\n");
		exit(EXIT_FAILURE);
	}	
	noecho();
	oldcur = curs_set(0);
	keypad(mainwin, TRUE);
	
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_GREEN);


	// Initial position
	getmaxyx(stdscr, my, mx);
	x = floor(mx/2);
	y = floor(my/2);

	
	int ch;
	while(1) {
		int key = getch();
		dir=key;
	}
}
