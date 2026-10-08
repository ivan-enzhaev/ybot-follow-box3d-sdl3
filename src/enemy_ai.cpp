#include "enemy_ai.h"
#include "app.h"
#include "physics_debug.h"

#include <DetourCrowd.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshBuilder.h>
#include <DetourNavMeshQuery.h>

#include <behaviortree_cpp/behavior_tree.h>
#include <behaviortree_cpp/bt_factory.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"
#include <ozz/animation/runtime/blending_job.h>
#pragma GCC diagnostic pop

#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/base/maths/soa_transform.h>

#include <cmath>
#include <cstring>
#include <iostream>
#include <memory>
#include <vector>

// ----------------------------------------------------------------------------
// BehaviorTree.CPP Custom Nodes
// ----------------------------------------------------------------------------
class IsPlayerDetected : public BT::ConditionNode
{
public:
    IsPlayerDetected(const std::string &name, const BT::NodeConfig &config)
        : BT::ConditionNode(name, config)
    {
    }

    static BT::PortsList providedPorts()
    {
        return { BT::InputPort<float>("range") };
    }

    BT::NodeStatus tick() override
    {
        float range = 35.0f;
        getInput("range", range);
        float dist = 100.0f;
        (void)config().blackboard->get("distance_to_player", dist);
        return (dist <= range) ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
    }
};

class IsPlayerInAttackRange : public BT::ConditionNode
{
public:
    IsPlayerInAttackRange(const std::string &name, const BT::NodeConfig &config)
        : BT::ConditionNode(name, config)
    {
    }

    static BT::PortsList providedPorts()
    {
        return { BT::InputPort<float>("range") };
    }

    BT::NodeStatus tick() override
    {
        float range = 2.0f;
        getInput("range", range);
        float dist = 100.0f;
        (void)config().blackboard->get("distance_to_player", dist);
        return (dist <= range) ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
    }
};

class ChasePlayer : public BT::SyncActionNode
{
public:
    ChasePlayer(const std::string &name, const BT::NodeConfig &config)
        : BT::SyncActionNode(name, config)
    {
    }

    static BT::PortsList providedPorts() { return {}; }

    BT::NodeStatus tick() override
    {
        config().blackboard->set("ai_state", 1); // 1 = Chasing
        return BT::NodeStatus::SUCCESS;
    }
};

class MeleeAttack : public BT::SyncActionNode
{
public:
    MeleeAttack(const std::string &name, const BT::NodeConfig &config)
        : BT::SyncActionNode(name, config)
    {
    }

    static BT::PortsList providedPorts() { return {}; }

    BT::NodeStatus tick() override
    {
        config().blackboard->set("ai_state", 2); // 2 = Close / Attacking
        return BT::NodeStatus::SUCCESS;
    }
};

class Idle : public BT::SyncActionNode
{
public:
    Idle(const std::string &name, const BT::NodeConfig &config)
        : BT::SyncActionNode(name, config)
    {
    }

    static BT::PortsList providedPorts() { return {}; }

    BT::NodeStatus tick() override
    {
        config().blackboard->set("ai_state", 0); // 0 = Idle
        return BT::NodeStatus::SUCCESS;
    }
};

static const char *enemyBtXml = R"(
<root BTCPP_format="4">
  <BehaviorTree ID="FollowerTree">
    <Fallback>
      <Sequence>
        <IsPlayerInAttackRange range="2.2"/>
        <MeleeAttack/>
      </Sequence>
      <Sequence>
        <IsPlayerDetected range="40.0"/>
        <ChasePlayer/>
      </Sequence>
      <Idle/>
    </Fallback>
  </BehaviorTree>
</root>
)";

static dtNavMesh *s_navMesh = nullptr;
static dtNavMeshQuery *s_navQuery = nullptr;
static dtCrowd *s_crowd = nullptr;

static std::vector<EnemyAgent> s_enemies;
static std::vector<BT::Tree> s_enemyTrees;
static std::vector<BT::Blackboard::Ptr> s_blackboards;

