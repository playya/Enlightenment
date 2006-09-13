
#include <Evas.h>
#include <Ecore.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct _E_Phys_Point E_Phys_Point;
typedef struct _E_Phys_Point E_Phys_Vector;
typedef struct _E_Phys_Particle E_Phys_Particle;

typedef struct _E_Phys_Constraint E_Phys_Constraint;
typedef struct _E_Phys_Force E_Phys_Force;
typedef struct _E_Phys_Force_Gravity E_Phys_Force_Gravity;
typedef struct _E_Phys_Force_Spring E_Phys_Force_Spring;
typedef struct _E_Phys_Force_Collision E_Phys_Force_Collision;
typedef struct _E_Phys_Constraint_Boundary E_Phys_Constraint_Boundary;
typedef struct _E_Phys_Constraint_Stick E_Phys_Constraint_Stick;
typedef struct _E_Phys_Constraint_Anchor E_Phys_Constraint_Anchor;
typedef struct _E_Phys_World E_Phys_World;

E_Phys_World    *e_phys_world_add(int w, int h);
void             e_phys_world_free(E_Phys_World *world);
void             e_phys_world_go(E_Phys_World *world);
void             e_phys_world_stop(E_Phys_World *world);
void             e_phys_world_update_func_set(E_Phys_World *world,
                   void (*func) (void *data, E_Phys_World *world),
                   void *data);
void             e_phys_world_size_set(E_Phys_World *world, int w, int h);
E_Phys_Particle *e_phys_world_nearest_particle(E_Phys_World *world,
                   int x, int y);

E_Phys_Particle *e_phys_particle_add(E_Phys_World *world, float mass,
                   float x, float y, float vx, float vy);
void             e_phys_particle_free(E_Phys_Particle *p);
void             e_phys_particle_size_set(E_Phys_Particle *p,
                   float w, float h);

void e_phys_force_init(E_Phys_Force *force, E_Phys_World *world,
       void (*apply_func) (E_Phys_Force *force),
       void (*free_func) (E_Phys_Force *force));
void e_phys_force_free(E_Phys_Force *force);

E_Phys_Force_Collision *e_phys_force_collision_add(E_Phys_World *world);
E_Phys_Force_Spring    *e_phys_force_spring_add(E_Phys_World *world,
                          E_Phys_Particle *p1, E_Phys_Particle *p2, int k, int len);
E_Phys_Force_Spring    *e_phys_force_modified_spring_add(E_Phys_World *world,
                          E_Phys_Particle *p1, E_Phys_Particle *p2, int k, int len);
E_Phys_Force_Gravity   *e_phys_force_gravity_add(E_Phys_World *world, float g);


void e_phys_constraint_init(E_Phys_Constraint *con, E_Phys_World *world,
       void (*apply_func) (E_Phys_Constraint *con),
       void (*free_func) (E_Phys_Constraint *con));
void e_phys_constraint_del(E_Phys_Constraint *con);
void e_phys_constraint_free(E_Phys_Constraint *con);

E_Phys_Constraint_Boundary *e_phys_constraint_boundary_add(E_Phys_World *world);
E_Phys_Constraint_Stick    *e_phys_constraint_stick_add(
                              E_Phys_Particle *p1,
                              E_Phys_Particle *p2,
                              int len);
E_Phys_Constraint_Anchor   *e_phys_constraint_anchor_add(
                              E_Phys_Particle *p, float x, float y);





#define E_PHYS_CONSTRAINT(x) ((E_Phys_Constraint *)x)
#define E_PHYS_FORCE(x) ((E_Phys_Force *)x)

struct _E_Phys_Point
{
  float x, y;
};

struct _E_Phys_Particle {
  E_Phys_World *world;
  float m;
  float w, h; // extended size
  E_Phys_Point cur;
  E_Phys_Point prev;
  E_Phys_Vector force;
};

struct _E_Phys_Constraint {
  E_Phys_World *world;
  void (*apply) (E_Phys_Constraint *con);
  void (*free) (E_Phys_Constraint *con);
};

struct _E_Phys_Force {
  E_Phys_World *world;
  void (*apply) (E_Phys_Force *force);
  void (*free) (E_Phys_Force *force);
};

struct _E_Phys_Force_Collision {
  E_Phys_Force force;
  float e; // elasticity?
};

struct _E_Phys_Force_Spring {
  E_Phys_Force force;
  E_Phys_Particle *p1;
  E_Phys_Particle *p2;
  float len;
  float k;
  int modified;
};

struct _E_Phys_Force_Gravity {
  E_Phys_Force force;
  float g;
};

struct _E_Phys_Constraint_Boundary {
  E_Phys_Constraint con;
  float e; // elasticity (0 fully inelastic -> 1 fully elastic)
  Evas_List *exclusions;
};

struct _E_Phys_Constraint_Stick {
  E_Phys_Constraint con;
  E_Phys_Particle *p1;
  E_Phys_Particle *p2;
  float len;
};

struct _E_Phys_Constraint_Anchor {
  E_Phys_Constraint con;
  E_Phys_Particle *p;
  E_Phys_Point pos;
};

struct _E_Phys_World
{
  int w, h;

  Evas_List *particles;
  Evas_List *forces;
  Evas_List *constraints;

  Ecore_Timer *timer;

  float dt;
  int constraint_iter;
  float friction;

  void (*update_func) (void *data, E_Phys_World *world);
  void *update_data;
};
