#pragma once

#include <box3d/box3d.h>
#include <cglm/cglm.h>
#include <memory>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/simd_math.h>
#include <vector>

struct App;

struct EnemyAgent
{
    b3BodyId bodyId;
    int crowdAgentId = -1;
    vec3 position = { 0.0f, 0.0f, 0.0f };
    float yaw = 0.0f;
    int aiState = 0; // 0 = Idle, 1 = Chasing, 2 = Attacking

    // Per-enemy Ozz animation sampling contexts (Idle, Walk, Left, Right)
    std::unique_ptr<ozz::animation::SamplingJob::Context> idleContext;
    std::unique_ptr<ozz::animation::SamplingJob::Context> walkContext;
    std::unique_ptr<ozz::animation::SamplingJob::Context> leftContext;
    std::unique_ptr<ozz::animation::SamplingJob::Context> rightContext;

    // Animation playback timers
    float idleTime = 0.0f;
    float walkTime = 0.0f;
    float leftTime = 0.0f;
    float rightTime = 0.0f;

    // 4-layer blend weights
    float idleWeight = 1.0f;
    float walkWeight = 0.0f;
    float leftWeight = 0.0f;
    float rightWeight = 0.0f;

    ozz::vector<ozz::math::Float4x4> modelMatrices;

    EnemyAgent() = default;
    EnemyAgent(EnemyAgent &&) noexcept = default;
    EnemyAgent &operator=(EnemyAgent &&) noexcept = default;
    EnemyAgent(const EnemyAgent &) = delete;
    EnemyAgent &operator=(const EnemyAgent &) = delete;
};

void initEnemyAI(App *app, int numEnemies = 3);
void updateEnemyAI(App *app, float deltaTime);
void renderEnemyRobots(App *app);
void renderEnemyAIDebug(App *app);
void cleanupEnemyAI(App *app);
