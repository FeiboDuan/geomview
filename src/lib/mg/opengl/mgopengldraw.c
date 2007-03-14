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

#if HAVE_CONFIG_H
# include "config.h"
#endif

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "mgP.h"
#include "mgopenglP.h"
#include "polylistP.h"
#include "quadP.h"
#include "bsptreeP.h"

#ifdef GLUT
# include <GL/glut.h>
#else
#define GL_GLEXT_PROTOTYPES
# include <GL/gl.h>
#endif

#ifndef alloca
#include <alloca.h>
#endif

void	mgopengl_polygon( int nv, HPoint3 *v, int nn, Point3 *n,
	      	          int nc,ColorA *c );
void	mgopengl_line( HPoint3 *p1, HPoint3 *p2 );
void	mgopengl_polyline( int nv, HPoint3 *verts, int nc, ColorA *colors, int wrap );
void	mgopengl_polylist(int np, Poly *p, int nv, Vertex *v, int plflags);
void	mgopengl_drawnormal(HPoint3 *p, Point3 *n);

void	mgopengl_closer();
void	mgopengl_farther();

#ifndef NO_ZNUDGE
# define mgopengl_v4ffunc(v)  glVertex4fv(&(v)->x);
#else
# define mgopengl_v4ffunc(v)  mgopengl_v4fcloser(v)
#endif


/*-----------------------------------------------------------------------
 * Function:	mgopengl_polygon
 * Description:	draw a polygon
 * Author:	mbp, munzner
 * Date:	Mon Jul 29 16:53:56 1991
 * Notes:	See mg.doc.
 *
 *              do later: Different shading cases separated into different
 *		loops for speed.
 */
void
mgopengl_polygon(int nv,  HPoint3 *V, 
		 int nn,  Point3 *N, 
		 int nc,  ColorA *C)
{
  int i;	
  HPoint3 *v;
  Point3 *n;
  ColorA *c;
  int cinc, ninc;
  int flag;


  flag = _mgc->astk->ap.flag;
  if ((_mgc->astk->mat.override & MTF_DIFFUSE) &&
      !(_mgc->astk->flags & MGASTK_SHADER))
    nc = 0;
  cinc = (nc > 1);
  ninc = (nn > 1);
  if(nc == 0)
    C = (ColorA*)&_mgc->astk->ap.mat->diffuse;


  /* reestablish correct drawing color if necessary */

  if (flag & APF_FACEDRAW) {
    MAY_LIGHT();
    glColorMaterial(GL_FRONT_AND_BACK, _mgopenglc->lmcolor);
    glEnable(GL_COLOR_MATERIAL);
    glBegin(GL_POLYGON);
    if (nc <= 1)
      D4F(&(_mgc->astk->ap.mat->diffuse));
    for (n = N, c = C, v = V, i = 0; i<nv; ++i, ++v) {
      if (nc) { D4F(c); c += cinc; }
      if (nn) { N3F(n,v); n += ninc; }
      glVertex4fv((float *)v);
    }
    glEnd();
  }

  if( flag & (APF_EDGEDRAW|APF_NORMALDRAW) ) {
    if(_mgopenglc->znudge) mgopengl_closer();
    glDisable(GL_COLOR_MATERIAL);
    DONT_LIGHT();
    if (flag & APF_EDGEDRAW) {
      glColor3fv((float *)&_mgc->astk->ap.mat->edgecolor);
      glBegin(GL_LINE_LOOP);
      for (v = V, i = 0; i<nv; ++i, ++v)
	mgopengl_v4ffunc(v);
      glEnd();
    }

    if (flag & APF_NORMALDRAW) {
      glColor3fv((float *)&_mgc->astk->ap.mat->normalcolor);
      for (n = N, v = V, i = 0; i<nv; ++i, ++v, n += ninc) {
	mgopengl_drawnormal(v, n);
      }
    }
    if(_mgopenglc->znudge) mgopengl_farther();
  }
}

