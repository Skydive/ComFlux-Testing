
/*
 * double_sink.c
 */

#include <endpoint.h>
#include <middleware.h>
#include <load_mw_config.h>

#include <stdio.h>
#include <unistd.h>


void print_callback_a(MESSAGE *msg) {
	ENDPOINT *ep = msg->ep;

	/* parsing the message and extracting the values */
	JSON* elem_msg = msg->_msg_json;
	int value = json_get_int(elem_msg, "value");
	char* datetime = json_get_str(elem_msg, "datetime");

	/* process the extracted values */
	printf("A: Sink ep handler:\n "
		"\t-- number: %d \n "
		"\t-- string: %s\n", value, datetime);
}

void print_callback_b(MESSAGE *msg) {
	ENDPOINT *ep = msg->ep;

	/* parsing the message and extracting the values */
	JSON* elem_msg = msg->_msg_json;
	int value = json_get_int(elem_msg, "value");
	char* datetime = json_get_str(elem_msg, "datetime");

	/* process the extracted values */
	printf("B: Sink ep handler:\n "
				 "\t-- number: %d \n "
				 "\t-- string: %s\n", value, datetime);
}

int main(int argc, char *argv[])
{
	char *mw_cfg_path = NULL;
	char *src_addr = NULL;

	if(argc < 3) {
		printf("Usage: ./simple_source [mw_cfg_path] sink_addr\n"
				"\tmw_cfg_path		is the path to the config file for the middleware;\n"
				"\t                 default improved_sink.json\n"
				"\tsource_addr      is the address of the source we want to map to\n");

		mw_cfg_path = "improved_sink.json";
		src_addr=argv[2];
	} else {
		src_addr=argv[2];
		mw_cfg_path = argv[1];
	}

	/* load and apply configuration */
	int load_cfg_result = load_mw_config(mw_cfg_path);
	printf("Loading configuration: %s\n", load_cfg_result==0?"ok":"error");
	printf("\tApp log level: %d\n", config_get_app_log_lvl());

	/* start core */
	char* app_name = mw_init("sink_cpt", config_get_core_log_lvl(), 1);
	printf("Initialising core: %s\n", app_name!=NULL?"ok":"error");
	printf("\tApp name: %s\n", app_name);

	/* load coms modules for the core */
	load_cfg_result = config_load_com_libs();
	printf("Load coms module result: %s\n", load_cfg_result==0?"ok":"error");

	/* Declare and register endpoints */
	ENDPOINT *ep_snk_a = endpoint_new_snk_file(
		"ep_sink_a", /* name */
		"example snk endpoint", /* description */
		"example_schemata/datetime_value.json", /* message schemata */
		&print_callback_a); /* handler for incoming messages */

	ENDPOINT *ep_snk_b = endpoint_new_snk_file(
		"ep_sink_b", /* name */
		"example snk endpoint", /* description */
		"example_schemata/datetime_value.json", /* message schemata */
		&print_callback_b); /* handler for incoming messages */

	/* build the query */
	int map_result = endpoint_map_to(ep_snk_a, src_addr, "[ \"ep_name = 'ep_source_a'\" ]", "");
	printf("A: Map result: %d \n", map_result);

	map_result = endpoint_map_to(ep_snk_b, src_addr, "[ \"ep_name = 'ep_source_b'\" ]", "");
	printf("B: Map result: %d \n", map_result);


	while(1)
		sleep(1);

	return 0;
}
