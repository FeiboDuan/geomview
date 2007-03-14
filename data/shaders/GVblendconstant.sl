/* Copyright (C) 2007 Claus-Justus Heine
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

/* Implement Geomview's "apply = blend" for constant shading
 *
 * Maybe mgrib should set bgalpha to 0 if transparency is not enabled
 * (0 because this will leave the alpha channel of the surface
 * unaffected).
 */
surface
GVblendconstant(string texturename = ""; color bgcolor = 0;)
{
  float channels, Lt, Ot;
  color Ct;

  Ci = Cs;
  Oi = Os;

  /* texture support a la GL_BLEND */
  if (texturename != "" &&
      textureinfo(texturename, "channels", channels) == 1.0) {
    if (channels < 3) {
      Lt = float texture(texturename[0]);
      Ot = float texture(texturename[1], "fill", 1.0, "width", 0.0);
      Ci = Lt * Ci + (1.0 - Lt) * bgcolor;
    } else {
      Ct = color texture(texturename);
      Ot = float texture(texturename[3], "fill", 1.0, "width", 0.0);
      Ci = Ct * Ci + (1.0 - Ct) * bgcolor;
    }
    Oi *= Ot;
  }

  Ci *= Os;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