// ----------------------------------------------------------------------------
// Build NavMesh aligned with ground level (y = -1.9f)
// ----------------------------------------------------------------------------
static bool buildGroundNavMesh(float halfSize = 25.0f, float groundY = -1.9f)
{
    const int N = 8;
    const float cs = 1.0f;
    const float ch = 0.2f;
    const float bmin[3] = { -halfSize, groundY - 1.0f, -halfSize };
    const float bmax[3] = { halfSize, groundY + 5.0f, halfSize };

    std::vector<unsigned short> verts;
    for (int z = 0; z <= N; ++z)
    {
        for (int x = 0; x <= N; ++x)
        {
            float vx = -halfSize + (2.0f * halfSize) * ((float)x / N);
            float vy = groundY;
            float vz = -halfSize + (2.0f * halfSize) * ((float)z / N);

            unsigned short qx = (unsigned short)std::round((vx - bmin[0]) / cs);
            unsigned short qy = (unsigned short)std::round((vy - bmin[1]) / ch);
            unsigned short qz = (unsigned short)std::round((vz - bmin[2]) / cs);

            verts.push_back(qx);
            verts.push_back(qy);
            verts.push_back(qz);
        }
    }

    std::vector<unsigned short> polys;
    std::vector<unsigned char> polyAreas;
    std::vector<unsigned short> polyFlags;

    for (int z = 0; z < N; ++z)
    {
        for (int x = 0; x < N; ++x)
        {
            unsigned short v0 = (unsigned short)(z * (N + 1) + x);
            unsigned short v1 = (unsigned short)(z * (N + 1) + x + 1);
            unsigned short v2 = (unsigned short)((z + 1) * (N + 1) + x + 1);
            unsigned short v3 = (unsigned short)((z + 1) * (N + 1) + x);

            polys.push_back(v0);
            polys.push_back(v1);
            polys.push_back(v2);
            polys.push_back(0xffff);
            polys.push_back(0xffff);
            polys.push_back(0xffff);
            polyAreas.push_back(1);
            polyFlags.push_back(1);

            polys.push_back(v0);
            polys.push_back(v2);
            polys.push_back(v3);
            polys.push_back(0xffff);
            polys.push_back(0xffff);
            polys.push_back(0xffff);
            polyAreas.push_back(1);
            polyFlags.push_back(1);
        }
    }

    dtNavMeshCreateParams params;
    memset(&params, 0, sizeof(params));
    params.verts = verts.data();
    params.vertCount = (int)(verts.size() / 3);
    params.polys = polys.data();
    params.polyAreas = polyAreas.data();
    params.polyFlags = polyFlags.data();
    params.polyCount = (int)(polyAreas.size());
    params.nvp = 3;
    params.bmin[0] = bmin[0];
    params.bmin[1] = bmin[1];
    params.bmin[2] = bmin[2];
    params.bmax[0] = bmax[0];
    params.bmax[1] = bmax[1];
    params.bmax[2] = bmax[2];
    params.walkableHeight = 2.0f;
    params.walkableRadius = 0.6f;
    params.walkableClimb = 0.5f;
    params.cs = cs;
    params.ch = ch;
    params.buildBvTree = true;

    unsigned char *navData = nullptr;
    int navDataSize = 0;
    if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
    {
        SDL_Log("Failed to create Detour NavMesh data!");
        return false;
    }

    s_navMesh = dtAllocNavMesh();
    if (!s_navMesh || dtStatusFailed(s_navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA)))
    {
        SDL_Log("Failed to initialize Detour NavMesh!");
        return false;
    }

    s_navQuery = dtAllocNavMeshQuery();
    s_navQuery->init(s_navMesh, 2048);

    s_crowd = dtAllocCrowd();
    s_crowd->init(20, 0.6f, s_navMesh);

    return true;
}

