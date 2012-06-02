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
	bool isFloat = false;
	int floatingPoint = -1;
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
			if(s[i] == '.')	
			{
				isFloat = true;
				floatingPoint = substr.size();
			}
			substr = substr + s[i];
		}
	}

	//an integer is given, just use stringstream to convert to int
	stringstream ss;
	ss << substr;
	ss >> val;
	if(!isFloat)
		return (float) val;

	//is a floating point number
	
	//extract the "floating part"
	float retval = 0;
	int k = substr.size()-1;
	while(substr[k] != '.')
	{
		retval += substr[k] - '0';
		retval = retval / 10.;
		k--;
	}

	//return the whole number
	retval = retval + (float) val;
	return retval;
	
}

void printCam(Camera* c)
{
	printf("=======================\n");
	printf("Type: Camera\n");
	printf("Origin: %f %f %f\n", c->origin.x, c->origin.y, c->origin.z);
	printf("Direction: %f %f %f\n", c->direction.x, c->direction.y, c->direction.z);
	printf("Up: %f %f %f\n", c->up.x, c->up.y, c->up.z);
	printf("Z: %f %f\n", c->zmin, c->zmax);
	printf("Width: %f\n", c->width);
	printf("Perspective: %d\n", c->perspective);
	printf("=======================\n\n");

}

void printLight(Light *l)
{
	printf("=====================\n");
	printf("Type: Light\n");
	printf("Origin: %f %f %f\n", l->origin.x, l->origin.y, l->origin.z);
	printf("Color: %f %f %f\n", l->color.r, l->color.g, l->color.b);
	printf("=====================\n\n");
}

void printSphere(Sphere *s)
{
	printf("=========================\n");
	printf("Type: Sphere\n");
	printf("Origin: %f %f %f\n", s->center.x, s->center.y, s->center.z);
	printf("Radius: %f\n", s->radius);
	printf("Material:\n");
	printf("  Color: %f %f %f\n", s->material.color.r, s->material.color.g, s->material.color.b);
	printf("  Reflection: %f\n", s->material.reflection);
	printf("  Transparency: %f\n", s->material.transparency);
	printf("==========================\n\n");
}

void printPlane(Plane *p)
{
	printf("=======================\n");
	printf("Type: Plane\n");
	printf("Center: %f %f %f\n", p->center.x, p->center.y, p->center.z);
	printf("Normal: %f %f %f\n", p->normal.x, p->normal.y, p->normal.z);
	printf("Size: %f x %f\n", p->width, p->height);
	printf("Material:\n");
	printf("  Color: %f %f %f\n", p->material.color.r, p->material.color.g, p->material.color.b);
	printf("  Reflection: %f\n", p->material.reflection);
	printf("  Transparency: %f\n", p->material. transparency);
	if(p->hastexture)
		printf("Has Texture: \n");
	else
		printf("No Texture.\n");
	printf("========================\n\n");
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
	vector<Light*> m_lights;
	vector<Sphere*> m_spheres;
	vector<Plane*> m_planes;    

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
				Camera *newCam = new Camera();		
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
				getline(fd, line);
				if(line != "end") 
					printf("I think something is wrong\n");
				printCam(newCam);
				m_cameras.push_back(newCam);
			}
			else if(line == "light")
			{
				Light *newLight = new Light();
				//origin
				getline(fd, line);
				newLight->origin.x = getNum(line, 0);
				newLight->origin.y = getNum(line, 1);
				newLight->origin.z = getNum(line, 2);
				//color
				getline(fd, line);
				newLight->color.r = 1;
				newLight->color.g = 1;
				newLight->color.b = 1;
				//end
				getline(fd,line);
				if(line != "end")
					printf("Maybe something is wrong\n");
				printLight(newLight);
				m_lights.push_back(newLight);
			}
			else if(line == "sphere")
			{
				Sphere *newSphere = new Sphere();
				//origin
				getline(fd, line);
				newSphere->center.x = getNum(line, 0);
				newSphere->center.y = getNum(line, 1);
				newSphere->center.z = getNum(line, 2);
				//radius
				getline(fd, line);
				newSphere->radius = getNum(line, 0);
				//color
				getline(fd,line);
				newSphere->material.color.r = getNum(line, 0);
				newSphere->material.color.g = getNum(line, 1);
				newSphere->material.color.b = getNum(line, 2);
				//reflect
				getline(fd,line);
				newSphere->material.reflection = getNum(line, 0);
				//transperency	
				getline(fd, line);
				newSphere->material.transparency = getNum(line, 0);
				//end
				getline(fd, line);
				if(line != "end")
					printf("Hmm. Is something wrong?\n");
			
				printSphere(newSphere);
				m_spheres.push_back(newSphere);
			}
			else if(line == "plane")
			{
				Plane *newPlane = new Plane();
				//origin
				getline(fd, line);
				newPlane->center.x = getNum(line, 0);
				newPlane->center.y = getNum(line, 1);
				newPlane->center.z = getNum(line, 2);
				//normal
				getline(fd, line);
				newPlane->normal.x = getNum(line, 0);
				newPlane->normal.y = getNum(line, 1);
				newPlane->normal.z = getNum(line, 2);
				//up
				getline(fd, line);
				newPlane->up.x = getNum(line, 0);
				newPlane->up.y = getNum(line, 1);
				newPlane->up.z = getNum(line, 2);
				//size
				getline(fd, line);
				newPlane->width = getNum(line, 0);
				newPlane->height = getNum(line, 1);
				//color
				getline(fd, line);
				newPlane->material.color.r = getNum(line, 0);
				newPlane->material.color.g = getNum(line, 1);
				newPlane->material.color.b = getNum(line, 2);
				//reflect
				getline(fd, line);
				newPlane->material.reflection = getNum(line, 0);
				//transparency
				getline(fd, line);
				newPlane->material.transparency = getNum(line, 0);
				//texture (optional)
				getline(fd, line);
				if(line != "end")
				{
					//TODO: check if it's a texture
					//texture
					newPlane->hastexture = true;
					//TODO: set the texture	
					
					getline(fd, line);
					if(line != "end")
						printf("HMMMMM\n");
				}
				else
				{
					newPlane->hastexture = false;
				}
				//end	
				printPlane(newPlane);
			
				m_planes.push_back(newPlane);	
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

	// freeing dynamically allocated objects
	for(int i = 0; i < m_cameras.size(); i++)
	{
		printf("Deleting a camera\n");
		delete m_cameras[i];
	}
	for(int i = 0; i < m_lights.size(); i++)
	{
		printf("Deleting a light\n");
		delete m_lights[i];
	}
	for(int i = 0; i < m_spheres.size(); i++)
	{
		printf("Deleting a sphere\n");
		delete m_spheres[i];
	}
	for(int i = 0; i < m_planes.size(); i++)
	{
		printf("Deleting a plane\n");
		delete m_planes[i];
	}
    	return 0;
}
