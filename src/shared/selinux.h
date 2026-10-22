/*
 * Copyright (c) 2014-2015 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#నిర్వచితం_కాకపోతే RC_SELINUX_UTIL_H
#నిర్వచించు RC_SELINUX_UTIL_H

#నిర్వచితమైతే HAVE_SELINUX

పూర్ణాంకం selinux_util_open(శూన్య);
పూర్ణాంకం selinux_util_label(స్థిరరాశి అక్షరం *మార్గం);
పూర్ణాంకం selinux_util_close(శూన్య);

శూన్య selinux_అమరిక(అక్షరం **argv);

#లేకపోతే

/* always return false for selinux_util_open() */
#నిర్వచించు selinux_util_open() (0)
#నిర్వచించు selinux_util_label(x) చేయి { } అయితే (0)
#నిర్వచించు selinux_util_close() చేయి { } అయితే (0)

#నిర్వచించు selinux_అమరిక(x) చేయి { } అయితే (0)

#అయితే_ముగింపు


#అయితే_ముగింపు
