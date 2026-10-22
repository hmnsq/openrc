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

#अगर_अघोषित _INITREQ_H
#घोषणा _INITREQ_H

/*
 * The #defines and structures below are taken from initreq.h in
 * sysvinit and must be used by any program wishing to communicate with
 * it.
 */

#घोषणा INIT_MAGIC 0x03091969
#घोषणा INIT_CMD_START		0
#घोषणा INIT_CMD_RUNLVL		1
#घोषणा INIT_CMD_POWERFAIL	2
#घोषणा INIT_CMD_POWERFAILNOW	3
#घोषणा INIT_CMD_POWEROK	4
#घोषणा INIT_CMD_BSD		5
#घोषणा INIT_CMD_SETENV		6
#घोषणा INIT_CMD_UNSETENV	7

/*
 *	This is what BSD 4.4 uses when talking to init.
 *	Linux doesn't use this right now.
 */
काष्ठा init_request_bsd {
	अक्षर	gen_id[8];		/* Beats me.. telnetd uses "fe" */
	अक्षर	tty_id[16];		/* Tty name minus /dev/tty      */
	अक्षर	host[64];	/* Hostname                     */
	अक्षर	term_type[16];		/* Terminal type                */
	पूर्णांक	संकेत;			/* Signal to send               */
	पूर्णांक	pid;			/* Process to send to           */
	अक्षर	exec_name[128];	        /* Program to execute           */
	अक्षर	reserved[128];		/* For future expansion.        */
};

/*
 *	Because of legacy interfaces, "runlevel" and "sleeptime"
 *	aren't in a separate struct in the union.
 *
 *	The weird sizes are because init expects the whole
 *	struct to be 384 bytes.
 */
काष्ठा init_request {
	पूर्णांक	magic;			/* Magic number                 */
	पूर्णांक	cmd;			/* What kind of request         */
	पूर्णांक	runlevel;		/* Runlevel to change to        */
	पूर्णांक	sleeptime;		/* Time between TERM and KILL   */
	जोड़ {
		काष्ठा init_request_bsd	bsd;
		अक्षर			तथ्य[368];
	} i;
};

#पूर्ण_अगर
