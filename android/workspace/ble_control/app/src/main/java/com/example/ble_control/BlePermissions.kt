package com.example.ble_control

import android.Manifest
import android.content.Context
import android.content.pm.PackageManager
import android.os.Build
import androidx.core.app.ActivityCompat

class BlePermissions {
    fun checkBlePermission(context: Context) : Boolean {
        return (
                (ActivityCompat.checkSelfPermission(context, Manifest.permission.BLUETOOTH_CONNECT) == PackageManager.PERMISSION_GRANTED)
                        || (ActivityCompat.checkSelfPermission(context, Manifest.permission.BLUETOOTH_SCAN) == PackageManager.PERMISSION_GRANTED)
                );
    }

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
        const val REQUEST_PERMISSION_CODE = 123
    }
}