void initEnemyAI(App *app, int numEnemies)
{
    if (!buildGroundNavMesh(25.0f, -1.9f))
        return;

    BT::BehaviorTreeFactory factory;
    factory.registerNodeType<IsPlayerDetected>("IsPlayerDetected");
    factory.registerNodeType<IsPlayerInAttackRange>("IsPlayerInAttackRange");
    factory.registerNodeType<ChasePlayer>("ChasePlayer");
    factory.registerNodeType<MeleeAttack>("MeleeAttack");
    factory.registerNodeType<Idle>("Idle");

    float spawnOffsets[4][2] = {
        { -6.0f, -6.0f },
        { 6.0f, -6.0f },
        { -8.0f, 6.0f },
        { 8.0f, 6.0f }
    };

    s_enemies.clear();
    s_enemyTrees.clear();
    s_blackboards.clear();

    int numJoints = app->ozzSkeleton.num_joints();

    for (int i = 0; i < numEnemies; ++i)
    {
        float startX = spawnOffsets[i % 4][0];
        float startZ = spawnOffsets[i % 4][1];

        EnemyAgent enemy;
        enemy.position[0] = startX;
        enemy.position[1] = 0.0f;
        enemy.position[2] = startZ;

        // Capsule physics body for enemy robot (matches player height and radius)
        b3BodyDef bodyDef = b3DefaultBodyDef();
        bodyDef.type = b3_dynamicBody;
        bodyDef.position = (b3Vec3) { startX, 0.5f, startZ };

        b3MotionLocks locks = { 0 };
        locks.angularX = true;
        locks.angularY = true;
        locks.angularZ = true;
        bodyDef.motionLocks = locks;

        enemy.bodyId = b3CreateBody(app->worldId, &bodyDef);

        b3Capsule capsule = {
            .center1 = { 0.0f, -0.7f, 0.0f },
            .center2 = { 0.0f, 0.7f, 0.0f },
            .radius = 0.5f
        };
        b3ShapeDef capsuleShapeDef = b3DefaultShapeDef();
        capsuleShapeDef.density = 1.0f;
        capsuleShapeDef.baseMaterial.friction = 0.5f;
        capsuleShapeDef.baseMaterial.restitution = 0.0f;
        b3CreateCapsuleShape(enemy.bodyId, &capsuleShapeDef, &capsule);

        // Crowd steering agent
        dtCrowdAgentParams ap;
        memset(&ap, 0, sizeof(ap));
        ap.radius = 0.6f;
        ap.height = 1.8f;
        ap.maxAcceleration = 6.0f;
        ap.maxSpeed = 2.4f + (i * 0.2f);
        ap.collisionQueryRange = ap.radius * 6.0f;
        ap.pathOptimizationRange = ap.radius * 12.0f;
        ap.updateFlags = DT_CROWD_ANTICIPATE_TURNS | DT_CROWD_OPTIMIZE_VIS | DT_CROWD_OPTIMIZE_TOPO | DT_CROWD_OBSTACLE_AVOIDANCE | DT_CROWD_SEPARATION;
        ap.obstacleAvoidanceType = 0;
        ap.separationWeight = 2.5f;

        float startPos[3] = { startX, -1.9f, startZ };
        enemy.crowdAgentId = s_crowd->addAgent(startPos, &ap);

        // Allocate sampling contexts for all 4 animation layers (Idle, Walk, Left, Right)
        enemy.idleContext = std::make_unique<ozz::animation::SamplingJob::Context>(numJoints);
        enemy.walkContext = std::make_unique<ozz::animation::SamplingJob::Context>(numJoints);
        enemy.leftContext = std::make_unique<ozz::animation::SamplingJob::Context>(numJoints);
        enemy.rightContext = std::make_unique<ozz::animation::SamplingJob::Context>(numJoints);
        enemy.modelMatrices.resize(numJoints);

        auto blackboard = BT::Blackboard::create();
        blackboard->set("distance_to_player", 100.0f);
        blackboard->set("ai_state", 0);

        BT::Tree tree = factory.createTreeFromText(enemyBtXml, blackboard);

        s_enemies.push_back(std::move(enemy));
        s_enemyTrees.push_back(std::move(tree));
        s_blackboards.push_back(blackboard);
    }

    std::cout << "Recast NavMesh and BehaviorTree initialized with "
              << numEnemies
              << " follower robots."
              << std::endl;
}

