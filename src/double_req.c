#include <endpoint.h>
#include <middleware.h>
#include <load_mw_config.h>

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

void print_callback_a(MESSAGE *msg)
{
	ENDPOINT *ep = msg->ep;

	/* parsing the message and extracting the values */
	JSON* elem_msg = msg->_msg_json;
	// int value = json_get_int(elem_msg, "value");
	// char* datetime = json_get_str(elem_msg, "datetime");

	/* process the extracted values */
	printf("A: REQUEST CALLBACK: %s\n", json_to_str(elem_msg));
}

void print_callback_b(MESSAGE *msg)
{
	ENDPOINT *ep = msg->ep;

	/* parsing the message and extracting the values */
	JSON* elem_msg = msg->_msg_json;
	// int value = json_get_int(elem_msg, "value");
	// char* datetime = json_get_str(elem_msg, "datetime");

	/* process the extracted values */
	printf("B: REQUEST CALLBACK: %s\n", json_to_str(elem_msg));
}

int main(int argc, char* argv[])
{
	char *mw_cfg_path = NULL;
	char *src_addr = NULL;

	if (argc < 3) {
		printf("Usage: ./simple_req [mw_cfg_path] addr\n"
			   "\tmw_cfg_path     is the path to the config file for the middleware;\n"
			   "\t                default mw_cfg.json\n"
			   "\tsink_addr       is the address of the response component\n");

		mw_cfg_path = "mw_cfg.json";
		src_addr = argv[2];

	} else {
		src_addr = argv[2];
		mw_cfg_path = argv[1];
	}

	/* load and apply configuration */
	int load_cfg_result = load_mw_config(mw_cfg_path);
	printf("Loading configuration: %s\n", load_cfg_result==0?"ok":"error");
	printf("\tApp log level: %d\n", config_get_app_log_lvl());

	/* start core */
	char* app_name = mw_init("request_cpt", config_get_core_log_lvl(), 1);
	printf("Initialising core: %s\n", app_name!=NULL?"ok":"error");
	printf("\tApp name: %s\n", app_name);

	/* load coms modules for the core */
	load_cfg_result = config_load_com_libs();
	printf("Load coms module result: %s\n", load_cfg_result==0?"ok":"error");

	/* Declare and register endpoints */
	ENDPOINT* ep_req_a = endpoint_new_req_file(
			"ep_req_a",
			"example req endpoint",
			"example_schemata/datetime_value.json", /* request schemata */
			"example_schemata/datetime_value.json", /* response schemata */
			&print_callback_a);
	ENDPOINT* ep_req_b = endpoint_new_req_file(
			"ep_req_b",
			"example req endpoint",
			"example_schemata/datetime_value.json", /* request schemata */
			"example_schemata/datetime_value.json", /* response schemata */
			&print_callback_b);

	int map_result = endpoint_map_to(ep_req_a, src_addr, "[ \"ep_name = 'ep_resp_a'\" ]", "");
	printf("Map result: %d \n", map_result);
	map_result = endpoint_map_to(ep_req_b, src_addr, "[ \"ep_name = 'ep_resp_b'\" ]", "");
	printf("Map result: %d \n", map_result);

	time_t rawtime;
	struct tm* timeinfo;

	JSON* msg_json;
	char* message;
	char* msgid;

	int i = 0;
	while (i < 1000)
	{
			i++;

			time ( &rawtime );
			timeinfo = localtime ( &rawtime );

			msg_json = json_new(NULL);
			json_set_int(msg_json, "value", rand() % 10);
			json_set_str(msg_json, "datetime", asctime(timeinfo));

			message = json_to_str(msg_json);
			msgid = endpoint_send_request(ep_req_a, message);
			printf("A: Request %s: %s\n", msgid, message);
			free(msgid);
			free(message);

			//TODO: SLEEP REQUIRED. (3 secs - 0.5s not enough)
			//TODO: Fix double free/segfault caused by FIXING merging messages w/o sleep
			//TODO: Is this problem is independent of the queue-fix? But we only have one thread writing to it, one reading from it.
			//TODO: !?!?
			sleep(3);

			message = json_to_str(msg_json);
			msgid = endpoint_send_request(ep_req_b, message);
			printf("B: Request %s: %s\n", msgid, message);
			free(msgid);
			free(message);

			/*MESSAGE* msg_a = endpoint_send_request_blocking(ep_req_a, message);
			printf("A: Request %s: %s", msg_a->msg_id, message);
			message_free(msg_a);

			// PROBLEM: Cannot block to wait for response on a non-queueing endpoint

			MESSAGE* msg_b = endpoint_send_request_blocking(ep_req_b, message);
			printf("B: Request %s: %s", msg_b->msg_id, message);
			message_free(msg_a);*/

			json_free(msg_json);

			sleep(3);
	}
	return 0;
}
