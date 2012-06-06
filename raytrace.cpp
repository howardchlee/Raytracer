#include <stdio.h>
#include <iostream>
#include <vector>
#include <string.h>
#include "EasyBMP.h"
#include <fstream>
#include <sstream>
#include <math.h>

using namespace std;

#define MAXSTEPS 3

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

//#include "rtdebug.h"

// Static scene definition
Light light   = {{0,10,0}, {1,1,1}};
Camera camera = {{0,0,0},  {0,0,1}, {0,1,0}, 0, 10, 10, false};
Sphere sphere = {{0,0,5},  {{1,0,0}, 0, 0}, 2};

#define MY_NAN 100000

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
					//texture
					newPlane->hastexture = true;
					//TODO: parse and set the texture
					string textureName = "";
					bool copy = false;
					for(size_t i = 0; i < line.size(); i++)
					{
						if(copy)
						{
							textureName = textureName + line[i];
						}
						if(line[i] == ' ')
							copy = true;
					}
					cout << "texture is " << textureName << endl;	
					newPlane->texture.ReadFromFile(textureName.c_str());

					/*//PRINTTEXTURE
					for(int i = 0; i < newPlane->texture.TellWidth();i++)
					{
						for(int j = 0; j < newPlane->texture.TellHeight(); j++)
						{
							RGBApixel thispixel = *newPlane->texture(i,j);
							Color retCol;
							retCol.r = ((float)thispixel.Red)/255;
							retCol.g = ((float)thispixel.Green)/255;
							retCol.b = ((float)thispixel.Blue)/255;
							printf("(%d, %d): %f %f %f\n", i, j, retCol.r, retCol.g, retCol.b);
						}
					}*/
					
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

Vec3 Normalize(Vec3 u)
{
	return Mult(1/sqrt(Dot(u,u)), u);
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
	{
		return MY_NAN;
	}

	float dist1 = a-d;
	float dist2 = a+d;
	//return the smaller one.
	return (dist1 < dist2)? dist1:dist2;
}


// checks if a point is on a plane
bool isOnPlane(Point p, Plane *plane)
{
	bool facingX = plane->normal.x;
	bool facingY = plane->normal.y;
	bool facingZ = plane->normal.z;
	bool upX = plane->up.x;
	bool upY = plane->up.y;
	bool upZ = plane->up.z;
	if(facingX)
	{
		if(p.x != plane->center.x) return false;
	}
	else if(facingY)
	{
		if(p.y != plane->center.y) return false;
	}
	else if(facingZ)
	{
		if(p.z != plane->center.z) return false;
	}

	if(facingX)
	{
		if(upY)
		{
			if( (abs(p.y-plane->center.y) <= plane->height/2) && (abs(p.z-plane->center.z) <= plane->width/2))
				return true;
		}
		if(upZ)
		{
			if( (abs(p.z-plane->center.z) <= plane->height/2) && (abs(p.y-plane->center.y) <= plane->width/2))
				return true;
		}
	}
	else if(facingY)
	{
		if(upX)
		{
			if( (abs(p.x-plane->center.x) <= plane->height/2) && (abs(p.z-plane->center.z) <= plane->width/2))
				return true;
		}
		if(upZ)
		{	
			if( (abs(p.z-plane->center.z) <= plane->height/2) && (abs(p.x-plane->center.x) <= plane->width/2))
				return true;
		}
	}
	else if(facingZ)
	{
		if(upX)
		{
			if( (abs(p.x-plane->center.x) <= plane->height/2) && (abs(p.y-plane->center.y) <= plane->width/2))
				return true;
		}
		if(upY)
		{
			if( (abs(p.y-plane->center.y) <= plane->height/2) && (abs(p.x-plane->center.x) <= plane->width/2))
				return true;
		}
	}
	return false;
	
}

