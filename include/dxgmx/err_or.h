
#ifndef _DXGMX_ERR_OR_H
#define _DXGMX_ERR_OR_H

#define DEFINE_ERR_OR(_type)                                                   \
    struct _S_ErrorOr_##_type                                                  \
    {                                                                          \
        int error;                                                             \
        _type value;                                                           \
    };

#define ERR_OR(_type) struct _S_ErrorOr_##_type

#define ERR(_type, _err)                                                       \
    (struct _S_ErrorOr_##_type)                                                \
    {                                                                          \
        .error = _err                                                          \
    }

#define VALUE(_type, _val)                                                     \
    (struct _S_ErrorOr_##_type)                                                \
    {                                                                          \
        .value = _val                                                          \
    }

#endif // !_DXGMX_ERR_OR_H
