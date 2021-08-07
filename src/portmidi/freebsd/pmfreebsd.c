/* pmfreebsd.c -- PortMidi os-dependent code */

#include "stdlib.h"
#include "portmidi.h"
#include "pmutil.h"
#include "pminternal.h"
#include "porttime.h"

#include "pmfreebsdsndio.h"
PmDeviceID pm_default_input_device_id = -1;
PmDeviceID pm_default_output_device_id = -1;

extern pm_fns_node pm_sndio_in_dictionary;
extern pm_fns_node pm_sndio_out_dictionary;

void pm_init()
{
    pm_freebsdsndio_init();
    // this is set when we return to Pm_Initialize, but we need it
    // now in order to (successfully) call Pm_CountDevices()
    pm_initialized = TRUE;
    pm_default_input_device_id = 0;
    pm_default_output_device_id = 1;
}

void pm_term(void)
{
   pm_freebsdsndio_term()
}

PmDeviceID Pm_GetDefaultInputDeviceID() {
    Pm_Initialize();
    return pm_default_input_device_id;
}

PmDeviceID Pm_GetDefaultOutputDeviceID() {
    Pm_Initialize();
    return pm_default_output_device_id;
}

void *pm_alloc(size_t s) { return malloc(s); }

void pm_free(void *ptr) { free(ptr); }
