/*
 * $Id: file.c,v 1.9 2002/01/04 04:45:47 sibaz Exp $
 *
 * Copyright (c) 1990,1994 Regents of The University of Michigan.
 * All Rights Reserved.  See COPYRIGHT.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <atalk/logger.h>
#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file.h"

int markline( pf, start, linelength, crlflength )
    char		**start;
    int			*linelength, *crlflength;
    struct papfile	*pf;
{
    char		*p;

    if ( pf->pf_datalen == 0 && ( pf->pf_state & PF_EOF )) {
	return( 0 );
    }

    *start = pf->pf_data;

    /* get a line */
    for ( *linelength=0; *linelength < pf->pf_datalen; (*linelength)++) {
	if (pf->pf_data[*linelength] == '\n' ||
	    pf->pf_data[*linelength] == '\r') {
	    break;
	}
    }

    if ( *linelength >= pf->pf_datalen ) {
	if ( pf->pf_state & PF_EOF ) {
	    append( pf, "\n", 1 );
	} else if (*linelength < 1024) {
	    return( -1 );
	}
    }

    p = pf->pf_data + *linelength;

    *crlflength=0;
    while(*crlflength < pf->pf_datalen-*linelength && 
    (p[*crlflength]=='\r' || p[*crlflength]=='\n')) {
	(*crlflength)++;
    }

    /* success, return 1 */
    return( 1 );
}

void morespace( pf, data, len )
    struct papfile	*pf;
    const char		*data;
    int			len;
{
    char		*nbuf;
    int			nsize;

    if ( pf->pf_data != pf->pf_buf ) {			/* pull up */
	bcopy( pf->pf_data, pf->pf_buf, pf->pf_datalen);
	pf->pf_data = pf->pf_buf;
    }

    if ( pf->pf_datalen + len > pf->pf_bufsize ) {	/* make more space */
	nsize = (( pf->pf_bufsize + len ) / PF_MORESPACE +
		(( pf->pf_bufsize + len ) % PF_MORESPACE != 0 )) * PF_MORESPACE;
	if ( pf->pf_buf ) {
	    if (( nbuf = (char *)realloc( pf->pf_buf, nsize )) == 0 ) {
		exit( 1 );
	    }
	} else {
	    if (( nbuf = (char *)malloc( nsize )) == 0 ) {
		exit( 1 );
	    }
	}
	pf->pf_bufsize = nsize;
	pf->pf_data = nbuf + ( pf->pf_data - pf->pf_buf );
	pf->pf_buf = nbuf;
    }

    bcopy( data, pf->pf_data + pf->pf_datalen, len );
    pf->pf_datalen += len;
}


void append(pf, data, len)
    struct papfile	*pf;
    const char		*data;
    int			len;
{
    if ((pf->pf_data + pf->pf_datalen + len) >
	(pf->pf_buf + pf->pf_bufsize)) {
		morespace(pf, data, len);
    } else {
	memcpy(pf->pf_data + pf->pf_datalen, data, len);
	pf->pf_datalen += len;
    }
}


void spoolerror( out, str )
    struct papfile	*out;
    char		*str;
{
    char	*pserr1 = "%%[ Error: ";
    char	*pserr2 = " ]%%\n";

    if ( str == NULL ) {
	str = "Spooler error.";
    }

    append( out, pserr1, strlen( pserr1 ));
    append( out, str, strlen( str ));
    append( out, pserr2, strlen( pserr2 ));
}
