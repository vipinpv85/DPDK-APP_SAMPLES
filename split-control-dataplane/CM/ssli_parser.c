/*
 * ssli_parser.c
 *
 *  REVISION HISTORY:
 *  Created on: Sep 9, 2014
 *      Author: ushus
 */

/**
 * \brief    Entry point of the program.
 *
 * \param    argc Number of arguments.
 * \param    argv An array of argument strings.
 *
 * \return   Return 0.
 */


/*
 * Describe what this file is about. Use at least 10 lines for
 * description.
 */

/* I N C L U D E S  */



#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "cparser.h"
#include "cparser_io.h"
#include "cparser_priv.h"
#include "cparser_token.h"
#include "cparser_tree.h"

/* D E F I N E S */


/* P R O T O T Y P E S */

/* Only those that cannot go into header file may come here */


/* G L O B A L S */
int interactive;

/* P U B L I C   F U N C T I O N S */
int cli_main (void)
{
	cparser_t parser;
	char *config_file = NULL;
	int debug = 0;

	memset(&parser, 0, sizeof(parser));

#if 0
	while (-1 != (ch = getopt(argc, argv, "pic:d"))) {
		switch (ch) {
		case 'p':
			printf("pid = %d\n", getpid());
			break;
		case 'i':
			interactive = 1;
			break;
		case 'c':
			config_file = optarg;
			break;
		case 'd':
			debug = 1;
			break;
		}
	}
#endif

    //printf("pid = %d\n", getpid());
    interactive = 1;
    //debug = 1;

	parser.cfg.root = &cparser_root;
	parser.cfg.ch_complete = '\t';
	/*
	 * Instead of making sure the terminal setting of the target and
	 * the host are the same. ch_erase and ch_del both are treated
	 * as backspace.
	 */
	parser.cfg.ch_erase = '\b';
	parser.cfg.ch_del = 127;
	parser.cfg.ch_help = '?';
	parser.cfg.flags = (debug ? CPARSER_FLAGS_DEBUG : 0);
	strcpy(parser.cfg.prompt, "XS> ");
	parser.cfg.fd = STDOUT_FILENO;
	cparser_io_config(&parser);
	interactive =1;
#if 0
	status = SSLI_Initialize();
	if(status == 1)
	printf("\nCLI initialiation success\n");
	else
	{
		printf("\nCLI initialiation Fialed\n");
	}
#endif
	if (CPARSER_OK != cparser_init(&parser.cfg, &parser)) {
		printf ("Fail to initialize parser.\n");
		return -1;
	}
	if (interactive) {
		if (config_file) {
			(void)cparser_load_cmd(&parser, config_file);
		}
		cparser_run(&parser);
	}

    return 0;
}

#if 0
int main ()
{
    parser_execute ();
    return 0;
}
#endif

