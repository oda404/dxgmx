/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_FS_PATH_H
#define _DXGMX_FS_PATH_H

#include <dxgmx/fs/fs.h>

int path_make_relative(const FileSystem* fs, char* path);

#endif // !_DXGMX_FS_PATH_H
