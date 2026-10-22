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

#جيڪڏهن_وصف_نه_ٿيل _INITREQ_H
#وصف_ڪريو _INITREQ_H

/*
 * The #defines and structures below are taken from initreq.h in
 * sysvinit and must be used by any program wishing to communicate with
 * it.
 */

#وصف_ڪريو INIT_MAGIC 0x03091969
#وصف_ڪريو INIT_CMD_شروعات		0
#وصف_ڪريو INIT_CMD_RUNLVL		1
#وصف_ڪريو INIT_CMD_POWERFAIL	2
#وصف_ڪريو INIT_CMD_POWERFAILNOW	3
#وصف_ڪريو INIT_CMD_POWEROK	4
#وصف_ڪريو INIT_CMD_BSD		5
#وصف_ڪريو INIT_CMD_SETENV		6
#وصف_ڪريو INIT_CMD_UNSETENV	7

/*
 *	This is what BSD 4.4 uses when talking to init.
 *	Linux doesn't use this right now.
 */
بنية init_request_bsd {
	حرف	gen_id[8];		/* Beats me.. telnetd uses "fe" */
	حرف	tty_id[16];		/* Tty name minus /dev/tty      */
	حرف	host[64];	/* Hostname                     */
	حرف	رڪن_type[16];		/* Terminal type                */
	صحيح	إشارة;			/* Signal to send               */
	صحيح	pid;			/* Process to send to           */
	حرف	exec_name[128];	        /* Program to execute           */
	حرف	reserved[128];		/* For future expansion.        */
};

/*
 *	Because of legacy interfaces, "runlevel" and "sleeptime"
 *	aren't in a separate struct in the union.
 *
 *	The weird sizes are because init expects the whole
 *	struct to be 384 bytes.
 */
بنية init_request {
	صحيح	magic;			/* Magic number                 */
	صحيح	cmd;			/* What kind of request         */
	صحيح	runlevel;		/* Runlevel to change to        */
	صحيح	sleeptime;		/* Time between TERM and KILL   */
	اتحاد {
		بنية init_request_bsd	bsd;
		حرف			معلومات[368];
	} i;
};

#جيڪڏهن_پورو
