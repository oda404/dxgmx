
#ifndef _DXGMX_FB_IOCTL_H
#define _DXGMX_FB_IOCTL_H

#include <dxgmx/fb_user.h>
#include <dxgmx/fs/vnode.h>

int fbio_get_info(VirtualNode* vnode, FBInfo* info);

#endif // !_DXGMX_FB_IOCTL_H
