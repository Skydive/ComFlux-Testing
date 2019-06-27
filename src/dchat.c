/*
 * dchat.c
 *
 *	Created on: 27 Jun 2018
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


typedef struct ep_node {
	ENDPOINT* val;
	struct ep_node* next;
} ep_node_t;

bool ep_node_push_back(ep_node_t* head, ENDPOINT* val) {
	node_t* cur = head;
	while(cur->next != NULL)
		cur = cur->next;
	cur->next = malloc(sizeof(ep_node_t));
	if(cur->next == NULL)
		return false;
	cur->next->val = val;
	cur->next->next = NULL;
	return true;
}

bool ep_node_push_front(ep_node_t** head, ENDPOINT* val) {
	node_t* new_node;
	new_node = malloc(sizeof(ep_node_t));
	if(new_node == NULL)
		return false;
	new_node->val = val;
	new_node->next = *head;
	*head = new_node;
}


bool ep_node_remove_entry(ep_node_t** head, ep_node_t* entry) {
	while((*head) != entry) {
		head = &(*head)->next;
		if(*head==NULL)return false;
	}
	*head=entry->next;
	return true;
}
bool ep_node_remove_val(ep_node_t** head, ENDPOINT* val) {
	while((*head)->val != val) {
		head = &(*head)->next;
		if(*head==NULL)return false;
	}
	*head=entry->next;
	return true;
}
int ep_node_len(ep_node_t* head) {
	int i=0;
	while((head = head->next) != NULL)
		i++;
	return i;
}

ep_node_t* endpoints_head = NULL;

ENDPOINT* ep_msg_src, ep_snk_assign_req, ep_snk_assign_res;



void print_message_other(time_t utime, const char* nickname, const char* message) {
	char timebuf[32];
	struct tm *ts;
	ts = localtime(&utime);
	strftime(timebuf, sizeof(timebuf), "%H:%M:%S", ts);


	printf("[%s] %s: %s\n", timebuf, nickname, message);
}


void endpoint_create_msg_sink() {

}

void msg_received_callback(MESSAGE *msg) {
	ENDPOINT *ep = msg->ep;

	JSON* elem_msg = msg->_msg_json;
	char* uid = json_get_str(elem_msg, "uid");
	time_t utime = json_get_long(elem_msg, "utime");
	char* src_address = json_get_str(elem_msg, "src_address");
	char* nickname = json_get_str(elem_msg, "nickname");
	char* msg = json_get_str(elem_msg, "text");

	print_message_other(utime, nickname, msg);
}

void response_sink_assign(MESSAGE* msg) {
	ENDPOINT* ep = msg->ep;
	JSON* elem_msg = msg->_msg_json;
	char* src_address = json_get_str(elem_msg, "src_address");


	char timebuf[32];
	time_t cur_time = time(NULL);
	struct tm *ts;
	strftime(timebuf, sizeof(timebuf), "%H:%M:%S", ts);


	printf("[%s]: [ip] has requested a connection, creating new sink. (%d)", timebuf, count);
	int count = ep_node_count(endpoints_head)+1;

	char buffer[32];
	sprintf(buffer, "ep_msg_snk_%d", );

	ENDPOINT* snk_ep = endpoint_new_snk_file(buffer, "", "example_schemata/dchat_msg.json", &msg_received_callback);
	ep_node_push_front(&endpoints_head, snk_ep);

	JSON* msg_json = json_new(NULL);
	json_set_str(msg_json, "src_address", src_address);
	json_set_int(msg_json, "sink_number", count);

	char* message = json_to_str(msg_json);
	endpoint_send_response(ep, msg->msg_id, message);

	free(message);
	json_free(msg_json);
}

void request_sink_assign(MESSAGE* msg) {
	ENDPOINT *ep = msg->ep;

	JSON* elem_msg = msg->_msg_json;
	char* sink_number = json_get_str(elem_msg, "sink_number");
	char* src_address = json_get_str(elem_msg, "src_address");


	char* buffer[32];
	sprintf(buffer, "ep_name = 'ep_msg_snk_'")

	Array *ep_query_array = array_new(ELEM_TYPE_STR);
	array_add(ep_query_array, "ep_name = 'ep_assign_snk_resp'");
	JSON *ep_query_json = json_new(NULL);
	json_set_array(ep_query_json, NULL, ep_query_array);
	char* ep_query_str = json_to_str(ep_query_json);
	char* cpt_query_str = "";

	/* map according to the query */
	int map_result = endpoint_map_to(ep_, link_addr, ep_query_str, cpt_query_str);
	printf("Map result: %d \n", map_result);
}