float intersectPlaneAt(Ray *r, Plane *p)
{
	// based on the scenes, they can only face an axis
	bool facingX=true, facingY=true, facingZ=true;
	if(p->normal.x == 0) facingX = false;
	if(p->normal.y == 0) facingY = false;
	if(p->normal.z == 0) facingZ = false;

	//unit vector from origin to plane perpendicularly (-normal)
	Vec3 u_vec = Mult(-1, p->normal);
	float cosangle = Dot(r->direction, u_vec);
	if(cosangle <= 0) return MY_NAN;
	// now we look at the triangle with u_vec as the opposite side
	// where the hypothenuse parallel to ray
	float hyph = 1./cosangle;	
	Vec3 hyphVector = Mult(hyph, r->direction);

	// find the actual perpendicular distance
	float pd;
	if(facingX)
		pd = abs(p->center.x - r->origin.x);		
	else if(facingY)
		pd = abs(p->center.y - r->origin.y);
	else if(facingZ)
		pd = abs(p->center.z - r->origin.z);
	// then use that ratio to find the "hitting point" of the ray
	// to the plane, to see if it actually hits the plane
	Vec3 rayVector = Mult(pd, hyphVector);
	Point hittingPoint = Add(r->origin, rayVector);
	if(facingX)
	{
		if(hittingPoint.x - p->center.x < 0.0001) hittingPoint.x = p->center.x;
	}
	else if(facingY)
	{
		if(hittingPoint.y - p->center.y < 0.0001) hittingPoint.y = p->center.y;
	}
	else
	{
		if(hittingPoint.z - p->center.z < 0.0001) hittingPoint.z = p->center.z;
	}

	if(isOnPlane(hittingPoint, p))
		return hyph * pd;		
	else
		return MY_NAN;
}

Color illuminate(Point p, Ray *normal, vector<Light*> m_lights, vector<Sphere*> m_spheres, vector<Plane*> m_planes, size_t thisSphereIndex, size_t thisPlaneIndex)
{
	//Since I am not sure how to do this, I will just do this by calculating
	//how "close" the reflected ray is compared to the vector from the point
	//to the light source.  This way, if the reflected ray IS directed to the
	//light source then color = the light source of the color.  So here it makes
	//sense to dot the 2 rays and use the angle 

	//for each light source in the scene, we find a unit vector from the point
	//directed to the light
	Color retCol;
	retCol.r = 0;
	retCol.g = 0;
	retCol.b = 0;
	for(size_t i = 0; i < m_lights.size(); i++)
	{
		Light *l = m_lights[i];
		Vec3 vectorToLight = Subtract(l->origin, p);
		float magnitude = sqrt(MagSquared(vectorToLight));
		vectorToLight = Normalize(vectorToLight); // unit vector = vector/mag
		
		bool illuminated = true;
		// now we check if the point is illuminated at all (if any object
		// if blockng the way)
		Ray *ray = new Ray();
		ray->origin = p;
		ray->direction = vectorToLight;
		for(size_t j = 0; j < m_spheres.size(); j++)
		{
			if(intersectSphereAt(ray, m_spheres[j]) < magnitude)
			{
				if(j == thisSphereIndex) continue;
				cout << "BLOCKED by Sphere #" << j << endl;
				illuminated = false;
				break;
			}
		}
		for(size_t j = 0; j < m_planes.size(); j++)
		{
			if(intersectPlaneAt(ray, m_planes[j]) < magnitude)
			{
				if(j == thisPlaneIndex) continue;
				cout << "Light " << i << " BLOCKED by Plane #" << j << endl;
				illuminated = false;
				break;
			}
		}
		delete ray;

		if(!illuminated)
		{
			continue;
		}

		// this point is illuminated.
		// calculate how close the reflected ray is to the lightsource
		float cosAngle = Dot(vectorToLight, normal->direction);
		if(cosAngle < 0)
			continue;
		retCol.r += l->color.r * cosAngle;
		retCol.g += l->color.g * cosAngle;
		retCol.b += l->color.b * cosAngle;
		//printf("VecToLight: %f %f %f; Reflected %f %f %f; retColor: %f %f %f\n\n", p.x, p.y, p.z, vectorToLight.x, vectorToLight.y, vectorToLight.z, retCol.r, retCol.g, retCol.b);
	}
	if(retCol.r > 1) retCol.r = 1;
	if(retCol.g > 1) retCol.g = 1;
	if(retCol.b > 1) retCol.b = 1;
	//printf("for point (%f %f %f), illuminate = (%f %f %f)\n", p.x, p.y, p.z, retCol.r, retCol.g, retCol.b);
	return retCol;
}

