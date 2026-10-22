/*
 * Copyright (c) 2017 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#ifndef __RC_SCHEDULES_H
#define __RC_SCHEDULES_H

#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <sys/types.h>

ശൂന്യം free_schedulelist(ശൂന്യം);
പൂർണ്ണസംഖ്യ parse_signal(സ്ഥിരം അക്ഷരം *applet, സ്ഥിരം അക്ഷരം *sig);
ശൂന്യം parse_schedule(സ്ഥിരം അക്ഷരം *applet, സ്ഥിരം അക്ഷരം *string, പൂർണ്ണസംഖ്യ timeout);
പൂർണ്ണസംഖ്യ do_stop(സ്ഥിരം അക്ഷരം *applet, സ്ഥിരം അക്ഷരം *exec, സ്ഥിരം അക്ഷരം *സ്ഥിരം *argv,
		pid_t pid, uid_t uid, പൂർണ്ണസംഖ്യ sig, bool group, bool test, bool quiet);
പൂർണ്ണസംഖ്യ run_stop_schedule(സ്ഥിരം അക്ഷരം *applet,
		സ്ഥിരം അക്ഷരം *exec, സ്ഥിരം അക്ഷരം *സ്ഥിരം *argv,
		pid_t pid, uid_t uid, bool group,
		bool test, bool progress, bool quiet);

#endif
