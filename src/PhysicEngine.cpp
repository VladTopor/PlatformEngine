#include "PhysicEngine.hpp"

float ticks = 30.0f;
b2Vec2 gravity(0.0,10);
b2World phys_world(gravity);
float timeStep = 1.0f / ticks;
int32 velocityIterations = 8;
int32 positionIterations = 3;

/*class ContactListener: public b2ContactListener {

    void BeginContact(b2Contact* contact) override {1
        std::cout << &contact->GetFixtureA()->GetUserData().pointer << " " << &contact->GetFixtureB()->GetUserData().pointer << std::endl;
    }

};
ContactListener listener;*/
void startPhysics() {
    //phys_world.SetContactListener(&listener);
}
void PhysicObjectDynamic::start(float x, float y, float width, float height, int id_set) {
    id = id_set;
    bodyDef.type = b2_dynamicBody;  // Set body type before creating the body
    bodyDef.position.Set(x, y);
    bodyDef.userData.pointer = reinterpret_cast<uintptr_t>(&id);
    body = phys_world.CreateBody(&bodyDef);
    dynamicBox.SetAsBox(width, height);

    fixtureDef.shape = &dynamicBox;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;

    body->CreateFixture(&fixtureDef);
}
void PhysicObjectStatic::start(float x, float y, float width, float height, int id_set) {
    id = id_set;
    bodyDef.position.Set(x, y);
    bodyDef.userData.pointer = reinterpret_cast<uintptr_t>(&id);
    body = phys_world.CreateBody(&bodyDef);

    shape.SetAsBox(width, height);
    body->CreateFixture(&shape, 0.0f);

}
void PhysicObjectStatic::remove() {
    phys_world.DestroyBody(body);
}
void PhysicObjectDynamic::remove() {
    phys_world.DestroyBody(body);
}

void simulate() {

    phys_world.Step(timeStep, velocityIterations, positionIterations);

}
void setGravity(b2Vec2 grav) {
    phys_world.SetGravity(grav);
}
void reset() {
    //std::cout << phys_world.GetBodyCount();
    b2Body *bodies = phys_world.GetBodyList();
    for (int i; i<phys_world.GetBodyCount(); i++) {
        phys_world.DestroyBody(&bodies[i]);
    }
}

bool haveContact(b2Body *body1, b2Body *body2) {
    int n = phys_world.GetContactCount();
    b2Contact* contacts = phys_world.GetContactList();
    for (int i=0; i<n; i++) {
        //std::cout << (i) << std::endl;
        b2Fixture *fixtureA = contacts[i].GetFixtureA();
        b2Fixture *fixtureB = contacts[i].GetFixtureB();
        if ((fixtureA == body1->GetFixtureList()) && (fixtureB == body2->GetFixtureList())) {
            return true;
        }
    }
    return false;
}
void setTickrate(int tickrate) {
    ticks = tickrate;
    timeStep = 1.0f / ticks;
}
b2World* getWorld() {
    return &phys_world;
}