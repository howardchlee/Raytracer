#include <stdio.h>
#include <iostream>
#include <vector>
#include <string.h>
#include "EasyBMP.h"
#include <fstream>
#include <sstream>
#include <math.h>

using namespace std;

typedef struct _Point {
    float x,y,z;
} Point;

typedef Point Vec3;

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

typedef struct _Ray {
    Point origin;
    Vec3 direction;   // Normalized direction of the ray.
} Ray;

#include "rtdebug.h"

// Static scene definition
Light light   = {{0,10,0}, {1,1,1}};
Camera camera = {{0,0,0},  {0,0,1}, {0,1,0}, 0, 10, 10, false};
Sphere sphere = {{0,0,5},  {{1,0,0}, 0, 0}, 2};

#define MY_NAN -100000

#define W 320
#define H 240
const float aspect = W/H;

/*******************************************************************************
 * getNum
 * 
 * This function takes a string and an index, and returns the floating point
 * number that is after the 'which'th blank space in s.  
 *
 ******************************************************************************/

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

/*******************************************************************************
 *  parseInput
 * 
 *  This function takes a vector of each object type as well as a filename, 
 *  parse the file specified by the file, and place the objects of the scenes 
 *  in their corresponding vector.
 *
 *  This function returns the total number of objects created
 * 
 ******************************************************************************/
int parseInput(vector<Camera*> &m_cameras, vector<Light*> &m_lights, vector<Sphere*> &m_spheres, vector<Plane*> &m_planes, char *filename)
{
	string line;
	int count = 0;
	ifstream fd(filename);
	if(fd.is_open())
	{
		while(fd.good())
		{	
			getline(fd, line);
			if(line[0] == '#') continue; //ignore comments
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
				//printCam(newCam);
				m_cameras.push_back(newCam);
				count++;
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
				//printLight(newLight);
				m_lights.push_back(newLight);
				count++;
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
			
				//printSphere(newSphere);
				m_spheres.push_back(newSphere);
				count++;
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
				if(line != "end" && line[0] != '#')  //transparencycould be commented out
				{
					//TODO: check if it's a texture
					//texture
					newPlane->hastexture = true;
					//TODO: set the texture	
					
					getline(fd, line);
					//end	
					if(line != "end")
						printf("HMMMMM\n");
				}
				else
				{
					newPlane->hastexture = false;
				}
				//printPlane(newPlane);
			
				m_planes.push_back(newPlane);	
				count++;		
			}
		}
		fd.close();
	}
	return count;
}

// Vector Manipulation Functions
Vec3 Add(Vec3 u, Vec3 v)
{
	Vec3 newVec;
	newVec.x = u.x+v.x;
	newVec.y = u.y+v.y;
	newVec.z = u.z+v.z;
	return newVec;
}

Vec3 Subtract(Vec3 u, Vec3 v)
{
	Vec3 newVec;
	newVec.x = u.x-v.x;
	newVec.y = u.y-v.y;
	newVec.z = u.z-v.z;
	return newVec;
}

// Scalar multiplication
Vec3 Mult(float s, Vec3 u)
{
	Vec3 newVec;
	newVec.x = s * u.x;
	newVec.y = s * u.y;
	newVec.z = s * u.z;
	return newVec;
}

// Given Vectors u and v, returns <u,v>
float Dot(Vec3 u, Vec3 v)
{
	return (u.x*v.x)+(u.y*v.y)+(u.z*v.z);		
}

// Cross Product
Vec3 Cross(Vec3 u, Vec3 v)
{
	Vec3 newVec;
	newVec.x = u.y*v.z-v.y*u.z;
	newVec.y = v.x*u.z-u.x*v.z;
	newVec.z = u.x*v.y-u.y*v.x;
	return newVec;
}

// Given Vector u, returns ||u||^2
float MagSquared(Vec3 u)
{
	return Dot(u,u);
}