void
mgopengl_quads(int count, HPoint3 *V, Point3 *N, ColorA *C, int qflags)
{
  int i;	
  HPoint3 *v;
  Point3 *n;
  ColorA *c;
  int flag;

#define QUAD(stuff)  {				\
    int k = 4;					\
    glBegin(GL_POLYGON);			\
    do { stuff; } while(--k > 0);		\
    glEnd();					\
  }

  if(count <= 0)
    return;

  flag = _mgc->astk->ap.flag;
  if ((_mgc->astk->mat.override & MTF_DIFFUSE) &&
      !(_mgc->astk->flags & MGASTK_SHADER)) {
    C = NULL;
  }

  /* reestablish correct drawing color if necessary */

  if ((flag & APF_FACEDRAW) && !(qflags & GEOM_ALPHA)) {

    glColorMaterial(GL_FRONT_AND_BACK, _mgopenglc->lmcolor);
    glEnable(GL_COLOR_MATERIAL);

    MAY_LIGHT();

    i = count;
    v = V; c = C; n = N;
    if(c) {
      if(n) {
	do {
	  QUAD( (D4F(c++), N3F(n++,v), glVertex4fv((float*)v++)) );
	} while(--i > 0);
      } else {
	/* Colors, no normals */
	do {
	  QUAD( (D4F(c++), glVertex4fv((float*)v++)) );
	} while(--i > 0);
      }
    } else {
      c = (ColorA*)&_mgc->astk->ap.mat->diffuse;
      if(n) {
	D4F(c);
	do {
	  QUAD( (N3F(n++, v), glVertex4fv((float*)v++)) );
	} while(--i > 0);
      } else {
	D4F(c);
	do {
	  QUAD( (glVertex4fv((float*)v++)) );
	} while(--i > 0);
      }
    }
  }

  if( flag & (APF_EDGEDRAW|APF_NORMALDRAW) ) {
    if(_mgopenglc->znudge) mgopengl_closer();
    glDisable(GL_COLOR_MATERIAL);
    DONT_LIGHT();

    if (flag & APF_EDGEDRAW) {
      glColor3fv((float *)&_mgc->astk->ap.mat->edgecolor);
      i = count; v = V;
      do {
	int k = 4;
	glBegin(GL_LINE_LOOP);
	do { mgopengl_v4ffunc(v++); } while(--k > 0);
	glEnd();
      } while(--i > 0);
    }

    if (flag & APF_NORMALDRAW && N) {
      glColor3fv((float *)&_mgc->astk->ap.mat->normalcolor);
      i = count*4; v = V; n = N;
      do {
	mgopengl_drawnormal(v++, n++);
      } while(--i > 0);
    }
    if(_mgopenglc->znudge) mgopengl_farther();
  }
}


void mgopengl_line( HPoint3 *p1, HPoint3 *p2 )
{
  DONT_LIGHT();
  glBegin(GL_LINE_STRIP);
  glVertex4fv((float *)p1);
  glVertex4fv((float *)p2);
  glEnd();
}

void mgopengl_point(HPoint3 *v)
{
  HPoint3 a;
  HPoint3 *p, *q;
  float vw;

  DONT_LIGHT();

  if(_mgc->astk->ap.linewidth > 1) {
    
    if(!(_mgc->has & HAS_POINT))
      mg_makepoint();
    /* Compute w component of point after projection to screen */
    vw = v->x * _mgc->O2S[0][3] + v->y * _mgc->O2S[1][3]
      + v->z * _mgc->O2S[2][3] + v->w * _mgc->O2S[3][3];
    if(vw <= 0) return;
    
#define  PUT(p)					\
    a.x = v->x + p->x*vw; a.y = v->y + p->y*vw;	\
    a.z = v->z + p->z*vw; a.w = v->w + p->w*vw;	\
    glVertex4fv((float *)&a)
	  
    p = VVEC(_mgc->point, HPoint3);
    q = p + VVCOUNT(_mgc->point);
    glBegin(GL_TRIANGLE_STRIP);
    PUT(p);
    do {
      p++;
      PUT(p);
      if(p >= q) break;
      q--;
      PUT(q);
    } while(p < q);
    glEnd();
  } else {
    glBegin(GL_POINTS);
    glVertex4fv((float *)v);
    glEnd();
  }
}
  

