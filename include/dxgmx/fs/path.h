/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_FS_PATH_H
#define _DXGMX_FS_PATH_H

#include <dxgmx/fs/fs.h>

/**
 * A few notes on file paths in dxgmx:
 *  - They should always come into the kernel from userspace as absolute paths.
 *  - Before they get passed down into their respective drivers they become
 * relative to their mountpoint. Ex: We have a devfs on /dev and try to access
 * /dev/fb. The path comes into the kernel as /dev/fb but before being passed to
 * the driver it becomes /fb.
 *  - The kernel doesn't care about trailing slashes. For me a trailing slash on
 * a filename indicates a dir, but doing that in the kernel gives us no
 * benefit and actually kind of complicates things. All trailing slashes are
 * stripped and the file type comes from that file's mode.
 */

/**
 * Make a path relative to a filesystem's mountpoint. The changes are made in
 * place.
 *
 * Returns 0 on success.
 */
int path_make_relative(char* path, const FileSystem* fs);

/**
 * Transform a path to it's final filename, ex: /dev/fb -> fb. The changes are
 * made in place.
 *
 * Returns 0 on success.
 */
int path_make_filename(char* path);

/**
 * This function takes an absolute path and makes it canonical. The paths is
 * modified in place, and the resulting path will always be smaller or equal to
 * the original one. Canonical in this case means:
 *  - Removing multiple consecutive '/'s in a path, ex: //dev//fb -> /dev/fb
 *  - Removing trailing slashes, ex: /dev/fb/ -> /dev/fb
 *  - Removing ./ and . from path, ex: both /dev/./ and /dev/. -> /dev
 *  - Resolving ../ in path, ex: /dev/dir/../ -> /dev
 */
int path_make_canonical(char* path);

#endif // !_DXGMX_FS_PATH_H