void updateEnemyAI(App *app, float deltaTime)
{
    if (!s_crowd || !s_navQuery)
        return;

    b3Vec3 playerCapPos = b3Body_GetPosition(app->capsuleId);
    vec3 playerPos = { playerCapPos.x, playerCapPos.y, playerCapPos.z };

    dtQueryFilter filter;
    filter.setIncludeFlags(0xffff);
    filter.setExcludeFlags(0);

    float extents[3] = { 4.0f, 4.0f, 4.0f };
    dtPolyRef playerPolyRef = 0;
    float nearestTargetPos[3] = { 0 };
    float targetPos[3] = { playerPos[0], -1.9f, playerPos[2] };

    s_navQuery->findNearestPoly(targetPos, extents, &filter, &playerPolyRef, nearestTargetPos);

    for (size_t i = 0; i < s_enemies.size(); ++i)
    {
        auto &enemy = s_enemies[i];
        float dx = playerPos[0] - enemy.position[0];
        float dz = playerPos[2] - enemy.position[2];
        float dist = sqrtf(dx * dx + dz * dz);

        s_blackboards[i]->set("distance_to_player", dist);
        s_enemyTrees[i].tickOnce();

        int state = 0;
        (void)s_blackboards[i]->get("ai_state", state);
        enemy.aiState = state;

        if (state == 1 && playerPolyRef)
        {
            s_crowd->requestMoveTarget(enemy.crowdAgentId, playerPolyRef, nearestTargetPos);
        }
        else if (state == 2)
        {
            s_crowd->resetMoveTarget(enemy.crowdAgentId);
        }
    }

    s_crowd->update(deltaTime, nullptr);

    float idleDur = app->ozzIdleAnimation.duration();
    float walkDur = app->ozzWalkAnimation.duration();
    float leftDur = app->ozzLeftAnimation.duration();
    float rightDur = app->ozzRightAnimation.duration();

    const float NATURAL_TURN_RATE = 2.5f;

    for (auto &enemy : s_enemies)
    {
        const dtCrowdAgent *ag = s_crowd->getAgent(enemy.crowdAgentId);
        if (ag && ag->active)
        {
            b3Vec3 curBodyPos = b3Body_GetPosition(enemy.bodyId);
            b3Vec3 curBodyVel = b3Body_GetLinearVelocity(enemy.bodyId);

            float dtSafe = (deltaTime > 0.001f) ? deltaTime : 0.016f;
            float vx = (ag->npos[0] - curBodyPos.x) / dtSafe;
            float vz = (ag->npos[2] - curBodyPos.z) / dtSafe;

            float horizontalSpeed = sqrtf(vx * vx + vz * vz);
            if (horizontalSpeed > 5.0f)
            {
                vx = (vx / horizontalSpeed) * 5.0f;
                vz = (vz / horizontalSpeed) * 5.0f;
                horizontalSpeed = 5.0f;
            }

            b3Body_SetLinearVelocity(enemy.bodyId, (b3Vec3) { vx, curBodyVel.y, vz });

            // -------------------------------------------------------------
            // Desired Facing Angle & Steering Angular Speed
            // -------------------------------------------------------------
            float targetAngle = enemy.yaw;
            if (horizontalSpeed > 0.2f)
            {
                targetAngle = atan2f(vx, vz);
            }
            else
            {
                float toPx = playerPos[0] - curBodyPos.x;
                float toPz = playerPos[2] - curBodyPos.z;
                if (sqrtf(toPx * toPx + toPz * toPz) > 0.1f)
                {
                    targetAngle = atan2f(toPx, toPz);
                }
            }

            // Calculate shortest angular difference (-PI to +PI)
            float angleDiff = targetAngle - enemy.yaw;
            while (angleDiff > GLM_PIf)
                angleDiff -= 2.0f * GLM_PIf;
            while (angleDiff < -GLM_PIf)
                angleDiff += 2.0f * GLM_PIf;

            // Turn smoothly towards target angle with max turn rate
            float maxTurnRate = 3.5f; // rad/s
            float turnAmount = angleDiff;
            if (turnAmount > maxTurnRate * deltaTime)
                turnAmount = maxTurnRate * deltaTime;
            if (turnAmount < -maxTurnRate * deltaTime)
                turnAmount = -maxTurnRate * deltaTime;

            enemy.yaw += turnAmount;

            // Real-time angular turn rate in rad/sec
            float turnRate = turnAmount / dtSafe;

            float halfAngle = enemy.yaw * 0.5f;
            b3Quat targetRot = {
                .v = { 0.0f, sinf(halfAngle), 0.0f },
                .s = cosf(halfAngle)
            };
            b3Body_SetTransform(enemy.bodyId, curBodyPos, targetRot);

            b3Vec3 finalPos = b3Body_GetPosition(enemy.bodyId);
            enemy.position[0] = finalPos.x;
            enemy.position[1] = finalPos.y;
            enemy.position[2] = finalPos.z;

            // -------------------------------------------------------------
            // 4-Layer Animation Blend Weights (Idle, Walk, Left, Right)
            // -------------------------------------------------------------
            float targetWalk = (horizontalSpeed > 0.3f) ? 1.0f : 0.0f;
            float targetLeft = (turnRate > 0.4f) ? fminf(turnRate / 2.5f, 1.0f) : 0.0f;
            float targetRight = (turnRate < -0.4f) ? fminf(-turnRate / 2.5f, 1.0f) : 0.0f;

            float totalActive = targetWalk + targetLeft + targetRight;
            if (totalActive > 1.0f)
            {
                targetWalk /= totalActive;
                targetLeft /= totalActive;
                targetRight /= totalActive;
            }
            float targetIdle = (totalActive > 0.0f) ? 0.0f : 1.0f;

            float blendSpeed = 8.0f;
            enemy.walkWeight += (targetWalk - enemy.walkWeight) * blendSpeed * deltaTime;
            enemy.leftWeight += (targetLeft - enemy.leftWeight) * blendSpeed * deltaTime;
            enemy.rightWeight += (targetRight - enemy.rightWeight) * blendSpeed * deltaTime;
            enemy.idleWeight += (targetIdle - enemy.idleWeight) * blendSpeed * deltaTime;

            enemy.walkWeight = fmaxf(0.0f, fminf(1.0f, enemy.walkWeight));
            enemy.leftWeight = fmaxf(0.0f, fminf(1.0f, enemy.leftWeight));
            enemy.rightWeight = fmaxf(0.0f, fminf(1.0f, enemy.rightWeight));
            enemy.idleWeight = fmaxf(0.0f, fminf(1.0f, enemy.idleWeight));

            float sumWeights = enemy.walkWeight + enemy.leftWeight + enemy.rightWeight + enemy.idleWeight;
            if (sumWeights > 0.0001f)
            {
                enemy.walkWeight /= sumWeights;
                enemy.leftWeight /= sumWeights;
                enemy.rightWeight /= sumWeights;
                enemy.idleWeight /= sumWeights;
            }

            // Advance all 4 animation timers with speed matching
            if (idleDur > 0.0f)
            {
                enemy.idleTime = fmodf(enemy.idleTime + deltaTime, idleDur);
                if (enemy.idleTime < 0.0f)
                    enemy.idleTime += idleDur;
            }
            if (walkDur > 0.0f && horizontalSpeed > 0.1f)
            {
                float playSpeed = horizontalSpeed / 2.0f;
                enemy.walkTime = fmodf(enemy.walkTime + deltaTime * playSpeed, walkDur);
                if (enemy.walkTime < 0.0f)
                    enemy.walkTime += walkDur;
            }

            float turnPlaySpeed = fmaxf(0.1f, fabsf(turnRate) / NATURAL_TURN_RATE);
            if (leftDur > 0.0f && (turnRate > 0.3f || enemy.leftWeight > 0.01f))
            {
                enemy.leftTime = fmodf(enemy.leftTime + deltaTime * turnPlaySpeed, leftDur);
                if (enemy.leftTime < 0.0f)
                    enemy.leftTime += leftDur;
            }
            if (rightDur > 0.0f && (turnRate < -0.3f || enemy.rightWeight > 0.01f))
            {
                enemy.rightTime = fmodf(enemy.rightTime + deltaTime * turnPlaySpeed, rightDur);
                if (enemy.rightTime < 0.0f)
                    enemy.rightTime += rightDur;
            }

            // Sample 1: Idle
            ozz::vector<ozz::math::SoaTransform> idleLocals(app->ozzSkeleton.num_soa_joints());
            ozz::animation::SamplingJob idleJob;
            idleJob.animation = &app->ozzIdleAnimation;
            idleJob.ratio = (idleDur > 0.0f) ? (enemy.idleTime / idleDur) : 0.0f;
            idleJob.context = enemy.idleContext.get();
            idleJob.output = ozz::make_span(idleLocals);
            idleJob.Run();

            // Sample 2: Walk
            ozz::vector<ozz::math::SoaTransform> walkLocals(app->ozzSkeleton.num_soa_joints());
            ozz::animation::SamplingJob walkJob;
            walkJob.animation = &app->ozzWalkAnimation;
            walkJob.ratio = (walkDur > 0.0f) ? (enemy.walkTime / walkDur) : 0.0f;
            walkJob.context = enemy.walkContext.get();
            walkJob.output = ozz::make_span(walkLocals);
            walkJob.Run();

            // Sample 3: Turn Left (REVERSED)
            ozz::vector<ozz::math::SoaTransform> leftLocals(app->ozzSkeleton.num_soa_joints());
            ozz::animation::SamplingJob leftJob;
            leftJob.animation = &app->ozzLeftAnimation;
            leftJob.ratio = (leftDur > 0.0f) ? (1.0f - (enemy.leftTime / leftDur)) : 0.0f;
            leftJob.context = enemy.leftContext.get();
            leftJob.output = ozz::make_span(leftLocals);
            leftJob.Run();

            // Sample 4: Turn Right (NORMAL)
            ozz::vector<ozz::math::SoaTransform> rightLocals(app->ozzSkeleton.num_soa_joints());
            ozz::animation::SamplingJob rightJob;
            rightJob.animation = &app->ozzRightAnimation;
            rightJob.ratio = (rightDur > 0.0f) ? (enemy.rightTime / rightDur) : 0.0f;
            rightJob.context = enemy.rightContext.get();
            rightJob.output = ozz::make_span(rightLocals);
            rightJob.Run();

            // -------------------------------------------------------------
            // 4-Layer Blending Job (Idle, Walk, Left, Right)
            // -------------------------------------------------------------
            ozz::animation::BlendingJob::Layer layers[4];
            layers[0].transform = ozz::make_span(idleLocals);
            layers[0].weight = enemy.idleWeight;

            layers[1].transform = ozz::make_span(walkLocals);
            layers[1].weight = enemy.walkWeight;

            layers[2].transform = ozz::make_span(leftLocals);
            layers[2].weight = enemy.leftWeight;

            layers[3].transform = ozz::make_span(rightLocals);
            layers[3].weight = enemy.rightWeight;

            ozz::vector<ozz::math::SoaTransform> blendedLocals(app->ozzSkeleton.num_soa_joints());
            ozz::animation::BlendingJob blendJob;
            blendJob.rest_pose = app->ozzSkeleton.joint_rest_poses();
            blendJob.layers = ozz::make_span(layers);
            blendJob.output = ozz::make_span(blendedLocals);
            blendJob.threshold = 0.05f;
            blendJob.Run();

            // Convert to Model Matrices
            ozz::animation::LocalToModelJob convJob;
            convJob.skeleton = &app->ozzSkeleton;
            convJob.input = ozz::make_span(blendedLocals);
            convJob.output = ozz::make_span(enemy.modelMatrices);
            convJob.Run();
        }
    }
}

