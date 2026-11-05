#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModulePhysics.h"

#include <math.h>
#include <algorithm>

// ---------------- PhysBody ----------------
void PhysBody::GetPosition(int& x, int& y) const
{
    b2Vec2 pos = body->GetPosition();
    x = (int)METERS_TO_PIXELS(pos.x);
    y = (int)METERS_TO_PIXELS(pos.y);
}

float PhysBody::GetRotation() const
{
    return body->GetAngle();
}

// ---------------- ModulePhysics ----------------
ModulePhysics::ModulePhysics(Application* app, bool start_enabled) : Module(app, start_enabled)
{
    debug = false;
}

ModulePhysics::~ModulePhysics() {}

bool ModulePhysics::Start()
{
    LOG("Creating Physics 2D environment");
    world = new b2World(b2Vec2(0.0f, 10.0f));
    world->SetContactListener(this);

    b2BodyDef groundDef;
    groundDef.type = b2_staticBody;
    ground = world->CreateBody(&groundDef);

    return true;
}

update_status ModulePhysics::PreUpdate()
{
    const float fixedStep = 1.0f / 60.0f;

    float frameTime = GetFrameTime();
    if (frameTime > 0.1f) frameTime = 0.1f;
    accumulator += frameTime;

    while (accumulator >= fixedStep)
    {
        world->Step(fixedStep, 12, 6);
        accumulator -= fixedStep;
    }

    return UPDATE_CONTINUE;
}

void ModulePhysics::SetGravity(float g)
{
    world->SetGravity(b2Vec2(0.0f, g));
}

float ModulePhysics::GetGravity() const
{
    return world->GetGravity().y;
}

PhysBody* ModulePhysics::CreateCircle(int x, int y, int radius, b2BodyType type)
{
    b2BodyDef def;
    def.type = type;
    def.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

    PhysBody* pbody = new PhysBody();
    def.userData.pointer = reinterpret_cast<uintptr_t>(pbody);
    pbody->body = world->CreateBody(&def);

    b2CircleShape shape;
    shape.m_radius = PIXEL_TO_METERS(radius);

    b2FixtureDef fixture;
    fixture.shape = &shape;
    fixture.density = 1.0f;
    fixture.friction = 0.3f;
    fixture.restitution = 0.5f;
    pbody->body->CreateFixture(&fixture);

    if (type == b2_dynamicBody)
        pbody->body->SetBullet(true);

    pbody->body->SetLinearDamping(0.2f);
    pbody->body->SetAngularDamping(0.2f);

    bodies.push_back(pbody);
    return pbody;
}

PhysBody* ModulePhysics::CreateRectangle(int x, int y, int width, int height, b2BodyType type)
{
    b2BodyDef def;
    def.type = type;
    def.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

    PhysBody* pbody = new PhysBody();
    def.userData.pointer = reinterpret_cast<uintptr_t>(pbody);
    pbody->body = world->CreateBody(&def);

    b2PolygonShape shape;
    shape.SetAsBox(PIXEL_TO_METERS(width) * 0.5f, PIXEL_TO_METERS(height) * 0.5f);

    b2FixtureDef fixture;
    fixture.shape = &shape;
    fixture.density = 1.0f;
    fixture.friction = 0.3f;
    fixture.restitution = 0.3f;
    pbody->body->CreateFixture(&fixture);

    bodies.push_back(pbody);
    return pbody;
}

PhysBody* ModulePhysics::CreateChain(int x, int y, const int* points, int size)
{
    b2BodyDef def;
    def.type = b2_staticBody;
    def.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

    PhysBody* pbody = new PhysBody();
    def.userData.pointer = reinterpret_cast<uintptr_t>(pbody);
    pbody->body = world->CreateBody(&def);

    int count = size / 2;
    b2Vec2* verts = new b2Vec2[count];
    for (int i = 0; i < count; ++i)
        verts[i].Set(PIXEL_TO_METERS(points[i * 2]), PIXEL_TO_METERS(points[i * 2 + 1]));

    b2ChainShape shape;
    shape.m_radius = PIXEL_TO_METERS(5.0f);
    shape.CreateLoop(verts, count);
    delete[] verts;

    b2FixtureDef fixture;
    fixture.shape = &shape;
    fixture.friction = 0.2f;
    fixture.restitution = 0.4f;
    fixture.density = 1.0f;
    pbody->body->CreateFixture(&fixture);

    bodies.push_back(pbody);
    return pbody;
}

