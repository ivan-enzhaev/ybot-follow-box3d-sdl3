#pragma once

// OpenGL Loader
#if defined(__EMSCRIPTEN__) || defined(__ANDROID__)
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif

// SDL3 Core & TTF
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

// Dear ImGui
#include <imgui.h>

// Math Library
#include <cglm/cglm.h>

// Ozz Animation Library
#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/simd_math.h>

// Physics Engine
#include <box3d/box3d.h>

#include <vector>

struct Mat4Wrapper
{
    mat4 m;
};

struct App
{
    SDL_Window *window = nullptr;
    SDL_GLContext glContext = nullptr;
    GLuint shaderProgram = 0;
    GLuint vao = 0, vbo = 0;
    GLuint textTextureID = 0;
    TTF_Font *font = nullptr;
    int textW = 0;
    int textH = 0;
    mat4 mvpMatrix;
    GLint uMvpMatrixLocation = -1;
    ImGuiIO *io = nullptr;
    float scale_factor = 1.0f;
    GLuint cubeVao = 0, cubeVbo = 0, cubeUvVbo = 0, cubeEbo = 0;
    GLsizei cubeIndexCount = 0;
    GLenum cubeIndexType = 0;
    mat4 projView2D;
    mat4 projView3D;
    float cubeRotationAngle = 0.0f;
    GLuint cubeTextureID = 0;

    // MSAA Framebuffer Objects
    GLuint msaaFBO = 0;
    GLuint msaaColorRBO = 0;
    GLuint msaaDepthRBO = 0;
    int fboWidth = 0;
    int fboHeight = 0;
    const int msaaSamples = 4;

    // Ozz-animation fields
    ozz::animation::Skeleton ozzSkeleton;
    ozz::animation::Animation ozzIdleAnimation;
    ozz::animation::Animation ozzWalkAnimation;
    ozz::animation::Animation ozzLeftAnimation;
    ozz::animation::Animation ozzRightAnimation;

    ozz::animation::SamplingJob::Context ozzIdleSamplingContext;
    ozz::animation::SamplingJob::Context ozzWalkSamplingContext;
    ozz::animation::SamplingJob::Context ozzLeftSamplingContext;
    ozz::animation::SamplingJob::Context ozzRightSamplingContext;

    float ozzIdleAnimationTime = 0.0f;
    float ozzWalkAnimationTime = 0.0f;
    float ozzLeftAnimationTime = 0.0f;
    float ozzRightAnimationTime = 0.0f;

    ozz::vector<ozz::math::Float4x4> ozzModelMatrices;
    GLuint lineShaderProgram = 0;
    GLint uLineMvpMatrixLocation = -1;
    GLint uLineColorLocation = -1;
    GLuint lineVao = 0, lineVbo = 0;

    // Skinning & Lighting Fields
    GLuint skinningShaderProgram = 0;
    GLint uSkinningMvpMatrixLocation = -1;
    GLint uSkinningModelMatrixLocation = -1;
    GLint uSkinningViewPosLocation = -1;
    GLint uSkinningLightDirLocation = -1;
    GLint uJointMatricesLocation = -1;

    GLuint ybotVao = 0;
    GLuint ybotVbo = 0;       // Positions
    GLuint ybotUvVbo = 0;     // UVs
    GLuint ybotJointVbo = 0;  // Joint Indices (ivec4)
    GLuint ybotWeightVbo = 0; // Joint Weights (vec4)
    GLuint ybotNormalVbo = 0; // Normals (vec3)
    GLuint ybotEbo = 0;       // Indices
    GLsizei ybotIndexCount = 0;
    GLenum ybotIndexType = 0;
    GLuint ybotTextureID = 0;
    std::vector<Mat4Wrapper> ybotInverseBindMatrices;

    bool showDebugSkeleton = false;
    bool showRobot = true;
    bool showPhysicsDebug = true;

    // Physics Engine Fields
    b3WorldId worldId = { 0 };
    b3DebugDraw dd;
    b3BodyId capsuleId = { 0 };

    // Input state flags
    bool moveUp = false;
    bool moveDown = false;
    bool rotateLeft = false;
    bool rotateRight = false;

    float charYaw = 0.0f;

    float joystickX = 0.0f;
    float joystickY = 0.0f;
    float moveX_joystick = 0.0f;
    float moveZ_joystick = 0.0f;

    bool isMobile = false;
    bool needsReposition = false;
};
