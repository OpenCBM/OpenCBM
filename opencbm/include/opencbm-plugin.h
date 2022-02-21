/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2008-2009, 2011 Spiro Trikaliotis
 *  Copyright 2011      Wolfgang Moser
 */

/*! **************************************************************
** \file include/opencbm-plugin.h \n
** \author Spiro Trikaliotis \n
** \n
** \brief Plugin DLL interface
**
****************************************************************/

#ifndef OPENCBM_PLUGIN_H
#define OPENCBM_PLUGIN_H

#include "opencbm.h"

/*! \brief types of configuration data

 \remark
   Specify which type of configuration data is expected
   by struct opencbm_plugin_configuration_data_t.
*/
enum opencbm_plugin_configuration_data_type {
    OPENCBM_PLUGIN_CONFIGURATION_DATA_TYPE_LASTENTRY, /**< this is the last (dummy) entry; does not contain any data */
    OPENCBM_PLUGIN_CONFIGURATION_DATA_TYPE_INTEGER,   /**< the expected data is (signed) integer */
    OPENCBM_PLUGIN_CONFIGURATION_DATA_TYPE_UINTEGER,  /**< the expected data is an (unsigned) integer */
    OPENCBM_PLUGIN_CONFIGURATION_DATA_TYPE_STRING,    /**< the expected data is an arbitrary string */
    OPENCBM_PLUGIN_CONFIGURATION_DATA_TYPE_BOOLEAN    /**< the expected data is a boolean (1/true/yes or 0/false/no) */
};

/*! \brief specification of needed configuration data

 \remark
   This is used by opencbm_plugin_get_list_of_configuration_parameter_t() and
   opencbm_plugin_set_configuration_parameter_t()
*/
typedef
struct opencbm_plugin_configuration_data_t {
  enum opencbm_plugin_configuration_data_type   type;     /**< the type of the configuration entry */
  const char                                  * name;     /**< the name of the configuration entry */
  int                                           isValid;  /**< is 0 if the entry is not valid; otherwise, if the entry is valid, it is set to 1. */
  union {
            signed int   integer;        /**< the returned value if type is OPENCBM_PLUGIN_CONFIGURATION_DATA_TYPE_INTEGER */
          unsigned int   uinteger;       /**< the returned value if type is OPENCBM_PLUGIN_CONFIGURATION_DATA_TYPE_INTEGER */
          char         * string;         /**< the returned value if type is OPENCBM_PLUGIN_CONFIGURATION_DATA_TYPE_STRING.

                                           \note
                                           The plugin must copy this
                                           value if it needs it after
                                           opencbm_plugin_set_configuration_parameter()
                                           has returned!
                                           */
          unsigned int   boolean;        /**< the returned value (1 or 0) if type is OPENCBM_PLUGIN_CONFIGURATION_DATA_TYPE_BOOLEAN */
  };
} opencbm_plugin_configuration_data_t;

/*! \brief @@@@@ \todo document

\return
*/
typedef int CBMAPIDECL opencbm_plugin_init_t(void);

/*! \brief @@@@@ \todo document

\return
*/
typedef void CBMAPIDECL opencbm_plugin_uninit_t(void);

/*! \brief @@@@@ \todo document

 \param Port

 \return
*/
typedef const char * CBMAPIDECL opencbm_plugin_get_driver_name_t(const char * const Port);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Port

 \return
*/
typedef int CBMAPIDECL opencbm_plugin_driver_open_t(CBM_FILE *HandleDevice, const char * const Port);

/*! \brief @@@@@ \todo document

 \param HandleDevice
*/
typedef void CBMAPIDECL opencbm_plugin_driver_close_t(CBM_FILE HandleDevice);

/*! \brief @@@@@ \todo document

 \param HandleDevice
*/
typedef void CBMAPIDECL opencbm_plugin_lock_t(CBM_FILE HandleDevice);

/*! \brief @@@@@ \todo document

 \param HandleDevice
*/
typedef void CBMAPIDECL opencbm_plugin_unlock_t(CBM_FILE HandleDevice);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Buffer

 \param Count

 \return
