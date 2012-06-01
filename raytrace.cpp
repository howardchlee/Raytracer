#include <stdio.h>
#include <iostream>
#include <vector>
#include <string.h>
#include "EasyBMP.h"
#include <fstream>
#include <sstream>

using namespace std;

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

typedef struct _Plane { Point center;
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

// Helper functions for parsing
float getNum(const string& s, int which)
{
	int i=0, j=0;
	bool copy = false;
	int val;
	string substr="";
	for( ; i < s.size(); i++)
	{
		if(s[i] == ' ')
		{
			if(j == which)
			{
				copy = true;
			}
			if(j == which + 1)
			{
				copy = false;
				break;
			}
			j++;
		}
		else if(copy)
		{
			substr = substr + s[i];
		}
	}
	stringstream ss;
	ss << substr;
	ss >> val;
	return (float) val;
}

void printCam(Camera* c)
{
	printf("Type: Camera\n");
	printf("Origin: %f %f %f\n", c->origin.x, c->origin.y, c->origin.z);
	printf("Direction: %f %f %f\n", c->direction.x, c->direction.y, c->direction.z);
	printf("Up: %f %f %f\n", c->up.x, c->up.y, c->up.z);
	printf("Z: %f %f\n", c->zmin, c->zmax);
	printf("Width: %f\n", c->width);
	printf("Perspectiive: %d\n", c->perspective);

}

int main(int argc, char * argv[])
{
	//make sure that an argument is provided
	if(argc != 2)
	{
		fprintf(stderr, "Usage: ./raytrace [scene specification]\n");
	}
	//same the specification filename
	char *filename = argv[1];

	vector<Camera*> m_cameras;
    
	//parse the specification file.
	string line;
	ifstream fd(filename);
	if(fd.is_open())
	{
		while(fd.good())
		{	
			getline(fd, line);
			if(line == "camera")
			{
				Camera* newCam = new Camera();		
				//origin
				getline(fd,line);
				newCam->origin.x = getNum(line, 0);
				newCam->origin.y = getNum(line, 1);
				newCam->origin.z = getNum(line, 2);	
				//direction
				getline(fd,line);
				newCam->direction.x = getNum(line, 0);
				newCam->direction.y = getNum(line, 1);
				newCam->direction.z = getNum(line, 2);
				//up
				getline(fd, line);
				newCam->up.x = getNum(line, 0);
				newCam->up.y = getNum(line, 1);
				newCam->up.z = getNum(line, 2);
				//z
				getline(fd, line);
				newCam->zmin = getNum(line, 0);
				newCam->zmax = getNum(line, 1);
				//width
				getline(fd, line);
				newCam->width = getNum(line, 0);
				//perspective
				getline(fd,line);
				newCam->perspective = (bool)getNum(line, 0);
				//end

				printCam(newCam);
				m_cameras.push_back(newCam);
			}
		}
		fd.close();
	}

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

	for(int i = 0; i < m_cameras.size(); i++)
	{
		delete m_cameras[i];
	}
    	return 0;
}
