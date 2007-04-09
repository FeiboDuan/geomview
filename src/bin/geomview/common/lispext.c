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

/*
 * geomview custom lisp object types
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "ooglutil.h"
#include "drawer.h"
#include "lisp.h"
#include "lispext.h"
#include "streampool.h"
#include "handleP.h"
#include "camera.h"
#include "geom.h"
#include "appearance.h"
#include "window.h"
#include "transform.h"
#include "fsa.h"
#include "lang.h"
#include "freelist.h"

extern HandleOps GeomOps, CamOps, WindowOps;
extern HandleOps TransOps, NTransOps, ImageOps, AppearanceOps;

LObject *L0, *L1;

static Fsa lang_fsa = NULL;
#define REJECT -1

static char **keyword_names;
static int n_keywords;

static DEF_FREELIST(HandleRefStruct);
static DEF_FREELIST(TransformStruct);

#ifdef NEW
# undef NEW
#endif
#define NEW(type, name)				\
{						\
  HandleRefStruct *hrname;			\
  FREELIST_NEW(HandleRefStruct, hrname);	\
  name = (type##Struct *)hrname;		\
}
#ifdef DELETE
# undef DELETE
#endif
#define DELETE(name) FREELIST_FREE(HandleRefStruct, name)

/************************************************************************
 CAMERA LISP OBJECT
 ************************************************************************/
static CameraStruct *camcopy(CameraStruct *old)
{
  CameraStruct *newc;
  
  NEW(Camera, newc);

  if(old) *newc = *old;
  else newc->cam = NULL, newc->h = NULL;
  if (newc->cam) RefIncr((Ref*)(newc->cam));
  if (newc->h) RefIncr((Ref*)newc->h);
  return newc;
}

int camfromobj(LObject *obj, CameraStruct **x)
{
  if (obj->type != LCAMERA) return 0;
  *x = LCAMERAVAL(obj);
  return 1;
}

LObject *cam2obj(CameraStruct **x)
{
  CameraStruct *copy = camcopy(*x);
  return LNew( LCAMERA, &copy );
}

void camfree(CameraStruct **x)
{
  if (*x) {
    if ((*x)->cam) CamDelete( (*x)->cam );
    if ((*x)->h) HandleDelete( (*x)->h );
    DELETE(*x);
  }
}

int cammatch(CameraStruct **a, CameraStruct **b)
{ 
  if ((*a)->h && ((*a)->h == (*b)->h)) return 1;
  if ((*a)->cam && ((*a)->cam == (*b)->cam)) return 1;
  return 0;
}

void camwrite(FILE *fp, CameraStruct **x)
{
  CamFSave( (*x)->cam, fp, "lisp output stream" );
}

void campull(va_list *a_list, CameraStruct **x)
{
  *x = va_arg(*a_list, CameraStruct*);
}

LObject *camparse(Lake *lake)
{
  CameraStruct *newc;

  NEW(Camera, newc);

  newc->h = NULL; newc->cam = NULL;
  if (CamOps.strmin(POOL(lake), (Handle **)&newc->h,
		    (Ref **)(void *)&(newc->cam)) == 0) {
    return Lnil;
  } else
    if(newc->h && !newc->h->permanent) {
      HandleDelete(newc->h);
      newc->h = NULL;
    }
    return LNew( LCAMERA, &newc );
}

LType LCamerap = {
  "camera",
  sizeof(CameraStruct*),
  camfromobj,
  cam2obj,
  camfree,
  camwrite,
  cammatch,
  campull,
  camparse,
  LTypeMagic
};

/************************************************************************
 * WINDOW LISP OBJECT 							*
 ************************************************************************/

static WindowStruct *wncopy(WindowStruct *old)
{
  WindowStruct *neww;

  NEW(Window, neww);

  if(old) *neww = *old;
  else neww->wn = NULL, neww->h = NULL;
  if (neww->wn) RefIncr((Ref*)(neww->wn));
  if (neww->h) RefIncr((Ref*)neww->h);
  return neww;
}

