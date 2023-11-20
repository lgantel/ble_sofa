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
-- File Name: BleService.tk
-- Description: Class used to manage BLE operations
--
-- Last update: 2023-09-09
--
-------------------------------------------------------------------------------*/

package com.example.ble_sofa_app

import android.app.Service
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
import android.bluetooth.BluetoothGattService
import android.bluetooth.BluetoothProfile
import android.content.Intent
import android.os.Binder
import android.os.Handler
import android.os.IBinder
import android.os.Looper
import android.util.Log
import java.util.UUID

/**
 * @brief BLE Android service
 */
class BleService : Service() {
    //----------------------------------------------------------------
    // Private variables
    //----------------------------------------------------------------
    /** @brief Service binder */
    private val binder = LocalBinder()
    /** @brief Local binder definition */
    inner class LocalBinder : Binder() {
        fun getService() : BleService {
            return this@BleService
        }
    }

    /** @brief Bluetooth adapter */
    private var bluetoothAdapter : BluetoothAdapter? = null
    /** @brief Service intent to communicate with the main activity */
    private var appIntent : Intent? = null

    /** @brief Instance to the Bluetooth GATT server */
    private var bluetoothGatt : BluetoothGatt? = null

    /** @brief BLE permissions manager */
    private var blePermission : BlePermissions = BlePermissions()

    /** @brief Bluetooth connection state */
    private var connectionState = STATE_DISCONNECTED

    //----------------------------------------------------------------
    // Private functions
    //----------------------------------------------------------------
    /**
     * @brief Send a message to the activity via an intent
     */
    private fun broadcastUpdate(action: String) {
        val intent = Intent(action)
        sendBroadcast(intent)
    }

