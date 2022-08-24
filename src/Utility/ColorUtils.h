#pragma once

//-- Utilities -----
// r,g,b values are from 0 to 1
// h = [0,360], s = [0,1], v = [0,1]
//		if s == 0, then h = -1 (undefined)
void RGBtoHSV(float r, float g, float b, float& h, float& s, float& v);

// r,g,b values are from 0 to 1
// h = [0,360], s = [0,1], v = [0,1]
//		if s == 0, then h = -1 (undefined)
void HSVtoRGB(float h, float s, float v, float& r, float& g, float& b);
