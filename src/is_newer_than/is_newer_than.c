/*
 * Copyright (c) 2016 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#અન્તર્ભાવ <stdlib.h>

#અન્તર્ભાવ "rc.h"

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
	પૂર્ણાંક i;

	જો (argc < 3)
		પાછા EXIT_FAILURE;

	/* This test is correct as it's not present in baselayout */
	માટે (i = 2; i < argc; ++i)
		જો (!rc_newer_than(argv[1], argv[i], ખાલી, ખાલી))
			પાછા EXIT_FAILURE;

	પાછા EXIT_SUCCESS;
}
