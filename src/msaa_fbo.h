#pragma once

#include "app.h"

inline void setupMSAAFramebuffer(App *app, int width, int height)
{
    if (app->fboWidth == width && app->fboHeight == height)
        return;

    app->fboWidth = width;
    app->fboHeight = height;

    if (app->msaaFBO == 0)
        glGenFramebuffers(1, &app->msaaFBO);
    if (app->msaaColorRBO == 0)
        glGenRenderbuffers(1, &app->msaaColorRBO);
    if (app->msaaDepthRBO == 0)
        glGenRenderbuffers(1, &app->msaaDepthRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, app->msaaFBO);

    glBindRenderbuffer(GL_RENDERBUFFER, app->msaaColorRBO);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, app->msaaSamples, GL_RGBA8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, app->msaaColorRBO);

    glBindRenderbuffer(GL_RENDERBUFFER, app->msaaDepthRBO);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, app->msaaSamples, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, app->msaaDepthRBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        SDL_Log("MSAA Framebuffer is incomplete!");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

inline void cleanupMSAAFramebuffer(App *app)
{
    if (app->msaaFBO) glDeleteFramebuffers(1, &app->msaaFBO);
    if (app->msaaColorRBO) glDeleteRenderbuffers(1, &app->msaaColorRBO);
    if (app->msaaDepthRBO) glDeleteRenderbuffers(1, &app->msaaDepthRBO);
}
