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

 /*
  * Load a PolyList object from an "off"-format file.
  * 
  */

#include <string.h>
#include <stdlib.h>
#include "polylistP.h"

#define	SIZEOF_CMAP	256
static ColorA	*colormap;

static void
LoadCmap(char *file)
{
    FILE *fp;
    colormap = OOGLNewNE(ColorA, 256, "PolyList colormap");
    if((file = findfile(NULL, file)) != NULL &&
			   (fp = fopen(file,"r")) != NULL) {
	fgetnf(fp, SIZEOF_CMAP*4, (float *)colormap, 0);
	fclose(fp);
    }
}

PolyList *
PolyListFLoad(FILE *file, char *fname)
{
    register PolyList *pl;
    int edges;
    register int i;
    char *token;
    register Vertex *v;
    int binary = 0;
    int preread = 0;
    int headerseen = 0;
    int flags = 0;
    int makenorm = 0;
    int dimn = 3;
    static ColorA black = { 0,0,0,0 };

    if (file == NULL)
	return NULL;

    token = GeomToken(file);

    if(strncmp(token, "ST", 2) == 0) {
	flags |= PL_HASST;
	token += 2;
    }
    if(*token == 'C') {
	flags |= PL_HASVCOL;	/* Per-vertex colors */
	token++;
    }
    if(*token == 'N') {
	flags |= PL_HASVN;	/* Per-vertex normals */
	token++;
    }
    if(*token == '4') {
	dimn = 4;
	token++;
    }
    if(*token == 'S') {		/* "smooth surface": we compute vtx normals */
	makenorm = 1;
	token++;
    }
    if(strcmp(token, "OFF") == 0) {
	headerseen = 1;
	if(fnextc(file, 1) == 'B' && fexpectstr(file, "BINARY") == 0) {
	    binary = 1;
	    if(fnextc(file, 1) != '\n')	/* Expect \n after BINARY */
		return NULL;
	    (void) getc(file);		/* Toss \n */
	}
    } else {			/* Hack, in case first token was a number */
	if(dimn == 4)
	    token--;
	preread = strtol(token, &token, 10);
	if(*token != '\0')
	    return NULL;
	/* May be a header-less OFF */
	flags = 0;
	dimn = 3;
    }


    pl = OOGLNewE(PolyList, "PolyListFLoad PolyList");
    GGeomInit(pl, PolyListMethods(), PLMAGIC, NULL);
    pl->p = NULL;
    pl->vl = NULL;
    pl->n_verts = preread;  /* In case prefetched token was our vert count */
    pl->flags = flags;
    pl->geomflags = (dimn == 4) ? VERT_4D : 0;

    if((!preread && fgetni(file, 1, &pl->n_verts, binary) <= 0) ||
       fgetni(file, 1, &pl->n_polys, binary) <= 0 ||
       fgetni(file, 1, &edges, binary) <= 0) {
		if(headerseen)
		    OOGLSyntax(file, "PolyList %s: Bad vertex/face/edge counts", fname);
		goto bogus;
    }

    pl->vl = OOGLNewNE(Vertex, pl->n_verts, "PolyListFLoad vertices");

    for(v = pl->vl, i = 0; i < pl->n_verts; v++, i++) {
	if((fgetnf(file, dimn, (float *)&v->pt, binary) < dimn)
	   ||
	   (flags & PL_HASVN && fgetnf(file, 3, (float *)&v->vn, binary) < 3)
	   ||
	   (flags & PL_HASVCOL && fgetnf(file, 4, (float *)&v->vcol, binary) < 4)
	   ||
	   (flags & PL_HASST && fgetnf(file, 2, &v->st[0], binary) < 2)
	   ) {
		OOGLSyntax(file, "PolyList %s: Bad vertex %d (of %d)", fname, i, pl->n_verts);
		goto bogus;
	}
	if (dimn == 3)  v->pt.w = 1.0;
    }

    pl->p = OOGLNewNE(Poly, pl->n_polys, "PolyListFLoad polygons");
    for(i = 0; i < pl->n_polys; ) {
	register Poly *p;
	register int k,index;

	p = &pl->p[i];
	if(fgetni(file, 1, &p->n_vertices, binary) <= 0 || p->n_vertices <= 0) {
	   OOGLSyntax(file, "PolyList %s: bad %d'th polygon (of %d)",
		fname, i, pl->n_polys);
	   goto bogus_face;
	}
	p->v = OOGLNewNE(Vertex *, p->n_vertices, "PolyListFLoad polygon");
	i++;

	for(k = 0; k < p->n_vertices; k++) {
	    int index;

	    if(fgetni(file, 1, &index, binary) <= 0 ||
	       index < 0 || index >= pl->n_verts) {
		    OOGLSyntax(file, "PolyList: %s: bad index %d on %d'th polygon (of %d)", 
			fname, index, i, p->n_vertices);
		    goto bogus_face;
	    }
	    p->v[k] = &pl->vl[index];
	}

	/* Pick up the color, if any.
	 * In ASCII format, take whatever's left before end-of-line
	 * (or end-of-file, or close-brace)
	 */
	p->pcol = black;
	if(binary) {
	    int ncol;

	    if(fgetni(file, 1, &ncol, 1) <= 0
	       || fgetnf(file, ncol, (float *)&p->pcol, 1) < ncol)
		goto bogus_face_color;
	    k = ncol;
	} else {
	    int c;
	    for(k = 0; k < 4 &&
		    (c = fnextc(file, 1)) != '\n' && c!='}' && c!=EOF; k++) {
		if(fgetnf(file, 1, ((float *)(&p->pcol))+k, 0) < 1) {
		    goto bogus_face_color;
		}
	    }
	}

	if((flags & PL_HASVCOL) == 0) {
	    if(k > 0)
		pl->flags |= PL_HASPCOL;

	    if(k != 1 && (p->pcol.r>1||p->pcol.g>1||p->pcol.b>1||p->pcol.a>1)) {
		p->pcol.r /= 255, p->pcol.g /= 255,
		p->pcol.b /= 255, p->pcol.a /= 255;
	    }

	    switch(k) {
	    case 0:
		p->pcol.r = 170/255.0;		/* Gray */
		p->pcol.g = p->pcol.r;
	    case 2:
		p->pcol.b = p->pcol.g;
	    case 3:
		p->pcol.a = 170/255.0;		/* Semitransparent */
		break;
	    case 1:				/* use colormap */
		if ( colormap == NULL )
			LoadCmap("cmap.fmap");
		index = p->pcol.r;
		if((unsigned)index >= SIZEOF_CMAP) index = 0;
		p->pcol = colormap[index];
	    }				/* case 4, all components supplied */
	}
    }

    if(makenorm && !(flags & PL_HASVN)) {
	pl->flags |= PL_HASVN;
	pl->flags &= ~PL_HASPN;		/* Leave vertex-normals only */
    }
    return pl;

  
  bogus_face_color:
    OOGLSyntax(file, "PolyList %s: bad face color on face %d (of %d)",
	fname, i, pl->n_polys);
  bogus_face:
    pl->n_polys = i;
  bogus:
    GeomDelete((Geom *)pl);
    return NULL;
}
