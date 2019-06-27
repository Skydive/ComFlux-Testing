/*
 * simple_source.c
 *
 *	Created on: 7 Sep 2016
 *		Author: Raluca Diaconu
 */


#include <endpoint.h>
#include <middleware.h>
#include <load_mw_config.h>

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include <stdlib.h>

#include <signal.h>

void exit_handler(int s) {
	printf("Terminating process...");
	exit(1);
}

int main(int argc, char *argv[])
{
	char *mw_cfg_path = NULL;
	if(argc < 2) {
		printf("Usage: ./simple_source mw_cfg_path\n"
			"\tmw_cfg_path			is the path to the config file for the middleware;\n"
			"\t\tdefault improved_source.json\n");

		mw_cfg_path = "improved_source.json";
	} else {
		mw_cfg_path = argv[1];
	}
	int load_cfg_result = load_mw_config(mw_cfg_path);
	printf("Loading configuration: %s\n", load_cfg_result==0?"ok":"error");
	printf("\tApp log level: %d\n", config_get_app_log_lvl());

	/* start core */
	char* app_name = mw_init(
		"ncurses_remote", /* name of the app */
		config_get_app_log_lvl(), /* log level for app */
		1); /* use sockpair, only available option atm */


	printf("Registering exit handler...\n");
	signal(SIGINT, exit_handler);

	printf("Initialising core: %s\n", app_name!=NULL?"ok":"error");
	printf("\tApp name: %s\n", app_name);

	/* load coms modules for the core */
	load_cfg_result = config_load_com_libs();
	printf("Load coms module result: %s\n", load_cfg_result==0?"ok":"error");

	/* Declare and register endpoints */
	ENDPOINT *ep_src = endpoint_new_src_file("ep_dir_src", "example src endpoint", "example_schemata/ncurses_dir_value.json");
	ENDPOINT* ep_clr_src = endpoint_new_src_file("ep_bkclr_src", "src endpoint background color", "example_schemata/ncurses_color_value.json");

	while(1) {
		char line[1024];
		fgets(line, 1024, stdin);


		if(strlen(line) > 2) {
			char* token = strtok(line, " ");
			int i=0;
			char args[5][64];
			while(token != NULL) {
				if(i>=5)break;
				strcpy(args[i], token);
				token = strtok(NULL, " ");
				i++;
			}

			if(strstr(args[0], "setbk")) {
				int cpair = atoi(args[1]);
				if(cpair == 0) {
					printf("Invalid color pair\n");
					continue;
				}

				JSON* msg_json = json_new(NULL);
				json_set_int(msg_json, "color", cpair);
				char* message = json_to_str(msg_json);

				printf("Sending background: %d\n", cpair);
				endpoint_send_message(ep_clr_src, message);

				free(message);
				json_free(msg_json);
			}
		} else {
			int dir = line[0];
			JSON* msg_json = json_new(NULL);
			json_set_int(msg_json, "dir", dir);
			char* message = json_to_str(msg_json);

			printf("Sending message: %s\n\n", message);

			endpoint_send_message(ep_src, message);

			free(message);
			json_free(msg_json);
		}

		Array* connections = NULL;
		int i;
		JSON* conn_json;
		char* module;
		int conn;
		char* metadata;

		connections = ep_get_all_connections(ep_src);
		printf("Number of connections = %d\n", array_size(connections));

		for (i=0; i<array_size(connections); i++) {
			conn_json = array_get(connections, i);
			module = json_get_str(conn_json, "module");
			conn = json_get_int(conn_json, "conn");
			metadata = mw_get_remote_metdata(module, conn);
			printf("\t#%d: connection %d, module %s, manifest: %s\n",
							i, conn, module, metadata);
			free(metadata);
			free(module);
			json_free(conn_json);
		}

		array_free(connections);
	}

	return 0;
}