// Murs sòlids
PhysBody* ModulePhysics::CreateSolidChain(int x, int y, const int* points, int size, float thickness)
{
    b2BodyDef def;
    def.type = b2_staticBody;
    def.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

    PhysBody* pbody = new PhysBody();
    def.userData.pointer = reinterpret_cast<uintptr_t>(pbody);
    pbody->body = world->CreateBody(&def);

    int count = size / 2;
    b2Vec2* verts = new b2Vec2[count];
    for (int i = 0; i < count; ++i)
        verts[i].Set(PIXEL_TO_METERS(points[i * 2]), PIXEL_TO_METERS(points[i * 2 + 1]));

    float halfThickness = PIXEL_TO_METERS(thickness) * 0.5f;

    for (int i = 0; i < count; ++i)
    {
        int j = (i + 1) % count; 

      
        float dx = verts[j].x - verts[i].x;
        float dy = verts[j].y - verts[i].y;
        float len = sqrtf(dx * dx + dy * dy);

        if (len < 0.0001f) continue; 

        
        float nx = -dy / len;
        float ny = dx / len;

      
        float ox = nx * halfThickness;
        float oy = ny * halfThickness;

        // 4 cantonades del rectangle
        b2Vec2 v1(verts[i].x + ox, verts[i].y + oy);
        b2Vec2 v2(verts[j].x + ox, verts[j].y + oy);
        b2Vec2 v3(verts[j].x - ox, verts[j].y - oy);
        b2Vec2 v4(verts[i].x - ox, verts[i].y - oy);

        b2Vec2 triangle1[3] = { v1, v2, v3 };
        b2Vec2 triangle2[3] = { v1, v3, v4 };

        b2PolygonShape shape1;
        shape1.Set(triangle1, 3);

        b2PolygonShape shape2;
        shape2.Set(triangle2, 3);

        b2FixtureDef fixture;
        fixture.friction = 0.2f;
        fixture.restitution = 0.4f;
        fixture.density = 1.0f;

        fixture.shape = &shape1;
        pbody->body->CreateFixture(&fixture);

        fixture.shape = &shape2;
        pbody->body->CreateFixture(&fixture);
    }

    delete[] verts;
    bodies.push_back(pbody);
    return pbody;
}

// restAngle: inclinació cap avall en repòs; upAngle: inclinació cap amunt quan colpeja (radiants)
b2RevoluteJoint* ModulePhysics::CreateFlipper(int pivot_x, int pivot_y, int length, int thickness, bool left, float restAngle, float upAngle)
{
    float hx = PIXEL_TO_METERS(length) * 0.5f;
    float hy = PIXEL_TO_METERS(thickness) * 0.5f;

    b2BodyDef def;
    def.type = b2_dynamicBody;
    def.position.Set(PIXEL_TO_METERS(pivot_x), PIXEL_TO_METERS(pivot_y));

    PhysBody* pbody = new PhysBody();
    def.userData.pointer = reinterpret_cast<uintptr_t>(pbody);
    pbody->body = world->CreateBody(&def);

    b2PolygonShape shape;
    shape.SetAsBox(hx, hy, b2Vec2(left ? hx : -hx, 0.0f), 0.0f);

    b2FixtureDef fixture;
    fixture.shape = &shape;
    fixture.density = 1.0f;
    fixture.restitution = 0.2f;
    fixture.friction = 0.3f;
    pbody->body->CreateFixture(&fixture);
    bodies.push_back(pbody);

    // El flipper esquerre s'estén cap a +x i baixa girant en sentit positiu;
    // el dret s'estén cap a -x i baixa girant en sentit negatiu
    float rest = left ? restAngle : -restAngle;
    float up = left ? -upAngle : upAngle;

    // L'articulació es crea amb el cos a angle 0, així els límits són angles absoluts
    b2RevoluteJointDef jd;
    jd.Initialize(ground, pbody->body, pbody->body->GetPosition());
    jd.enableLimit = true;
    jd.lowerAngle = fminf(rest, up);
    jd.upperAngle = fmaxf(rest, up);
    jd.enableMotor = true;
    jd.maxMotorTorque = 100.0f;
    jd.motorSpeed = left ? 15.0f : -15.0f;

    b2RevoluteJoint* joint = (b2RevoluteJoint*)world->CreateJoint(&jd);

    // Comença ja en repòs
    pbody->body->SetTransform(pbody->body->GetPosition(), rest);
    return joint;
}