*/
typedef int CBMAPIDECL opencbm_plugin_raw_write_t(CBM_FILE HandleDevice, const void *Buffer, size_t Count);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Buffer

 \param Count

 \return
*/
typedef int CBMAPIDECL opencbm_plugin_raw_read_t(CBM_FILE HandleDevice, void *Buffer, size_t Count);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param DeviceAddress

 \param SecondaryAddress

 \return
*/
typedef int CBMAPIDECL opencbm_plugin_open_t(CBM_FILE HandleDevice, unsigned char DeviceAddress, unsigned char SecondaryAddress);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param DeviceAddress

 \param SecondaryAddress

 \return
*/
typedef int CBMAPIDECL opencbm_plugin_close_t(CBM_FILE HandleDevice, unsigned char DeviceAddress, unsigned char SecondaryAddress);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param DeviceAddress

 \param SecondaryAddress

 \return
*/
typedef int CBMAPIDECL opencbm_plugin_listen_t(CBM_FILE HandleDevice, unsigned char DeviceAddress, unsigned char SecondaryAddress);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param DeviceAddress

 \param SecondaryAddress

 \return
*/
typedef int CBMAPIDECL opencbm_plugin_talk_t(CBM_FILE HandleDevice, unsigned char DeviceAddress, unsigned char SecondaryAddress);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \return
*/
typedef int CBMAPIDECL opencbm_plugin_unlisten_t(CBM_FILE HandleDevice);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \return
*/
typedef int CBMAPIDECL opencbm_plugin_untalk_t(CBM_FILE HandleDevice);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \return
*/
typedef int CBMAPIDECL opencbm_plugin_get_eoi_t(CBM_FILE HandleDevice);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \return
*/
typedef int CBMAPIDECL opencbm_plugin_clear_eoi_t(CBM_FILE HandleDevice);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \return
*/
typedef int CBMAPIDECL opencbm_plugin_reset_t(CBM_FILE HandleDevice);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \return
*/
typedef unsigned char CBMAPIDECL opencbm_plugin_pp_read_t(CBM_FILE HandleDevice);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Byte
*/
typedef void CBMAPIDECL opencbm_plugin_pp_write_t(CBM_FILE HandleDevice, unsigned char Byte);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \return
*/
typedef int CBMAPIDECL opencbm_plugin_iec_poll_t(CBM_FILE HandleDevice);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Line

*/
typedef void CBMAPIDECL opencbm_plugin_iec_set_t(CBM_FILE HandleDevice, int Line);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Line
*/
typedef void CBMAPIDECL opencbm_plugin_iec_release_t(CBM_FILE HandleDevice, int Line);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Line
*/
typedef void CBMAPIDECL opencbm_plugin_iec_setrelease_t(CBM_FILE HandleDevice, int Set, int Release);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Line

 \param State

 \return
*/
typedef int CBMAPIDECL opencbm_plugin_iec_wait_t(CBM_FILE HandleDevice, int Line, int State);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \return
*/
typedef unsigned char CBMAPIDECL opencbm_plugin_parallel_burst_read_t(CBM_FILE HandleDevice);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Value
*/
typedef void CBMAPIDECL opencbm_plugin_parallel_burst_write_t(CBM_FILE HandleDevice, unsigned char Value);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Buffer

 \param Length

 \return
*/
typedef int CBMAPIDECL opencbm_plugin_parallel_burst_read_n_t(CBM_FILE HandleDevice, unsigned char *Buffer, unsigned int Length);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Buffer

 \param Length

 \return
*/
typedef int CBMAPIDECL opencbm_plugin_parallel_burst_write_n_t(CBM_FILE HandleDevice, unsigned char *Buffer, unsigned int Length);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Buffer

 \param Length

 \return
*/
typedef int CBMAPIDECL opencbm_plugin_parallel_burst_read_track_t(CBM_FILE HandleDevice, unsigned char *Buffer, unsigned int Length);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Buffer

 \param Length

 \return
