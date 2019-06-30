package com.mumumusuc.droidpro

import android.bluetooth.*

import android.content.Context
import android.util.Log
import java.util.concurrent.Executors
import android.bluetooth.BluetoothProfile
import android.bluetooth.BluetoothDevice
import com.mumumusuc.droidpro.eventbus.JcConnectEvent
import com.mumumusuc.droidpro.eventbus.JcConnectStatus
import com.mumumusuc.droidpro.eventbus.JcOutputEvent
import org.greenrobot.eventbus.EventBus

class BtHidClient(
    private val context: Context,
    config: HidDeviceConfig
) : IHidClient, BluetoothProfile.ServiceListener {
    private var remoteDevice: BluetoothDevice? = null
    private var hidDevice: BluetoothHidDevice? = null
    private val btAdapter = BluetoothAdapter.getDefaultAdapter()
    private val executor = Executors.newCachedThreadPool()
    private val sdp = BluetoothHidDeviceAppSdpSettings(
        config.SERVICE_NAME,
        config.SERVICE_DESC,
        config.SERVICE_PROV,
        config.SUBCLASS,
        config.descriptor()
    )

    override fun onServiceConnected(profile: Int, proxy: BluetoothProfile) {
        if (profile == BluetoothProfile.HID_DEVICE) {
            val device = proxy as BluetoothHidDevice
            device.registerApp(sdp, null, null, executor, hidCallback)
            hidDevice = device
        }
    }

    override fun onServiceDisconnected(profile: Int) {
        if (profile == BluetoothProfile.HID_DEVICE) {
            val device = hidDevice
            device!!.unregisterApp()
            hidDevice = null
        }
    }

    private val hidCallback = object : BluetoothHidDevice.Callback() {
        override fun onSetReport(device: BluetoothDevice?, type: Byte, id: Byte, data: ByteArray?) {
            Log.d(TAG, "onSetReport")
        }

        override fun onSetProtocol(device: BluetoothDevice?, protocol: Byte) {
            Log.d(TAG, "onSetProtocol")
        }

        override fun onGetReport(device: BluetoothDevice, type: Byte, id: Byte, bufferSize: Int) {
            Log.d(TAG, "[hidCallback] onGetReport: device=$device type=$type id=$id bufferSize=$bufferSize")
        }

        override fun onConnectionStateChanged(device: BluetoothDevice, state: Int) {
            Log.d(TAG, "[hidCallback] onConnectionStateChanged: ${device.name}(${device.address})")
            when (state) {
                BluetoothProfile.STATE_DISCONNECTED -> {
                    Log.d(TAG, "STATE_DISCONNECTED")
                    remoteDevice = null
                    EventBus.getDefault().post(JcConnectEvent(JcConnectStatus.DISCONNECTED))
                }
                BluetoothProfile.STATE_CONNECTING -> {
                    Log.d(TAG, "STATE_CONNECTING")
                    device.setPin(byteArrayOf(0, 0, 0, 0))
                    EventBus.getDefault().post(JcConnectEvent(JcConnectStatus.CONNECTING))
                }
                BluetoothProfile.STATE_CONNECTED -> {
                    Log.d(TAG, "STATE_CONNECTED")
                    remoteDevice = device
                    EventBus.getDefault().post(JcConnectEvent(JcConnectStatus.CONNECTED))
                }
                BluetoothProfile.STATE_DISCONNECTING -> {
                    Log.d(TAG, "STATE_DISCONNECTING")
                    EventBus.getDefault().post(JcConnectEvent(JcConnectStatus.DISCONNECTING))
                }
            }
        }

        override fun onInterruptData(device: BluetoothDevice, reportId: Byte, data: ByteArray) {
            EventBus.getDefault().post(JcOutputEvent(reportId.toInt(), data))
        }
    }

    override fun register() {
        //context.registerReceiver(btPairReceiver, IntentFilter(BluetoothDevice.ACTION_PAIRING_REQUEST))
        btAdapter.name = "Pro Controller"
        btAdapter.getProfileProxy(context, this, BluetoothProfile.HID_DEVICE)
    }

    override fun unregister() {
        //context.unregisterReceiver(btPairReceiver)
        val hid = hidDevice ?: return
        btAdapter.closeProfileProxy(BluetoothProfile.HID_DEVICE, hid)
    }

    override fun connect(name: String): Boolean {
        for (dev in btAdapter.bondedDevices) {
            if (dev.name == name) {
                hidDevice?.disconnect(dev)
                val con = hidDevice?.connect(dev) ?: false
                Log.d(TAG, "find device $dev, connect = $con")
                return con
            }
        }
        Log.d(TAG, "$name not found")
        return false
    }

    override fun disconnect(): Boolean {
        val remote = remoteDevice ?: return false
        return hidDevice?.disconnect(remote) ?: false
    }

    override fun resume(): Boolean {
        var enabled = btAdapter.isEnabled
        if (!enabled)
            enabled = btAdapter.enable()
        hidDevice?.registerApp(sdp, null, null, executor, hidCallback)
        try {
            val clz = BluetoothAdapter::class.java
            val mtd = clz.getDeclaredMethod("setScanMode", Int::class.java)
            mtd.isAccessible = true
            mtd.invoke(btAdapter, BluetoothAdapter.SCAN_MODE_CONNECTABLE_DISCOVERABLE)
        } catch (e: Exception) {
            e.printStackTrace()
        }
        return enabled
    }

    override fun pause() {
        val remote = remoteDevice ?: return
        hidDevice?.disconnect(remote)
        try {
            val clz = BluetoothAdapter::class.java
            val mtd = clz.getDeclaredMethod("setScanMode", Int::class.java)
            mtd.isAccessible = true
            mtd.invoke(btAdapter, BluetoothAdapter.SCAN_MODE_NONE)
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    override fun sendInputReport(reportId: Int, data: ByteArray): Boolean {
        val hid = hidDevice ?: return false
        val rem = remoteDevice ?: return false
        return hid.sendReport(rem, reportId, data)
    }

    /*
        private val btPairReceiver = object : BroadcastReceiver() {
            override fun onReceive(context: Context, intent: Intent) {
                val action = intent.action
                Log.d(TAG, action)
                if (action != BluetoothDevice.ACTION_PAIRING_REQUEST)
                    return
                val remote = intent.getParcelableExtra<BluetoothDevice>(BluetoothDevice.EXTRA_DEVICE)
                Log.d(TAG,"require pairing : ${remote.name}[${remote.address}]")
                remote.setPairingConfirmation(true)
            }
        }
    */
    companion object {
        private val TAG = BtHidClient::class.java.simpleName
    }
}