int wnfromobj(LObject *obj, WindowStruct **x)
{
  if (obj->type != LWINDOW) return 0;
  *x = LWINDOWVAL(obj);
  return 1;
}

LObject *wn2obj(WindowStruct **x)
{
  WindowStruct *copy = wncopy(*x);
  return LNew( LWINDOW, &copy );
}

void wnfree(WindowStruct **x)
{
  if (*x) {
    if ((*x)->wn) WnDelete( (*x)->wn );
    if ((*x)->h) HandleDelete( (*x)->h );
    DELETE(*x);
  }
}

int wnmatch(WindowStruct **a, WindowStruct **b)
{ 
  if ((*a)->h && ((*a)->h == (*b)->h)) return 1;
  if ((*a)->wn && ((*a)->wn == (*b)->wn)) return 1;
  return 0;
}

void wnwrite(FILE *fp, WindowStruct * *x)
{
  Pool *p = PoolStreamTemp("", NULL, fp, 1, &WindowOps);
  if(p == NULL)
    return;
  (void) WnStreamOut(p, (*x)->h, (*x)->wn);
  PoolDelete(p);
}

void wnpull(va_list *a_list, WindowStruct **x)
{
  *x = va_arg(*a_list, WindowStruct*);
}

LObject *wnparse(Lake *lake)
{
  WindowStruct *neww;

  NEW(Window, neww);

  neww->h = NULL; neww->wn = NULL;
  if (WindowOps.strmin(POOL(lake),(Handle **)&neww->h,
		       (Ref **)(void *)&(neww->wn)) == 0) {
    return Lnil;
  } else
    if(neww->h && !neww->h->permanent) {
      HandleDelete(neww->h);
      neww->h = NULL;
    }
    return LNew( LWINDOW, &neww );
}

LType LWindowp = {
  "window",
  sizeof(WindowStruct*),
  wnfromobj,
  wn2obj,
  wnfree,
  wnwrite,
  wnmatch,
  wnpull,
  wnparse,
  LTypeMagic
};

/************************************************************************
 * GEOM LISP OBJECT 							*
 ************************************************************************/

static GeomStruct *geomcopy(GeomStruct *old)
{
  GeomStruct *newg;

  NEW(Geom, newg);
  if(old) *newg = *old;
  else newg->geom = NULL, newg->h = NULL;
  if (newg->geom) RefIncr((Ref*)(newg->geom));
  if (newg->h) RefIncr((Ref*)newg->h);
  return newg;
}

int geomfromobj(LObject *obj, GeomStruct **x)
{
  if (obj->type != LGEOM) return 0;
  *x = LGEOMVAL(obj);
  return 1;
}

LObject *geom2obj(GeomStruct **x)
{
  GeomStruct *copy = geomcopy(*x);
  return LNew( LGEOM, &copy );
}

void geomfree(GeomStruct **x)
{
  if (*x) {
    if ((*x)->geom) GeomDelete( (*x)->geom );
    if ((*x)->h) HandleDelete( (*x)->h );
    DELETE(*x);
  }
}

int geommatch(GeomStruct **a, GeomStruct **b)
{ 
  if ((*a)->h && ((*a)->h == (*b)->h)) return 1;
  if ((*a)->geom && ((*a)->geom == (*b)->geom)) return 1;
  return 0;
}

void geomwrite(FILE *fp, GeomStruct **x)
{
  GeomFSave( (*x)->geom, fp, "lisp output stream" );
}

void geompull(va_list *a_list, GeomStruct **x)
{
  *x = va_arg(*a_list, GeomStruct*);
}

LObject *geomparse(Lake *lake)
{
  GeomStruct *newg;

  NEW(Geom, newg);

  newg->h = NULL; newg->geom = NULL;
  if (GeomOps.strmin(POOL(lake), (Handle **)&newg->h, 
		     (Ref **)(void *)&(newg->geom)) == 0) {
    return Lnil;
  } else {
    if(newg->h && !newg->h->permanent) {
      HandleDelete(newg->h);
      newg->h = NULL;
    }
    return LNew( LGEOM, &newg );
  }
}