*/
typedef int CBMAPIDECL opencbm_plugin_parallel_burst_read_track_var_t(CBM_FILE HandleDevice, unsigned char *Buffer, unsigned int Length);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Buffer

 \param Length

 \return
*/
typedef int CBMAPIDECL opencbm_plugin_parallel_burst_write_track_t(CBM_FILE HandleDevice, unsigned char *Buffer, unsigned int Length);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Buffer

 \param Length

 \return
*/
typedef int CBMAPIDECL opencbm_plugin_tap_prepare_capture_t(CBM_FILE HandleDevice, int *Status);
typedef int CBMAPIDECL opencbm_plugin_tap_prepare_write_t(CBM_FILE HandleDevice, int *Status);
typedef int CBMAPIDECL opencbm_plugin_tap_get_sense_t(CBM_FILE HandleDevice, int *Status);
typedef int CBMAPIDECL opencbm_plugin_tap_wait_for_stop_sense_t(CBM_FILE HandleDevice, int *Status);
typedef int CBMAPIDECL opencbm_plugin_tap_wait_for_play_sense_t(CBM_FILE HandleDevice, int *Status);
typedef int CBMAPIDECL opencbm_plugin_tap_motor_on_t(CBM_FILE HandleDevice, int *Status);
typedef int CBMAPIDECL opencbm_plugin_tap_motor_off_t(CBM_FILE HandleDevice, int *Status);
typedef int CBMAPIDECL opencbm_plugin_tap_start_capture_t(CBM_FILE HandleDevice, unsigned char *Buffer, unsigned int Buffer_Length, int *Status, int *BytesRead);
typedef int CBMAPIDECL opencbm_plugin_tap_start_write_t(CBM_FILE HandleDevice, unsigned char *Buffer, unsigned int Length, int *Status, int *BytesWritten);
typedef int CBMAPIDECL opencbm_plugin_tap_get_ver_t(CBM_FILE HandleDevice, int *Status);
typedef int CBMAPIDECL opencbm_plugin_tap_download_config_t(CBM_FILE HandleDevice, unsigned char *Buffer, unsigned int Buffer_Length, int *Status, int *BytesRead);
typedef int CBMAPIDECL opencbm_plugin_tap_upload_config_t(CBM_FILE HandleDevice, unsigned char *Buffer, unsigned int Length, int *Status, int *BytesWritten);
typedef int CBMAPIDECL opencbm_plugin_tap_break_t(CBM_FILE HandleDevice);

/*! \brief read a block of data from the OpenCBM backend with protocol serial-1

 \param HandleDevice
   Pointer to a CBM_FILE which will contain the file handle of the OpenCBM backend

 \param data
    Pointer to a buffer which will contain the data read from the OpenCBM backend

 \param size
    The number of bytes to read from the OpenCBM backend

 \return
    The number of bytes actually read, 0 on OpenCBM backend error.
    If there is a fatal error, returns -1.
*/
typedef int CBMAPIDECL opencbm_plugin_s1_read_n_t (CBM_FILE HandleDevice,       unsigned char *data, unsigned int size);

/*! \brief write a block of data to the OpenCBM backend with protocol serial-1

 \param HandleDevice
   Pointer to a CBM_FILE which will contain the file handle of the OpenCBM backend

 \param data
    Pointer to buffer which contains the data to be written to the OpenCBM backend

 \param size
    The length of the data buffer to be written to the OpenCBM backend

 \return
    The number of bytes actually written, 0 on OpenCBM backend error.
    If there is a fatal error, returns -1.
*/
typedef int CBMAPIDECL opencbm_plugin_s1_write_n_t(CBM_FILE HandleDevice, const unsigned char *data, unsigned int size);

/*! \brief read a block of data from the OpenCBM backend with protocol serial-2

 \param HandleDevice
   Pointer to a CBM_FILE which will contain the file handle of the OpenCBM backend

 \param data
    Pointer to a buffer which will contain the data read from the OpenCBM backend

 \param size
    The number of bytes to read from the OpenCBM backend

 \return
    The number of bytes actually read, 0 on OpenCBM backend error.
    If there is a fatal error, returns -1.
*/
typedef int CBMAPIDECL opencbm_plugin_s2_read_n_t (CBM_FILE HandleDevice,       unsigned char *data, unsigned int size);

