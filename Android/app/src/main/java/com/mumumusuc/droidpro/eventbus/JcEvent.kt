package com.mumumusuc.droidpro.eventbus

import com.mumumusuc.joycon.Joycon.NsButton

enum class JcConnectStatus {
    CONNECTING, CONNECTED, DISCONNECTING, DISCONNECTED, REQUEST
}

class JcPlayerChangeEvent(val player: Int)

class JcConnectEvent(val status: JcConnectStatus)

class JcInputEvent(val reportId: Int)

class JcOutputEvent(val reportId: Int, val data: ByteArray)

class JcButtonEvent(val mask: NsButton, val value: Boolean)

class JcStickEvent(val ls: Pair<Int, Int>?, val rs: Pair<Int, Int>?)

class JcImuEvent(val accl: IntArray?, val gyro: IntArray?)