/*
 * Copyright (C) 2021 Arm Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; version 2.1 only. with the special
 * exception on linking described in file LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 */

#include "libxl_internal.h"

static int libxl__device_virtio_net_setdefault(libxl__gc *gc, uint32_t domid,
                                                libxl_device_virtio_net *virtio_net,
                                                bool hotplug)
{
    return libxl__resolve_domid(gc, virtio_net->backend_domname,
                                &virtio_net->backend_domid);
}

static int libxl__virtio_net_from_xenstore(libxl__gc *gc, const char *libxl_path,
                                            libxl_devid devid,
                                            libxl_device_virtio_net *virtio_net)
{
    const char *be_path;
    int rc;

    virtio_net->devid = devid;
    rc = libxl__xs_read_mandatory(gc, XBT_NULL,
                                  GCSPRINTF("%s/backend", libxl_path),
                                  &be_path);
    if (rc) return rc;

    rc = libxl__backendpath_parse_domid(gc, be_path, &virtio_net->backend_domid);
    if (rc) return rc;

    return 0;
}

static void libxl__update_config_virtio_net(libxl__gc *gc,
                                             libxl_device_virtio_net *dst,
                                             libxl_device_virtio_net *src)
{
    dst->devid = src->devid;
}

static int libxl_device_virtio_net_compare(libxl_device_virtio_net *d1,
                                            libxl_device_virtio_net *d2)
{
    return COMPARE_DEVID(d1, d2);
}

static void libxl__device_virtio_net_add(libxl__egc *egc, uint32_t domid,
                                          libxl_device_virtio_net *virtio_net,
                                          libxl__ao_device *aodev)
{
    libxl__device_add_async(egc, domid, &libxl__virtio_net_devtype, virtio_net, aodev);
}

static int libxl__set_xenstore_virtio_net(libxl__gc *gc, uint32_t domid,
                                           libxl_device_virtio_net *virtio_net,
                                           flexarray_t *back, flexarray_t *front,
                                           flexarray_t *ro_front)
{
    int rc;
    unsigned int i;

    for (i = 0; i < virtio_net->num_netifs; i++) {
        rc = flexarray_append_pair(ro_front, GCSPRINTF("%d/interface", i),
                                   GCSPRINTF("%s", virtio_net->netifs[i].interface));
        if (rc) return rc;

        rc = flexarray_append_pair(ro_front, GCSPRINTF("%d/tap", i),
                                   GCSPRINTF("%d", virtio_net->netifs[i].tap));
        if (rc) return rc;

        rc = flexarray_append_pair(ro_front, GCSPRINTF("%d/base", i),
                                   GCSPRINTF("%lu", virtio_net->netifs[i].base));
        if (rc) return rc;

        rc = flexarray_append_pair(ro_front, GCSPRINTF("%d/irq", i),
                                   GCSPRINTF("%u", virtio_net->netifs[i].irq));
        if (rc) return rc;
    }

    return 0;
}

static LIBXL_DEFINE_UPDATE_DEVID(virtio_net)
static LIBXL_DEFINE_DEVICE_FROM_TYPE(virtio_net)
static LIBXL_DEFINE_DEVICES_ADD(virtio_net)

DEFINE_DEVICE_TYPE_STRUCT(virtio_net, VIRTIO_NET, virtio_nets,
    .update_config = (device_update_config_fn_t) libxl__update_config_virtio_net,
    .from_xenstore = (device_from_xenstore_fn_t) libxl__virtio_net_from_xenstore,
    .set_xenstore_config = (device_set_xenstore_config_fn_t) libxl__set_xenstore_virtio_net
);

/*
 * Local variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