b2PrismaticJoint* ModulePhysics::CreateKicker(int x, int y, int width, int height, int travel)
{
    PhysBody* plunger = CreateRectangle(x, y, width, height);

    b2PrismaticJointDef jd;
    jd.Initialize(ground, plunger->body, plunger->body->GetPosition(), b2Vec2(0.0f, 1.0f));
    jd.enableLimit = true;
    jd.lowerTranslation = 0.0f;
    jd.upperTranslation = PIXEL_TO_METERS(travel);
    jd.enableMotor = true;
    jd.maxMotorForce = 1000.0f;
    jd.motorSpeed = -25.0f;

    return (b2PrismaticJoint*)world->CreateJoint(&jd);
}

PhysBody* ModulePhysics::CreateRectangleSensor(int x, int y, int width, int height)
{
    b2BodyDef def;
    def.type = b2_staticBody;
    def.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

    PhysBody* pbody = new PhysBody();
    def.userData.pointer = reinterpret_cast<uintptr_t>(pbody);
    pbody->body = world->CreateBody(&def);

    b2PolygonShape shape;
    shape.SetAsBox(PIXEL_TO_METERS(width) * 0.5f, PIXEL_TO_METERS(height) * 0.5f);

    b2FixtureDef fixture;
    fixture.shape = &shape;
    fixture.isSensor = true;
    pbody->body->CreateFixture(&fixture);

    bodies.push_back(pbody);
    return pbody;
}

void ModulePhysics::DestroyBody(PhysBody* pbody)
{
    if (pbody == nullptr) return;

    if (mouse_joint && (mouse_joint->GetBodyA() == pbody->body || mouse_joint->GetBodyB() == pbody->body))
    {
        world->DestroyJoint(mouse_joint);
        mouse_joint = nullptr;
    }

    world->DestroyBody(pbody->body);
    bodies.erase(std::remove(bodies.begin(), bodies.end(), pbody), bodies.end());
    delete pbody;
}

void ModulePhysics::BeginContact(b2Contact* contact)
{
    PhysBody* a = (PhysBody*)contact->GetFixtureA()->GetBody()->GetUserData().pointer;
    PhysBody* b = (PhysBody*)contact->GetFixtureB()->GetBody()->GetUserData().pointer;

    if (a && a->listener) a->listener->OnCollision(a, b);
    if (b && b->listener) b->listener->OnCollision(b, a);
}

