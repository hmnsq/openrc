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

#నిర్వచితం_కాకపోతే _INITREQ_H
#నిర్వచించు _INITREQ_H

/*
 * The #defines and structures below are taken from initreq.h in
 * sysvinit and must be used by any program wishing to communicate with
 * it.
 */

#నిర్వచించు INIT_MAGIC 0x03091969
#నిర్వచించు INIT_CMD_ప్రారంభం		0
#నిర్వచించు INIT_CMD_RUNLVL		1
#నిర్వచించు INIT_CMD_POWERFAIL	2
#నిర్వచించు INIT_CMD_POWERFAILNOW	3
#నిర్వచించు INIT_CMD_POWEROK	4
#నిర్వచించు INIT_CMD_BSD		5
#నిర్వచించు INIT_CMD_SETENV		6
#నిర్వచించు INIT_CMD_UNSETENV	7

/*
 *	This is what BSD 4.4 uses when talking to init.
 *	Linux doesn't use this right now.
 */
నిర్మాణం init_request_bsd {
	అక్షరం	gen_id[8];		/* Beats me.. telnetd uses "fe" */
	అక్షరం	tty_id[16];		/* Tty name minus /dev/tty      */
	అక్షరం	host[64];	/* Hostname                     */
	అక్షరం	పదం_type[16];		/* Terminal type                */
	పూర్ణాంకం	సంకేతం;			/* Signal to send               */
	పూర్ణాంకం	pid;			/* Process to send to           */
	అక్షరం	exec_name[128];	        /* Program to execute           */
	అక్షరం	reserved[128];		/* For future expansion.        */
};

/*
 *	Because of legacy interfaces, "runlevel" and "sleeptime"
 *	aren't in a separate struct in the union.
 *
 *	The weird sizes are because init expects the whole
 *	struct to be 384 bytes.
 */
నిర్మాణం init_request {
	పూర్ణాంకం	magic;			/* Magic number                 */
	పూర్ణాంకం	cmd;			/* What kind of request         */
	పూర్ణాంకం	runlevel;		/* Runlevel to change to        */
	పూర్ణాంకం	sleeptime;		/* Time between TERM and KILL   */
	సంఘం {
		నిర్మాణం init_request_bsd	bsd;
		అక్షరం			దత్తాంశం[368];
	} i;
};

#అయితే_ముగింపు
