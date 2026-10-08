#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Normalizes asset paths across platforms.
     * On Android, automatically strips the "assets/" prefix.
     */
    const char *getAssetPath(const char *path);

    /**
     * Reads a file into a null-terminated string.
     * Automatically handles the Android "assets/" prefix.
     * The caller is responsible for calling SDL_free() on the returned pointer.
     */
    char *readFile(const char *path);

#ifdef __cplusplus
}
#endif

#endif // FILE_UTILS_H