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

#اگر_غیر_تعریف_شدہ _INITREQ_H
#تعریف_کرو _INITREQ_H

/*
 * The #defines and structures below are taken from initreq.h in
 * sysvinit and must be used by any program wishing to communicate with
 * it.
 */

#تعریف_کرو INIT_MAGIC 0x03091969
#تعریف_کرو INIT_CMD_آغاز		0
#تعریف_کرو INIT_CMD_RUNLVL		1
#تعریف_کرو INIT_CMD_POWERFAIL	2
#تعریف_کرو INIT_CMD_POWERFAILNOW	3
#تعریف_کرو INIT_CMD_POWEROK	4
#تعریف_کرو INIT_CMD_BSD		5
#تعریف_کرو INIT_CMD_SETENV		6
#تعریف_کرو INIT_CMD_UNSETENV	7

/*
 *	This is what BSD 4.4 uses when talking to init.
 *	Linux doesn't use this right now.
 */
ساخت init_request_bsd {
	حرف	gen_id[8];		/* Beats me.. telnetd uses "fe" */
	حرف	tty_id[16];		/* Tty name minus /dev/tty      */
	حرف	host[64];	/* Hostname                     */
	حرف	رکن_type[16];		/* Terminal type                */
	صحیح	اشارہ;			/* Signal to send               */
	صحیح	pid;			/* Process to send to           */
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
ساخت init_request {
	صحیح	magic;			/* Magic number                 */
	صحیح	cmd;			/* What kind of request         */
	صحیح	runlevel;		/* Runlevel to change to        */
	صحیح	sleeptime;		/* Time between TERM and KILL   */
	اتحاد {
		ساخت init_request_bsd	bsd;
		حرف			ڈیٹا[368];
	} i;
};

#اگر_ختم
