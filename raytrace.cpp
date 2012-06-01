#include <stdio.h>
#include <string.h>
#include "EasyBMP.h"

typedef struct _Point {
    float x,y,z;
} Point;

typedef struct _Color {
    float r,g,b;        // Valid range 0-1
} Color;

typedef struct _Material {
    Color color;
    float reflection;   // Valid range 0-1
    float transparency; // Valid range 0-1
} Material;

typedef struct _Sphere {
    Point center;
    Material material;
    float radius;
} Sphere;

typedef struct _Plane {
    Point center;
    Point normal;
    Point up;
    float width;
    float height;
    bool hastexture;
    Material material;
    BMP texture;
} Plane;

typedef struct _Camera {
    Point origin;
    Point direction;
    Point up;
    float zmin, zmax; // zmin is also the location of the image plane
    float width;      // Width at zmin, Height should be based on aspect ratio
    bool  perspective;
} Camera;

typedef struct _Light {
    Point origin;
    Color color;
} Light;

// Static scene definition
Light light   = {{0,10,0}, {1,1,1}};
Camera camera = {{0,0,0},  {0,0,1}, {0,1,0}, 0, 10, 10, false};
Sphere sphere = {{0,0,5},  {{1,0,0}, 0, 0}, 2};

#define W 320
#define H 240
const float aspect = W/H;

int main(int argc, char * argv[])
{
    BMP image;
    image.SetSize(W,H);
    image.SetBitDepth(32);
    for(int x=0; x<W; x++)
        for(int y=0; y<H; y++)
        {
            image(x,y)->Red   = 255;
            image(x,y)->Green = 0;
            image(x,y)->Blue  = 0;
            image(x,y)->Alpha = 0;
        }

    image.WriteToFile("output.bmp");

    BMP texture;
    texture.ReadFromFile("output.bmp");
    return 0;
}