void ModulePhysics::UpdateMouseJoint()
{
    Vector2 mouse = GetMousePosition();
    b2Vec2 p(PIXEL_TO_METERS(mouse.x), PIXEL_TO_METERS(mouse.y));

    if (mouse_joint == nullptr && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        b2Body* selected = nullptr;
        for (b2Body* b = world->GetBodyList(); b && !selected; b = b->GetNext())
        {
            if (b->GetType() != b2_dynamicBody) continue;
            for (b2Fixture* f = b->GetFixtureList(); f; f = f->GetNext())
            {
                if (f->TestPoint(p)) { selected = b; break; }
            }
        }

        if (selected)
        {
            b2MouseJointDef def;
            def.bodyA = ground;
            def.bodyB = selected;
            def.target = p;
            b2LinearStiffness(def.stiffness, def.damping, 5.0f, 0.7f, def.bodyA, def.bodyB);
            def.maxForce = 100.0f * selected->GetMass();
            mouse_joint = (b2MouseJoint*)world->CreateJoint(&def);
        }
    }
    else if (mouse_joint && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        mouse_joint->SetTarget(p);
        b2Vec2 a = mouse_joint->GetAnchorB();
        DrawLine((int)METERS_TO_PIXELS(a.x), (int)METERS_TO_PIXELS(a.y), (int)mouse.x, (int)mouse.y, RED);
    }
    else if (mouse_joint && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        world->DestroyJoint(mouse_joint);
        mouse_joint = nullptr;
    }
}

update_status ModulePhysics::PostUpdate()
{
    if (IsKeyPressed(KEY_F1)) debug = !debug;

    if (!debug)
    {
        if (mouse_joint) { world->DestroyJoint(mouse_joint); mouse_joint = nullptr; }
        return UPDATE_CONTINUE;
    }

    UpdateMouseJoint();

    for (b2Body* b = world->GetBodyList(); b; b = b->GetNext())
    {
        for (b2Fixture* f = b->GetFixtureList(); f; f = f->GetNext())
        {
            switch (f->GetType())
            {
            case b2Shape::e_circle:
            {
                b2CircleShape* shape = (b2CircleShape*)f->GetShape();
                b2Vec2 pos = b->GetPosition();
                DrawCircleLines((int)METERS_TO_PIXELS(pos.x), (int)METERS_TO_PIXELS(pos.y),
                    METERS_TO_PIXELS(shape->m_radius), RED);
            }
            break;

            case b2Shape::e_polygon:
            {
                b2PolygonShape* shape = (b2PolygonShape*)f->GetShape();
                for (int i = 0; i < shape->m_count; ++i)
                {
                    b2Vec2 a = b->GetWorldPoint(shape->m_vertices[i]);
                    b2Vec2 c = b->GetWorldPoint(shape->m_vertices[(i + 1) % shape->m_count]);
                    DrawLine((int)METERS_TO_PIXELS(a.x), (int)METERS_TO_PIXELS(a.y),
                        (int)METERS_TO_PIXELS(c.x), (int)METERS_TO_PIXELS(c.y), RED);
                }
            }
            break;

            case b2Shape::e_chain:
            {
                b2ChainShape* shape = (b2ChainShape*)f->GetShape();
                for (int i = 0; i < shape->m_count - 1; ++i)
                {
                    b2Vec2 a = b->GetWorldPoint(shape->m_vertices[i]);
                    b2Vec2 c = b->GetWorldPoint(shape->m_vertices[i + 1]);
                    DrawLine((int)METERS_TO_PIXELS(a.x), (int)METERS_TO_PIXELS(a.y),
                        (int)METERS_TO_PIXELS(c.x), (int)METERS_TO_PIXELS(c.y), GREEN);
                }
            }
            break;

            case b2Shape::e_edge:
            {
                b2EdgeShape* shape = (b2EdgeShape*)f->GetShape();
                b2Vec2 a = b->GetWorldPoint(shape->m_vertex1);
                b2Vec2 c = b->GetWorldPoint(shape->m_vertex2);
                DrawLine((int)METERS_TO_PIXELS(a.x), (int)METERS_TO_PIXELS(a.y),
                    (int)METERS_TO_PIXELS(c.x), (int)METERS_TO_PIXELS(c.y), BLUE);
            }
            break;
            }
        }
    }
    return UPDATE_CONTINUE;
}

bool ModulePhysics::CleanUp()
{
    LOG("Destroying physics world");

    mouse_joint = nullptr;

    for (PhysBody* pb : bodies) delete pb;
    bodies.clear();

    delete world;
    world = nullptr;
    return true;
}