void mgopengl_polyline( int nv, HPoint3 *v, int nc, ColorA *c, int wrapped )
{

  DONT_LIGHT();

  /* note we don't reset to current material color because we could be
   * in the middle of a list of lines and should inherit the color from 
   * the last color call.
   */
  
  if(!(wrapped & 2)) {
    /* First member of batch */
    if(_mgopenglc->znudge) mgopengl_closer();
    if(nc)
      glDisable(GL_COLOR_MATERIAL);
  }
  if (nv == 1) {
    if(nc > 0) glColor4fv((float *)c);
    mgopengl_point(v);
  } 
  else if(nv > 0) {
    glBegin(GL_LINE_STRIP);
    if(wrapped & 1) {
      if(nc > 0) glColor4fv((float *)(c + nc - 1));
      mgopengl_v4ffunc(v + nv - 1);
    }

    do {
      if(--nc >= 0) glColor4fv((float *)c++);
      mgopengl_v4ffunc(v++);
    } while(--nv > 0);
    glEnd();
  }
  if(!(wrapped & 4) && _mgopenglc->znudge) mgopengl_farther();
}


/* Slave routine for mgopengl_trickypolygon() below. */
static int tessplflags;

void tessvert(Vertex *vp)
{
  if (tessplflags & PL_HASVCOL) D4F(&vp->vcol);
  if (tessplflags & PL_HASVN) N3F(&vp->vn, &vp->pt);
  if (tessplflags & PL_HASST) glTexCoord2fv((GLfloat *)&vp->st);
  glVertex4fv(&vp->pt.x);
}

void tesscombine(GLdouble coords[3], GLdouble *vertex_data[4],
                GLfloat weight[4], Vertex **dataOut, Vertex ***ptr_extra_verts)
{
   Vertex *vertex;
   int list_len, alloc_cnt, realloc_cnt;

   vertex = OOGLNewE(Vertex, "extra vertex");
   vertex->pt.x = coords[0];
   vertex->pt.y = coords[1];
   vertex->pt.z = coords[2];
   vertex->pt.w = 1;
   *dataOut = vertex;

   /* Keep list of pointers to allocated Vertex's. 
    * First Vertex is a dummy, list length in pt.x, allocated size in pt.y */
   list_len = (*ptr_extra_verts)[0]->pt.x + 0.5;
   alloc_cnt = (*ptr_extra_verts)[0]->pt.y + 0.5;
   if(list_len == alloc_cnt) {
      realloc_cnt = alloc_cnt * 2; /* Double allocated size */
      (*ptr_extra_verts)[0]->pt.y = realloc_cnt;
      *ptr_extra_verts = OOGLRenewNE(Vertex *, *ptr_extra_verts, realloc_cnt,
				     "renew extra vertex pointers");
   }
   (*ptr_extra_verts)[list_len] = vertex;
   (*ptr_extra_verts)[0]->pt.x = list_len + 1;
}


/*
 * Called when we're asked to deal with a possibly-concave polygon.
 * Note we can only be called if APF_CONCAVE mode is set.
 *
 * We assume that we are called with an actually concave polygon; this
 * is indicated by (p->flags & POLY_CONCAVE).
 */
