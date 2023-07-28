package com.example.ble_control

import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.content.Context
import android.os.Handler
import android.util.Log

/**
 * @brief Read-Only BLE Device Map
 */
class ROBleDeviceMap<String, BluetoothDevice>(private val protectedMap: Map<String, BluetoothDevice>): Map<String, BluetoothDevice> by protectedMap

class BleScanner(val activity: MainActivity, private val handler: Handler) {
    // List of scanned BLE devices
    private val bleDeviceMap = mutableMapOf<String, BluetoothDevice>();
    // BLE permissions manager
    private var blePermission : BlePermissions = BlePermissions()

    private val bluetoothAdapter: BluetoothAdapter? by lazy {
        val bluetoothManager = activity.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        bluetoothManager.adapter
    }

    private val scanCallback = object : ScanCallback() {
        override fun onScanResult(callbackType: Int, result: ScanResult) {
            // Discovery of a new BLE device
            val device: BluetoothDevice = result.device
            val rssi: Int = result.rssi

            if (!blePermission.checkBlePermission(activity)) {
                blePermission.requestBlePermissions(activity);
                return
            }
            else {
                Log.d(TAG, "Device found: ${device.name} (${device.address}), RSSI = $rssi dBm")
                if (!bleDeviceMap.contains(device.address)) {
                    bleDeviceMap[device.address] = device;
                    println("Added device ${device.name}");
                    activity.notifyDeviceListUpdate()
                }
            }
        }
    }

    fun startScan() {
        if (!blePermission.checkBlePermission(activity)) {
            blePermission.requestBlePermissions(activity);
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
        if (!blePermission.checkBlePermission(activity)) {
            blePermission.requestBlePermissions(activity);
            return
        }
        bluetoothAdapter?.bluetoothLeScanner?.stopScan(scanCallback)
        Log.d(TAG, "BLE scanning stopped")
        return
    }

    fun getDeviceList() : ROBleDeviceMap<String, BluetoothDevice> {
        return ROBleDeviceMap(this.bleDeviceMap)
    }

    companion object {
        const val TAG = "BleScanner"
    }
}