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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _INITREQ_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _INITREQ_H

/*
 * The #defines and structures below are taken from initreq.h in
 * sysvinit and must be used by any program wishing to communicate with
 * it.
 */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INIT_MAGIC 0x03091969
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INIT_CMD_ਸ਼ੁਰੂ		0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INIT_CMD_RUNLVL		1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INIT_CMD_POWERFAIL	2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INIT_CMD_POWERFAILNOW	3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INIT_CMD_POWEROK	4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INIT_CMD_BSD		5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INIT_CMD_SETENV		6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INIT_CMD_UNSETENV	7

/*
 *	This is what BSD 4.4 uses when talking to init.
 *	Linux doesn't use this right now.
 */
ਬਣਤਰ init_request_bsd {
	ਅੱਖਰ	gen_id[8];		/* Beats me.. telnetd uses "fe" */
	ਅੱਖਰ	tty_id[16];		/* Tty name minus /dev/tty      */
	ਅੱਖਰ	host[64];	/* Hostname                     */
	ਅੱਖਰ	ਪਦ_type[16];		/* Terminal type                */
	ਪੂਰਨ	ਸੰਕੇਤ;			/* Signal to send               */
	ਪੂਰਨ	pid;			/* Process to send to           */
	ਅੱਖਰ	exec_name[128];	        /* Program to execute           */
	ਅੱਖਰ	reserved[128];		/* For future expansion.        */
};

/*
 *	Because of legacy interfaces, "runlevel" and "sleeptime"
 *	aren't in a separate struct in the union.
 *
 *	The weird sizes are because init expects the whole
 *	struct to be 384 bytes.
 */
ਬਣਤਰ init_request {
	ਪੂਰਨ	magic;			/* Magic number                 */
	ਪੂਰਨ	cmd;			/* What kind of request         */
	ਪੂਰਨ	runlevel;		/* Runlevel to change to        */
	ਪੂਰਨ	sleeptime;		/* Time between TERM and KILL   */
	ਸੰਘ {
		ਬਣਤਰ init_request_bsd	bsd;
		ਅੱਖਰ			ਅੰਕੜੇ[368];
	} i;
};

#ਜੇ_ਅੰਤ
