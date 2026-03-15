#include "PhysicsWorld.h"

// =============================================================================
// ContactListener
// =============================================================================

void ContactListener::BeginContact(b2Contact* contact) {
    handleContact(contact, true);
}

void ContactListener::EndContact(b2Contact* contact) {
    handleContact(contact, false);
}

void ContactListener::handleContact(b2Contact* contact, bool begin) {
    b2Fixture* fixtureA = contact->GetFixtureA();   // Lab: arrow operator
    b2Fixture* fixtureB = contact->GetFixtureB();   // Lab: arrow operator

    // Check both fixtures — one might be a sensor
    b2Fixture* fixtures[2] = { fixtureA, fixtureB };

    for (b2Fixture* fixture : fixtures) {
        uintptr_t userData = fixture->GetUserData().pointer;

        if (userData == Constants::FIXTURE_FOOT_SENSOR) {
            // Foot sensor — ground detection
            b2Body* ownerBody = fixture->GetBody();                        // Lab: arrow operator
            Entity* entity = reinterpret_cast<Entity*>(
                ownerBody->GetUserData().pointer);                         // Lab: pointer cast

            if (entity) {
                if (begin) entity->beginGroundContact();                   // Lab: arrow operator
                else       entity->endGroundContact();                     // Lab: arrow operator
            }
        }
        else if (userData == Constants::FIXTURE_LEFT_WALL_SENSOR) {
            // Left wall sensor
            b2Body* ownerBody = fixture->GetBody();
            Entity* entity = reinterpret_cast<Entity*>(
                ownerBody->GetUserData().pointer);

            if (entity) {
                if (begin) entity->beginWallContact(true);
                else       entity->endWallContact(true);
            }
        }
        else if (userData == Constants::FIXTURE_RIGHT_WALL_SENSOR) {
            // Right wall sensor
            b2Body* ownerBody = fixture->GetBody();
            Entity* entity = reinterpret_cast<Entity*>(
                ownerBody->GetUserData().pointer);

            if (entity) {
                if (begin) entity->beginWallContact(false);
                else       entity->endWallContact(false);
            }
        }
    }
}

// =============================================================================
// PhysicsWorld
// =============================================================================

PhysicsWorld::PhysicsWorld(float windowWidth, float windowHeight)
    : windowWidth(windowWidth)
    , windowHeight(windowHeight)
    , worldWidth(windowWidth / Constants::PPM)
    , worldHeight(windowHeight / Constants::PPM)
{
    // Gravity points down: negative Y in Box2D's Y-up system
    b2Vec2 gravity(0.0f, -Constants::GRAVITY);
    world = new b2World(gravity);                    // Lab: single-level pointer via new

    // Register contact listener so foot sensors trigger ground detection
    world->SetContactListener(&contactListener);     // Lab: arrow operator
}

PhysicsWorld::~PhysicsWorld() {
    delete world;       // Lab: pointer cleanup
    world = nullptr;
}

void PhysicsWorld::step() {
    world->Step(Constants::TIME_STEP,                // Lab: arrow operator
                Constants::VELOCITY_ITERATIONS,
                Constants::POSITION_ITERATIONS);
}

b2Body* PhysicsWorld::createDynamicBody(float centerX, float centerY,
                                        float halfWidth, float halfHeight,
                                        float density, float friction,
                                        bool fixedRotation) {
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(centerX, centerY);
    bodyDef.fixedRotation = fixedRotation;           // Prevent spinning on collision

    b2Body* body = world->CreateBody(&bodyDef);      // Lab: arrow operator

    b2PolygonShape shape;
    shape.SetAsBox(halfWidth, halfHeight);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;
    fixtureDef.density = density;
    fixtureDef.friction = friction;
    fixtureDef.userData.pointer = Constants::FIXTURE_BODY;

    body->CreateFixture(&fixtureDef);                // Lab: arrow operator

    return body;
}

b2Body* PhysicsWorld::createStaticBody(float centerX, float centerY,
                                       float halfWidth, float halfHeight) {
    b2BodyDef bodyDef;
    bodyDef.type = b2_staticBody;
    bodyDef.position.Set(centerX, centerY);

    b2Body* body = world->CreateBody(&bodyDef);      // Lab: arrow operator

    b2PolygonShape shape;
    shape.SetAsBox(halfWidth, halfHeight);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;
    fixtureDef.friction = 0.5f;
    fixtureDef.userData.pointer = Constants::FIXTURE_BODY;

    body->CreateFixture(&fixtureDef);                // Lab: arrow operator

    return body;
}

void PhysicsWorld::addSensorFixture(b2Body* body, float halfWidth, float halfHeight,
                                    const b2Vec2& offset, uintptr_t userData) {
    b2PolygonShape sensorShape;
    sensorShape.SetAsBox(halfWidth, halfHeight, offset, 0.0f);

    b2FixtureDef sensorDef;
    sensorDef.shape = &sensorShape;
    sensorDef.isSensor = true;                       // Detects overlap, no collision response
    sensorDef.userData.pointer = userData;

    body->CreateFixture(&sensorDef);                 // Lab: arrow operator
}

// =============================================================================
// Coordinate Conversion
// =============================================================================

sf::Vector2f PhysicsWorld::toScreen(const b2Vec2& worldPos) const {
    // Box2D Y-up → SFML Y-down: flip Y axis
    return sf::Vector2f(
        worldPos.x * Constants::PPM,
        windowHeight - worldPos.y * Constants::PPM
    );
}

b2Vec2 PhysicsWorld::toWorld(const sf::Vector2f& screenPos) const {
    return b2Vec2(
        screenPos.x / Constants::PPM,
        (windowHeight - screenPos.y) / Constants::PPM
    );
}

float PhysicsWorld::toPixels(float meters) const {
    return meters * Constants::PPM;
}

float PhysicsWorld::toMeters(float pixels) const {
    return pixels / Constants::PPM;
}

// =============================================================================
// Accessors
// =============================================================================

b2World* PhysicsWorld::getWorld() {
    return world;
}

float PhysicsWorld::getWorldWidth() const {
    return worldWidth;
}

float PhysicsWorld::getWorldHeight() const {
    return worldHeight;
}