LType LGeomp = {
  "geometry",
  sizeof(GeomStruct*),
  geomfromobj,
  geom2obj,
  geomfree,
  geomwrite,
  geommatch,
  geompull,
  geomparse,
  LTypeMagic
};

/************************************************************************
 * AP LISP OBJECT							*
 ************************************************************************/

static ApStruct *apcopy(ApStruct *old)
{
  ApStruct *newap;
  
  NEW(Ap, newap);
  if(old) *newap = *old;
  else newap->ap = NULL, newap->h = NULL;
  if (newap->ap) RefIncr((Ref*)(newap->ap));
  if (newap->h) RefIncr((Ref*)newap->h);
  return newap;
}

int apfromobj(LObject *obj, ApStruct **x)
{
  if (obj->type != LAP) return 0;
  *x = LAPVAL(obj);
  return 1;
}

LObject *ap2obj(ApStruct **x)
{
  ApStruct *copy = apcopy(*x);
  return LNew( LAP, &copy );
}

void apfree(ApStruct **x)
{
  if (*x) {
    if ((*x)->ap) ApDelete( (*x)->ap );
    if ((*x)->h) HandleDelete( (*x)->h );
    DELETE(*x);
  }
}

int apmatch(ApStruct **a, ApStruct **b)
{ 
  if ((*a)->h && ((*a)->h == (*b)->h)) return 1;
  if ((*a)->ap && ((*a)->ap == (*b)->ap)) return 1;
  return 0;
}

void apwrite(FILE *fp, ApStruct * *x)
{
  ApFSave((*x)->ap, fp, "lisp output stream");
}

void appull(va_list *a_list, ApStruct **x)
{
  *x = va_arg(*a_list, ApStruct*);
}

LObject *apparse(Lake *lake)
{
  ApStruct *newap;

  NEW(Ap, newap);

  newap->h = NULL; newap->ap = NULL;
  if (ApStreamIn(POOL(lake), &newap->h, &(newap->ap)) == 0) {
    return Lnil;
  } else
    if(newap->h && !newap->h->permanent) {
      HandleDelete(newap->h);
      newap->h = NULL;
    }
    return LNew( LAP, &newap );
}

LType LApp = {
  "ap",
  sizeof(ApStruct *),
  apfromobj,
  ap2obj,
  apfree,
  apwrite,
  apmatch,
  appull,
  apparse,
  LTypeMagic
};


/************************************************************************
 * TRANSFORM LISP OBJECT						*
 ************************************************************************/

static TransformStruct *tmcopy(TransformStruct *old)
{
  TransformStruct *newt;

  FREELIST_NEW(TransformStruct, newt);

  if (old) *newt = *old;
  else newt->h = NULL;
  if (newt->h) RefIncr((Ref*)newt->h);
  return newt;
}

int tmfromobj(LObject *obj, TransformStruct **x)
{
  if (obj->type != LTRANSFORM) return 0;
  *x = LTRANSFORMVAL(obj);
  return 1;
}

LObject *tm2obj(TransformStruct **x)
{
  TransformStruct *copy = tmcopy(*x);
  return LNew(LTRANSFORM, &copy);
}

void tmfree(TransformStruct **x)
{
  if (*x) {
    if ((*x)->h) HandleDelete((*x)->h);
    FREELIST_FREE(TransformStruct, *x);
  }
}

int tmmatch(TransformStruct **a, TransformStruct **b)
{ 
  if ((*a)->h && ((*a)->h == (*b)->h)) return 1;
  if ((*a)->tm && ((*a)->tm == (*b)->tm)) return 1;
  return TmCompare((*a)->tm, (*b)->tm, (float)0.0);
}

