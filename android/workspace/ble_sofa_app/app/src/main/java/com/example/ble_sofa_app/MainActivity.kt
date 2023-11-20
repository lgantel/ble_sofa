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
-- File Name: MainActivity.tk
-- Description: Main activity
--
-- Last update: 2023-09-09
--
-------------------------------------------------------------------------------*/
package com.example.ble_sofa_app

import android.bluetooth.BluetoothGattCharacteristic
import android.content.BroadcastReceiver
import android.content.ComponentName
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.content.ServiceConnection
import android.content.pm.PackageManager
import android.graphics.Color
import android.os.Bundle
import android.os.Handler
import android.os.IBinder
import android.util.Log
import android.widget.AdapterView.OnItemClickListener
import android.widget.ArrayAdapter
import android.widget.Button
import android.widget.ListView
import android.widget.TextView
import androidx.activity.ComponentActivity
import java.util.UUID

class MainActivity : ComponentActivity() {
    private lateinit var bleScanner : BleScanner

    // Service for Bluetooth operations
    private var bleService : BleService? = null

    // BLE scanning state
    private var scanning : Boolean = false

    // BLE connection state
    private var connected : Boolean = false

    // Relay states
    private var relay1State : Boolean = false
    private var relay2State : Boolean = false

    // List of scanned devices
    private var deviceList : MutableList<String> = mutableListOf()
    private var arrayAdapter : ArrayAdapter<String>? = null

    // BLE permissions manager
    private var blePermission : BlePermissions = BlePermissions()

    //--------------------------------
    // Activity life cycle
    //--------------------------------

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // Update connection state textview
        updateConnectionState(R.string.disconnected)

        val scanButton = findViewById<Button>(R.id.scanBtn)
        scanButton.setOnClickListener {
            if (!scanning) {
                Log.d("console", "Start scanning...")
                scanButton.setText(R.string.stop_scan)
                bleScanner.startScan()
                scanning = true
            }
            else {
                Log.d("console", "Stop scanning.")
                bleScanner.stopScan()
                scanButton.setText(R.string.start_scan)
                scanning = false
            }
        }

        arrayAdapter = ArrayAdapter(this, R.layout.bledevicelist_item, deviceList)

        var listView: ListView = findViewById<ListView>(R.id.bleDevicesList)
        listView.adapter = arrayAdapter
        listView.onItemClickListener =
            OnItemClickListener { parent, view, position, id ->
                // Get the device address
                val itemValue = listView.getItemAtPosition(position) as String
                val pattern = Regex("(?<=\\().+?(?=\\))")
                val address = pattern.findAll(itemValue).map{it.value}.toList()[0]

                Log.d(TAG, "Click on device: $address")

                // Check BLE service availability
                if (bleService != null) {
                    Log.d(TAG, "Check connectivity")
                    // If no device is connected, connect it
                    if (bleService!!.getConnectionState() == BleService.STATE_DISCONNECTED) {
                        Log.d(TAG, "Connect to device")
                        if (bleService!!.connect(address)) {
                            view.setBackgroundColor(Color.GREEN)
                        }
                    }
                    else {
                        Log.d(TAG, "Disconnect from device")
                        if (bleService!!.close()) {
                            view.setBackgroundColor(Color.WHITE)
                        }
                    }
                }
            }

        // Create the BLE Scanner instance
        bleScanner = BleScanner(this, Handler())

        // Get the BLE service intent for communication
        val gattServiceIntent = Intent(this, BleService::class.java)
        // Bind with the service
        if (bindService(gattServiceIntent, serviceConnection, Context.BIND_AUTO_CREATE)) {
            Log.d(TAG, "GATT service bound")
        }
        else {
            Log.d(TAG, "Failed to bind GATT service")
        }

        // Define and setup buttons to control the relays
        val relay1Button = findViewById<Button>(R.id.relay1Btn);
        updateButtonState(relay1Button, false, R.string.relay1_on)
        val relay2Button = findViewById<Button>(R.id.relay2Btn);
        updateButtonState(relay2Button, false, R.string.relay2_on)

        relay1Button.setOnClickListener {
            if (!relay1State) {
                Log.d(TAG, "Turn on Relay1...")
                this.writeRelaysState(byteArrayOf(0x01))
                relay1State = true
                updateButtonState(relay1Button, true, R.string.relay1_off)
                // Turning relay1 on automatically turns relay2 off
                relay2State = false
                updateButtonState(relay2Button, false, R.string.relay2_on)
            }
            else {
                Log.d(TAG, "Turn off Relay1.")
                this.writeRelaysState(byteArrayOf(0x00))
                relay1State = false
                updateButtonState(relay1Button, false, R.string.relay1_on)
            }
        }

