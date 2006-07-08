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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "skelP.h"

static GeomClass *aSkelMethods = NULL;

int SkelPresent = 1;

static char skelName[] = "skel";

char *
SkelName()
{
	return skelName;
}


GeomClass      *
SkelMethods()
{
	if (!aSkelMethods)
	{
		aSkelMethods = GeomClassCreate(skelName);

		aSkelMethods->name = SkelName;
		aSkelMethods->methods = (GeomMethodsFunc *) SkelMethods;
		aSkelMethods->create = (GeomCreateFunc *) SkelCreate;
		aSkelMethods->fload = (GeomFLoadFunc *) SkelFLoad;
		aSkelMethods->fsave = (GeomFSaveFunc *) SkelFSave;
		aSkelMethods->bound = (GeomBoundFunc *) SkelBound;
		aSkelMethods->Delete = (GeomDeleteFunc *) SkelDelete;
		aSkelMethods->draw = (GeomDrawFunc *) SkelDraw;
		aSkelMethods->copy = (GeomCopyFunc *) SkelCopy;
		aSkelMethods->pick = (GeomPickFunc *) SkelPick;
		aSkelMethods->transform = 
		  (GeomTransformFunc *) SkelTransform;
		aSkelMethods->transformto =
		  (GeomTransformToFunc *) SkelTransform; /* == TransformTo */
	}
	return aSkelMethods;
}
