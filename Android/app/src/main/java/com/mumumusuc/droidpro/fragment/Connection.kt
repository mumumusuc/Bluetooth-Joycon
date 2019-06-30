package com.mumumusuc.droidpro.fragment

import android.os.Bundle
import android.support.v4.app.Fragment
import android.view.LayoutInflater
import android.view.MotionEvent
import android.view.View
import android.view.ViewGroup
import com.mumumusuc.droidpro.R
import com.mumumusuc.droidpro.eventbus.JcButtonEvent
import com.mumumusuc.droidpro.eventbus.JcConnectEvent
import com.mumumusuc.droidpro.eventbus.JcConnectStatus
import com.mumumusuc.droidpro.eventbus.JcOutputEvent
import com.mumumusuc.joycon.Joycon
import org.greenrobot.eventbus.EventBus


class Connection : Fragment() {
    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View {
        val root = inflater.inflate(R.layout.connection, container, false)
        root.findViewById<View>(R.id.device_label)?.setOnClickListener {
            EventBus.getDefault().post(JcConnectEvent(JcConnectStatus.REQUEST))
        }

        root.findViewById<View>(R.id.ns_logo)?.setOnTouchListener { view: View, motionEvent: MotionEvent ->
            when (motionEvent.action) {
                MotionEvent.ACTION_DOWN -> {
                    EventBus.getDefault().post(JcButtonEvent(Joycon.NsButton.A, true))
                    EventBus.getDefault().post(JcOutputEvent(0x30, byteArrayOf()))
                    return@setOnTouchListener true
                }
                MotionEvent.ACTION_UP -> {
                    EventBus.getDefault().post(JcButtonEvent(Joycon.NsButton.A, false))
                    EventBus.getDefault().post(JcOutputEvent(0x30, byteArrayOf()))
                    return@setOnTouchListener true
                }
                else -> return@setOnTouchListener false
            }
        }
        return root
    }
}