float intersectSphereAt(Ray *r, Sphere *s)
{
	//a bunch of linear algebra ..

	// First we try to draw a right angled triangle using
	// the origin of the ray and the center of the sphere
	// as our hypothenuse.

	Vec3 hyph;
	hyph = Subtract(s->center, r->origin);
	
	// Then we find the length of the other sides of the 
        // triange using dot products (since ||r->direction|| = 1)
	float a, b, b_squared;
	a = Dot(r->direction, hyph); 
	b_squared = MagSquared(hyph) - a*a;
	
	// since b, and radius are both positive, 
	// b > radius <=> b^2 > r^2
	if(b_squared > (s->radius * s->radius))
	{
		//if point C of the triangle is outside the circle
		//the ray doesn't hit the sphere
		return MY_NAN;  
	}

	//The ray could intersect with the sphere twice, and point C
	//(the point that has the right angle) of the triangle
	//should be the midpoint of the 2 intersecting points.
	//The distance from C to any of those points can be found by
	//drawing another smaller right angled triangle inside the 
	//circle, with r as the hypothenuse and b as a side.
	float d = sqrt(s->radius * s->radius - b_squared);
	//cout << "c is " << sqrt(MagSquared(hyph)) << " and a is " << a << " and b_squared is " << b_squared << " and d is " << d << endl;
	
	//if d > a then the ray started inside the sphere.
	if(d > a)
		return MY_NAN;

	float dist1 = a-d;
	float dist2 = a+d;
	//return the smaller one.
	return (dist1 < dist2)? dist1:dist2;
}

/*float intersectPlaneAt(Ray r, Plane p)
{

}*/

int main(int argc, char * argv[])
{
	//make sure that an argument is provided
	if(argc != 2)
	{
		fprintf(stderr, "Usage: ./raytrace [scene specification]\n");
	}
	//same the specification filename
	char *filename = argv[1];

	// find the pre '-' portion of the output filename
	string outputFilename = "";

	// find the last '/' within the filename
	size_t lastSlash = -1;
	for(size_t i = 0; filename[i] != '\0'; i++)
	{
		if(filename[i] == '/') lastSlash = i;
	}

	//extract the part before the .txt
	for(size_t i = lastSlash+1; ; i++)
	{
		if(filename[i] == '.') break;
		if(filename[i] == '\0') break;
		outputFilename = outputFilename + filename[i];
	}

	vector<Camera*> m_cameras;
	vector<Light*> m_lights;
	vector<Sphere*> m_spheres;
	vector<Plane*> m_planes;    

	//parse the specification file.
	int numObjects = parseInput(m_cameras, m_lights, m_spheres, m_planes, filename);
	cout << numObjects << " objects read.\n";
	
	//for camera,
	for(int i = 0; i < m_cameras.size(); i++)
	{
		//setup the image
		BMP image;
		image.SetSize(W,H);
		image.SetBitDepth(32);

		Camera *thisCam = m_cameras[i];
		Point c_org = thisCam->origin;
		Point c_dir = thisCam->direction; //Note that in our scenes,
						  //they are all normal.
		Point c_up = thisCam->up;
		//Left vector of the screen
		Point c_left = Cross(c_up, c_dir);
		float c_w = thisCam->width;
		// based on aspect radio
		float c_h = c_w * H / W;

		
		//draw the image
		if(!thisCam->perspective)
		{
			cout << "Camera is orthogonal\n";
			//orthogonal
			for(int x = 0; x < W; x++)
				for(int y = 0; y < H; y++)
				{
					//the coordinate this pixel is representing
					//within the scene
					float a1 = (-(c_w)/2) + ((c_w)*((float)x/W));
					float a2 = (-(c_h)/2) + ((c_h)*((float)y/H));
					Vec3 posOnScene = Add( Add(c_org, Mult(a1, c_left)), Mult(a2, c_up));
					// create a ray that starts from this pixel
					Ray *ray = new Ray;
					ray->origin = posOnScene;
					//cout << posOnScene.x << " " << posOnScene.y << " " << posOnScene.z << endl;
					ray->direction = c_dir;

					// find the closest object for each pixel
					// TODO: check all spheres
					float dist = intersectSphereAt(ray, m_spheres[0]);
					if(dist != MY_NAN)
					{
						//TODO change based on sphere
						image(x,y)->Red = 255;
						image(x,y)->Green = 0;
						image(x,y)->Blue = 0;
						image(x,y)->Alpha = 0;
					}
					else
					{
						image(x,y)->Red = 0;
						image(x,y)->Green = 255;
						image(x,y)->Blue = 0;
						image(x,y)->Alpha = 0;
					}
				}
		}
		else
		{
			//perspective
			for(int x=0; x<W; x++)
        			for(int y=0; y<H; y++)
        			{
            				image(x,y)->Red   = 255;
            				image(x,y)->Green = 0;
            				image(x,y)->Blue  = 0;
            				image(x,y)->Alpha = 0;
        			}
		}
	
		// output the file
		// TODO:
		// NOTE: this only supports up to 10 cameras.  Then the file
		// name breaks down :(
		string camNumInString = "-0.bmp";
		camNumInString[1] = i+'0';
		string output = outputFilename + camNumInString;
		cout << "Output filename is " << output << endl;
	    	image.WriteToFile(output.c_str());
	}

	//BMP texture;
    	//texture.ReadFromFile("output.bmp");

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