// ----------------------------------------------------------------------------
// Render all Follower Robots using Skinning Shader & Y-Bot Mesh
// ----------------------------------------------------------------------------
void renderEnemyRobots(App *app)
{
    if (s_enemies.empty() || !app->showRobot)
        return;

    glUseProgram(app->skinningShaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, app->ybotTextureID);
    glBindVertexArray(app->ybotVao);

    float capsuleHalfHeight = 1.0f;
    vec3 yBotTranslation = { 0.0f, -capsuleHalfHeight, 0.0f };
    float capsuleScaleFactor = 1.33f;
    vec3 yBotScale = { capsuleScaleFactor, capsuleScaleFactor, capsuleScaleFactor };

    // Set Lighting parameters for followers
    glUniform3f(app->uSkinningViewPosLocation, 0.0f, 1.0f, 8.0f);
    glUniform3f(app->uSkinningLightDirLocation, 0.5f, 1.0f, 0.4f);

    const int activeJointIndices[] = {
        2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
        22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
        40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57,
        58, 59, 60, 61, 62, 63, 64, 65, 66
    };
    const int numActiveJoints = 65;
    std::vector<float> jointMatricesData(numActiveJoints * 16);

    for (const auto &enemy : s_enemies)
    {
        b3Vec3 capPos = b3Body_GetPosition(enemy.bodyId);
        b3Quat capRot = b3Body_GetRotation(enemy.bodyId);

        mat4 model;
        glm_mat4_identity(model);

        vec3 meshPos = { capPos.x, capPos.y, capPos.z };
        glm_translate(model, meshPos);

        versor rot = { capRot.v.x, capRot.v.y, capRot.v.z, capRot.s };
        mat4 rotMat;
        glm_quat_mat4(rot, rotMat);
        glm_mat4_mul(model, rotMat, model);

        glm_translate(model, yBotTranslation);
        glm_scale(model, yBotScale);

        mat4 mvp;
        glm_mat4_mul(app->projView3D, model, mvp);

        // Upload MVP and Model matrix for normal calculations
        glUniformMatrix4fv(app->uSkinningMvpMatrixLocation, 1, GL_FALSE, (const GLfloat *)mvp);
        glUniformMatrix4fv(app->uSkinningModelMatrixLocation, 1, GL_FALSE, (const GLfloat *)model);

        for (int idx = 0; idx < numActiveJoints; ++idx)
        {
            int jIdx = activeJointIndices[idx];
            const ozz::math::Float4x4 &m = enemy.modelMatrices[jIdx];

            mat4 ozzMat;
            memcpy(&ozzMat[0][0], &m.cols[0], 16 * sizeof(float));

            mat4 finalJointMat;
            if (idx < (int)app->ybotInverseBindMatrices.size())
            {
                glm_mat4_mul(ozzMat, app->ybotInverseBindMatrices[idx].m, finalJointMat);
            }
            else
            {
                glm_mat4_copy(ozzMat, finalJointMat);
            }
            memcpy(&jointMatricesData[idx * 16], &finalJointMat[0][0], 16 * sizeof(float));
        }

        glUniformMatrix4fv(app->uJointMatricesLocation, numActiveJoints, GL_FALSE, jointMatricesData.data());
        glDrawElements(GL_TRIANGLES, app->ybotIndexCount, app->ybotIndexType, 0);
    }

    glBindVertexArray(0);
}

