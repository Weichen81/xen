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

static int libxl__device_virtio_console_setdefault(libxl__gc *gc, uint32_t domid,
                                                libxl_device_virtio_console *virtio_console,
                                                bool hotplug)
{
    return libxl__resolve_domid(gc, virtio_console->backend_domname,
                                &virtio_console->backend_domid);
}

static int libxl__virtio_console_from_xenstore(libxl__gc *gc, const char *libxl_path,
                                            libxl_devid devid,
                                            libxl_device_virtio_console *virtio_console)
{
    const char *be_path;
    int rc;

    virtio_console->devid = devid;
    rc = libxl__xs_read_mandatory(gc, XBT_NULL,
                                  GCSPRINTF("%s/backend", libxl_path),
                                  &be_path);
    if (rc) return rc;

    rc = libxl__backendpath_parse_domid(gc, be_path, &virtio_console->backend_domid);
    if (rc) return rc;

    return 0;
}

static void libxl__update_config_virtio_console(libxl__gc *gc,
                                             libxl_device_virtio_console *dst,
                                             libxl_device_virtio_console *src)
{
    dst->devid = src->devid;
}

static int libxl_device_virtio_console_compare(libxl_device_virtio_console *d1,
                                            libxl_device_virtio_console *d2)
{
    return COMPARE_DEVID(d1, d2);
}

static void libxl__device_virtio_console_add(libxl__egc *egc, uint32_t domid,
                                          libxl_device_virtio_console *virtio_console,
                                          libxl__ao_device *aodev)
{
    libxl__device_add_async(egc, domid, &libxl__virtio_console_devtype, virtio_console, aodev);
}

static int libxl__set_xenstore_virtio_console(libxl__gc *gc, uint32_t domid,
                                           libxl_device_virtio_console *virtio_console,
                                           flexarray_t *back, flexarray_t *front,
                                           flexarray_t *ro_front)
{
    int rc;
    unsigned int i;

    for (i = 0; i < virtio_console->num_cons; i++) {
        rc = flexarray_append_pair(ro_front, GCSPRINTF("%d/tty", i),
                                   GCSPRINTF("%s", virtio_console->cons[i].tty));
        if (rc) return rc;

        rc = flexarray_append_pair(ro_front, GCSPRINTF("%d/printonly", i),
                                   GCSPRINTF("%d", virtio_console->cons[i].printonly));
        if (rc) return rc;

        rc = flexarray_append_pair(ro_front, GCSPRINTF("%d/base", i),
                                   GCSPRINTF("%lu", virtio_console->cons[i].base));
        if (rc) return rc;

        rc = flexarray_append_pair(ro_front, GCSPRINTF("%d/irq", i),
                                   GCSPRINTF("%u", virtio_console->cons[i].irq));
        if (rc) return rc;
    }

    return 0;
}

static LIBXL_DEFINE_UPDATE_DEVID(virtio_console)
static LIBXL_DEFINE_DEVICE_FROM_TYPE(virtio_console)
static LIBXL_DEFINE_DEVICES_ADD(virtio_console)

DEFINE_DEVICE_TYPE_STRUCT(virtio_console, VIRTIO_CONSOLE, virtio_consoles,
    .update_config = (device_update_config_fn_t) libxl__update_config_virtio_console,
    .from_xenstore = (device_from_xenstore_fn_t) libxl__virtio_console_from_xenstore,
    .set_xenstore_config = (device_set_xenstore_config_fn_t) libxl__set_xenstore_virtio_console
);

/*
 * Local variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
