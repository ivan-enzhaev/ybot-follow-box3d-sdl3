#pragma once

#include "app.h"
#include "file_utils.h"
#include "ozz_sdl_stream.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"
#include <ozz/animation/runtime/blending_job.h>
#pragma GCC diagnostic pop

#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/base/io/archive.h>
#include <ozz/base/maths/soa_transform.h>

inline bool loadPlayerAnimations(App *app)
{
    SDL_IOStream *skeletonIo = SDL_IOFromFile(getAssetPath("assets/models/y-bot/skeleton.ozz"), "rb");
    if (skeletonIo)
    {
        OzzSDLStream stream(skeletonIo);
        ozz::io::IArchive archive(&stream);
        archive >> app->ozzSkeleton;
    }

    auto loadAnim = [](const char *path, ozz::animation::Animation &anim) {
        SDL_IOStream *io = SDL_IOFromFile(getAssetPath(path), "rb");
        if (io)
        {
            OzzSDLStream stream(io);
            ozz::io::IArchive arc(&stream);
            arc >> anim;
        }
        else
        {
            SDL_Log("Failed to open ozz animation: %s", path);
        }
    };

    loadAnim("assets/models/y-bot/animation-idle-crouching.ozz", app->ozzIdleAnimation);
    loadAnim("assets/models/y-bot/animation-walk-crouching.ozz", app->ozzWalkAnimation);
    loadAnim("assets/models/y-bot/animation-turn-rifle.ozz", app->ozzLeftAnimation);
    loadAnim("assets/models/y-bot/animation-turn-rifle.ozz", app->ozzRightAnimation);

    int numJoints = app->ozzSkeleton.num_joints();
    app->ozzIdleSamplingContext.Resize(numJoints);
    app->ozzWalkSamplingContext.Resize(numJoints);
    app->ozzLeftSamplingContext.Resize(numJoints);
    app->ozzRightSamplingContext.Resize(numJoints);

    app->ozzIdleAnimationTime = 0.0f;
    app->ozzWalkAnimationTime = 0.0f;
    app->ozzLeftAnimationTime = 0.0f;
    app->ozzRightAnimationTime = 0.0f;
    app->ozzModelMatrices.resize(numJoints);
    return true;
}

