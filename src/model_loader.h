#pragma once

#include "app.h"
#include "file_utils.h"
#include "texture_utils.h"

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

inline bool loadCubeModel(App *app)
{
    cgltf_options options = {};
    cgltf_data *cubeData = nullptr;
    const char *cubePath = getAssetPath("assets/models/cube/check-grid-cube.glb");
    size_t glb_size = 0;
    void *glb_data = SDL_LoadFile(cubePath, &glb_size);
    if (!glb_data || cgltf_parse(&options, glb_data, glb_size, &cubeData) != cgltf_result_success)
        return false;

    cgltf_load_buffers(&options, cubeData, cubePath);
    cgltf_primitive &prim = cubeData->meshes[0].primitives[0];
    cgltf_accessor *posAcc = nullptr, *uvAcc = nullptr, *idxAcc = prim.indices;

    for (int i = 0; i < prim.attributes_count; ++i)
    {
        if (prim.attributes[i].type == cgltf_attribute_type_position)
            posAcc = prim.attributes[i].data;
        if (prim.attributes[i].type == cgltf_attribute_type_texcoord)
            uvAcc = prim.attributes[i].data;
    }

    glGenVertexArrays(1, &app->cubeVao);
    glBindVertexArray(app->cubeVao);

    size_t posSize = posAcc->count * 3 * sizeof(float);
    glGenBuffers(1, &app->cubeVbo);
    glBindBuffer(GL_ARRAY_BUFFER, app->cubeVbo);
    glBufferData(GL_ARRAY_BUFFER, posSize, (uint8_t *)posAcc->buffer_view->buffer->data + posAcc->buffer_view->offset + posAcc->offset, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    if (uvAcc)
    {
        size_t uvSize = uvAcc->count * 2 * sizeof(float);
        glGenBuffers(1, &app->cubeUvVbo);
        glBindBuffer(GL_ARRAY_BUFFER, app->cubeUvVbo);
        glBufferData(GL_ARRAY_BUFFER, uvSize, (uint8_t *)uvAcc->buffer_view->buffer->data + uvAcc->buffer_view->offset + uvAcc->offset, GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    }

    if (idxAcc)
    {
        size_t idxSize = idxAcc->count * idxAcc->stride;
        glGenBuffers(1, &app->cubeEbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->cubeEbo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxSize, (uint8_t *)idxAcc->buffer_view->buffer->data + idxAcc->buffer_view->offset + idxAcc->offset, GL_STATIC_DRAW);
        app->cubeIndexCount = (GLsizei)idxAcc->count;
        app->cubeIndexType = (idxAcc->component_type == cgltf_component_type_r_32u) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
    }

    glBindVertexArray(0);
    app->cubeTextureID = createTexture(getAssetPath("assets/models/cube/check-grid-512.webp"));
    cgltf_free(cubeData);
    SDL_free(glb_data);
    return true;
}

inline bool loadYBotModel(App *app)
{
    cgltf_options options = {};
    cgltf_data *ybotData = nullptr;
    const char *ybotPath = getAssetPath("assets/models/y-bot/y-bot.glb");
    size_t ybotSize = 0;
    void *ybotGlb = SDL_LoadFile(ybotPath, &ybotSize);
    if (!ybotGlb || cgltf_parse(&options, ybotGlb, ybotSize, &ybotData) != cgltf_result_success)
        return false;

    cgltf_load_buffers(&options, ybotData, ybotPath);
    cgltf_primitive &prim = ybotData->meshes[0].primitives[0];
    cgltf_accessor *posAcc = nullptr, *uvAcc = nullptr, *jointsAcc = nullptr, *weightsAcc = nullptr, *normAcc = nullptr, *idxAcc = prim.indices;

    for (int i = 0; i < prim.attributes_count; ++i)
    {
        if (prim.attributes[i].type == cgltf_attribute_type_position)
            posAcc = prim.attributes[i].data;
        if (prim.attributes[i].type == cgltf_attribute_type_texcoord)
            uvAcc = prim.attributes[i].data;
        if (prim.attributes[i].type == cgltf_attribute_type_joints)
            jointsAcc = prim.attributes[i].data;
        if (prim.attributes[i].type == cgltf_attribute_type_weights)
            weightsAcc = prim.attributes[i].data;
        if (prim.attributes[i].type == cgltf_attribute_type_normal)
            normAcc = prim.attributes[i].data;
    }

    glGenVertexArrays(1, &app->ybotVao);
    glBindVertexArray(app->ybotVao);

    if (posAcc)
    {
        glGenBuffers(1, &app->ybotVbo);
        glBindBuffer(GL_ARRAY_BUFFER, app->ybotVbo);
        glBufferData(GL_ARRAY_BUFFER, posAcc->count * 3 * sizeof(float), (uint8_t *)posAcc->buffer_view->buffer->data + posAcc->buffer_view->offset + posAcc->offset, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    }

    if (uvAcc)
    {
        glGenBuffers(1, &app->ybotUvVbo);
        glBindBuffer(GL_ARRAY_BUFFER, app->ybotUvVbo);
        glBufferData(GL_ARRAY_BUFFER, uvAcc->count * 2 * sizeof(float), (uint8_t *)uvAcc->buffer_view->buffer->data + uvAcc->buffer_view->offset + uvAcc->offset, GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    }

    if (jointsAcc)
    {
        glGenBuffers(1, &app->ybotJointVbo);
        glBindBuffer(GL_ARRAY_BUFFER, app->ybotJointVbo);
        glBufferData(GL_ARRAY_BUFFER, jointsAcc->count * jointsAcc->stride, (uint8_t *)jointsAcc->buffer_view->buffer->data + jointsAcc->buffer_view->offset + jointsAcc->offset, GL_STATIC_DRAW);
        GLenum jType = (jointsAcc->component_type == cgltf_component_type_r_8u) ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT;
        size_t cSize = (jType == GL_UNSIGNED_BYTE) ? sizeof(unsigned char) : sizeof(unsigned short);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, jType, GL_FALSE, (GLsizei)(4 * cSize), (void *)0);
    }

    if (weightsAcc)
    {
        glGenBuffers(1, &app->ybotWeightVbo);
        glBindBuffer(GL_ARRAY_BUFFER, app->ybotWeightVbo);
        glBufferData(GL_ARRAY_BUFFER, weightsAcc->count * 4 * sizeof(float), (uint8_t *)weightsAcc->buffer_view->buffer->data + weightsAcc->buffer_view->offset + weightsAcc->offset, GL_STATIC_DRAW);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    }

    if (normAcc)
    {
        glGenBuffers(1, &app->ybotNormalVbo);
        glBindBuffer(GL_ARRAY_BUFFER, app->ybotNormalVbo);
        glBufferData(GL_ARRAY_BUFFER, normAcc->count * 3 * sizeof(float),
            (uint8_t *)normAcc->buffer_view->buffer->data + normAcc->buffer_view->offset + normAcc->offset,
            GL_STATIC_DRAW);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    }

    if (idxAcc)
    {
        glGenBuffers(1, &app->ybotEbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->ybotEbo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxAcc->count * idxAcc->stride, (uint8_t *)idxAcc->buffer_view->buffer->data + idxAcc->buffer_view->offset + idxAcc->offset, GL_STATIC_DRAW);
        app->ybotIndexCount = (GLsizei)idxAcc->count;
        app->ybotIndexType = (idxAcc->component_type == cgltf_component_type_r_32u) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
    }

    glBindVertexArray(0);
    app->ybotTextureID = createTexture(getAssetPath("assets/models/y-bot/y-bot.webp"));

    if (ybotData->skins_count > 0 && ybotData->skins[0].inverse_bind_matrices)
    {
        cgltf_accessor *ibmAcc = ybotData->skins[0].inverse_bind_matrices;
        size_t numJ = ybotData->skins[0].joints_count;
        app->ybotInverseBindMatrices.resize(numJ);
        float *ibmData = (float *)((uint8_t *)ibmAcc->buffer_view->buffer->data + ibmAcc->buffer_view->offset + ibmAcc->offset);
        for (size_t i = 0; i < numJ; ++i)
        {
            memcpy(&app->ybotInverseBindMatrices[i], &ibmData[i * 16], 16 * sizeof(float));
        }
    }
    cgltf_free(ybotData);
    SDL_free(ybotGlb);
    return true;
}
