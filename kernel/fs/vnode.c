
#include <dxgmx/assert.h>
#include <dxgmx/fs/fs.h>
#include <dxgmx/fs/vnode.h>

int vnode_increase_refcount(VirtualNode* vnode)
{
    ++vnode->ref_count;
    return 0;
}

int vnode_decrease_refcount(VirtualNode* vnode)
{
    ASSERT(vnode->ref_count > 0);
    --vnode->ref_count;

    if (vnode->ref_count == 0 && (vnode->flags & VNODE_PENDING_RM))
    {
        // FIXME: how do we even propagate a failure here?
        vnode->owner->driver->rmnode(vnode);
    }
    return 0;
}