        relay2Button.setOnClickListener {
            if (!relay2State) {
                Log.d(TAG, "Turn on Relay2...")
                this.writeRelaysState(byteArrayOf(0x02))
                relay2State = true
                updateButtonState(relay2Button, true, R.string.relay2_off)
                // Turning relay2 on automatically turns relay1 off
                relay1State = false
                updateButtonState(relay1Button, false, R.string.relay1_on)
            }
            else {
                Log.d(TAG, "Turn off Relay2.")
                this.writeRelaysState(byteArrayOf(0x00))
                relay2State = false
                updateButtonState(relay2Button, false, R.string.relay2_on)
            }
        }
    }

    override fun onResume() {
        super.onResume()
        // Register the GATT update receiver to communicate with the BLE service
        registerReceiver(gattUpdateReceiver, makeGattUpdateIntentFilter())
    }

    override fun onPause() {
        super.onPause()
        // Unregister the GATT update receiver
        unregisterReceiver(gattUpdateReceiver)
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)

        Log.d("console", "onRequestPermissionResult")
        when (requestCode) {
            BlePermissions.REQUEST_PERMISSION_CODE -> {
                Log.d(TAG, "grantResults.isNotEmpty() = ${grantResults.isNotEmpty()}")
                Log.d(TAG, "grantResults[0] = ${grantResults[0]}")
                if (grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    Log.d(TAG, "Permission granted: Start scanning")
                    bleScanner.startScan()
                }
            }
        }
    }

    //--------------------------------
    // UI management
    //--------------------------------

    /**
     * @brief Method to be called by the BLE scanner to update the list of the scanned devices
     */
    fun notifyDeviceListUpdate() {
        // Check BLE permissions
        if (!blePermission.checkBlePermission(this)) {
            blePermission.requestBlePermissions(this)
        }
        // Update devices list
        deviceList.clear()
        val itemDevices = bleScanner.getDeviceList()
        itemDevices.forEach {itemDevice ->
            deviceList.add("${itemDevice.value.name} (${itemDevice.key})")
        }
        arrayAdapter?.notifyDataSetChanged()
    }

    /**
     * @brief Update the textview indicating the connection state
     * @param connectionState A string indicating the new connection state
     */
    private fun updateConnectionState(connectionState : Int) {
        val textView = findViewById<TextView>(R.id.connectionState)
        textView.setText(connectionState)
    }

    /**
     * @brief Update a button interface
     * @param button The button to be updated
     * @param buttonState The new button state
     * @param updatedText The new text to be displayed on the button
     */
    private fun updateButtonState(button: Button, buttonState: Boolean, updatedText: Int) {
        button.setText(updatedText)
        if (buttonState) { button.setBackgroundColor(Color.GREEN) }
        else { button.setBackgroundColor(Color.LTGRAY) }
    }

    //--------------------------------
    // BLE Service management
    //--------------------------------

    /**
     * @brief Code to manage Service lifecycle
     */
    private val serviceConnection : ServiceConnection = object : ServiceConnection {
        override fun onServiceConnected(
            componentName: ComponentName,
            service: IBinder
        ) {
            bleService = (service as BleService.LocalBinder).getService()
            bleService?.let { bluetooth ->
                // Initialize the service
                if (!bluetooth.initialize()) {
                    Log.e(TAG, "Unable to initialize Bluetooth")
                }
                // Perform device connection
                Log.d(TAG, "Bluetooth service initialized")
            }
        }

        override fun onServiceDisconnected(componentName: ComponentName) {
            bleService = null
        }
    }

    /**
     * @brief Broadcast communication with the BLE service
     */
    private val gattUpdateReceiver : BroadcastReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            when (intent.action) {
                BleService.ACTION_GATT_CONNECTED -> {
                    connected = true
                    updateConnectionState(R.string.connected)
                }
                BleService.ACTION_GATT_DISCONNECTED -> {
                    connected = false
                    updateConnectionState(R.string.disconnected)
                }
                BleService.ACTION_REQUIRE_PERMISSIONS -> {
                    blePermission.requestBlePermissions(this@MainActivity)
                }
            }
        }
    }

    /**
     * @brief Intent filter for broadcast communication with the BLE service
     */
    private fun makeGattUpdateIntentFilter() : IntentFilter? {
        return IntentFilter().apply {
            addAction(BleService.ACTION_GATT_CONNECTED)
            addAction(BleService.ACTION_GATT_DISCONNECTED)
        }
    }

    //--------------------------------
    // BLE relays control operations
    //--------------------------------

    /**
     * Setup the relays state using the dedicated GATT service
     */
    private fun writeRelaysState(payload: ByteArray) {
        // Primary service
        val relaysServiceUUID = UUID.fromString("0000ff10-0000-1000-8000-00805f9b34fb")
        // GATT characteristic
        val relaysStateCharUUID = UUID.fromString("0000ff11-0000-1000-8000-00805f9b34fb")
        // Write without response
        val writeType = BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE

        bleService?.writeCharacteristics(relaysServiceUUID, relaysStateCharUUID, writeType, payload)
    }

    companion object {
        /** @brief Debug TAG */
        private const val TAG = "MainActivity"
    }
}
