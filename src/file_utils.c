#include "file_utils.h"

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *getAssetPath(const char *path)
{
#if defined(__ANDROID__)
    if (path && strncmp(path, "assets/", 7) == 0)
    {
        return path + 7; // Skips "assets/" on Android
    }
#endif
    return path;
}

/**
 * Reads a file into a null-terminated string.
 * The caller is responsible for calling SDL_free() on the returned pointer.
 */
char *readFile(const char *path)
{
    // Normalize path for Android
    const char *actualPath = getAssetPath(path);

    // Open the file for reading in binary mode
    SDL_IOStream *io = SDL_IOFromFile(actualPath, "rb");
    if (!io)
    {
        SDL_Log("Failed to open the file: %s (resolved: %s) (%s)", path, actualPath, SDL_GetError());
        return NULL;
    }

    // Get the file size
    Sint64 size = SDL_GetIOSize(io);
    if (size < 0)
    {
        SDL_CloseIO(io);
        return NULL;
    }

    // Allocate memory for the content + null terminator
    char *content = (char *)SDL_malloc((size_t)size + 1);
    if (!content)
    {
        SDL_CloseIO(io);
        return NULL;
    }

    // Read the data
    size_t bytesRead = SDL_ReadIO(io, content, (size_t)size);
    content[bytesRead] = '\0'; // Null-terminate

    // Clean up
    SDL_CloseIO(io);

    if (bytesRead != (size_t)size)
    {
        SDL_free(content);
        return NULL;
    }

    return content;
}