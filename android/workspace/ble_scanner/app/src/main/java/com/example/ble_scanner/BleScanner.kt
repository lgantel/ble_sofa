package com.example.ble_scanner

import android.Manifest
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.content.Context
import android.content.pm.PackageManager
import android.os.Build
import android.os.Handler
import android.util.Log
import androidx.core.app.ActivityCompat

class BleScanner(val activity: MainActivity, val handler: Handler) {
    // List of scanned BLE devices
    private val BleDeviceMap = mutableMapOf<String, BluetoothDevice>();

    private val bluetoothAdapter: BluetoothAdapter? by lazy {
        val bluetoothManager = activity.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        bluetoothManager.adapter
    }

    fun checkBlePermission(activity: MainActivity) : Boolean {
        return (
                (ActivityCompat.checkSelfPermission(activity, Manifest.permission.BLUETOOTH_CONNECT) == PackageManager.PERMISSION_GRANTED)
          || (ActivityCompat.checkSelfPermission(activity, Manifest.permission.BLUETOOTH_SCAN) == PackageManager.PERMISSION_GRANTED)
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

    private val scanCallback = object : ScanCallback() {
        override fun onScanResult(callbackType: Int, result: ScanResult) {
            // Discovery of a new BLE device
            val device: BluetoothDevice = result.device
            val rssi: Int = result.rssi

            if (!checkBlePermission(activity)) {
                requestBlePermissions(activity);
                return
            }
            else {
                Log.d(TAG, "Device found: ${device.name} (${device.address}), RSSI = $rssi dBm")
                if (!BleDeviceMap.contains(device.address)) {
                    BleDeviceMap[device.address] = device;
                    println("Added device ${device.name}");
                }
            }
        }
    }

    fun startScan() {
        if (!checkBlePermission(activity)) {
            requestBlePermissions(activity);
            return
        }

        val scanSettings = ScanSettings.Builder()
            .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
            .build()

        // Scan for 10 seconds
        handler.postDelayed({
            stopScan()
        }, 30000)

        bluetoothAdapter?.bluetoothLeScanner?.startScan(null, scanSettings, scanCallback)
        Log.d(TAG, "BLE scanning started...")
    }

    fun stopScan() {
        if (!checkBlePermission(activity)) {
            requestBlePermissions(activity);
            return
        }
        bluetoothAdapter?.bluetoothLeScanner?.stopScan(scanCallback)
        Log.d(TAG, "BLE scanning stopped")
        return
    }

    companion object {
        const val REQUEST_PERMISSION_CODE = 123
        const val TAG = "BleScanner"
    }
}