/*! \brief write a block of data to the OpenCBM backend with protocol serial-2

 \param HandleDevice
   Pointer to a CBM_FILE which will contain the file handle of the OpenCBM backend

 \param data
    Pointer to buffer which contains the data to be written to the OpenCBM backend

 \param size
    The length of the data buffer to be written to the OpenCBM backend

 \return
    The number of bytes actually written, 0 on OpenCBM backend error.
    If there is a fatal error, returns -1.
*/
typedef int CBMAPIDECL opencbm_plugin_s2_write_n_t(CBM_FILE HandleDevice, const unsigned char *data, unsigned int size);

/*! \brief read a block of data from the OpenCBM backend with protocol parallel/d64copy

 \param HandleDevice
   Pointer to a CBM_FILE which will contain the file handle of the OpenCBM backend

 \param data
    Pointer to a buffer which will contain the data read from the OpenCBM backend

 \param size
    The number of bytes to read from the OpenCBM backend

 \return
    The number of bytes actually read, 0 on OpenCBM backend error.
    If there is a fatal error, returns -1.
*/
typedef int CBMAPIDECL opencbm_plugin_s3_read_n_t (CBM_FILE HandleDevice,       unsigned char *data, unsigned int size);

/*! \brief write a block of data to the OpenCBM backend with protocol serial-3

 \param HandleDevice
    Pointer to a CBM_FILE which will contain the file handle of the OpenCBM backend

 \param data
    Pointer to buffer which contains the data to be written to the OpenCBM backend

 \param size
    The length of the data buffer to be written to the OpenCBM backend

 \return
    The number of bytes actually written, 0 on OpenCBM backend error.
    If there is a fatal error, returns -1.
*/
typedef int CBMAPIDECL opencbm_plugin_s3_write_n_t(CBM_FILE HandleDevice, const unsigned char *data, unsigned int size);

/*! \brief read a block of data from the OpenCBM backend with protocol parallel/d64copy

 \param HandleDevice
    Pointer to a CBM_FILE which will contain the file handle of the OpenCBM backend

 \param data
    Pointer to a buffer which will contain the data read from the OpenCBM backend

 \param size
    The number of bytes to read from the OpenCBM backend

 \return
    The number of bytes actually read, 0 on OpenCBM backend error.
    If there is a fatal error, returns -1.
*/
typedef int CBMAPIDECL opencbm_plugin_pp_dc_read_n_t (CBM_FILE HandleDevice,       unsigned char *data, unsigned int size);

/*! \brief write a block of data to the OpenCBM backend with protocol parallel/d64copy

 \param HandleDevice
   Pointer to a CBM_FILE which will contain the file handle of the OpenCBM backend

 \param data
    Pointer to buffer which contains the data to be written to the OpenCBM backend

 \param size
    The length of the data buffer to be written to the OpenCBM backend

 \return
    The number of bytes actually written, 0 on OpenCBM backend error.
    If there is a fatal error, returns -1.
*/
typedef int CBMAPIDECL opencbm_plugin_pp_dc_write_n_t(CBM_FILE HandleDevice, const unsigned char *data, unsigned int size);

/*! \brief read a block of data from the OpenCBM backend with protocol parallel/cbmcopy

 \param HandleDevice
   Pointer to a CBM_FILE which will contain the file handle of the OpenCBM backend

 \param data
    Pointer to a buffer which will contain the data read from the OpenCBM backend

 \param size
    The number of bytes to read from the OpenCBM backend

 \return
    The number of bytes actually read, 0 on OpenCBM backend error.
    If there is a fatal error, returns -1.
*/
typedef int CBMAPIDECL opencbm_plugin_pp_cc_read_n_t (CBM_FILE HandleDevice,       unsigned char *data, unsigned int size);

