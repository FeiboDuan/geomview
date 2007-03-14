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

/* Implement Geomview's "apply = blend" for non-constant shading
 *
 * See also glTexEnvf(3).
 *
 * Note that we ignore the alpha value of the background color. This
 * is just what OpenGL does, as Geomview never uses GL_INTENSITY as
 * texture format.
 *
 * Note also that the background color is unaffected by lighting. This
 * means: we apply first the normal plastic shader, and then
 * interpolate between the shaded colour and the background color.
 *
 * If the texture has an alpha channel, then Oi = Os * Ot.
 */
surface
GVblendplastic(float Ka = 1, Kd = .5, Ks = .5, roughness = .1;
	       color specularcolor = 1;
	       string texturename = ""; color bgcolor = 0;)
{
  normal Nf;
  vector V;
  float channels;
  float Ot, Lt;
  color Ct;

  /* normal plastic shader */
  Ci = Cs;
  Oi = Os;

  Nf = faceforward (normalize(N),I);
  V = -normalize(I);

  Ci = Ci * (Ka*ambient() + Kd*diffuse(Nf)) +
    specularcolor * Ks*specular(Nf,V,roughness);
  Ci *= Os;

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
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
