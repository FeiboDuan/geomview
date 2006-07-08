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

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

/* Authors: Olaf Holt */


#include <ooglutil.h>
#include "hpointn.h"
#include <math.h>

HPointN *
HPtNCreate(int dim, const HPtNCoord *vec)
{
    HPointN *pt = OOGLNewE(HPointN, "new HPointN");
/*    pt->space = 0;  			*/
    if(dim <= 0) dim = 1;
    pt->dim = dim;
    pt->v = OOGLNewNE(HPtNCoord, dim, "new HPointN data");
	if(vec == NULL) {
	    memset(pt->v, 0, dim*sizeof(HPtNCoord));
	    pt->v[dim-1] = 1;
	} else
	    memcpy(pt->v, vec, dim*sizeof(HPtNCoord));
    return pt;
}

void
HPtNDelete(HPointN *pt)
{
    if(pt) {
	if(pt->v) OOGLFree(pt->v);
	OOGLFree(pt);
    }
}