/*! \brief write a block of data to the OpenCBM backend with protocol parallel/cbmcopy

 \param HandleDevice
   Pointer to a CBM_FILE which will contain the file handle of the OpenCBM backend

 \param data
    Pointer to buffer which contains the data to be written to the OpenCBM backend

 \param size
    The length of the data buffer to be written to the OpenCBM backend

 \return
    The number of bytes actually written, 0 on OpenCBM backend error.
    If there is a fatal error, returns -1.
*/
typedef int CBMAPIDECL opencbm_plugin_pp_cc_write_n_t(CBM_FILE HandleDevice, const unsigned char *data, unsigned int size);


/*! \brief @@@@@ \todo document

 \param HandleDevice

 \return
*/
typedef int CBMAPIDECL opencbm_plugin_iec_dbg_read_t(CBM_FILE HandleDevice);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Value

 \return
*/
typedef int CBMAPIDECL opencbm_plugin_iec_dbg_write_t(CBM_FILE HandleDevice, unsigned char Value);

/*! \brief Get a memory area which contains the configuration data

 \return
    Pointer to a memory area that contains the names and the types of the
    configuration data that are wanted.

 \remark
    The plugin fills the buffer and returns it with this function.
    The opencbm DLL/.SO will fill in the known data values into this buffer.

    The DLL/SO will call this function, and afterwards, it will call
    the opencbm_plugin_set_configuration_parameter_t function.
*/
typedef opencbm_plugin_configuration_data_t * CBMAPIDECL opencbm_plugin_get_list_of_configuration_parameter_t(void);

/*! \brief Get a memory area which contains the configuration data

 \return
    Pointer to a memory area that contains the names and the types of the
    configuration data that are wanted.

 \remark
    The plugin fills the buffer and returns it with this function.
    The opencbm DLL/.SO will fill in the known data values into this buffer.

    The DLL/SO will call the function
    opencbm_plugin_get_list_of_configuration_parameter_t first; afterwards,
    it will call this function with the data filled in.

    The DLL/SO is only allowed to fill the buffer given by
    opencbm_plugin_get_list_of_configuration_parameter_t function.

    After the opencbm_plugin_set_configuration_parameter_t returns, the data
    areas pointed to (for the strings) cannot be accessed anymore.
    Thus, if the strings are needed, they must be copied to a safe place.
*/
typedef void CBMAPIDECL opencbm_plugin_set_configuration_parameter_t(opencbm_plugin_configuration_data_t * ConfigurationData);

/*! \brief holds all callbacks of the plugin

  This structure contains all callbacks available in the plugin.

  \note
    Not all functions are mandatory. Thus, there might be NULL pointers available!
*/
typedef
struct opencbm_plugin_s {
    opencbm_plugin_init_t                       * opencbm_plugin_init;                       /*!< pointer to a opencbm_plugin_init_t() function */
    opencbm_plugin_uninit_t                     * opencbm_plugin_uninit;                     /*!< pointer to a opencbm_plugin_uninit() function */

