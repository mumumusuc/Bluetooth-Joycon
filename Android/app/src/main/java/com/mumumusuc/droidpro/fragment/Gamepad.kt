package com.mumumusuc.droidpro.fragment

import android.content.Context
import android.os.Bundle
import android.os.Vibrator
import android.support.v4.app.Fragment
import android.util.Log
import android.view.LayoutInflater
import android.view.MotionEvent
import android.view.MotionEvent.ACTION_DOWN
import android.view.MotionEvent.ACTION_UP
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import com.mumumusuc.droidpro.R
import com.mumumusuc.droidpro.eventbus.*
import com.mumumusuc.joycon.Joycon
import com.mumumusuc.joycon.Joycon.NsButton
import io.github.controlwear.virtual.joystick.android.JoystickView
import org.greenrobot.eventbus.EventBus
import org.greenrobot.eventbus.Subscribe
import org.greenrobot.eventbus.ThreadMode

class Gamepad : Fragment() {
    private val vibrator by lazy { context!!.getSystemService(Context.VIBRATOR_SERVICE) as Vibrator }
    private val onButtonTouched = object : View.OnTouchListener {
        override fun onTouch(view: View, ev: MotionEvent): Boolean {
            if (view.tag !is NsButton) {
                return view.onTouchEvent(ev)
            }
            val mask = view.tag as NsButton
            when (ev.action) {
                ACTION_DOWN -> {
                    vibrator.vibrate(100)
                    EventBus.getDefault().post(JcButtonEvent(mask, true))
                }
                ACTION_UP -> EventBus.getDefault().post(JcButtonEvent(mask, false))
                else -> return false
            }
            return false
        }
    }

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        val root = inflater.inflate(R.layout.gamepad, container, false)
        if (root is ViewGroup) {
            for (i in 0 until root.childCount) {
                val child = root.getChildAt(i)
                val tag = child.tag
                if (tag is String) {
                    child.tag = NsButton.valueOf(tag)
                    child.setOnTouchListener(onButtonTouched)
                }
            }
        }
        root.findViewById<View>(R.id.ns_logo).setOnClickListener {
            EventBus.getDefault().post(JcInputEvent(0x30))
        }
        root.findViewById<JoystickView>(R.id.left_stick).setOnMoveListener { angle, strength ->
            val jsHalf = Joycon.STICK_MAX_VALUE / 2
            val x = (jsHalf * strength / 100.0 * Math.cos(angle / 180.0 * Math.PI)).toInt() + jsHalf
            val y = (jsHalf * strength / 100.0 * Math.sin(angle / 180.0 * Math.PI)).toInt() + jsHalf
            EventBus.getDefault().post(JcStickEvent(Pair(x, y), null))
        }
        root.findViewById<JoystickView>(R.id.right_stick).setOnMoveListener { angle, strength ->
            val jsHalf = Joycon.STICK_MAX_VALUE / 2
            val x = (jsHalf * strength / 100.0 * Math.cos(angle / 180.0 * Math.PI)).toInt() + jsHalf
            val y = (jsHalf * strength / 100.0 * Math.sin(angle / 180.0 * Math.PI)).toInt() + jsHalf
            EventBus.getDefault().post(JcStickEvent(null, Pair(x, y)))
        }
        return root
    }

    override fun onAttach(context: Context?) {
        super.onAttach(context)
        EventBus.getDefault().register(this)
    }

    override fun onDetach() {
        super.onDetach()
        EventBus.getDefault().unregister(this)
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    fun onPlayerChangeEvent(ev: JcPlayerChangeEvent) {
        view?.findViewById<ImageView>(R.id.player)?.setImageLevel(ev.player)
    }
}