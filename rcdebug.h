#include <iostream>
#include <stdio.h>

struct _Sphere;
struct _Camera;
struct _Plane;
struct _Light;


void printCam(_Camera* c)
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

void printLight(_Light *l)
{
        printf("=====================\n");
        printf("Type: Light\n");
        printf("Origin: %f %f %f\n", l->origin.x, l->origin.y, l->origin.z);
        printf("Color: %f %f %f\n", l->color.r, l->color.g, l->color.b);
        printf("=====================\n\n");
}

void printSphere(_Sphere *s)
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

void printPlane(_Plane *p)
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


