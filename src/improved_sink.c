/*
 * simple_sink.c
 *
 *  Created on: 7 Sep 2016
 *      Author: Raluca Diaconu
 */

#include <endpoint.h>
#include <middleware.h>
#include <load_mw_config.h>

#include <stdio.h>
#include <unistd.h>


void print_callback(MESSAGE *msg) {
	ENDPOINT *ep = msg->ep;

	/* parsing the message and extracting the values */
	JSON* elem_msg = msg->_msg_json;
	int value = json_get_int(elem_msg, "value");
	char* datetime = json_get_str(elem_msg, "datetime");

	/* process the extracted values */
	printf("Sink ep handler:\n "
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

	JSON* manifest = json_new(NULL);
	json_set_str(manifest, "app_name", app_name);
	json_set_str(manifest, "author", "khalid");
	mw_add_manifest(json_to_str(manifest));

	/* Declare and register endpoints */
	ENDPOINT *ep_snk = endpoint_new_snk_file(
		"ep_sink", /* name */
		"example snk endpoint", /* description */
		"example_schemata/datetime_value.json", /* message schemata */
		&print_callback); /* handler for incoming messages */

	/* build the query */
	Array *ep_query_array = array_new(ELEM_TYPE_STR);
	array_add(ep_query_array, "ep_name = 'ep_source'");
	JSON *ep_query_json = json_new(NULL);
	json_set_array(ep_query_json, NULL, ep_query_array);
	char* ep_query_str = json_to_str(ep_query_json);
	char* cpt_query_str = "";

	printf("\nAdding RDC at address 127.0.0.1:1508\n");
	mw_add_rdc("comtcp", "127.0.0.1:1508");
	mw_register_rdcs();


	/* map according to the query */
	//endpoint_map_lookup(ep_snk, ep_query_str, cpt_query_str, 10);
	int map_result = endpoint_map_to(ep_snk, src_addr, ep_query_str, cpt_query_str);
	printf("Map result: %d \n", map_result);

	while(1)
		sleep(1);

	return 0;
}