static void
mgopengl_trickypolygon( Poly *p, int plflags ) 
{
  int i, list_len;
  const int init_alloc_sz=2;
  Vertex *vp, **extra_verts;
  static GLUtesselator *glutess;
  double *dpts = (double *)alloca(3*p->n_vertices*sizeof(double));
  double *dp;

  if(glutess == NULL) {
    /* Create GLU-library triangulation handle, just once */
    glutess = gluNewTess();
    gluTessProperty(glutess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);
#ifdef _WIN32	/* Windows idiocy.  We shouldn't need to cast standard funcs! */
    gluTessCallback(glutess, GLU_BEGIN, (GLUtessBeginProc)glBegin);
    gluTessCallback(glutess, GLU_VERTEX, (GLUtessVertexProc)tessvert);
    gluTessCallback(glutess, GLU_TESS_COMBINE_DATA, (GLUtessCombineDataProc)tesscombine);
    gluTessCallback(glutess, GLU_END, (GLUtessEndProc)glEnd);
#else		/* Any reasonable OpenGL implementation */
    gluTessCallback(glutess, GLU_BEGIN, glBegin);
    gluTessCallback(glutess, GLU_VERTEX, tessvert);
    gluTessCallback(glutess, GLU_TESS_COMBINE_DATA, tesscombine);
    gluTessCallback(glutess, GLU_END, glEnd);
#endif
  }
  
  extra_verts = OOGLNewNE(Vertex *, init_alloc_sz, "extra vertices");
  extra_verts[0] = OOGLNewE(Vertex, "Dummy Header Vertex");
  extra_verts[0]->pt.x = 1;             /* Holder for list length */
  extra_verts[0]->pt.y = init_alloc_sz; /* Holder for allocation size */
  tessplflags = plflags;
  gluTessBeginPolygon(glutess, &extra_verts);
  gluTessBeginContour(glutess);
  for(i = 0, dp = dpts; i < p->n_vertices; i++, dp += 3) {
    vp = (p->v)[i];
    dp[0] = vp->pt.x / vp->pt.w;
    dp[1] = vp->pt.y / vp->pt.w;
    dp[2] = vp->pt.z / vp->pt.w;
    gluTessVertex(glutess, dp, vp);
  }
  gluTessEndContour(glutess);
  gluTessEndPolygon(glutess);

  list_len = extra_verts[0]->pt.x;
  for(i=0; i<list_len; i++) {
    OOGLFree(extra_verts[i]);
  }
  OOGLFree(extra_verts);
}

/* The work-horse for mgopengl_bsptree():
 *
 * Strategy: first render everything back of the side the camera is
 * on, then everything coincident with the currentd node's plane, then
 * then everything in front of the side the camera is on.
 *
 * We assume the camera is not coincident with any one of the
 * BSPTree's planes.
 *
 * plfl_and and plfl_or are flags to be added or substracted from the
 * node's plflags to implement appearance overrides.
 */
