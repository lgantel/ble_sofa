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
-- File Name: BleScanner.tk
-- Description: Class used to perform BLE scanning
--
-- Last update: 2023-09-09
--
-------------------------------------------------------------------------------*/

package com.example.ble_sofa_app

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

/**
 * @brief Manage BLE Scanner operations
 * @param activity A reference to the MainActivity
 * @param handler The MainActivity handler
 */
class BleScanner(val activity: MainActivity, private val handler: Handler) {
    /** @brief  List of scanned BLE devices */
    private val bleDeviceMap = mutableMapOf<String, BluetoothDevice>();
    /** @brief  BLE permissions manager */
    private var blePermission : BlePermissions = BlePermissions()

    /** @brief Bluetooth adapter */
    private val bluetoothAdapter: BluetoothAdapter? by lazy {
        val bluetoothManager = activity.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        bluetoothManager.adapter
    }

    /**
     * @brief Scanning callback - Store found devices into private device list
     */
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

    /**
     * @brief Start the scanning process, scan for 10 seconds
     */
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
        }, 10000)

        bluetoothAdapter?.bluetoothLeScanner?.startScan(null, scanSettings, scanCallback)
        Log.d(TAG, "BLE scanning started...")
    }

    /**
     * @brief Stop the scanning process
     */
    fun stopScan() {
        if (!blePermission.checkBlePermission(activity)) {
            blePermission.requestBlePermissions(activity);
            return
        }
        bluetoothAdapter?.bluetoothLeScanner?.stopScan(scanCallback)
        Log.d(TAG, "BLE scanning stopped")
        return
    }

    /**
     * @brief Get a reference to the device list (read-only)
     * @return A map containing the device name and the associated BluetoothDevice
     */
    fun getDeviceList() : ROBleDeviceMap<String, BluetoothDevice> {
        return ROBleDeviceMap(this.bleDeviceMap)
    }

    companion object {
        /** @brief TAG used for debug */
        const val TAG = "BleScanner"
    }
}