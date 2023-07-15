package com.example.ble_scanner

import android.content.pm.PackageManager
import android.os.Bundle
import android.os.Handler
import android.util.Log
import android.widget.ArrayAdapter
import android.widget.Button
import android.widget.ListView
import androidx.activity.ComponentActivity

class MainActivity : ComponentActivity() {
    private lateinit var bleScanner: BleScanner

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val startScanButton = findViewById<Button>(R.id.startScanBtn)
        val stopScanButton = findViewById<Button>(R.id.stopScanBtn)

        val users = arrayOf("Device 0", "Device 1", "Device 2")
        val arrayAdapter = ArrayAdapter(this, R.layout.bledevicelist_item, users)

        var listView: ListView = findViewById<ListView>(R.id.bleDevicesList)
        listView.adapter = arrayAdapter

        bleScanner = BleScanner(this, Handler())

        startScanButton.setOnClickListener {
            Log.d("console", "Start scanning...")
            bleScanner.startScan()
        }

        stopScanButton.setOnClickListener {
            Log.d("console", "Stop scanning.")
            bleScanner.stopScan()
        }


    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)

        Log.d("console", "onRequestPermissionResult")
        when (requestCode) {
            BleScanner.REQUEST_PERMISSION_CODE -> {
                Log.d("console", "grantResults.isNotEmpty() = ${grantResults.isNotEmpty()}")
                Log.d("console", "grantResults[0] = ${grantResults[0]}")
                if (grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    Log.d("console", "Permission granted: Start scanning")
                    bleScanner.startScan()
                }
            }
        }
    }
}