static void mgopengl_bsptree_recursive(BSPTreeNode *tree,
				       HPoint3 *camera,
				       int *plfl_and,
				       int *plfl_or,
				       const void **cur_app)
{
  HPt3Coord scp;
  int sign;
  BSPTreeNode *first, *last;
  PolyListNode *plist;
  const Appearance *ap;
  Material *mat;

  scp = HPt3DotPt3(camera, (Point3 *)(void *)&tree->plane) - tree->plane.w;
  sign = fpos(scp) - fneg(scp);
  
  if (sign >= 0) {
    first = tree->back;
    last  = tree->front;
  } else {
    first = tree->front;
    last  = tree->back;
  }
  
  /* render all polygons back of us */
  if (first) {
    mgopengl_bsptree_recursive(first, camera, plfl_and, plfl_or, cur_app);
  }

  for (plist = tree->polylist; plist; plist = plist->next) {
    Vertex **v;
    Poly   *p      = plist->poly;
    int    j       = p->n_vertices;
    int    plflags = p->flags;
    bool   apchg   = false;

    if (*plist->tagged_app == NULL) {
      continue;
    }
    
    if (*cur_app != *plist->tagged_app) {

      apchg = true;

      /* set our appearance now */
      mgtaggedappearance(*plist->tagged_app);

      /* record our appearance as the current one */
      *cur_app = *plist->tagged_app;

      ap = mggetappearance();
    } else {
      ap = mggetappearance();
    }

    if (!(ap->flag & APF_FACEDRAW) || !(ap->flag & APF_TRANSP)) {
      continue;
    }

    mat = ap->mat;

    if (apchg) {

      /* set new plfl_and/_or values */
      *plfl_and = ~0;
      *plfl_or  =  0;
      switch(ap->shading) {
      case APF_FLAT:   *plfl_and &= ~PL_HASVN; break;
      case APF_SMOOTH: *plfl_and &= ~PL_HASPN; break;
      default:         *plfl_and &= ~(PL_HASVN|PL_HASPN); break;
      }

      if (mat->override & MTF_DIFFUSE) {
	if (!(_mgc->astk->flags & MGASTK_SHADER)) {
	  /* software shading will not work yet */
	  *plfl_and &= ~GEOM_COLOR;
	}
      }

      /* Decide whether this polygon possibly has an alpha channel.
       * Same logic as in GeomHasAlpha().
       */
      if ((ap->flag & APF_TEXTURE) &&
	  ap->tex && ap->tex->apply != TXF_DECAL &&
	  ap->tex->image && (ap->tex->image->channels % 2 == 0)) {
	*plfl_or |= COLOR_ALPHA;
      } else if ((mat->valid & MTF_ALPHA) &&
		 ((mat->override & MTF_ALPHA) ||
		  !((plflags & *plfl_and) & (GEOM_COLOR)))) {
	if (mat->diffuse.a < 1.0) {
	  *plfl_or  |= COLOR_ALPHA;
	} else {
	  *plfl_and &= ~COLOR_ALPHA;
	}
      }
      if (!(ap->flag & APF_TEXTURE) || (ap->tex == NULL)) {
	*plfl_and &= ~PL_HASST;
      }

      glColorMaterial(GL_FRONT_AND_BACK, _mgopenglc->lmcolor);
      glEnable(GL_COLOR_MATERIAL);
      MAY_LIGHT();
    }

    plflags &= *plfl_and;
    plflags |= *plfl_or;

    /* We may want to do something else here if ever we should start
     * to use BSP-tress for the buffer etc. render engines. For now we
     * only render translucent objects here, and leave the rest to the
     * ordinary drawing engines.
     */
    if (!(plflags & COLOR_ALPHA)) {
      continue;
    }

#if DEBUG_BSPTREE /* make the sub-division visible */
    if (ap->flag & (APF_EDGEDRAW|APF_NORMALDRAW)) {
      if(_mgopenglc->znudge) mgopengl_closer();
      glDisable(GL_COLOR_MATERIAL);
      DONT_LIGHT();

      if (ap->flag & APF_EDGEDRAW) {
	glColor3fv((float *)&ap->mat->edgecolor);
	glBegin(GL_LINE_LOOP);
	for (j=0, v=p->v; j < p->n_vertices; j++, v++) {
	  mgopengl_v4ffunc(&(*v)->pt);
	}
	glEnd();
      }

      if (ap->flag & APF_NORMALDRAW) {
	glColor3fv((float *)&_mgc->astk->ap.mat->normalcolor);
	if (plflags & PL_HASPN) {
	  for (j=0, v = p->v; j < p->n_vertices; j++, v++)
	    mgopengl_drawnormal(&(*v)->pt, &p->pn);
	} else if (plflags & PL_HASVN) {
	  for (v = p->v, j = 0; j < p->n_vertices; j++, v++) {
	    mgopengl_drawnormal(&(*v)->pt, &(*v)->vn);
	  }
	}
      }
      if(_mgopenglc->znudge) mgopengl_farther();

      apchg = 1;
      glColorMaterial(GL_FRONT_AND_BACK, _mgopenglc->lmcolor);
      glEnable(GL_COLOR_MATERIAL);
      MAY_LIGHT();
    }
#endif

    if (apchg) {
      /* Disable write access to the depth buffer and enable
       * alpha-blending. The blend function used here will work
       * without alpha-buffer support, because only the source alpha
       * value is used.
       */
      glDepthMask(GL_FALSE);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_BLEND);

      /* Load textures if ap has changed */
      if (plflags & PL_HASST) {
	mgopengl_needtexture();
      }
    }

    /* reestablish correct drawing color if necessary*/
    if (!(plflags & (PL_HASPCOL|PL_HASVCOL))) {
      D4F(&(mat->diffuse));
    }

    if (plflags & PL_HASPCOL) {
      D4F(&p->pcol);
    }
    if (plflags & PL_HASPN) {
      N3F(&p->pn, &(*p->v)->pt);
    }
    v = p->v;

    /* normal algorithm */
    glBegin(GL_POLYGON);
    switch (plflags & (PL_HASVCOL|PL_HASVN|PL_HASST)) {
    case 0:
      do {
	glVertex4fv((float *)&(*v)->pt);
	v++;
      } while(--j > 0);
      break;
    case PL_HASVCOL:
      do {
	D4F(&(*v)->vcol);
	glVertex4fv((float *)&(*v)->pt);
	v++;
      } while(--j > 0);
      break;
    case PL_HASVN:
      do {
	N3F(&(*v)->vn, &(*v)->pt);
	glVertex4fv((float *)&(*v)->pt);
	v++;
      } while(--j > 0);
      break;
    case PL_HASVCOL|PL_HASVN:
      do {
	D4F(&(*v)->vcol);
	N3F(&(*v)->vn, &(*v)->pt);
	glVertex4fv((float *)&(*v)->pt);
	v++;
      } while(--j > 0);
      break;
    default:
      do {
	if (plflags & PL_HASVCOL) D4F(&(*v)->vcol);
	if (plflags & PL_HASVN) N3F(&(*v)->vn, &(*v)->pt);
	if (plflags & PL_HASST) glTexCoord2fv((GLfloat *)&(*v)->st);
	glVertex4fv((float *)&(*v)->pt);
	v++;
      } while(--j > 0);
      break;
    }
    glEnd(); /* GL_POLYGON */
  }
  
  /* Render everything in front of us */
  if (last) {
    mgopengl_bsptree_recursive(last, camera, plfl_and, plfl_or, cur_app);
  }
}

