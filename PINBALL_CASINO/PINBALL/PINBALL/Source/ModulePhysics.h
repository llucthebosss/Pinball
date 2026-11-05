#pragma once
#include "Module.h"
#include "Globals.h"
#include "box2d/box2d.h"
#include <vector>

#define PIXELS_PER_METER 50.0f
#define PIXEL_TO_METERS(p)  ((p) / PIXELS_PER_METER)
#define METERS_TO_PIXELS(m) ((m) * PIXELS_PER_METER)

class PhysBody
{
public:
    PhysBody() : body(nullptr), listener(nullptr) {}

    void GetPosition(int& x, int& y) const;
    float GetRotation() const;

    b2Body* body;
    Module* listener;
};

class ModulePhysics : public Module, public b2ContactListener
{
public:
    ModulePhysics(Application* app, bool start_enabled = true);
    ~ModulePhysics();

    bool Start();
    update_status PreUpdate();
    update_status PostUpdate();
    bool CleanUp();

    PhysBody* CreateCircle(int x, int y, int radius, b2BodyType type = b2_dynamicBody);
    PhysBody* CreateRectangle(int x, int y, int width, int height, b2BodyType type = b2_dynamicBody);

    // Cadena de línia fina
    PhysBody* CreateChain(int x, int y, const int* points, int size);

    // Murs sòlids: un rectangle per cada segment
    PhysBody* CreateSolidChain(int x, int y, const int* points, int size, float thickness = 8.0f);

    b2RevoluteJoint* CreateFlipper(int pivot_x, int pivot_y, int length, int thickness, bool left, float restAngle, float upAngle);
    b2PrismaticJoint* CreateKicker(int x, int y, int width, int height, int travel);

    void BeginContact(b2Contact* contact) override;
    PhysBody* CreateRectangleSensor(int x, int y, int width, int height);
    void DestroyBody(PhysBody* pbody);
    void SetGravity(float g);
    float GetGravity() const;
private:
    void UpdateMouseJoint();

    b2World* world = nullptr;
    b2Body* ground = nullptr;
    b2MouseJoint* mouse_joint = nullptr;
    std::vector<PhysBody*> bodies;
    float accumulator = 0.0f;
    bool debug;
};