void tmwrite(FILE *fp, TransformStruct **x)
{
  TransFSave((*x)->tm, fp, "lisp output stream");
}

void tmpull(va_list *a_list, TransformStruct **x)
{
  *x = va_arg(*a_list, TransformStruct*);
}

LObject *tmparse(Lake *lake)
{
  TransformStruct *newt;
  
  FREELIST_NEW(TransformStruct, newt);

  newt->h = NULL;
  if (TransStreamIn(POOL(lake), (Handle **)&newt->h, newt->tm) == false) {
    return Lnil;
  } else
    if(newt->h && !newt->h->permanent) {
      HandleDelete(newt->h);
      newt->h = NULL;
    }
    return LNew( LTRANSFORM, &newt );
}

LType LTransformp = {
  "transform",
  sizeof(TransformStruct *),
  tmfromobj,
  tm2obj,
  tmfree,
  tmwrite,
  tmmatch,
  tmpull,
  tmparse,
  LTypeMagic
};

/************************************************************************
 * N-D TRANSFORM LISP OBJECT						*
 ************************************************************************/

static TmNStruct *tmncopy(TmNStruct *old)
{
  TmNStruct *newt;

  NEW(TmN, newt);

  if(old) *newt = *old;
  else newt->tm = NULL, newt->h = NULL;
  if (newt->tm) RefIncr((Ref*)(newt->tm));
  if (newt->h) RefIncr((Ref*)newt->h);
  return newt;
}

int tmnfromobj(LObject *obj, TmNStruct **x)
{
  if (obj->type != LTRANSFORMN) return 0;
  *x = LTRANSFORMNVAL(obj);
  return 1;
}

LObject *tmn2obj( TmNStruct **x )
{
  TmNStruct *copy = tmncopy(*x);
  return LNew( LTRANSFORMN, &copy );
}

void tmnfree(TmNStruct **x)
{
  if (*x) {
    if ((*x)->tm) TmNDelete( (*x)->tm );
    if ((*x)->h) HandleDelete( (*x)->h );
    DELETE(*x);
  }
}

int tmnmatch(TmNStruct **a, TmNStruct **b)
{ 
  if ((*a)->h && ((*a)->h == (*b)->h)) return 1;
  if ((*a)->tm && ((*a)->tm == (*b)->tm)) return 1;
  return 0;
}

void tmnwrite(FILE *fp, TmNStruct **x)
{
  TmNPrint( fp, (*x)->tm );
}

void tmnpull(va_list *a_list, TmNStruct **x)
{
  *x = va_arg(*a_list, TmNStruct*);
}

LObject *tmnparse(Lake *lake)
{
  TmNStruct *newt;

  NEW(TmN, newt);

  newt->h = NULL;
  newt->tm = NULL;
  if (NTransOps.strmin(POOL(lake), (Handle **)&newt->h,
		       (Ref **)(void *)(&newt->tm)) == 0) {
    return Lnil;
  } else {
    if(newt->h && !newt->h->permanent) {
      HandleDelete(newt->h);
      newt->h = NULL;
    }
    return LNew( LTRANSFORMN, &newt );
  }
}

LType LTransformNp = {
  "ntransform",
  sizeof(TmNStruct*),
  tmnfromobj,
  tmn2obj,
  tmnfree,
  tmnwrite,
  tmnmatch,
  tmnpull,
  tmnparse,
  LTypeMagic
};

/************************************************************************
 * IMAGE LISP OBJECT						        *
 ************************************************************************/

static ImgStruct *imgcopy(ImgStruct *old)
{
  ImgStruct *newi;

  NEW(Img, newi);

  if(old) *newi = *old;
  else newi->img = NULL, newi->h = NULL;
  if (newi->img) RefIncr((Ref*)(newi->img));
  if (newi->h) RefIncr((Ref*)newi->h);
  return newi;
}

int imgfromobj(LObject *obj, ImgStruct **x)
{
  if (obj->type != LIMAGE) return 0;
  *x = LIMAGEVAL(obj);
  return 1;
}

