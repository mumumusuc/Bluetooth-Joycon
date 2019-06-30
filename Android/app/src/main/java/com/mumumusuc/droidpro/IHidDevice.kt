package com.mumumusuc.droidpro

import android.bluetooth.BluetoothHidDevice

interface IHidClient {
    fun register()
    fun unregister()
    fun resume(): Boolean
    fun pause()
    fun connect(name: String): Boolean
    fun disconnect(): Boolean
    fun sendInputReport(reportId: Int, data: ByteArray): Boolean
}

abstract class HidDeviceConfig {
    open val SERVICE_NAME: String = "BlueHID"
    open val SERVICE_DESC: String = "Android HID"
    open val SERVICE_PROV: String = "Android"
    open val SUBCLASS: Byte = BluetoothHidDevice.SUBCLASS1_NONE
    protected open val BCD_VERSION: Int get() = 0x0111
    protected open val COUNTRY_CODE: Byte get() = 0x00
    protected open val DESC_NUM: Byte get() = 0x01
    protected open val DESC_TYEP: Byte get() = 0x22
    protected abstract val DESC_LEN: Int
    protected abstract val DESCRIPTOR: ByteArray

    private fun int2Le(uint16: Int): ByteArray {
        return byteArrayOf(uint16.and(0xFF).toByte(), uint16.shr(8).and(0xFF).toByte())
    }

    fun descriptor(): ByteArray {
        val r = mutableListOf<Byte>(0x09/*bLength*/, 0x21 /*bDescriptorType*/)
        r.addAll(int2Le(BCD_VERSION).asList())
        r.add(COUNTRY_CODE)
        r.add(DESC_NUM)
        r.add(DESC_TYEP)
        r.addAll(int2Le(DESC_LEN).toList())
        r.addAll(DESCRIPTOR.toList())
        return r.toByteArray()
    }
}