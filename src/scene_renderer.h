#pragma once

#include "app.h"
#include "file_utils.h"
#include "shader_program.h"
#include "texture_utils.h"

inline bool initSceneShaders(App *app)
{
    char *vertexSource = readFile(getAssetPath("assets/shaders/texture.vert"));
    char *fragmentSource = readFile(getAssetPath("assets/shaders/texture.frag"));
    if (!vertexSource || !fragmentSource) return false;
    app->shaderProgram = createShaderProgram(vertexSource, fragmentSource);
    SDL_free(vertexSource);
    SDL_free(fragmentSource);
    if (!app->shaderProgram) return false;

    glUseProgram(app->shaderProgram);
    app->uMvpMatrixLocation = glGetUniformLocation(app->shaderProgram, "uMvpMatrix");
    glUniform1i(glGetUniformLocation(app->shaderProgram, "ourTexture"), 0);

    char *lineVert = readFile(getAssetPath("assets/shaders/line.vert"));
    char *lineFrag = readFile(getAssetPath("assets/shaders/line.frag"));
    if (!lineVert || !lineFrag) return false;
    app->lineShaderProgram = createShaderProgram(lineVert, lineFrag);
    SDL_free(lineVert);
    SDL_free(lineFrag);
    if (!app->lineShaderProgram) return false;

    app->uLineMvpMatrixLocation = glGetUniformLocation(app->lineShaderProgram, "uMvpMatrix");
    app->uLineColorLocation = glGetUniformLocation(app->lineShaderProgram, "uColor");

    char *skinVert = readFile(getAssetPath("assets/shaders/skinning.vert"));
    char *skinFrag = readFile(getAssetPath("assets/shaders/skinning.frag"));
    if (!skinVert || !skinFrag) return false;
    app->skinningShaderProgram = createShaderProgram(skinVert, skinFrag);
    SDL_free(skinVert);
    SDL_free(skinFrag);
    if (!app->skinningShaderProgram) return false;

    glUseProgram(app->skinningShaderProgram);
    app->uSkinningMvpMatrixLocation = glGetUniformLocation(app->skinningShaderProgram, "uMvpMatrix");
    app->uSkinningModelMatrixLocation = glGetUniformLocation(app->skinningShaderProgram, "uModelMatrix");
    app->uSkinningViewPosLocation = glGetUniformLocation(app->skinningShaderProgram, "uViewPos");
    app->uSkinningLightDirLocation = glGetUniformLocation(app->skinningShaderProgram, "uLightDir");
    app->uJointMatricesLocation = glGetUniformLocation(app->skinningShaderProgram, "uJointMatrices");
    glUniform1i(glGetUniformLocation(app->skinningShaderProgram, "ourTexture"), 0);

    // Line dynamic buffer
    glGenVertexArrays(1, &app->lineVao);
    glGenBuffers(1, &app->lineVbo);
    glBindVertexArray(app->lineVao);
    glBindBuffer(GL_ARRAY_BUFFER, app->lineVbo);
    glBufferData(GL_ARRAY_BUFFER, 4096 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glBindVertexArray(0);

    return true;
}

inline bool initTextOverlay(App *app)
{
    float quadVerts[] = {
        -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, -0.5f, 1.0f, 1.0f
    };
    glGenVertexArrays(1, &app->vao);
    glGenBuffers(1, &app->vbo);
    glBindVertexArray(app->vao);
    glBindBuffer(GL_ARRAY_BUFFER, app->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    app->font = TTF_OpenFont(getAssetPath("assets/fonts/LiberationSans-Regular.ttf"), 36.0f);
    if (app->font)
    {
        SDL_Color color = { 170, 255, 195, 255 };
        SDL_Surface *s = TTF_RenderText_Blended_Wrapped(app->font, "Y-Bot Follower (BT.CPP + Recast)", 0, color, 0);
        if (s)
        {
            SDL_Surface *conv = SDL_ConvertSurface(s, SDL_PIXELFORMAT_RGBA32);
            SDL_DestroySurface(s);
            if (conv)
            {
                app->textW = conv->w;
                app->textH = conv->h;
                app->textTextureID = createTextureFromSurface(conv);
                SDL_DestroySurface(conv);
            }
        }
    }
    return true;
}

inline void renderPlayerRobot(App *app)
{
    glUseProgram(app->skinningShaderProgram);
    glBindTexture(GL_TEXTURE_2D, app->ybotTextureID);
    glBindVertexArray(app->ybotVao);

    b3Vec3 pPos = b3Body_GetPosition(app->capsuleId);
    b3Quat pRot = b3Body_GetRotation(app->capsuleId);

    mat4 model;
    glm_mat4_identity(model);
    glm_translate(model, (float[]) { pPos.x, pPos.y, pPos.z });
    versor pV = { pRot.v.x, pRot.v.y, pRot.v.z, pRot.s };
    mat4 rotM;
    glm_quat_mat4(pV, rotM);
    glm_mat4_mul(model, rotM, model);
    glm_translate(model, (float[]) { 0.0f, -1.0f, 0.0f });
    glm_scale(model, (float[]) { 1.33f, 1.33f, 1.33f });

    mat4 mvp;
    glm_mat4_mul(app->projView3D, model, mvp);

    glUniformMatrix4fv(app->uSkinningMvpMatrixLocation, 1, GL_FALSE, (const GLfloat *)mvp);
    glUniformMatrix4fv(app->uSkinningModelMatrixLocation, 1, GL_FALSE, (const GLfloat *)model);
    glUniform3f(app->uSkinningViewPosLocation, 0.0f, 1.0f, 8.0f);
    glUniform3f(app->uSkinningLightDirLocation, 0.5f, 1.0f, 0.4f);

    const int numActiveJoints = 65;
    const int activeJointIndices[] = {
        2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
        22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
        40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57,
        58, 59, 60, 61, 62, 63, 64, 65, 66
    };
    std::vector<float> jData(numActiveJoints * 16);
    for (int idx = 0; idx < numActiveJoints; ++idx)
    {
        int i = activeJointIndices[idx];
        mat4 ozzMat;
        memcpy(&ozzMat[0][0], &app->ozzModelMatrices[i].cols[0], 16 * sizeof(float));
        mat4 finalJ;
        if (idx < (int)app->ybotInverseBindMatrices.size())
            glm_mat4_mul(ozzMat, app->ybotInverseBindMatrices[idx].m, finalJ);
        else
            glm_mat4_copy(ozzMat, finalJ);
        memcpy(&jData[idx * 16], &finalJ[0][0], 16 * sizeof(float));
    }
    glUniformMatrix4fv(app->uJointMatricesLocation, numActiveJoints, GL_FALSE, jData.data());
    glDrawElements(GL_TRIANGLES, app->ybotIndexCount, app->ybotIndexType, 0);
    glBindVertexArray(0);
}