LObject *img2obj( ImgStruct **x )
{
  ImgStruct *copy = imgcopy(*x);
  return LNew( LIMAGE, &copy );
}

void imgfree(ImgStruct **x)
{
  if (*x) {
    if ((*x)->img) ImgDelete( (*x)->img );
    if ((*x)->h) HandleDelete( (*x)->h );
    DELETE(*x);
  }
}

int imgmatch(ImgStruct **a, ImgStruct **b)
{ 
  if ((*a)->h && ((*a)->h == (*b)->h)) return true;
  if ((*a)->img && ((*a)->img == (*b)->img)) return true;
  return false;
}

void imgwrite(FILE *fp, ImgStruct **x)
{
  ImgFSave((*x)->img, fp, "lisp output stream");
}

void imgpull(va_list *a_list, ImgStruct **x)
{
  *x = va_arg(*a_list, ImgStruct*);
}

LObject *imgparse(Lake *lake)
{
  ImgStruct *newi;

  NEW(Img, newi);
  
  newi->h = NULL;
  newi->img = NULL;
  if (ImageOps.strmin(POOL(lake), (Handle **)&newi->h,
		      (Ref **)(void *)(&newi->img)) == 0) {
    return Lnil;
  } else {
    if(newi->h && !newi->h->permanent) {
      HandleDelete(newi->h);
      newi->h = NULL;
    }
    return LNew( LIMAGE, &newi );
  }
}

LType LImagep = {
  "image",
  sizeof(ImgStruct*),
  imgfromobj,
  img2obj,
  imgfree,
  imgwrite,
  imgmatch,
  imgpull,
  imgparse,
  LTypeMagic
};

/************************************************************************
 * ID LISP OBJECT							*
 ************************************************************************/

int idfromobj(obj, x)
    LObject *obj;
    int *x;
{
  if (obj->type == LSTRING) {
    *x = drawer_idbyname(LSTRINGVAL(obj));
    if (*x == NOID) return 0;
  } else if (obj->type == LID) {
    *x = LIDVAL(obj);
  } else return 0;
  return 1;
}

LObject *id2obj(int *x)
{
  return LNew( LID, x );
}

static void idfree(int *x)
{}

int idmatch(a, b)
    int *a,*b;
{
  return drawer_idmatch(*a,*b);
}

void idwrite(fp, x)
    FILE *fp;
    int *x;
{
  fprintf(fp, "\"%s\"", drawer_id2name(*x));
}

LObject *idparse(Lake *lake)
{
  LObject *obj = LSexpr(lake);
  int id;
  if (obj->type == LSTRING) {
    id  = drawer_idbyname(LSTRINGVAL(obj));
    if (id == NOID) return Lnil;
    OOGLFree(LSTRINGVAL(obj));
    obj->type = LID;
    obj->cell.i = id;
    return obj;
  } else {
    LFree(obj);
    return Lnil;
  }
}
    
static void idpull(a_list, x)
    va_list *a_list;
    int *x;
{
  *x = va_arg(*a_list, int);
}

LType LIdp = {
  "id",
  sizeof(int),
  idfromobj,
  id2obj,
  idfree,
  idwrite,
  idmatch,
  idpull,
  LSexpr,
  LTypeMagic
  };


/************************************************************************
 * KEYWORD LISP OBJECT							*
 ************************************************************************/

int keywordfromobj(obj, x)
    LObject *obj;
    int *x;
{
  if (obj->type == LSTRING) {
    *x = (int)(long)fsa_parse(lang_fsa, LSTRINGVAL(obj));
    if (*x == REJECT) return 0;
  } else if (obj->type == LKEYWORD) {
    *x = LKEYWORDVAL(obj);
  } else return 0;
  return 1;
}

LObject *keyword2obj(x)
    int *x;
{
  return LNew( LKEYWORD, x );
}