    opencbm_plugin_get_driver_name_t            * opencbm_plugin_get_driver_name;            /*!< pointer to a opencbm_plugin_get_driver_name_t() function */
    opencbm_plugin_driver_open_t                * opencbm_plugin_driver_open;                /*!< pointer to a opencbm_plugin_driver_open_t() function */
    opencbm_plugin_driver_close_t               * opencbm_plugin_driver_close;               /*!< pointer to a opencbm_plugin_driver_close_t() function */
    opencbm_plugin_lock_t                       * opencbm_plugin_lock;                       /*!< pointer to a opencbm_plugin_lock_t() function */
    opencbm_plugin_unlock_t                     * opencbm_plugin_unlock;                     /*!< pointer to a opencbm_plugin_unlock_t() function */
    opencbm_plugin_raw_write_t                  * opencbm_plugin_raw_write;                  /*!< pointer to a opencbm_plugin_raw_write_t() function */
    opencbm_plugin_raw_read_t                   * opencbm_plugin_raw_read;                   /*!< pointer to a opencbm_plugin_raw_read_t() function */
    opencbm_plugin_open_t                       * opencbm_plugin_open;                       /*!< pointer to a opencbm_plugin_open_t() function */
    opencbm_plugin_close_t                      * opencbm_plugin_close;                      /*!< pointer to a opencbm_plugin_close_t() function */
    opencbm_plugin_listen_t                     * opencbm_plugin_listen;                     /*!< pointer to a opencbm_plugin_listen_t() function */
    opencbm_plugin_talk_t                       * opencbm_plugin_talk;                       /*!< pointer to a opencbm_plugin_talk_t() function */
    opencbm_plugin_unlisten_t                   * opencbm_plugin_unlisten;                   /*!< pointer to a opencbm_plugin_unlisten_t() function */
    opencbm_plugin_untalk_t                     * opencbm_plugin_untalk;                     /*!< pointer to a opencbm_plugin_untalk_t() function */
    opencbm_plugin_get_eoi_t                    * opencbm_plugin_get_eoi;                    /*!< pointer to a opencbm_plugin_get_eoi_t() function */
    opencbm_plugin_clear_eoi_t                  * opencbm_plugin_clear_eoi;                  /*!< pointer to a opencbm_plugin_clear_eoi_t() function */
    opencbm_plugin_reset_t                      * opencbm_plugin_reset;                      /*!< pointer to a opencbm_plugin_reset_t() function */
    opencbm_plugin_pp_read_t                    * opencbm_plugin_pp_read;                    /*!< pointer to a opencbm_plugin_pp_read_t() function */
    opencbm_plugin_pp_write_t                   * opencbm_plugin_pp_write;                   /*!< pointer to a opencbm_plugin_write_t() function */
    opencbm_plugin_iec_poll_t                   * opencbm_plugin_iec_poll;                   /*!< pointer to a opencbm_plugin_iec_poll_t() function */
    opencbm_plugin_iec_set_t                    * opencbm_plugin_iec_set;                    /*!< pointer to a opencbm_plugin_iec_set_t() function */
    opencbm_plugin_iec_release_t                * opencbm_plugin_iec_release;                /*!< pointer to a opencbm_plugin_iec_release_t() function */
    opencbm_plugin_iec_setrelease_t             * opencbm_plugin_iec_setrelease;             /*!< pointer to a opencbm_plugin_iec_setrelease_t() function */
    opencbm_plugin_iec_wait_t                   * opencbm_plugin_iec_wait;                   /*!< pointer to a opencbm_plugin_iec_wait_t() function */

    opencbm_plugin_parallel_burst_read_t        * opencbm_plugin_parallel_burst_read;        /*!< pointer to a opencbm_plugin_parallel_burst_read_t() function */
    opencbm_plugin_parallel_burst_write_t       * opencbm_plugin_parallel_burst_write;       /*!< pointer to a opencbm_plugin_parallel_burst_write_t() function */
    opencbm_plugin_parallel_burst_read_n_t      * opencbm_plugin_parallel_burst_read_n;      /*!< pointer to a opencbm_plugin_parallel_burst_read_n_t() function */
    opencbm_plugin_parallel_burst_write_n_t     * opencbm_plugin_parallel_burst_write_n;     /*!< pointer to a opencbm_plugin_parallel_burst_write_n_t() function */
    opencbm_plugin_parallel_burst_read_track_t  * opencbm_plugin_parallel_burst_read_track;  /*!< pointer to a opencbm_plugin_parallel_burst_read_track_t() function */
    opencbm_plugin_parallel_burst_write_track_t * opencbm_plugin_parallel_burst_write_track; /*!< pointer to a opencbm_plugin_parallel_burst_write_track_t() function */
    opencbm_plugin_parallel_burst_read_track_var_t * opencbm_plugin_parallel_burst_read_track_var;  /*!< pointer to a opencbm_plugin_parallel_burst_read_track_var_t() function */

