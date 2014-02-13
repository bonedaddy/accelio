/*
 * Copyright (c) 2013 Mellanox Technologies®. All rights reserved.
 *
 * This software is available to you under a choice of one of two licenses.
 * You may choose to be licensed under the terms of the GNU General Public
 * License (GPL) Version 2, available from the file COPYING in the main
 * directory of this source tree, or the Mellanox Technologies® BSD license
 * below:
 *
 *      - Redistribution and use in source and binary forms, with or without
 *        modification, are permitted provided that the following conditions
 *        are met:
 *
 *      - Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 *      - Neither the name of the Mellanox Technologies® nor the names of its
 *        contributors may be used to endorse or promote products derived from
 *        this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "reg_utils.h"

extern int client_main(int argc, char *argv[]);
extern int server_main(int argc, char *argv[]);

char  REG_DEBUG = 0;


struct params {
	int	argc;
	int	pad;
	char	**argv;
};

struct program_vars {
	char		threads_num[8];
	char		client_dlen[8];
	char		server_dlen[8];
	char		queue_depth[8];
	unsigned long	seed;
};


static void *client_thread(void *data)
{
	struct params *params	= data;
	client_main(params->argc, params->argv);
	return NULL;
}

static void *server_thread(void *data)
{
	struct params *params	= data;
	server_main(params->argc, params->argv);
	return NULL;
}

void rand_params(struct program_vars *vars)
{
	int var;
	int max_dlen;
	int max_qdepth;
	int threads_num;


	time((time_t *)&vars->seed);
	srandom(vars->seed);

	/* threads number [1,24] */
	do {
		var = random() % 8;
	} while (var == 0);
	sprintf(vars->threads_num, "%d", var);
	threads_num = var;

	max_qdepth = (threads_num > 2) ? 100 : 300;

	/* queue_depth [1,300] */
	do {
		var = random() % max_qdepth;
	} while (var == 0);
	sprintf(vars->queue_depth, "%d", var);

	max_dlen = (threads_num > 3) ? 262144 : 524288;

	/* client_dlen [0,1048576] */
	var = random() % max_dlen;
	sprintf(vars->client_dlen, "%d", var);

	/* server_dlen [0,1048576] */
	var = random() % max_dlen;
	sprintf(vars->server_dlen, "%d", var);
}


int main(int argc, char *argv[])
{
	char *argvv[] = {
		argv[0],
		argv[1],	/* address */
		argv[2],	/* port	   */
		"\0",		/* threads num */
		"\0",		/* queue depth */
		"\0",		/* client dlen */
		"\0",		/* server dlen */
		"\0"
	};

	struct params params  = {
		.argc = argc,
		.argv = argvv
	};
	pthread_t stid, ctid;
	struct	 program_vars vars;

start:
	rand_params(&vars);

	argvv[3] = vars.threads_num;
	argvv[4] = vars.queue_depth;
	argvv[5] = vars.client_dlen;
	argvv[6] = vars.server_dlen;

	fprintf(stderr, "seed:%lu, threads num:%s, queue_depth:%s, "\
			"client_dlen:%s, server_dlen:%s [start]\n",
			vars.seed,
			vars.threads_num,
			vars.queue_depth,
			vars.client_dlen,
			vars.server_dlen);

	pthread_create(&stid, NULL, server_thread, &params);
	sleep(1);
	pthread_create(&ctid, NULL, client_thread, &params);

	pthread_join(stid, NULL);
	pthread_join(ctid, NULL);

	fprintf(stderr, "seed:%lu, threads num:%s, queue_depth:%s, " \
			"client_dlen:%s, server_dlen:%s [pass]\n",
			vars.seed,
			vars.threads_num,
			vars.queue_depth,
			vars.client_dlen,
			vars.server_dlen);

	goto start;

	return 0;
}