Color trace(Ray *ray, vector<Sphere*> m_spheres, vector<Plane*> m_planes, vector<Light*> m_lights, float zmin, float zmax, int steps)
{
	//if(steps == 0)
	//	printf("Tracing step %d: ray origin: %f %f %f, direction: %f %f %f\n", steps, ray->origin.x, ray->origin.y, ray->origin.z, ray->direction.x, ray->direction.y, ray->direction.z);	
	if(steps >= MAXSTEPS)
	{
		Color retCol;
		retCol.r = 0;
		retCol.g = 0;
		retCol.b = 0;
		return retCol;
	}
	// check all spheres
	float closestSphere = MY_NAN;
	size_t closestSphereIndex = 0;
	for(size_t i = 0; i < m_spheres.size(); i++)
	{
		float dist = intersectSphereAt(ray, m_spheres[i]);
		//printf("dist: %f, zmin: %f, zmax: %f, closestSphere: %f\n", dist, zmin, zmax, closestSphere);
		//also have to make sure it's within the zmin zmax range
		if(dist>zmin && dist <= closestSphere && dist <= zmax) 
		{
			closestSphere = dist;
			closestSphereIndex = i;
		}
	}

	//check all planes
	float closestPlane = MY_NAN;
	size_t closestPlaneIndex = 0;
	for(size_t i = 0; i < m_planes.size(); i++)
	{
		float dist = intersectPlaneAt(ray, m_planes[i]);
		if(dist>zmin && dist <= closestPlane && dist <= zmax)
		{
			closestPlane = dist;
			closestPlaneIndex = i;
		}							
	}
	
	float closestObject = (closestSphere < closestPlane)? closestSphere : closestPlane;
	
	if(closestObject != MY_NAN)
	{
		ebmpBYTE c_red;
		ebmpBYTE c_green;
		ebmpBYTE c_blue;
		//posOnScene.y =  -posOnScene.y;
				
		if(closestSphere < closestPlane)
		{
			Sphere *thisSphere = m_spheres[closestSphereIndex];
			// Calculate the illumination of that point
			Point hittingPoint = Add(ray->origin, Mult(closestSphere, ray->direction));
			Ray *reflected = new Ray();
			Ray *normalRay = new Ray();
			normalRay->origin = hittingPoint;
			reflected->origin = hittingPoint;
			Vec3 normal = Subtract(hittingPoint, thisSphere->center);
			normal = Normalize( normal);
			Vec3 incident = Subtract(hittingPoint, ray->origin);
			incident = Normalize( incident);
			normalRay->direction = normal;
			reflected->direction = Add(Mult(2 *Dot(Mult(-1, incident), normal), normal), incident);
			Color illColor = illuminate(hittingPoint, normalRay, m_lights, m_spheres, m_planes, closestSphereIndex, MY_NAN);
			
			Color reflectionRay = trace(reflected, m_spheres, m_planes, m_lights, 0, MY_NAN-11, steps+1);
			reflectionRay.r = thisSphere->material.reflection * reflectionRay.r;
			reflectionRay.g = thisSphere->material.reflection * reflectionRay.g;
			reflectionRay.b = thisSphere->material.reflection * reflectionRay.b;

			//printf("HittingPoint: %f %f %f; PosOnScreen: %f %f %f \n", hittingPoint.x, hittingPoint.y, hittingPoint.z, ray->origin.x, ray->origin.y, ray->origin.z);
			//cout << "Reflection: " << thisSphere->material.reflection << " ; Transparency: " << thisSphere->material.transparency << endl;
		
			Ray * trans = new Ray();
			trans->origin = hittingPoint;
			trans->direction = incident;
			//TODO: so the sphere doesn't hit itself :(
			Color transRay = trace(trans, m_spheres, m_planes, m_lights, 0.001, zmax, steps+1);
			transRay.r = thisSphere->material.transparency * transRay.r;
			transRay.g = thisSphere->material.transparency * transRay.g;
			transRay.b = thisSphere->material.transparency * transRay.b;
			/*if(steps == 0)
				printf("reflectionRay: %f %f %f transRay: %f %f %f\n", reflectionRay.r, reflectionRay.g, reflectionRay.b, transRay.r, transRay.g, transRay.b);*/

			delete reflected;
			delete normalRay;
			delete trans;
			// Color the pixel based on the nearest object
			Color drawColor;
			drawColor.r = m_spheres[closestSphereIndex]->material.color.r + reflectionRay.r + transRay.r;
			drawColor.g = m_spheres[closestSphereIndex]->material.color.g + reflectionRay.g + transRay.g;
			drawColor.b = m_spheres[closestSphereIndex]->material.color.b + reflectionRay.b + transRay.b;
			if(illColor.r == 0 && illColor.g == 0 && illColor.b == 0)
			{
				drawColor.r = 0;
				drawColor.g = 0;
				drawColor.b = 0;
			}
			else
			{
				if(drawColor.r > 1) drawColor.r = 1 ;
				if(drawColor.g > 1) drawColor.g = 1;
				if(drawColor.b > 1) drawColor.b = 1;
				drawColor.r *= illColor.r;
				drawColor.g *= illColor.g;
				drawColor.b *=  illColor.b;
			}
			if(drawColor.r < 0 || drawColor.r!= drawColor.r) drawColor.r = 0;
			if(drawColor.g < 0 || drawColor.g!= drawColor.g) drawColor.g = 0;
			if(drawColor.b < 0 || drawColor.b!= drawColor.b) drawColor.b = 0;
			return drawColor;
		}
		else
		{
			Plane *thisPlane = m_planes[closestPlaneIndex];
			if(!(thisPlane->hastexture))
			{
				// Calculate the illumination of that point
				Point hittingPoint = Add(ray->origin, Mult(closestPlane, ray->direction));
				Ray *reflected = new Ray();
				Ray *normalRay = new Ray();
				normalRay->origin = hittingPoint;
				reflected->origin = hittingPoint;
				Vec3 normal = thisPlane->normal;
				normal = Normalize(normal);
				Vec3 incident = Subtract(hittingPoint, ray->origin);
				incident = Normalize( incident);
				normalRay->direction = normal;
				reflected->direction = Add(Mult(2 *Dot(Mult(-1, incident), normal), normal), incident);
				Color illColor = illuminate(hittingPoint, normalRay, m_lights, m_spheres, m_planes, MY_NAN, closestPlaneIndex);

				Color reflectionRay = trace(reflected, m_spheres, m_planes, m_lights, 0, MY_NAN-1, steps+1);
				reflectionRay.r = thisPlane->material.reflection * reflectionRay.r;
				reflectionRay.g = thisPlane->material.reflection * reflectionRay.g;
				reflectionRay.b = thisPlane->material.reflection * reflectionRay.b;
	
				Ray * trans = new Ray();
				trans->origin = hittingPoint;
				trans->direction = incident;
				Color transRay = trace(trans, m_spheres, m_planes, m_lights, 0, zmax, steps+1);
				transRay.r = thisPlane->material.transparency * transRay.r;
				transRay.g = thisPlane->material.transparency * transRay.g;
				transRay.b = thisPlane->material.transparency * transRay.b;

				/*if(reflectionRay.r != 0)
					printf("Reflection Ray: %f %f %f\n", reflectionRay.r, reflectionRay.g, reflectionRay.b);
				*/
				delete trans;
				delete reflected;
				delete normalRay;
				// Color the pixel based on the nearest object
				Color drawColor;
				drawColor.r = m_planes[closestPlaneIndex]->material.color.r + transRay.r + reflectionRay.r;
				drawColor.g = m_planes[closestPlaneIndex]->material.color.g + transRay.g + reflectionRay.g;
				drawColor.b = m_planes[closestPlaneIndex]->material.color.b + transRay.b + reflectionRay.b;
				if(illColor.r == 0 && illColor.g == 0 && illColor.b == 0)
				{
					drawColor.r = 0;
					drawColor.g = 0;
					drawColor.b = 0;
				}
				else
				{
					drawColor.r *= illColor.r;
					drawColor.g *= illColor.g;
					drawColor.b *=  illColor.b;
					if(drawColor.r > 1) drawColor.r = 1 ;
					if(drawColor.g > 1) drawColor.g = 1;
					if(drawColor.b > 1) drawColor.b = 1;
				}
				if(drawColor.r < 0 || drawColor.r!= drawColor.r) drawColor.r = 0;
				if(drawColor.g < 0 || drawColor.g!= drawColor.g) drawColor.g = 0;
				if(drawColor.b < 0 || drawColor.b!= drawColor.b) drawColor.b = 0;
				return drawColor;
			}
			else
			{
				//LBL: HASTEXTURE
				//print the correct pixel of the texture
				Point hittingPoint = Add(ray->origin, Mult(closestPlane, ray->direction));
				Ray *reflected = new Ray();
				Ray *normalRay = new Ray();
				normalRay->origin = hittingPoint;
				reflected->origin = hittingPoint;
				Vec3 normal = thisPlane->normal;
				normal = Normalize(normal);
				Vec3 incident = Subtract(hittingPoint, ray->origin);
				incident = Normalize( incident);
				normalRay->direction = normal;
				reflected->direction = Add(Mult(2 *Dot(Mult(-1, incident), normal), normal), incident);
				Color illColor = illuminate(hittingPoint, normalRay, m_lights, m_spheres, m_planes, MY_NAN, closestPlaneIndex);

				Color reflectionRay = trace(reflected, m_spheres, m_planes, m_lights, 0, MY_NAN-1, steps+1);
				reflectionRay.r = thisPlane->material.reflection * reflectionRay.r;
				reflectionRay.g = thisPlane->material.reflection * reflectionRay.g;
				reflectionRay.b = thisPlane->material.reflection * reflectionRay.b;
	
				Ray * trans = new Ray();
				trans->origin = hittingPoint;
				trans->direction = incident;
				Color transRay = trace(trans, m_spheres, m_planes, m_lights, 0, zmax, steps+1);
				transRay.r = thisPlane->material.transparency * transRay.r;
				transRay.g = thisPlane->material.transparency * transRay.g;
				transRay.b = thisPlane->material.transparency * transRay.b;

				if(reflectionRay.r != 0)
					printf("Reflection Ray: %f %f %f\n", reflectionRay.r, reflectionRay.g, reflectionRay.b);
				delete trans;
				delete reflected;
				
				// the position of this point within the texture
				// NOTE this is NOT robust.  This will only work on textures on planes
				// that have a normal vector parallel to the z axis
				float texture_i = (hittingPoint.x - thisPlane->center.x + thisPlane->width/2) / thisPlane->width * thisPlane->texture.TellWidth();
				float texture_j = (thisPlane->height-(hittingPoint.y-thisPlane->center.y + thisPlane->height/2))/thisPlane->height * thisPlane->texture.TellHeight();
				RGBApixel thispixel = thisPlane->texture.GetPixel(texture_i, texture_j);
				//RGBApixel thispixel = thisPlane->texture.GetPixel((hittingPoint.x-thisPlane->center.x+thisPlane->width/2)/thisPlane->width*thisPlane->texture.TellWidth(),(thisPlane->height-(hittingPoint.y-thisPlane->center.y+thisPlane->height/2))/thisPlane->height*thisPlane->texture.TellHeight());
				Color drawColor;
				drawColor.r = ((float)thispixel.Red)/255 + transRay.r + reflectionRay.r;
				drawColor.g = ((float)thispixel.Green)/255 + transRay.g + reflectionRay.g;
				drawColor.b = ((float)thispixel.Blue)/255 + transRay.b + reflectionRay.b;
				//printf("retCol: %f %f %f\n", retCol.r, retCol.g, retCol.b);
				if(illColor.r == 0 && illColor.g == 0 && illColor.b == 0)
				{
					drawColor.r = 0;
					drawColor.g = 0;
					drawColor.b = 0;
				}
				else
				{
					printf("illColor: %f %f %f\n", illColor.r, illColor.g, illColor.b);
					drawColor.r *= illColor.r;
					drawColor.g *= illColor.g;
					drawColor.b *=  illColor.b;
					if(drawColor.r > 1) drawColor.r = 1 ;
					if(drawColor.g > 1) drawColor.g = 1;
					if(drawColor.b > 1) drawColor.b = 1;
				}
				if(drawColor.r < 0 || drawColor.r!= drawColor.r) drawColor.r = 0;
				if(drawColor.g < 0 || drawColor.g!= drawColor.g) drawColor.g = 0;
				if(drawColor.b < 0 || drawColor.b!= drawColor.b) drawColor.b = 0;
				return drawColor;
			}
		}
	}
	else
	{
		if(steps == 0)
			cout << "MY_NAN\n";
		Color retCol = {0,0,0};
		return retCol;
	}

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
		float zmin = thisCam->zmin;
		float zmax = thisCam->zmax;

		
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
					float a2 = ((c_h)/2) - ((c_h)*((float)y/H));
					Vec3 posOnScene = Add( Add(c_org, Mult(a1, c_left)), Mult(a2, c_up));
					// create a ray that starts from this pixel
					Ray *ray = new Ray;
					ray->origin = posOnScene;
					//cout << posOnScene.x << " " << posOnScene.y << " " << posOnScene.z << endl;
					ray->direction = c_dir;
					Color drawColor = trace(ray, m_spheres, m_planes, m_lights, zmin, zmax, 0);
					delete ray;
					ebmpBYTE c_red = (ebmpBYTE) 255* drawColor.r;
					ebmpBYTE c_green = (ebmpBYTE) 255 * drawColor.g;
					ebmpBYTE c_blue = (ebmpBYTE) 255 * drawColor.b;
					image(x,y)->Red = c_red;
					image(x,y)->Green = c_green;
					image(x,y)->Blue = c_blue;
					image(x,y)->Alpha = 0;  //TODO
					
				}
		}
		else
		{
			//perspective
			Point translateCamera = Mult(zmin, c_dir);
			Point nearPlaneOrg = Add(c_org, translateCamera);  //so c_org is in the middle of the near plane
			for(int x=0; x<W; x++)
        			for(int y=0; y<H; y++)
        			{
					//the coordinate this pixel is representing
					//within the scene
					float a1 = (-(c_w)/2) + ((c_w)*((float)x/W));
					float a2 = ((c_h)/2) - ((c_h)*((float)y/H));
					Vec3 posOnScene = Add( Add(nearPlaneOrg, Mult(a1, c_left)), Mult(a2, c_up));
					// create a ray that starts from this pixel
					Ray *ray = new Ray;
					ray->origin = posOnScene;
					ray->direction = Subtract(posOnScene, c_org);
					ray->direction = Normalize( ray->direction);
					//printf("pos: %f %f %f; ray.d: %f %f %f\n", ray->origin.x, ray->origin.y, ray->origin.z, ray->direction.x, ray->direction.y, ray->direction.z);
					float d = Dot(ray->direction, c_dir);	
					float mymax = (zmax)/d;  //the end of the viewing volume
					Color drawColor = trace(ray, m_spheres, m_planes, m_lights, 0, mymax, 0);
					delete ray;
					printf("(%d, %d): %f %f %f\n", x, y, drawColor.r, drawColor.g, drawColor.b);
					ebmpBYTE c_red = (ebmpBYTE) 255* drawColor.r;
					ebmpBYTE c_green = (ebmpBYTE) 255 * drawColor.g;
					ebmpBYTE c_blue = (ebmpBYTE) 255 * drawColor.b;
					image(x,y)->Red = c_red;
					image(x,y)->Green = c_green;
					image(x,y)->Blue = c_blue;
					image(x,y)->Alpha = 0;  //TODO
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
