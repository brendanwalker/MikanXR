#include "ColorUtils.h"
#include "math.h"

//-- Utilities -----
// From: https://www.cs.rit.edu/~ncs/color/t_convert.html
// r,g,b values are from 0 to 1
// h = [0,360], s = [0,1], v = [0,1]
//		if s == 0, then h = -1 (undefined)
void RGBtoHSV(float r, float g, float b, float& h, float& s, float& v)
{
	float min = fminf(fminf(r, g), b);
	float max = fmaxf(fmaxf(r, g), b);

	v = max;				// v
	float delta = max - min;
	if (max != 0)
	{
		s = delta / max;		// s
	}
	else
	{
		// r = g = b = 0		// s = 0, v is undefined
		s = 0;
		h = -1;
		return;
	}

	if (r == max)
	{
		h = (g - b) / delta;		// between yellow & magenta
	}
	else if (g == max)
	{
		h = 2 + (b - r) / delta;	// between cyan & yellow
	}
	else
	{
		h = 4 + (r - g) / delta;	// between magenta & cyan
	}

	h *= 60;				// degrees
	if (h < 0)
	{
		h += 360;
	}
}

// From: https://www.cs.rit.edu/~ncs/color/t_convert.html
// r,g,b values are from 0 to 1
// h = [0,360], s = [0,1], v = [0,1]
//		if s == 0, then h = -1 (undefined)
void HSVtoRGB(float h, float s, float v, float& r, float& g, float& b)
{
	if (s == 0)
	{
		// achromatic (grey)
		r = g = b = v;
		return;
	}

	const float sector = h / 60;			// sector 0 to 5
	const int i = (int)(floorf(sector));
	const float f = sector - i;			// factorial part of h
	const float p = v * (1 - s);
	const float q = v * (1 - s * f);
	const float t = v * (1 - s * (1 - f));

	switch (i)
	{
	case 0:
		r = v;
		g = t;
		b = p;
		break;
	case 1:
		r = q;
		g = v;
		b = p;
		break;
	case 2:
		r = p;
		g = v;
		b = t;
		break;
	case 3:
		r = p;
		g = q;
		b = v;
		break;
	case 4:
		r = t;
		g = p;
		b = v;
		break;
	default:		// case 5:
		r = v;
		g = p;
		b = q;
		break;
	}
}