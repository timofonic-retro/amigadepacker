/* decrunch.c
 *
 * based on load.c from:
 * Extended Module Player
 *
 * Copyright (C) 1996-1999 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * CHANGES: (modified for uade by mld)
 * removed all xmp related code)
 * added "custom" labels of pp20 files
 * added support for external unrar decruncher
 * added support for the external XPK Lib for Unix (the xType usage *g*)
 *
 * TODO:
 * real builtin support for XPK lib for Unix
 *
 * This file is part of the Extended Module Player and is distributed
 * under the terms of the GNU General Public License. See doc/COPYING
 * for more information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <libgen.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>

#include "decrunch.h"
#include "ppdepack.h"
#include "unsqsh.h"
#include "mmcmp.h"


enum {
    BUILTIN_PP = 1,
    BUILTIN_SQSH,
    BUILTIN_MMCMP
};


int decrunch(const char *filename, FILE *out, int pretend)
{
    uint8_t b[12];
    int builtin, res;
    char *packer;
    size_t nbytes;
    FILE *in;
    char dstname[PATH_MAX] = "";

    if (filename[0]) {
	if ((in = fopen(filename, "r")) == NULL) {
	    fprintf(stderr, "Unknown file %s\n", filename);
	    goto error;
	}
    } else {
	in = stdin;
    }

    packer = NULL;
    builtin = 0;

    nbytes = fread(b, 1, sizeof b, in);
    if (nbytes < 12)
	goto error;

    if ((b[0] == 'P' && b[1] == 'X' && b[2] == '2' && b[3] == '0') ||
	(b[0] == 'P'  && b[1] == 'P'  && b[2] == '2'  && b[3] == '0')) {
        packer = "PowerPacker data";
	builtin = BUILTIN_PP;
    } else if (nbytes >= 12 && b[0] == 'X' && b[1] == 'P' && b[2] == 'K' && b[3] == 'F' &&
	b[8] == 'S' && b[9] == 'Q' && b[10] == 'S' && b[11] == 'H') {
	packer = "XPK SQSH";
	builtin = BUILTIN_SQSH;
    } else if (nbytes >= 8 && b[0] == 'z' && b[1] == 'i' && b[2] == 'R' && b[3] == 'C' &&
	b[4] == 'O' && b[5] == 'N' && b[6] == 'i' && b[7] == 'a') {
	packer = "MMCMP";
	builtin = BUILTIN_MMCMP;
    }

    fseek (in, 0, SEEK_SET);

    if (!packer)
	goto error;

    if (filename[0])
      fprintf(stderr, "File %s is in %s format.\n", filename, packer);
    else
      fprintf(stderr, "Stream is in %s format.\n", packer);

    if (pretend)
      return 0;

    if (out != stdout) {
	int fd;
	snprintf(dstname, sizeof dstname, "%s.XXXXXX", filename);
	if ((fd = mkstemp(dstname)) < 0) {
	    fprintf(stderr, "Could not create a temporary file: %s (%s)\n", dstname, strerror(errno));
	    goto error;
	}
	if ((out = fdopen(fd, "w")) == NULL) {
	    fprintf(stderr, "Could not fdopen temporary file: %s\n", dstname);
	    goto error;
	}
    }

    res = 0;
    switch (builtin) {
    case BUILTIN_PP:    
      res = decrunch_pp (in, out);
      break;
    case BUILTIN_SQSH:    
      res = decrunch_sqsh (in, out);
      break;
    case BUILTIN_MMCMP:    
      res = decrunch_mmcmp (in, out);
      break;
    }

    if (res < 0)
      goto error;

    fclose(in);
    if (out != stdout && out != NULL && dstname[0]) {
	fclose(out);
	if (rename(dstname, filename)) {
	    fprintf(stderr, "Rename error: %s -> %s (%s)\n", dstname, filename, strerror(errno));
	    unlink(dstname);
	}
    }
    return 0;

 error:
    if (in)
	fclose(in);
    if (out != stdout && out != NULL)
      fclose(out);
    if (dstname[0])
	unlink(dstname);
    return -1;
}