/*-----------------------------------------------------------------------
 * Function:	mgopengl_bsptree
 * Description:	draws a bsptree: binary space partition tree
 * Author:	Claus-Justus Heine
 * Date:	2006
 */

void mgopengl_bsptree(BSPTree *bsptree)
{
  int plfl_and = ~0, plfl_or = 0;
  const void *ap;

  if (!bsptree->tree) {
    /* The tree can be empty, e.g. when no object had a translucent
     * polygon, or when translucent objects are disabled globally.
     */
    return;
  }

  mgopengl_new_translucent(_mgc->xstk->T);

  /* First determine the position of the camera in the _current_
   * coordinate system. We do assume that all tree nodes share the
   * same coordinate system.
   *
   * This means that transparent INSTs with fancy co-ordinate systems
   * are not handled correctly here; or that the high-level code has
   * to convert them first.
   */
  if(!(_mgc->has & HAS_CPOS)) {
    mg_findcam();
  }

  ap = NULL;
  mgopengl_bsptree_recursive(bsptree->tree, &_mgc->cpos,
			     &plfl_and, &plfl_or, &ap);

  mgopengl_end_translucent();
}

/*-----------------------------------------------------------------------
 * Function:	mgopengl_polylist
 * Description:	draws a Polylist: collection of Polys
 * Author:	munzner
 * Date:	Wed Oct 16 20:21:56 1991
 * Notes:	see mg.doc
 */