    /**
     * @brief GATT Callback
     */
    private val bluetoothGattCallback = object : BluetoothGattCallback() {
        override fun onConnectionStateChange(gatt: BluetoothGatt?, status: Int, newState: Int) {
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                Log.d(TAG, "Successfully connected to GATT server")
                connectionState = STATE_CONNECTED
                broadcastUpdate(ACTION_GATT_CONNECTED)

                // Perform services discovery
                // Check and get permissions
                if (!blePermission.checkBlePermission(applicationContext)) {
                    Log.w(TAG, "Bluetooth permissions requested")
                    broadcastUpdate(ACTION_REQUIRE_PERMISSIONS)
                    return
                }

                bluetoothGatt = gatt
                Handler(Looper.getMainLooper()).post {
                    bluetoothGatt?.discoverServices()
                }
            }
            else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                Log.d(TAG, "Disconnected from GATT server")
                connectionState = STATE_DISCONNECTED
                broadcastUpdate(ACTION_GATT_DISCONNECTED)
            }
        }
    }

    //----------------------------------------------------------------
    // Public functions
    //----------------------------------------------------------------
    /**
     * @brief On call to bindService from the activity, return the binder instance
     */
    override fun onBind(intent: Intent) : IBinder? {
        appIntent = intent
        return binder
    }

    /**
     * @brief On call to unbinService from the activity, cleanup the current connection
     */
    override fun onUnbind(intent: Intent?): Boolean {
        close()
        return super.onUnbind(intent)
    }

    /**
     * @brief Initialize the service:
     *          - Create the Bluetooth adapter
     */
    fun initialize() : Boolean {
        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter()
        if (bluetoothAdapter == null) {
            Log.e(TAG, "Unable to obtain a BluetoothAdapter.")
            return false
        }
        return true
    }

    /**
     * @brief Connect to a Bluetooth device
     * @param address The device address
     * @return false if the application does not have the correct permissions,
     *               or if the device address is invalid,
     *               or if the Bluetooth adapter has not been correctly initialized,
     *         true if the connection succeeds
     * @note The failure reason is displayed in the Log console
     */
    fun connect(address: String): Boolean {
        bluetoothAdapter?.let { adapter ->
            try {
                val device = adapter.getRemoteDevice(address)

                // Check and get permissions
                if (!blePermission.checkBlePermission(this)) {
                    Log.w(TAG, "Bluetooth permissions requested")
                    broadcastUpdate(ACTION_REQUIRE_PERMISSIONS)
                    return false
                }

                // Connect to the GATT service on the device
                bluetoothGatt = device.connectGatt(this, false, bluetoothGattCallback)
                return true
            } catch (exception: IllegalArgumentException) {
                Log.w(TAG, "Device not found with provided address. Unable to connect")
                return false
            }
        } ?: run {
            Log.w(TAG, "BluetoothAdapter not initialized")
            return false
        }
    }

    /**
     * @brief Close the current Bluetooth connection:
     *          - Completely free the Bluetooth resources
     *          - Send an intent to the application to notify the disconnection,
     *            because close() does not call the onConnectionStateChange() method
     *            from the Gatt callback
     * @return false if the application does not have the correct permissions,
     *         true otherwise
     */
    fun close() : Boolean {
        bluetoothGatt?.let { gatt ->
            // Check and get permissions
            if (!blePermission.checkBlePermission(this)) {
                Log.w(TAG, "Bluetooth permissions requested")
                broadcastUpdate(ACTION_REQUIRE_PERMISSIONS)
                return false
            }

            gatt.close()
            bluetoothGatt = null
            // Notify main activity
            connectionState = STATE_DISCONNECTED
            broadcastUpdate(ACTION_GATT_DISCONNECTED)
        }

        return true
    }

    /**
     * @brief Get the current connection state
     * @return The current connection state
     */
    fun getConnectionState() : Int {
        return connectionState
    }

    /**
     * @brief Write a Gatt characteristic
     * @param uuidService The UUID of the Gatt service
     * @param uuidChar The UUID of the Gatt characteristic
     * @param writeType The type of write operation (WRITE_TYPE_DEFAULT or WRITE_TYPE_NO_RESPONSE)
     * @param payload The byte array to be written to the characteristic
     */
    fun writeCharacteristics(uuidService: UUID, uuidChar: UUID, writeType: Int, payload: ByteArray) {
        bluetoothGatt?.let { gatt ->
            // Check and get permissions
            if (!blePermission.checkBlePermission(this)) {
                Log.w(TAG, "Bluetooth permissions requested")
                broadcastUpdate(ACTION_REQUIRE_PERMISSIONS)
                return
            }

            val gattService : BluetoothGattService = gatt.getService(uuidService)
            if (gattService != null) {
                val serviceChar = gattService.getCharacteristic(uuidChar)
                if (serviceChar != null) {
                    serviceChar.writeType = writeType
                    serviceChar.value = payload
                    gatt.writeCharacteristic(serviceChar)
                }
                else {
                    Log.w(TAG, "Characteristic not found")
                }
            }
            else {
                Log.w(TAG, "Service not found")
            }
        } ?: error("Not connected to a BLE device!")
    }

    companion object {
        /** @brief Debug TAG */
        private const val TAG = "BleService"
        /** @brief Broadcast message: indicate a new connection to a GATT server */
        const val ACTION_GATT_CONNECTED = "com.example.ble_control.ACTION_GATT_CONNECTED"
        /** @brief Broadcast message: indicate a disconnection from a GATT server */
        const val ACTION_GATT_DISCONNECTED = "com.example.ble_control.ACTION_GATT_DISCONNECTED"
        /** @brief Broadcast message: request BLE runtime permissions */
        const val ACTION_REQUIRE_PERMISSIONS = "com.example.ble_control.ACTION_REQUIRE_PERMISSIONS"
        /** @brief  Enum for internal connection state - disconnected */
        const val STATE_DISCONNECTED = 0
        /** @brief  Enum for internal connection state - connected */
        const val STATE_CONNECTED = 2
    }
}
