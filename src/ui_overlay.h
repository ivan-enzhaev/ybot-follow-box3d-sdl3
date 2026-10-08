#pragma once

#include "app.h"
#include <imgui.h>

inline void renderUI(App *app, int windowW, int windowH)
{
    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)windowW, (float)windowH);

    if (app->isMobile)
    {
        bool isPortrait = (windowH > windowW);
        if (isPortrait)
            ImGui::SetNextWindowPos(ImVec2(20.0f * app->scale_factor, 70.0f * app->scale_factor), ImGuiCond_Always);
        else
            ImGui::SetNextWindowPos(ImVec2(50.0f * app->scale_factor, 20.0f * app->scale_factor), ImGuiCond_Always);
    }
    else
    {
        ImGui::SetNextWindowPos(ImVec2(20.0f * app->scale_factor, 20.0f * app->scale_factor), ImGuiCond_FirstUseEver);
    }

    ImGui::SetNextWindowSize(ImVec2(340.0f * app->scale_factor, 130.0f * app->scale_factor), ImGuiCond_FirstUseEver);

    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Checkbox("Show Robots", &app->showRobot);
    ImGui::Checkbox("Show Physics / AI Debug", &app->showPhysicsDebug);
    ImGui::End();

    // Virtual Joystick on Mobile
    if (app->isMobile)
    {
        int winPixelW, winPixelH;
        SDL_GetWindowSizeInPixels(app->window, &winPixelW, &winPixelH);

        float radius = 60.0f * app->scale_factor;
        float handleRadius = 25.0f * app->scale_factor;
        float joystickPadding = 20.0f * app->scale_factor;

        ImVec2 center(joystickPadding + radius + handleRadius, (float)winPixelH - joystickPadding - radius - handleRadius);
        float totalHalfSize = radius + handleRadius;
        ImGui::SetNextWindowPos(ImVec2(center.x - totalHalfSize, center.y - totalHalfSize));
        ImGui::SetNextWindowSize(ImVec2(totalHalfSize * 2.0f, totalHalfSize * 2.0f));

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                                 ImGuiWindowFlags_NoBackground |
                                 ImGuiWindowFlags_NoSavedSettings |
                                 ImGuiWindowFlags_NoMove;

        ImGui::Begin("VirtualJoystick", nullptr, flags);
        ImDrawList *draw_list = ImGui::GetWindowDrawList();

        draw_list->AddCircleFilled(center, radius, IM_COL32(100, 100, 100, 120));
        draw_list->AddCircle(center, radius, IM_COL32(255, 255, 255, 200), 32, 2.0f);

        ImGui::InvisibleButton("JoystickBtn", ImVec2(totalHalfSize * 2.0f, totalHalfSize * 2.0f));

        ImVec2 handlePos = center;
        if (ImGui::IsItemActive())
        {
            ImVec2 touchPos = ImGui::GetIO().MousePos;
            float dx = touchPos.x - center.x;
            float dy = touchPos.y - center.y;
            float dist = sqrtf(dx * dx + dy * dy);

            if (dist > 0.0f)
            {
                float clampDist = (dist > radius) ? radius : dist;
                handlePos.x = center.x + (dx / dist) * clampDist;
                handlePos.y = center.y + (dy / dist) * clampDist;

                app->moveX_joystick = (handlePos.x - center.x) / radius;
                app->moveZ_joystick = (handlePos.y - center.y) / radius;
            }
        }
        else
        {
            app->moveX_joystick = 0.0f;
            app->moveZ_joystick = 0.0f;
        }

        draw_list->AddCircleFilled(handlePos, handleRadius, IM_COL32(255, 255, 255, 220));
        ImGui::End();
    }
}
