/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 *
 * This file is part of Geomview.
 * 
 * Geomview is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * Geomview is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with Geomview; see the file COPYING.  If not, write
 * to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
 * USA, or visit http://www.gnu.org.
 */

#if defined(HAVE_CONFIG_H) && !defined(CONFIG_H_INCLUDED)
#include "config.h"
#endif

static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";

/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "discgrp.h"
#include "discgrpP.h"

DiscGrp *
DiscGrpSetPreDraw( DiscGrp *dg, void (*predraw)() )
{
    dg->predraw = predraw;
    return (dg);
}

DiscGrpElList *
DiscGrpElListCreate (DiscGrpElList *exist, ...)
{
    va_list a_list;
    DiscGrpElList *dgellist;
    DiscGrpEl 	*passedin;
    Transform 	*tformin;
    ColorA  	*colorin;
    char	(*wordin)[DG_WORDLENGTH] = NULL;
    int 	*attrin;
    int		attr, copy = 1;
    int i;

    va_start (a_list, exist);
    if (exist == NULL) {
	dgellist = OOGLNewE(DiscGrpElList, "DiscGrpElListCreate discgrp");
	memset(dgellist, 0, sizeof(DiscGrpElList));
	}
    else (dgellist = exist);

    while ((attr = va_arg (a_list, int))) switch (attr) {
	case CR_NELEM:
	    dgellist->num_el = va_arg (a_list, int);
	    if (dgellist->el_list)  OOGLFree(dgellist->el_list); 
	    dgellist->el_list = OOGLNewNE(DiscGrpEl, dgellist->num_el, "DiscGrpElListCreate: unable to allocate");
	    memset(dgellist->el_list, 0, sizeof(DiscGrpEl) * dgellist->num_el);
	    break;

	case DGCR_ELEM:		/* already packed up */
	    passedin = va_arg (a_list, DiscGrpEl *);
	    if (copy)	{
		for (i=0; i<dgellist->num_el; ++i)	{
		    dgellist->el_list[i] =  passedin[i];
		    TmCopy(passedin[i].tform, dgellist->el_list[i].tform);
		    }
		}
	    else	{
		if (dgellist->el_list) OOGLFree(dgellist->el_list);
		dgellist->el_list = passedin;
		}
	    break;

	case CR_ELEM:
	    tformin = va_arg (a_list, Transform *);
	    for (i=0; i<dgellist->num_el; ++i)	
		    TmCopy(tformin[i], dgellist->el_list[i].tform);
	    break;

	case CR_COLOR:
	    colorin = va_arg (a_list, ColorA *);
	    for (i=0; i<dgellist->num_el; ++i)	
		   dgellist->el_list[i].color = colorin[i];
	    break;

	/* we assume attributes is constant for this list */
	case DGCR_ATTRIBUTE:
	    attr = va_arg (a_list, int);
	    for (i=0; i<dgellist->num_el; ++i)	
		   dgellist->el_list[i].attributes = attr;
	    break;

	/* or can be passed in as a list */
	case DGCR_ATTRIBUTELIST:
	    attrin = va_arg (a_list, int *);
	    for (i=0; i<dgellist->num_el; ++i)	
		   dgellist->el_list[i].attributes = attrin[i];
	    break;

	case DGCR_WORD:
	    wordin = (char (*)[DG_WORDLENGTH] ) va_arg (a_list, char *);
	    for (i=0; i<dgellist->num_el; ++i)	
		   strcpy(dgellist->el_list[i].word,  wordin[i]);
	    break;

	default:
	    if (GeomDecorate (dgellist, &copy, attr, &a_list)) {
		GeomError (0, "DiscGrpElListCreate: Undefined option: %d", attr);
		OOGLFree (dgellist);
		return NULL;
	    }

	}
    va_end (a_list);
    return(dgellist);
}

DiscGrp *
DiscGrpCreate (DiscGrp *exist, GeomClass *classp, va_list *a_list)
{
    register DiscGrp *discgrp;
    int		attr, copy = 1;
    int		i;

    if (exist == NULL) {
	discgrp = OOGLNewE(DiscGrp, "DiscGrpCreate discgrp");
	memset(discgrp, 0, sizeof(DiscGrp));
	GGeomInit (discgrp, classp, DISCGRPMAGIC, NULL);
	discgrp->flag = DG_DDSLICE;  /* default is to slice off cusps */
	discgrp->name = NULL;
	discgrp->comment = NULL;
	discgrp->attributes = 0;
	discgrp->c2m = NULL;
	discgrp->fsa = NULL;
	discgrp->dimn = 3;	/* make a guess */
	discgrp->gens = NULL;
	discgrp->nhbr_list = NULL;
	discgrp->big_list = NULL;
	discgrp->cpoint = DGorigin;
	discgrp->camgeom = NULL;
	discgrp->ddgeom = NULL;
	discgrp->geom = NULL;
	discgrp->camgeomhandle = NULL;
	discgrp->ddgeomhandle = NULL;
	discgrp->geomhandle = NULL;
	discgrp->scale = .2;
	discgrp->enumdepth = 2;
	discgrp->enumdist = 5.0;
	discgrp->drawdist = 5.0;
	discgrp->predraw = NULL;
    } else {
	/* Check that exist is a DiscGrp... */
	discgrp = exist;
    }

    while ((attr = va_arg (*a_list, int))) switch (attr) {
	case DGCR_NAME:
	    discgrp->name = va_arg (*a_list, char *);
	    break;

	case DGCR_COMMENT:
	    discgrp->comment = va_arg (*a_list, char *);
	    break;

	case DGCR_FLAG:
	    discgrp->flag = va_arg (*a_list, int);
	    break;

	case DGCR_ATTRIBUTE:
	    discgrp->attributes = va_arg (*a_list, int);
	    break;

	case DGCR_GENS:
	    discgrp->gens = va_arg (*a_list, DiscGrpElList *);
	    break;

	case DGCR_BIGLIST:
	    discgrp->big_list = va_arg (*a_list, DiscGrpElList *);
	    break;

	case DGCR_CPOINT:
	    {
	    HPoint3 *cptr;
	    cptr = va_arg (*a_list, HPoint3 *);
	    discgrp->cpoint = *cptr;
	    }
	    break;

	case DGCR_ENUMDEPTH:
	    discgrp->enumdepth = va_arg (*a_list, int);
	    break;

	case DGCR_ENUMDIST:
	    discgrp->enumdist = ((float) va_arg (*a_list, double));
	    break;

	case DGCR_SCALE:
	    discgrp->scale = ((float) va_arg (*a_list, double));
	    break;

	case DGCR_CAMGEOM:
	    discgrp->camgeom = va_arg (*a_list, Geom *);
	    break;

	case DGCR_DDGEOM:
	    discgrp->ddgeom = va_arg (*a_list, Geom *);
	    break;

	case CR_GEOM:
	    discgrp->geom = va_arg (*a_list, Geom *);
	    break;

	default:
	    if (GeomDecorate (discgrp, &copy, attr, a_list)) {
		GeomError (0, "DiscGrpCreate: Undefined option: %d", attr);
		OOGLFree (discgrp);
		return NULL;
	    }
    }
    return discgrp;
}


