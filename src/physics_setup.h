#pragma once

#include "app.h"
#include "physics_debug.h"

inline bool initPhysicsWorld(App *app)
{
    app->dd = b3DefaultDebugDraw();
    app->dd.DrawShapeFcn = drawShape;
    app->dd.drawShapes = true;
    app->dd.context = app;
    app->dd.drawingBounds = (b3AABB) { (b3Vec3) { -100.f, -100.f, -100.f }, (b3Vec3) { 100.f, 100.f, 100.f } };

    b3WorldDef worldDef = b3DefaultWorldDef();
    worldDef.gravity = (b3Vec3) { 0.0f, -9.8f, 0.0f };
    worldDef.createDebugShape = createDebugShape;
    worldDef.destroyDebugShape = destroyDebugShape;
    app->worldId = b3CreateWorld(&worldDef);

    // Ground Box (50x50m)
    b3BodyDef groundDef = b3DefaultBodyDef();
    groundDef.position = (b3Vec3) { 0.f, -2.1f, 0.f };
    b3BodyId groundId = b3CreateBody(app->worldId, &groundDef);
    b3BoxHull groundBox = b3MakeBoxHull(25.f, 0.2f, 25.f);
    b3ShapeDef groundShapeDef = b3DefaultShapeDef();
    b3CreateHullShape(groundId, &groundShapeDef, &groundBox.base);

    // Player Capsule
    b3BodyDef capsuleBodyDef = b3DefaultBodyDef();
    capsuleBodyDef.type = b3_dynamicBody;
    capsuleBodyDef.position = (b3Vec3) { 0.0f, 1.0f, 0.0f };

    b3MotionLocks locks = { 0 };
    locks.angularX = true;
    locks.angularY = true;
    locks.angularZ = true;
    capsuleBodyDef.motionLocks = locks;

    app->capsuleId = b3CreateBody(app->worldId, &capsuleBodyDef);
    b3Capsule capsule = {
        .center1 = { 0.0f, -0.7f, 0.0f },
        .center2 = { 0.0f, 0.7f, 0.0f },
        .radius = 0.5f
    };
    b3ShapeDef capsuleShapeDef = b3DefaultShapeDef();
    capsuleShapeDef.density = 1.0f;
    capsuleShapeDef.baseMaterial.friction = 0.5f;
    capsuleShapeDef.baseMaterial.restitution = 0.0f;
    b3CreateCapsuleShape(app->capsuleId, &capsuleShapeDef, &capsule);

    return true;
}