inline void updatePlayerAnimation(App *app, float deltaTime, bool isWalking, bool isTurningLeft, bool isTurningRight, float turnSpeed, float moveSpeed)
{
    // 4-Layer Animation Blend Weights
    float targetWalk = isWalking ? 1.0f : 0.0f;
    float targetLeft = isTurningLeft ? 1.0f : 0.0f;
    float targetRight = isTurningRight ? 1.0f : 0.0f;

    float totalActive = targetWalk + targetLeft + targetRight;
    if (totalActive > 1.0f)
    {
        targetWalk /= totalActive;
        targetLeft /= totalActive;
        targetRight /= totalActive;
    }
    float targetIdle = (totalActive > 0.0f) ? 0.0f : 1.0f;

    static float currentWalkWeight = 0.0f;
    static float currentLeftWeight = 0.0f;
    static float currentRightWeight = 0.0f;
    static float currentIdleWeight = 1.0f;

    float blendSpeed = 8.0f;
    currentWalkWeight += (targetWalk - currentWalkWeight) * blendSpeed * deltaTime;
    currentLeftWeight += (targetLeft - currentLeftWeight) * blendSpeed * deltaTime;
    currentRightWeight += (targetRight - currentRightWeight) * blendSpeed * deltaTime;
    currentIdleWeight += (targetIdle - currentIdleWeight) * blendSpeed * deltaTime;

    currentWalkWeight = fmaxf(0.0f, fminf(1.0f, currentWalkWeight));
    currentLeftWeight = fmaxf(0.0f, fminf(1.0f, currentLeftWeight));
    currentRightWeight = fmaxf(0.0f, fminf(1.0f, currentRightWeight));
    currentIdleWeight = fmaxf(0.0f, fminf(1.0f, currentIdleWeight));

    float sumWeights = currentWalkWeight + currentLeftWeight + currentRightWeight + currentIdleWeight;
    if (sumWeights > 0.0001f)
    {
        currentWalkWeight /= sumWeights;
        currentLeftWeight /= sumWeights;
        currentRightWeight /= sumWeights;
        currentIdleWeight /= sumWeights;
    }

    const float NATURAL_TURN_RATE = 2.5f;

    float idleDur = app->ozzIdleAnimation.duration();
    if (idleDur > 0.0f)
    {
        app->ozzIdleAnimationTime = fmodf(app->ozzIdleAnimationTime + deltaTime, idleDur);
        if (app->ozzIdleAnimationTime < 0.0f) app->ozzIdleAnimationTime += idleDur;
    }

    float walkDur = app->ozzWalkAnimation.duration();
    if (walkDur > 0.0f && isWalking)
    {
        float walkSign = (moveSpeed < -0.01f) ? -1.0f : 1.0f;
        app->ozzWalkAnimationTime = fmodf(app->ozzWalkAnimationTime + (deltaTime * walkSign), walkDur);
        if (app->ozzWalkAnimationTime < 0.0f) app->ozzWalkAnimationTime += walkDur;
    }

    float leftDur = app->ozzLeftAnimation.duration();
    if (leftDur > 0.0f && (isTurningLeft || currentLeftWeight > 0.01f))
    {
        float turnPlaySpeed = isTurningLeft ? (turnSpeed / NATURAL_TURN_RATE) : 1.0f;
        app->ozzLeftAnimationTime = fmodf(app->ozzLeftAnimationTime + deltaTime * turnPlaySpeed, leftDur);
        if (app->ozzLeftAnimationTime < 0.0f) app->ozzLeftAnimationTime += leftDur;
    }

    float rightDur = app->ozzRightAnimation.duration();
    if (rightDur > 0.0f && (isTurningRight || currentRightWeight > 0.01f))
    {
        float turnPlaySpeed = isTurningRight ? (turnSpeed / NATURAL_TURN_RATE) : 1.0f;
        app->ozzRightAnimationTime = fmodf(app->ozzRightAnimationTime + deltaTime * turnPlaySpeed, rightDur);
        if (app->ozzRightAnimationTime < 0.0f) app->ozzRightAnimationTime += rightDur;
    }

    // Sampling
    ozz::vector<ozz::math::SoaTransform> idleLocals(app->ozzSkeleton.num_soa_joints());
    ozz::animation::SamplingJob idleJob;
    idleJob.animation = &app->ozzIdleAnimation;
    idleJob.ratio = (idleDur > 0.0f) ? (app->ozzIdleAnimationTime / idleDur) : 0.0f;
    idleJob.context = &app->ozzIdleSamplingContext;
    idleJob.output = ozz::make_span(idleLocals);
    idleJob.Run();

    ozz::vector<ozz::math::SoaTransform> walkLocals(app->ozzSkeleton.num_soa_joints());
    ozz::animation::SamplingJob walkJob;
    walkJob.animation = &app->ozzWalkAnimation;
    walkJob.ratio = (walkDur > 0.0f) ? (app->ozzWalkAnimationTime / walkDur) : 0.0f;
    walkJob.context = &app->ozzWalkSamplingContext;
    walkJob.output = ozz::make_span(walkLocals);
    walkJob.Run();

    ozz::vector<ozz::math::SoaTransform> leftLocals(app->ozzSkeleton.num_soa_joints());
    ozz::animation::SamplingJob leftJob;
    leftJob.animation = &app->ozzLeftAnimation;
    leftJob.ratio = (leftDur > 0.0f) ? (1.0f - (app->ozzLeftAnimationTime / leftDur)) : 0.0f;
    leftJob.context = &app->ozzLeftSamplingContext;
    leftJob.output = ozz::make_span(leftLocals);
    leftJob.Run();

    ozz::vector<ozz::math::SoaTransform> rightLocals(app->ozzSkeleton.num_soa_joints());
    ozz::animation::SamplingJob rightJob;
    rightJob.animation = &app->ozzRightAnimation;
    rightJob.ratio = (rightDur > 0.0f) ? (app->ozzRightAnimationTime / rightDur) : 0.0f;
    rightJob.context = &app->ozzRightSamplingContext;
    rightJob.output = ozz::make_span(rightLocals);
    rightJob.Run();

    // 4-Layer Blending
    ozz::animation::BlendingJob::Layer layers[4];
    layers[0].transform = ozz::make_span(idleLocals);
    layers[0].weight = currentIdleWeight;
    layers[1].transform = ozz::make_span(walkLocals);
    layers[1].weight = currentWalkWeight;
    layers[2].transform = ozz::make_span(leftLocals);
    layers[2].weight = currentLeftWeight;
    layers[3].transform = ozz::make_span(rightLocals);
    layers[3].weight = currentRightWeight;

    ozz::vector<ozz::math::SoaTransform> blendedLocals(app->ozzSkeleton.num_soa_joints());
    ozz::animation::BlendingJob blendJob;
    blendJob.rest_pose = app->ozzSkeleton.joint_rest_poses();
    blendJob.layers = ozz::make_span(layers);
    blendJob.output = ozz::make_span(blendedLocals);
    blendJob.threshold = 0.05f;
    blendJob.Run();

    // Local To Model
    ozz::animation::LocalToModelJob convJob;
    convJob.skeleton = &app->ozzSkeleton;
    convJob.input = ozz::make_span(blendedLocals);
    convJob.output = ozz::make_span(app->ozzModelMatrices);
    convJob.Run();
}