void mgopengl_polylist(int np, Poly *_p, int nv, Vertex *V, int plflags)
{
  int i,j;
  Poly *p;
  Vertex **v, *vp;
  struct mgastk *ma = _mgc->astk;
  int flag, shading;
  int nonsurf = -1;

  flag = ma->ap.flag;
  shading = ma->ap.shading;

  switch(shading) {
  case APF_FLAT: plflags &= ~PL_HASVN; break;
  case APF_SMOOTH: plflags &= ~PL_HASPN; break;
  default: plflags &= ~(PL_HASVN|PL_HASPN); break;
  }

  if ((_mgc->astk->mat.override & MTF_DIFFUSE)) {
    if (!(_mgc->astk->flags & MGASTK_SHADER)) {
      plflags &= ~GEOM_COLOR;
    }
  }

  if ((flag & APF_FACEDRAW) && !(plflags & GEOM_ALPHA)) {
    glColorMaterial(GL_FRONT_AND_BACK, _mgopenglc->lmcolor);
    glEnable(GL_COLOR_MATERIAL);
    MAY_LIGHT();
    /* reestablish correct drawing color if necessary*/
    if (!(plflags & (PL_HASPCOL | PL_HASVCOL))) {
      D4F(&(ma->ap.mat->diffuse));
    }
    if ((_mgc->astk->ap.flag & APF_TEXTURE) && (_mgc->astk->ap.tex != NULL)) {
      if (plflags & PL_HASST)
	mgopengl_needtexture();
    } else {
      plflags &= ~PL_HASST;
    }

    for (p = _p, i = 0; i < np; i++, p++) {
      if (plflags & PL_HASPCOL)
	D4F(&p->pcol);
      if (plflags & PL_HASPN)
	N3F(&p->pn, &(*p->v)->pt);
      v = p->v;
      if((j = p->n_vertices) <= 2) {
	nonsurf = i;
      } else if ((p->flags & POLY_CONCAVE) ||
		 (p->n_vertices > 4 && (flag & APF_CONCAVE))) {
	mgopengl_trickypolygon(p, plflags);
      } else { /* normal algorithm */
	glBegin(GL_POLYGON);
	switch(plflags & (PL_HASVCOL|PL_HASVN|PL_HASST)) {
	case 0:
	  do {
	    glVertex4fv((float *)&(*v)->pt);
	    v++;
	  } while(--j > 0);
	  break;
	case PL_HASVCOL:
	  do {
	    D4F(&(*v)->vcol);
	    glVertex4fv((float *)&(*v)->pt);
	    v++;
	  } while(--j > 0);
	  break;
	case PL_HASVN:
	  do {
	    N3F(&(*v)->vn, &(*v)->pt);
	    glVertex4fv((float *)&(*v)->pt);
	    v++;
	  } while(--j > 0);
	  break;
	case PL_HASVCOL|PL_HASVN:
	  do {
	    D4F(&(*v)->vcol);
	    N3F(&(*v)->vn, &(*v)->pt);
	    glVertex4fv((float *)&(*v)->pt);
	    v++;
	  } while(--j > 0);
	  break;
	default:
	  do {
	    if (plflags & PL_HASVCOL) D4F(&(*v)->vcol);
	    if (plflags & PL_HASVN) N3F(&(*v)->vn, &(*v)->pt);
	    if (plflags & PL_HASST) glTexCoord2fv((GLfloat *)&(*v)->st);
	    glVertex4fv((float *)&(*v)->pt);
	    v++;
	  } while(--j > 0);
	  break;
	}
	glEnd();
      }
    }
  }

  if (flag & (APF_EDGEDRAW|APF_NORMALDRAW) || nonsurf >= 0) {
    if(_mgopenglc->znudge) mgopengl_closer();
    glDisable(GL_COLOR_MATERIAL);
    DONT_LIGHT();

    if (flag & APF_EDGEDRAW) {
      glColor3fv((float *)&_mgc->astk->ap.mat->edgecolor);
      for (p = _p, i = 0; i < np; i++, p++) {
	glBegin(GL_LINE_LOOP);
	for (j=0, v=p->v; j < p->n_vertices; j++, v++) {
	  mgopengl_v4ffunc(&(*v)->pt);
	}
	glEnd();
      }
    }

    if (flag & APF_NORMALDRAW) {
      glColor3fv((float *)&_mgc->astk->ap.mat->normalcolor);
      if (plflags & PL_HASPN) {
	for (p = _p, i = 0; i < np; i++, p++) {
	  for (j=0, v=p->v; j < p->n_vertices; j++, v++)
	    mgopengl_drawnormal(&(*v)->pt, &p->pn);
	}
      } else if (plflags & PL_HASVN) {
	for (vp = V, i = 0; i < nv; i++, vp++) {
	  mgopengl_drawnormal(&vp->pt, &vp->vn);
	}
      }
    }


    if (nonsurf >= 0) {
      /* reestablish correct drawing color if necessary*/
      if (!(plflags & (PL_HASPCOL | PL_HASVCOL)))
	D4F(&(ma->ap.mat->diffuse));
      
      for(p = _p, i = 0; i <= nonsurf; p++, i++) {
	if (plflags & PL_HASPCOL)
	  D4F(&p->pcol);
	v = p->v;
	switch(j = p->n_vertices) {
	case 1:
	  if(plflags & PL_HASVCOL) glColor4fv((float *)&(*v)->vcol);
	  mgopengl_point(&(*v)->pt);
	  break;
	case 2:
	  glBegin(GL_LINE_STRIP);
	  do {
	    if(plflags & PL_HASVCOL) glColor4fv((float *)&(*v)->vcol);
	    glVertex4fv((float *)&(*v)->pt);
	    v++;
	  } while(--j > 0);
	  glEnd();
	  break;
	}
      }
    }
    if(_mgopenglc->znudge) mgopengl_farther();
  }
}

