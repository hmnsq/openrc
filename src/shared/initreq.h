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
কাঠামো init_request_bsd {
	অক্ষর	gen_id[8];		/* Beats me.. telnetd uses "fe" */
	অক্ষর	tty_id[16];		/* Tty name minus /dev/tty      */
	অক্ষর	host[64];	/* Hostname                     */
	অক্ষর	term_type[16];		/* Terminal type                */
	পূর্ণসংখ্যা	সংকেত;			/* Signal to send               */
	পূর্ণসংখ্যা	pid;			/* Process to send to           */
	অক্ষর	exec_name[128];	        /* Program to execute           */
	অক্ষর	reserved[128];		/* For future expansion.        */
};

/*
 *	Because of legacy interfaces, "runlevel" and "sleeptime"
 *	aren't in a separate struct in the union.
 *
 *	The weird sizes are because init expects the whole
 *	struct to be 384 bytes.
 */
কাঠামো init_request {
	পূর্ণসংখ্যা	magic;			/* Magic number                 */
	পূর্ণসংখ্যা	cmd;			/* What kind of request         */
	পূর্ণসংখ্যা	runlevel;		/* Runlevel to change to        */
	পূর্ণসংখ্যা	sleeptime;		/* Time between TERM and KILL   */
	সংঘ {
		কাঠামো init_request_bsd	bsd;
		অক্ষর			তথ্য[368];
	} i;
};

#endif