void handler(int signum) {
	switch(signum) {
	case SIGALRM:
		time_t cur_time = time(NULL);
		static time_t last_signal = cur_time;

		if(difftime(cur_time,last_signal) > 5.0) {
			printf("5 seconds...\n");
			last_signal = time(NULL);
		}

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

int main(int argc, char *argv[]) {
	char* mw_cfg_path = NULL;
	char* link_addr = NULL;
	if(argc < 2) {
		printf("Usage: ./dchat.out [mw_cfg_path] link_addr (optional)]\n"
				"\tmw_cfg_path		is the path to the config file for the middleware;\n"
				"\t									default improved_sink.json\n"
				"\tlink_addr			is the address of another chat we want to map to\n");
		mw_cfg_path = "improved_sink.json";
		link_addr=argv[2];
	} else {
		if(argc < 3)
			link_addr=argv[2];
		mw_cfg_path = argv[1];
	}

	int load_cfg_result = load_mw_config(mw_cfg_path);
	printf("Loading configuration: %s\n", load_cfg_result==0?"ok":"error");
	printf("\tApp log level: %d\n", config_get_app_log_lvl());

	char* app_name = mw_init("dchat", config_get_core_log_lvl(), 1);
	printf("Initialising core: %s\n", app_name!=NULL?"ok":"error");
	printf("\tApp name: %s\n", app_name);

	load_cfg_result = config_load_com_libs();
	printf("Load coms module result: %s\n", load_cfg_result==0?"ok":"error");

	ep_msg_src = endpoint_new_src_file("ep_msg_src", "snk endpoint msg src", "example_schemata/dchat_msg.json");

	ep_snk_assign_req = endpoint_new_req_file("ep_assign_snk_req", "req endpoint assign sink", "example_schemata/dchat_sink_assign_req.json", &request_sink_assign);
	ep_snk_assign_res = endpoint_new_resp_file("ep_assign_snk_res", "resp endpoint assign sink", "example_schemata/dchat_sink_assign_resp.json", &response_sink_assign);

	if(link_addr != NULL) {
		Array *ep_query_array = array_new(ELEM_TYPE_STR);
		array_add(ep_query_array, "ep_name = 'ep_assign_snk_res'");
		JSON *ep_query_json = json_new(NULL);
		json_set_array(ep_query_json, NULL, ep_query_array);
		char* ep_query_str = json_to_str(ep_query_json);
		char* cpt_query_str = "";

		/* map according to the query */
		int map_result = endpoint_map_to(ep_snk_assign_req, link_addr, ep_query_str, cpt_query_str);
		printf("Map result: %d \n", map_result);

		// Send LINK request
	}

	struct itimerval it;
	timerclear(&it.it_interval);
	timerclear(&it.it_value);
	it.it_interval.tv_usec = TIMESTEP;
	it.it_value.tv_usec		 = TIMESTEP;
	setitimer(ITIMER_REAL, &it, NULL);

	struct sigaction sa;
	sa.sa_handler = handler;
	sa.sa_flags		= 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT,	 &sa, NULL);
	sigaction(SIGALRM, &sa, NULL);
	sa.sa_handler = SIG_IGN;
	sigaction(SIGTSTP, &sa, NULL);

	while(1) {
		int key = getch();
		dir=key;
	}
}
