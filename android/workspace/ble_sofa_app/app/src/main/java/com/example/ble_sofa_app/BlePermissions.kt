/*--------------------------------------------------------------------------------
--                          _               _       _
--                         | |__ _ __ _ _ _| |_ ___| |
--                         | / _` / _` | ' \  _/ -_) |
--                         |_\__, \__,_|_||_\__\___|_|
--                           |___/
--
----------------------------------------------------------------------------------
--
-- Company: LGANTEL
-- Engineer: Laurent Gantel <laurent.gantel@gmail.com>
--
-- Project Name: BLE Sofa Application
-- Version: 0.1.0
-- File Name: BlePermissions.tk
-- Description: Class used to request and check BLE permissions
--
-- Last update: 2023-09-09
--
-------------------------------------------------------------------------------*/

package com.example.ble_sofa_app

import android.Manifest
import android.content.Context
import android.content.pm.PackageManager
import android.os.Build
import androidx.core.app.ActivityCompat

/**
 * @brief Utility class to manage BLE permissions
 */
class BlePermissions {
    /**
     * @brief Check if the application has the correct permissions in the Manifest file
     * @param context The caller context
     * @return true if the permissions have been correctly set, false otherwise
     */
    fun checkBlePermission(context: Context) : Boolean {
        return (
                (ActivityCompat.checkSelfPermission(context, Manifest.permission.BLUETOOTH_CONNECT) == PackageManager.PERMISSION_GRANTED)
                        || (ActivityCompat.checkSelfPermission(context, Manifest.permission.BLUETOOTH_SCAN) == PackageManager.PERMISSION_GRANTED)
                );
    }

    /**
     * @brief Used to request BLE permissions at runtime
     * @param activity The target activity (MainActivity)
     */
    fun requestBlePermissions(activity: MainActivity) {
        // Here request the missing permissions
        // onRequestPermissionsResult() is overridden elsewhere to handle the request results
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            ActivityCompat.requestPermissions(
                activity,
                arrayOf(Manifest.permission.BLUETOOTH_SCAN, Manifest.permission.BLUETOOTH_CONNECT),
                REQUEST_PERMISSION_CODE
            )
        }
    }

    companion object {
        /** @brief Request permission code used to request BLE permissions at runtime */
        const val REQUEST_PERMISSION_CODE = 123
    }
}