// ----------------------------------------------------------------------------
// Render NavMesh & Enemy AI Debug Visuals
// ----------------------------------------------------------------------------
void renderEnemyAIDebug(App *app)
{
    if (!app || !s_navMesh)
        return;

    b3Vec3 playerCapPos = b3Body_GetPosition(app->capsuleId);

    const dtNavMesh *navMesh = s_navMesh;
    std::vector<float> navLines;
    for (int i = 0; i < navMesh->getMaxTiles(); ++i)
    {
        const dtMeshTile *tile = navMesh->getTile(i);
        if (!tile || !tile->header)
            continue;

        for (int p = 0; p < tile->header->polyCount; ++p)
        {
            const dtPoly *poly = &tile->polys[p];
            for (int j = 0, k = (int)poly->vertCount - 1; j < (int)poly->vertCount; k = j++)
            {
                const float *v0 = &tile->verts[poly->verts[k] * 3];
                const float *v1 = &tile->verts[poly->verts[j] * 3];

                navLines.push_back(v0[0]);
                navLines.push_back(v0[1] + 0.05f);
                navLines.push_back(v0[2]);
                navLines.push_back(v1[0]);
                navLines.push_back(v1[1] + 0.05f);
                navLines.push_back(v1[2]);
            }
        }
    }

    glUseProgram(app->lineShaderProgram);
    glUniformMatrix4fv(app->uLineMvpMatrixLocation, 1, GL_FALSE, (const GLfloat *)app->projView3D);
    glUniform4f(app->uLineColorLocation, 0.2f, 0.8f, 1.0f, 0.5f);

    glBindVertexArray(app->lineVao);
    glBindBuffer(GL_ARRAY_BUFFER, app->lineVbo);
    glBufferData(GL_ARRAY_BUFFER, navLines.size() * sizeof(float), navLines.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINES, 0, (GLsizei)(navLines.size() / 3));

    std::vector<float> chaseLines;
    for (const auto &enemy : s_enemies)
    {
        if (enemy.aiState == 1)
        { // Chasing
            chaseLines.push_back(enemy.position[0]);
            chaseLines.push_back(enemy.position[1]);
            chaseLines.push_back(enemy.position[2]);

            chaseLines.push_back(playerCapPos.x);
            chaseLines.push_back(playerCapPos.y);
            chaseLines.push_back(playerCapPos.z);
        }
    }

    if (!chaseLines.empty())
    {
        glUniform4f(app->uLineColorLocation, 1.0f, 0.85f, 0.1f, 0.9f);
        glBufferData(GL_ARRAY_BUFFER, chaseLines.size() * sizeof(float), chaseLines.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINES, 0, (GLsizei)(chaseLines.size() / 3));
    }

    glBindVertexArray(0);
}

void cleanupEnemyAI(App *app)
{
    if (s_crowd)
    {
        dtFreeCrowd(s_crowd);
        s_crowd = nullptr;
    }
    if (s_navQuery)
    {
        dtFreeNavMeshQuery(s_navQuery);
        s_navQuery = nullptr;
    }
    if (s_navMesh)
    {
        dtFreeNavMesh(s_navMesh);
        s_navMesh = nullptr;
    }
    s_enemies.clear();
    s_enemyTrees.clear();
    s_blackboards.clear();
}