/*
 * Z-shift routines: for moving edges closer than faces, etc.
 */
void
mgopengl_init_zrange()
{
  struct mgopenglcontext *gl = _mgopenglc;

  gl->znudge = (double) _mgc->zfnudge * (gl->zmax - gl->zmin);

  gl->znear = gl->zmin + fabs(gl->znudge * (double)MAXZNUDGE);
  gl->zfar  = gl->zmax - fabs(gl->znudge * (double)MAXZNUDGE);
#ifndef NO_ZNUDGE
  glDepthRange(gl->znear, gl->zfar);
#endif
}

void
mgopengl_closer()
{
#ifndef NO_ZNUDGE
  glDepthRange( _mgopenglc->znear -= _mgopenglc->znudge,  _mgopenglc->zfar -= _mgopenglc->znudge );
#endif
}
void
mgopengl_farther()
{
#ifndef NO_ZNUDGE
  glDepthRange( _mgopenglc->znear += _mgopenglc->znudge,  _mgopenglc->zfar += _mgopenglc->znudge );
#endif
}

/* There is a basic problem now with 4-d points and 3-d normal vectors.
   For now, we'll just ignore the 4-th coordinate of the point when 
   computing the tip of the normal vector.  This will work OK with all
   existing models, but for genuine 4-d points it won't work.  But,
   come to think of it, what is the correct interpretation of the
   normal vector when the points live in 4-d?
*/
void
mgopengl_drawnormal(HPoint3 *p, Point3 *n)
{
  Point3 end, tp;
  HPt3Coord scale, w, s;

  if (p->w <= 0.0) return;
  if(p->w != 1) {
    HPt3ToPt3(p, &tp);
    p = (HPoint3 *)(void *)&tp;
  }

  scale = _mgc->astk->ap.nscale;
  if(_mgc->astk->ap.flag & APF_EVERT) {
    HPoint3 *cp = &_mgc->cpos;
    if (!(_mgc->has & HAS_CPOS)) {
      mg_findcam();
    }
    if ((w = cp->w) != 1.0 && w != 0.0) {
      s = (p->x*w-cp->x)*n->x + (p->y*w-cp->y)*n->y + (p->z*w-cp->z)*n->z;
    } else {
      s = (p->x-cp->x)*n->x + (p->y-cp->y)*n->y + (p->z-cp->z)*n->z;
    }
    if (s > 0) {
      scale = -scale;
    }
  }

  end.x = p->x + scale*n->x;
  end.y = p->y + scale*n->y;
  end.z = p->z + scale*n->z;

  DONT_LIGHT();

  /* cH: This is wrong. The current transformation need not be an
   * affine motion in which case the direction of the normals will
   * just come out wrong. I also wonder whether OpenGL's drawing stuff
   * does the right thing?
   */
  glBegin(GL_LINE_STRIP);
  glVertex3fv((float *)p);
  glVertex3fv((float *)&end);
  glEnd();
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
