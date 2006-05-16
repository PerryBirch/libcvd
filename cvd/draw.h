/*
        This file is part of the CVD Library.

        Copyright (C) 2005 The Authors

        This library is free software; you can redistribute it and/or
        modify it under the terms of the GNU Lesser General Public
        License as published by the Free Software Foundation; either
        version 2.1 of the License, or (at your option) any later version.

        This library is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
        Lesser General Public License for more details.

        You should have received a copy of the GNU Lesser General Public
        License along with this library; if not, write to the Free Software
        Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef CVD_DRAW_H_
#define CVD_DRAW_H_

#include <cvd/exceptions.h>
#include <cvd/image.h>
#include <cvd/image_ref.h>
#include <cvd/internal/builtin_components.h>
#include <cvd/internal/rgb_components.h>
#include <cvd/image_convert.h>
#include <cvd/utility.h>
#include <vector>
#include <algorithm>


namespace CVD {

namespace Exceptions {

    /// %Exceptions specific to drawing functions
    /// @ingroup gException
    namespace Draw {
        /// Base class for all Image_IO exceptions
        /// @ingroup gException
        struct All: public CVD::Exceptions::All {};

        /// Input ImageRef not within image dimensions
        /// @ingroup gException
        struct ImageRefNotInImage : public All {
            ImageRefNotInImage(const std::string & function)
            {
                what = "Input ImageRefs not in image in " + function;
            };
        };

        /// Input images have incompatible dimensions
        /// @ingroup gException
        struct IncompatibleImageSizes : public All {
            IncompatibleImageSizes(const std::string & function)
            {
                what = "Incompatible image sizes in " + function;
            };
        };
    };
};

/// traits type defining colors for pixel types
/// @ingroup gGraphics
template <class T, unsigned int N=Pixel::Component<T>::count> struct color {};

/// traits type defining colors for pixel types. For pixel types with one component
/// only gray scale color functions are defined.
/// @ingroup gGraphics
template <class T> struct color<T,1> {
    typedef typename Pixel::Component<T>::type TComp;
    /// returns pixel value corresponding to minimal saturation
    inline static const T& black() { static T c; Pixel::Component<T>::get(c,0) = 0; return c;}
    /// returns pixel value corresponding to 50% gray
    inline static const T& gray() { static T c; Pixel::Component<T>::get(c,0) = Pixel::traits<TComp>::max_intensity/2; return c;}
    /// returns pixel value corresponding to maximal saturation
    inline static const T& white() { static T c; Pixel::Component<T>::get(c,0) = Pixel::traits<TComp>::max_intensity; return c;}
};

/// traits type defining colors for pixel types. For pixel types with three component
/// the basic colors generated by combining minimum and maximum intensities for each component
/// are defined. The components are interpreted as R, G, B.
/// @ingroup gGraphics
template <class T> struct color<T,3> {
    typedef typename Pixel::Component<T>::type TComp;
    static const TComp hi;
    inline static T make(const TComp& a, const TComp& b, const TComp& c) { 
	T t; 
	Pixel::Component<T>::get(t,0)=a;
	Pixel::Component<T>::get(t,1)=b;
	Pixel::Component<T>::get(t,2)=c;
	return t;
    }
    inline static const T&   black() { static const T c = make(0,0,0); return c;}
    inline static const T&   white() { static const T c = make(hi,hi,hi); return c;}
    inline static const T&     red() { static const T c = make(hi,0,0); return c;}
    inline static const T&   green() { static const T c = make(0,hi,0); return c;}
    inline static const T&    blue() { static const T c = make(0,0,hi); return c;}
    inline static const T&    cyan() { static const T c = make(0,hi,hi); return c; }
    inline static const T& magenta() { static const T c = make(hi,0,hi); return c;}
    inline static const T&  yellow() { static const T c = make(hi,hi,0); return c;}
    inline const T& shade(const T& c, double b) {
	return make((TComp)(Pixel::Component<T>::get(c,0)*b), (TComp)(Pixel::Component<T>::get(c,1)*b), (TComp)(Pixel::Component<T>::get(c,2)*b));
    }
};
 template <class T> const typename color<T,3>::TComp color<T,3>::hi = Pixel::traits<TComp>::max_intensity;


/// draws a line defined by start and end coordinates with given color into an image.
/// @param im image to draw in
/// @param x1 x coordinate of start point
/// @param y1 y coordinate of start point
/// @param x2 x coordinate of end point
/// @param y2 y coordinate of end point
/// @param c color of the line
/// @ingroup gGraphics
template <class T>
void drawLine(Image<T>& im, double x1, double y1, double x2, double y2, const T& c)
{
    double dx = x2-x1;
    double dy = y2-y1;
    int w = im.size().x;
    int h = im.size().y;
    double len = abs(dx)+abs(dy);
    for(int t=0;t<=len;t++) {
        int x = (int)(x1 + t/(len)*dx+0.5);
        int y = (int)(y1 + t/(len)*dy+0.5);
        if (x >=0 && x <w && y>=0 && y<h)
	  im[y][x] = c;
    }
}

/// draws a line defined by two ImageRef with given color into an image.
/// @param im image to draw in
/// @param p1 start point
/// @param p2 end point
/// @param c color of the line
/// @ingroup gGraphics
template <class T>
void drawLine(Image<T>& im, const ImageRef& p1, const ImageRef& p2, const T& c)
{
    drawLine(im, double(p1.x), double(p1.y), double(p2.x), double(p2.y), c);
}

/// draws a polyline defined by a set of points into an image. A given offset is added
/// to all points.
/// @param im image to draw in
/// @param offset added to all points
/// @param points vector containing points
/// @param c color of the line
/// @ingroup gGraphics
template <class T>
void drawShape(Image<T>& im, const ImageRef& offset, const std::vector<ImageRef>& points, const T& c)
{
    for (unsigned int i=0; i<points.size()-1; i++)
        drawLine(im, points[i]+offset, points[i+1]+offset, c);
    drawLine(im, points.back()+offset, points.front()+offset, c);
}

/// draws a box defined by the upper left and the lower right corners into an image
/// @param im image to draw in
/// @param upperleft ImageRef of upper left corner
/// @param lowerright ImageRef of lower right corner
/// @param c color of the box
/// @ingroup gGraphics
template <class T>
void drawBox(Image<T> &im, const ImageRef & upperleft, ImageRef & lowerright, const T& c)
{
    drawLine(im, upperleft.x, upperleft.y, upperleft.x, lowerright.y, c);
    drawLine(im, upperleft.x, upperleft.y, lowerright.x, upperleft.y, c);
    drawLine(im, upperleft.x, lowerright.y, lowerright.x, lowerright.y, c);
    drawLine(im, lowerright.x, upperleft.y, lowerright.x, lowerright.y, c);
}

/// draws a cross defined by center point and length of the arms into an image
/// @param im image to draw in
/// @param p center point
/// @param len arm length
/// @param c color of the box
/// @ingroup gGraphics
template <class T>
void drawCross(Image<T>& im, const ImageRef& p, double len, const T& c)
{
    drawLine(im, p.x-len, p.y, p.x+len, p.y, c);
    drawLine(im, p.x, p.y-len, p.x, p.y+len, c);
}

/// returns coordinate for a circle of given radius around the origin. The result can be
/// used with drawShape to draw it into an image.
/// @param radius radius of the circle in pixels
/// @return vector containig ImageRef for the circle
/// @ingroup gGraphics
std::vector<ImageRef> getCircle(int radius);

/// joins two images side-by-side to create a larger image, any remaining empty region will be blacked out
/// @param a input image a to copy from
/// @param b input image b to copy from
/// @param J resulting joint image
/// @ingroup gGraphics
template <class S, class T, class U> void joinImages(const Image<S>& a, const Image<T>& b, Image<U>& J) {
  int h = std::max(a.size().y,b.size().y);
    J.resize(ImageRef(a.size().x+b.size().x, h));
    CVD::copy(a, J, a.size());
    CVD::copy(b, J, b.size(), ImageRef(), ImageRef(a.size().x, 0));
    ImageRef blackBegin, blackEnd;
    if (a.size().y < b.size().y) {
        blackBegin = ImageRef(0,a.size().y);
        blackEnd = ImageRef(a.size().x,J.size().y);
    } else {
        blackBegin = ImageRef(a.size().x, b.size().y);
        blackEnd = J.size();
    }
    for (int i = blackBegin.y; i<blackEnd.y; i++)
      for (int j= blackBegin.x; j<blackEnd.x; j++)
	J[i][j] = U();
}


/// similar to copy but combines values from two source images into the output image. In the current implementation
/// it simply adds the pixel values of two input images together and writes them into the output image.
/// @param a input image a to copy from
/// @param b input image b to copy from
/// @param out resulting combined image
/// @ingroup gGraphics
template <class S, class T, class U> void combineImages(const Image<S>& a, const Image<T>& b, Image<U>& out, const ImageRef & dst = ImageRef_zero, ImageRef size = ImageRef(), const ImageRef & from = ImageRef_zero)
{
    if(!a.in_image(dst))
        throw Exceptions::Draw::ImageRefNotInImage("combineImages");
    if(a.size() != out.size)
        throw Exceptions::Draw::IncompatibleImageSizes("combineImages");

    if( size == ImageRef_zero )
        size = b.size();

    if (size.x + dst.x >= a.size().x)
      size.x = a.size().x - dst.x;
    if (size.x + dst.x >= out.size().x)
      size.x = out.size().x - dst.x;
    if (size.y + dst.y >= a.size().y)
      size.y = a.size().y - dst.y;
    if (size.y + dst.y >= out.size().y)
      size.y = out.size().y - dst.y;

    if( &a != &out )
    {
        CVD::copy(a,out, a.size());
    }

    ImageRef sourceA = dst;
    ImageRef sourceB = from;
    ImageRef endA = dst + size;
    ImageRef endB = from + size;

    out[sourceA] += b[sourceB];
    while(sourceA.next(dst, endA))
    {
        sourceB.next(from, endB);
        out[sourceA] += b[sourceB];
    }
}

};
#endif // CVD_DRAW_H_
