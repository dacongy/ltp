/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *  AUTHOR		: Richard Logan
 *  CO-PILOT		: William Roske
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */

 /*
  * Tests that link(2) succeds with creating n links.
  */

#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "test.h"
#include "usctest.h"
#include "safe_macros.h"

static void setup(void);
static void help(void);
static void cleanup(void);

char *TCID = "link03";
int TST_TOTAL = 2;

#define BASENAME	"lkfile"

static char fname[255];
static int nlinks = 0;
static char *links_arg;

option_t options[] = {
	{"N:", NULL, &links_arg},
	{NULL, NULL, NULL}
};

int main(int ac, char **av)
{
	int lc;
	char *msg;
	struct stat buf;
	int i, links;
	char lname[255];

	if ((msg = parse_opts(ac, av, options, &help)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	if (links_arg) {
		nlinks = atoi(links_arg);

		if (nlinks == 0) {
			tst_brkm(TBROK, NULL,
			         "nlinks is not a positive number");
		}

		if (nlinks > 1000) {
			tst_resm(TINFO,
				 "nlinks > 1000 - may get errno:%d (EMLINK)",
				 EMLINK);
		}
	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		if (nlinks)
			links = nlinks;
		else
			links = (lc % 90) + 10;

		/* Create links - 1 hardlinks so that the st_nlink == links */
		for (i = 1; i < links; i++) {
			sprintf(lname, "%s%d", fname, i);
			TEST(link(fname, lname));

			if (TEST_RETURN == -1) {
				tst_brkm(TFAIL | TTERRNO, cleanup,
					 "link(%s, %s) Failed", fname, lname);
			}
		}

		SAFE_STAT(cleanup, fname, &buf);

		if (buf.st_nlink != (nlink_t)links) {
			tst_resm(TFAIL, "Wrong number of links for "
			         "'%s' have %i, should be %i",
				 fname, (int)buf.st_nlink, links);
			goto unlink;
		}

		for (i = 1; i < links; i++) {
			sprintf(lname, "%s%d", fname, i);
			SAFE_STAT(cleanup, lname, &buf);
			if (buf.st_nlink != (nlink_t)links) {
				tst_resm(TFAIL,
				         "Wrong number of links for "
					 "'%s' have %i, should be %i",
					 lname, (int)buf.st_nlink, links);
				goto unlink;
			}
		}

		tst_resm(TPASS, "link() passed and linkcounts=%d match", links);

unlink:
		for (i = 1; i < links; i++) {
			sprintf(lname, "%s%d", fname, i);
			SAFE_UNLINK(cleanup, lname);
		}
	}

	cleanup();
	tst_exit();
}

static void help(void)
{
	printf("  -N #links : create #links hard links every iteration\n");
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	sprintf(fname, "%s_%d", BASENAME, getpid());
	SAFE_TOUCH(cleanup, fname, 0700, NULL);
}

static void cleanup(void)
{
	TEST_CLEANUP;
	tst_rmdir();
}