static int keywordmatch(a, b)
    int *a,*b;
{
  return *a == *b;
}

void keywordwrite(fp, x)
    FILE *fp;
    int *x;
{
  fprintf(fp, "%s", keywordname(*x));
}

static void keywordfree(void *value)
{}

static void keywordpull(a_list, x)
    va_list *a_list;
    int *x;
{
  *x = va_arg(*a_list, int);
}

LObject *keywordparse(Lake *lake)
{
  LObject *obj = LSexpr(lake);
  int key;
  if (obj->type == LSTRING) {
    key  = (int)(long)fsa_parse(lang_fsa, LSTRINGVAL(obj));
    if (key == REJECT) return Lnil;
    OOGLFree(LSTRINGVAL(obj));
    obj->type = LKEYWORD;
    obj->cell.i = key;
    return obj;
  } else {
    LFree(obj);
    return Lnil;
  }
}

LType LKeywordp = {
  "keyword",
  sizeof(int),
  keywordfromobj,
  keyword2obj,
  keywordfree,
  keywordwrite,
  keywordmatch,
  keywordpull,
  LSexpr,
  LTypeMagic
  };


/************************************************************************
 * STRINGS LISP OBJECT							*
 * (a "strings" object is a string with possibly embedded spaces)	*
 ************************************************************************/

int stringsfromobj(obj, x)
    LObject *obj;
    char * *x;
{
  if (obj->type != LSTRING  && obj->type != LSTRINGS) return 0;
  *x = LSTRINGVAL(obj);
  return 1;
}

static LObject *stringsparse(Lake *lake)
{
  char *tok;
  int toklen, c, first=1;
  static char *delims = "()";
  vvec svv;

  VVINIT(svv, char, 80);

  *VVINDEX(svv, char, 0) = '\0';
  while ( LakeMore(lake,c) ) {
    tok = iobfdelimtok( delims, lake->streamin, 0 );
    toklen = strlen(tok);
    vvneeds(&svv, strlen(VVEC(svv,char))+toklen+2);
    if (!first) strcat(VVEC(svv,char), " ");
    else first = 0;
    strcat(VVEC(svv,char), tok);
  }
  VVCOUNT(svv) = strlen(VVEC(svv,char))+1;
  vvtrim(&svv);
  tok = VVEC(svv, char);
  return LNew( LSTRINGS, &tok );
}

LType LStringsp;		/* initialized in lispext_init() */

/**********************************************************************/
/**********************************************************************/

void lispext_init()
{
  LStringsp = *(LSTRING);
  LStringsp.name = "strings";
  LStringsp.fromobj = stringsfromobj;
  LStringsp.parse = stringsparse;

  {
    int zero=0, one=1;
    L0 = LNew( LINT, &zero );
    L1 = LNew( LINT, &one );
  }

  lang_fsa = fsa_initialize(NULL, (void*)REJECT);
  return;
}

void define_keyword(char *word, Keyword value)
{
  if (value+1 > n_keywords) {
    char **newwords = OOGLNewNE(char *, value+1, "New keyword list");
    memset(newwords, 0, sizeof(char *)*(value+1));
    memcpy(newwords, keyword_names, n_keywords*sizeof(char *));
    OOGLFree(keyword_names);
    keyword_names = newwords;
    keyword_names[value] = word;
    n_keywords = value+1;
  } else if (keyword_names[value] == NULL) {
    /* We allow aliases, but the name-list just picks up the first
     * name.
     */
    keyword_names[value] = word;
  }
  fsa_install(lang_fsa, word, (void*)(long)value);
}

/* returns < 0 if asked to parse something that isn't a keyword. */
Keyword parse_keyword(char *word)
{
  return (Keyword)(long)fsa_parse(lang_fsa, word);
}

char *keywordname(Keyword keyword)
{
  if (keyword < n_keywords && keyword_names[keyword] != NULL) {
    return keyword_names[keyword];
  } else {
    return "???";
  }
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
