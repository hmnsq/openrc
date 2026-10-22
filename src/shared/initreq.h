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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _INITREQ_H
#ವ್ಯಾಖ್ಯಾನಿಸು _INITREQ_H

/*
 * The #defines and structures below are taken from initreq.h in
 * sysvinit and must be used by any program wishing to communicate with
 * it.
 */

#ವ್ಯಾಖ್ಯಾನಿಸು INIT_MAGIC 0x03091969
#ವ್ಯಾಖ್ಯಾನಿಸು INIT_CMD_ಆರಂಭ		0
#ವ್ಯಾಖ್ಯಾನಿಸು INIT_CMD_RUNLVL		1
#ವ್ಯಾಖ್ಯಾನಿಸು INIT_CMD_POWERFAIL	2
#ವ್ಯಾಖ್ಯಾನಿಸು INIT_CMD_POWERFAILNOW	3
#ವ್ಯಾಖ್ಯಾನಿಸು INIT_CMD_POWEROK	4
#ವ್ಯಾಖ್ಯಾನಿಸು INIT_CMD_BSD		5
#ವ್ಯಾಖ್ಯಾನಿಸು INIT_CMD_SETENV		6
#ವ್ಯಾಖ್ಯಾನಿಸು INIT_CMD_UNSETENV	7

/*
 *	This is what BSD 4.4 uses when talking to init.
 *	Linux doesn't use this right now.
 */
ರಚನೆ init_request_bsd {
	ಅಕ್ಷರ	gen_id[8];		/* Beats me.. telnetd uses "fe" */
	ಅಕ್ಷರ	tty_id[16];		/* Tty name minus /dev/tty      */
	ಅಕ್ಷರ	host[64];	/* Hostname                     */
	ಅಕ್ಷರ	ಪದ_type[16];		/* Terminal type                */
	ಪೂರ್ಣಾಂಕ	ಸಂಕೇತ;			/* Signal to send               */
	ಪೂರ್ಣಾಂಕ	pid;			/* Process to send to           */
	ಅಕ್ಷರ	exec_name[128];	        /* Program to execute           */
	ಅಕ್ಷರ	reserved[128];		/* For future expansion.        */
};

/*
 *	Because of legacy interfaces, "runlevel" and "sleeptime"
 *	aren't in a separate struct in the union.
 *
 *	The weird sizes are because init expects the whole
 *	struct to be 384 bytes.
 */
ರಚನೆ init_request {
	ಪೂರ್ಣಾಂಕ	magic;			/* Magic number                 */
	ಪೂರ್ಣಾಂಕ	cmd;			/* What kind of request         */
	ಪೂರ್ಣಾಂಕ	runlevel;		/* Runlevel to change to        */
	ಪೂರ್ಣಾಂಕ	sleeptime;		/* Time between TERM and KILL   */
	ಒಕ್ಕೂಟ {
		ರಚನೆ init_request_bsd	bsd;
		ಅಕ್ಷರ			ದತ್ತಾಂಶ[368];
	} i;
};

#ಆದರೆ_ಅಂತ್ಯ
