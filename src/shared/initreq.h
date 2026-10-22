/*
 * initreq.h	- Interface to communicate with sysvinit via /run/initctl.
 */

/*
 * Copyright (c) 2019 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#ifndef _INITREQ_H
#define _INITREQ_H

/*
 * The #defines and structures below are taken from initreq.h in
 * sysvinit and must be used by any program wishing to communicate with
 * it.
 */

#define INIT_MAGIC 0x03091969
#define INIT_CMD_START		0
#define INIT_CMD_RUNLVL		1
#define INIT_CMD_POWERFAIL	2
#define INIT_CMD_POWERFAILNOW	3
#define INIT_CMD_POWEROK	4
#define INIT_CMD_BSD		5
#define INIT_CMD_SETENV		6
#define INIT_CMD_UNSETENV	7

/*
 *	This is what BSD 4.4 uses when talking to init.
 *	Linux doesn't use this right now.
 */
ساختار init_request_bsd {
	نویسه	gen_id[8];		/* Beats me.. telnetd uses "fe" */
	نویسه	tty_id[16];		/* Tty name minus /dev/tty      */
	نویسه	host[64];	/* Hostname                     */
	نویسه	term_type[16];		/* Terminal type                */
	صحیح	سیگنال;			/* Signal to send               */
	صحیح	pid;			/* Process to send to           */
	نویسه	exec_name[128];	        /* Program to execute           */
	نویسه	reserved[128];		/* For future expansion.        */
};

/*
 *	Because of legacy interfaces, "runlevel" and "sleeptime"
 *	aren't in a separate struct in the union.
 *
 *	The weird sizes are because init expects the whole
 *	struct to be 384 bytes.
 */
ساختار init_request {
	صحیح	magic;			/* Magic number                 */
	صحیح	cmd;			/* What kind of request         */
	صحیح	runlevel;		/* Runlevel to change to        */
	صحیح	sleeptime;		/* Time between TERM and KILL   */
	اجتماع {
		ساختار init_request_bsd	bsd;
		نویسه			data[368];
	} i;
};

#endif
