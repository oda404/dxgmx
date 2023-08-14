
#include <dxgmx/assert.h>
#include <dxgmx/fb.h>
#include <dxgmx/fb_ioctl.h>
#include <dxgmx/user.h>

int fbio_get_info(VirtualNode* vnode, FBInfo* info)
{
    FrameBuffer* fb = vnode->data;
    ASSERT(fb);
    FBInfo tmp = {.width = fb->width, .height = fb->height, .bpp = fb->bpp};
    return user_copy_to(info, &tmp, sizeof(FBInfo));
}
