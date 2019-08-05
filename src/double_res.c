/*
 *
 *  Created on: 7 Sep 2016
 *      Author: Raluca Diaconu
 */

#include <endpoint.h>
#include <middleware.h>
#include <load_mw_config.h>

#include <stdio.h>
#include <unistd.h>
#include <time.h>



ENDPOINT *ep_resp_a, *ep_resp_b;

void print_callback_a(MESSAGE* msg) {

	if (msg != NULL) {
		char* str = json_to_str(msg->_msg_json);
		char* reqid = msg->msg_id;
		printf("A: Recved req %s: %s\n", reqid, str);
		free(str);
		free(reqid);
	}

	JSON* msg_json = msg->_msg_json;
	char* message = json_to_str(msg_json);
	endpoint_send_response(ep_resp_a, msg->msg_id, message);

	Array* c = ep_get_all_connections(ep_resp_a);
	printf("A: Connection count: %d\n", array_size(c));
	array_free(c);


	free(message);
	json_free(msg_json);
}

void print_callback_b(MESSAGE* msg) {

	if (msg != NULL) {
		char* str = json_to_str(msg->_msg_json);
		char* reqid = msg->msg_id;
		printf("B: Recved req %s: %s\n", reqid, str);
		free(str);
		free(reqid);
	}

	JSON* msg_json = msg->_msg_json;
	char* message = json_to_str(msg_json);
	endpoint_send_response(ep_resp_b, msg->msg_id, message);

	Array* c = ep_get_all_connections(ep_resp_b);
	printf("B: Connection count: %d\n", array_size(c));
	array_free(c);


	free(message);
	json_free(msg_json);
}

int main(int argc, char* argv[]) {

	char *mw_cfg_path = NULL;

	if(argc<2)
	{
		printf("Usage: ./simple_response [mw_cfg_path]\n"
				"\tmw_cfg_path		is the path to the config file for the middleware;\n"
				"\t                 default mw_cfg.json\n");

		mw_cfg_path = "mw_cfg.json";
	}
	else
	{
		mw_cfg_path = argv[1];
	}

	/* load and apply configuration */
	int load_cfg_result = load_mw_config(mw_cfg_path);
	printf("Loading configuration: %s\n", load_cfg_result==0?"ok":"error");
	printf("\tApp log level: %d\n", config_get_app_log_lvl());

	/* start core */
	char* app_name = mw_init("response_cpt", config_get_core_log_lvl(), 1);
	printf("Initialising core: %s\n", app_name!=NULL?"ok":"error");
	printf("\tApp name: %s\n", app_name);

	/* load coms modules for the core */
	load_cfg_result = config_load_com_libs();
	printf("Load coms module result: %s\n", load_cfg_result==0?"ok":"error");


	/* Instantiate an ep. Generates core function calls */
	ep_resp_a = endpoint_new_resp_file("ep_resp_a",
																		 "example src endpoint",
																		 "example_schemata/datetime_value.json",
																		 "example_schemata/datetime_value.json",
																		 &print_callback_a);

	ep_resp_b = endpoint_new_resp_file("ep_resp_b",
																		 "example src endpoint",
																		 "example_schemata/datetime_value.json",
																		 "example_schemata/datetime_value.json",
																		 &print_callback_b);
	/* forever: wait for reqiests */
	while (1)
		sleep(1);

	return 0;
}