    opencbm_plugin_iec_dbg_read_t               * opencbm_plugin_iec_dbg_read;               /*!< pointer to a opencbm_plugin_iec_dbg_read_t() function */
    opencbm_plugin_iec_dbg_write_t              * opencbm_plugin_iec_dbg_write;              /*!< pointer to a opencbm_plugin_iec_dbg_write_t() function */
    opencbm_plugin_parallel_burst_read_t        * opencbm_plugin_srq_burst_read;        /*!< pointer to a opencbm_plugin_parallel_burst_read_t() function */
    opencbm_plugin_parallel_burst_write_t       * opencbm_plugin_srq_burst_write;       /*!< pointer to a opencbm_plugin_parallel_burst_write_t() function */
    opencbm_plugin_parallel_burst_read_n_t      * opencbm_plugin_srq_burst_read_n;      /*!< pointer to a opencbm_plugin_parallel_burst_read_n_t() function */
    opencbm_plugin_parallel_burst_write_n_t     * opencbm_plugin_srq_burst_write_n;     /*!< pointer to a opencbm_plugin_parallel_burst_write_n_t() function */
    opencbm_plugin_parallel_burst_read_track_t  * opencbm_plugin_srq_burst_read_track;  /*!< pointer to a opencbm_plugin_parallel_burst_read_track_t() function */
    opencbm_plugin_parallel_burst_write_track_t * opencbm_plugin_srq_burst_write_track; /*!< pointer to a opencbm_plugin_parallel_burst_write_track_t() function */

    opencbm_plugin_tap_prepare_capture_t        * opencbm_plugin_tap_prepare_capture;     /*!< pointer to a opencbm_plugin_tap_prepare_capture_t() function */
    opencbm_plugin_tap_prepare_write_t          * opencbm_plugin_tap_prepare_write;       /*!< pointer to a opencbm_plugin_tap_prepare_write_t() function */
    opencbm_plugin_tap_get_sense_t              * opencbm_plugin_tap_get_sense;           /*!< pointer to a opencbm_plugin_tap_get_sense_t() function */
    opencbm_plugin_tap_wait_for_stop_sense_t    * opencbm_plugin_tap_wait_for_stop_sense; /*!< pointer to a opencbm_plugin_tap_wait_for_stop_sense_t() function */
    opencbm_plugin_tap_wait_for_play_sense_t    * opencbm_plugin_tap_wait_for_play_sense; /*!< pointer to a opencbm_plugin_tap_wait_for_play_sense_t() function */
    opencbm_plugin_tap_motor_on_t               * opencbm_plugin_tap_motor_on;            /*!< pointer to a opencbm_plugin_tap_motor_on_t() function */
    opencbm_plugin_tap_motor_off_t              * opencbm_plugin_tap_motor_off;           /*!< pointer to a opencbm_plugin_tap_motor_off_t() function */
    opencbm_plugin_tap_start_capture_t          * opencbm_plugin_tap_start_capture;       /*!< pointer to a opencbm_plugin_tap_start_capture_t() function */
    opencbm_plugin_tap_start_write_t            * opencbm_plugin_tap_start_write;         /*!< pointer to a opencbm_plugin_tap_start_write_t() function */
    opencbm_plugin_tap_get_ver_t                * opencbm_plugin_tap_get_ver;             /*!< pointer to a opencbm_plugin_tap_get_ver_t() function */
    opencbm_plugin_tap_download_config_t        * opencbm_plugin_tap_download_config;     /*!< pointer to a opencbm_plugin_tap_download_config_t() function */
    opencbm_plugin_tap_upload_config_t          * opencbm_plugin_tap_upload_config;       /*!< pointer to a opencbm_plugin_tap_upload_config_t() function */
    opencbm_plugin_tap_break_t                  * opencbm_plugin_tap_break;               /*!< pointer to a opencbm_plugin_tap_break_t() function */

    opencbm_plugin_get_list_of_configuration_parameter_t * opencbm_plugin_get_list_of_configuration_parameter;
    opencbm_plugin_set_configuration_parameter_t         * opencbm_plugin_set_configuration_parameter;

} opencbm_plugin_t;

#endif // #ifndef OPENCBM_PLUGIN_H
