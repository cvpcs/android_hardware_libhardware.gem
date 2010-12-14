/*
 * Copyright (C) 2010 Motorola, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if defined(__DEVICE_shadow__) || defined(__DEVICE_droid2__)

#ifndef ANDROID_GESTURES_INTERFACE_H
#define ANDROID_GESTURES_INTERFACE_H

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <hardware/hardware.h>
#include <cutils/native_handle.h>

__BEGIN_DECLS

/**
 * The id of this module
 */
#define GESTURE_HARDWARE_MODULE_ID "gestures"

/**
 * Name of the sensors device to open
 */

#define GESTURE_HARDWARE_CONTROL    "control"
#define GESTURE_HARDWARE_DATA       "data"


/**
 * Handles must be higher than SENSORS_HANDLE_BASE and must be unique.
 * A Handle identifies a given sensors. The handle is used to activate
 * and/or deactivate sensors.
 * In this version of the API there can only be 256 handles.
 */
#define GESTURE_HANDLE_BASE                     0
#define GESTURE_HANDLE_BITS             8
#define GESTURE_HANDLE_COUNT           (1<<GESTURE_HANDLE_BITS)

/**
 * gesture types
 */
#define GESTURE_TYPE_DOUBLETAP      1
#define GESTURE_TYPE_SINGLETAP      2
/* BEGIN Motorola, DWRT64, 12/10/09, IKMAP-3241 /Application not correctly reporting detected gestures */
#define GESTURE_TYPE_OFFGLASS       13
/* END IKMAP-3241 */
/*For identifying the ICs those who providing geatures but not raw data */
#define GESTURE_IC_TYPE_OFFGLASS    16

/**
 * status of each gesture
 */
typedef struct {
    float v[3];
    int8_t status;
    uint8_t reserved[3];
} gestures_vec_t;

/**
 * Union of the various types of gesture data
 * that can be returned.
 */
typedef struct {
    /* gesture identifier */
    int             gesture;

    /* gesture data */
    gestures_vec_t   gesturedata;

    /* time is in nanosecond */
    int64_t         time;

    uint32_t        reserved;
} gestures_data_t;


struct gesture_t;

/**
 * Every hardware module must have a data structure named HAL_MODULE_INFO_SYM
 * and the fields of this data structure must begin with hw_module_t
 * followed by module specific information.
 */
struct gestures_module_t {
    struct hw_module_t common;

    /**
     * Enumerate all available gestures. The list is returned in "list".
     * @return number of sensors in the list
     */
    int (*get_gestures_list)(struct gestures_module_t* module,
            struct gesture_t const** list);
};

struct gesture_t {
    /* name of this gestures */
    const char*     name;
    /* handle that identifies this gestures. This handle is used to activate
     * and deactivate this gesture. The value of the handle must be 8 bits
     * in this version of the API. 
     */
    int             handle;
    /* this gesture's type. */
    int             type;

    /* type of the sensor that provides this gesture */
	int             sensorType;

    /* reserved fields, must be zero */
    void*           reserved[9];
};

/**
 * Every device data structure must begin with hw_device_t
 * followed by module specific public methods and attributes.
 */
struct gestures_control_device_t {
    struct hw_device_t common;
    
    /**
     * Returns a native_handle_t, which will be the parameter to
     * gesturess_data_device_t::open_data(). 
     * The caller takes ownership of this handle. This is intended to be
     * passed cross processes.
     *
     * @return a native_handle_t if successful, NULL on error
     */
    native_handle_t* (*open_data_source)(struct gestures_control_device_t *dev);
    /**
     * Releases any resources that were created by open_data_source.
     * This call is optional and can be NULL if not implemented
     * by the gesture HAL.
     *
     * @return 0 if successful, < 0 on error
     */
    int (*close_data_source)(struct gestures_control_device_t *dev);
    /** Activate/deactivate one sensor.
     *
     * @param handle is the handle of the sensor to change.
     * @param enabled set to 1 to enable, or 0 to disable the sensor.
     *
     * @return 0 on success, negative errno code otherwise
     */
    int (*activate)(struct gestures_control_device_t *dev, 
            int handle, int enabled);
    /**
     * Set the sensititvity for different gestures
     *
     * @return 0 if successful, < 0 on error
     */
    int (*set_sensitivity)(struct gestures_control_device_t *dev, int32_t handle, int32_t sens);
    /**
     * Causes sensors_data_device_t.poll() to return -EWOULDBLOCK immediately.
     */
    int (*wake)(struct gestures_control_device_t *dev);
};

struct gestures_data_device_t {
    struct hw_device_t common;

    /**
     * Prepare to read sensor data.
     *
     * This routine does NOT take ownership of the handle
     * and must not close it. Typically this routine would
     * use a duplicate of the nh parameter.
     *
     * @param nh from sensors_control_open.
     *
     * @return 0 if successful, < 0 on error
     */
    int (*data_open)(struct gestures_data_device_t *dev, native_handle_t* nh);
    /**
     * Caller has completed using the sensor data.
     * The caller will not be blocked in sensors_data_poll
     * when this routine is called.
     *
     * @return 0 if successful, < 0 on error
     */
    int (*data_close)(struct gestures_data_device_t *dev);
    /**
     * Return sensor data for one of the enabled sensors.
     *
     * @return sensor handle for the returned data, 0x7FFFFFFF when 
     * sensors_control_device_t.wake() is called and -errno on error
     *  
     */
    int (*poll)(struct gestures_data_device_t *dev, 
            gestures_data_t* data);
};


/** convenience API for opening and closing a device */

static inline int gestures_control_open(const struct hw_module_t* module, 
        struct gestures_control_device_t** device) {
    return module->methods->open(module, 
            GESTURE_HARDWARE_CONTROL, (struct hw_device_t**)device);
}

static inline int gestures_control_close(struct gestures_control_device_t* device) {
    return device->common.close(&device->common);
}

static inline int gestures_data_open(const struct hw_module_t* module, 
        struct gestures_data_device_t** device) {
    return module->methods->open(module, 
            GESTURE_HARDWARE_DATA, (struct hw_device_t**)device);
}

static inline int gestures_data_close(struct gestures_data_device_t* device) {
    return device->common.close(&device->common);
}


__END_DECLS

#endif  // ANDROID_SENSORS_INTERFACE_H

#endif // device_shadow || device_droid2
