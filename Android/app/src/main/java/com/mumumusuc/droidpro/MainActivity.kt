package com.mumumusuc.droidpro

import android.os.Bundle
import android.support.v7.app.AppCompatActivity
import android.transition.TransitionInflater
import android.view.View
import android.view.View.GONE
import android.view.View.VISIBLE
import android.widget.ProgressBar
import com.mumumusuc.droidpro.eventbus.JcConnectEvent
import com.mumumusuc.droidpro.eventbus.JcConnectStatus
import com.mumumusuc.droidpro.fragment.Connection
import com.mumumusuc.droidpro.fragment.Gamepad
import kotlinx.android.synthetic.main.activity_main.*
import org.greenrobot.eventbus.EventBus
import org.greenrobot.eventbus.Subscribe
import org.greenrobot.eventbus.ThreadMode


class MainActivity : AppCompatActivity() {
    private val jcProxy by lazy { JoyconProxy(this) }
    private val gamepad by lazy { Gamepad() }
    private val splash by lazy { Connection() }
    private val moveTransform by lazy { TransitionInflater.from(this).inflateTransition(android.R.transition.move) }
    private val fadeTransform by lazy { TransitionInflater.from(this).inflateTransition(android.R.transition.fade) }
    private val slideDownTransform by lazy {
        TransitionInflater.from(this).inflateTransition(android.R.transition.slide_bottom)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        jcProxy.init()
        supportFragmentManager
            .beginTransaction()
            .add(R.id.container, splash, splash::class.java.simpleName)
            .commit()
    }

    override fun onResume() {
        super.onResume()
        EventBus.getDefault().register(this)
        jcProxy.resume()
        container.systemUiVisibility =
            View.SYSTEM_UI_FLAG_FULLSCREEN or
                    View.SYSTEM_UI_FLAG_LOW_PROFILE or
                    View.SYSTEM_UI_FLAG_LAYOUT_STABLE or
                    View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY or
                    View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION or
                    View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
    }

    override fun onPause() {
        super.onPause()
        EventBus.getDefault().unregister(this)
        jcProxy.pause()
    }

    override fun onDestroy() {
        super.onDestroy()
        jcProxy.release()
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    fun onConectStatus(ev: JcConnectEvent) {
        when (ev.status) {
            JcConnectStatus.CONNECTING -> {
                splash.view?.findViewById<ProgressBar>(R.id.wait_connect)?.visibility = VISIBLE
            }
            JcConnectStatus.CONNECTED -> {
                splash.view?.findViewById<ProgressBar>(R.id.wait_connect)?.visibility = GONE
                switchToGamepad()
            }
            JcConnectStatus.DISCONNECTING -> {
            }
            JcConnectStatus.DISCONNECTED -> {
                jcProxy.disconnect()
                switchToSplash()
                splash.view?.findViewById<ProgressBar>(R.id.wait_connect)?.visibility = GONE
            }
            JcConnectStatus.REQUEST -> {
                jcProxy.connect()
            }
        }
    }

    private fun switchToGamepad() {
        if (gamepad.isAdded)
            return
        splash.sharedElementReturnTransition = moveTransform
        splash.exitTransition = slideDownTransform
        gamepad.sharedElementEnterTransition = moveTransform
        gamepad.enterTransition = fadeTransform
        val sharedElem = findViewById<View>(R.id.ns_logo)
        supportFragmentManager.beginTransaction()
            .replace(R.id.container, gamepad)
            .addSharedElement(sharedElem, "ns_logo")
            .commit()
    }

    private fun switchToSplash() {
        if (splash.isAdded)
            return
        splash.sharedElementEnterTransition = moveTransform
        splash.enterTransition = slideDownTransform
        gamepad.sharedElementReturnTransition = moveTransform
        gamepad.exitTransition = fadeTransform
        val sharedElem = findViewById<View>(R.id.ns_logo)
        supportFragmentManager.beginTransaction()
            .replace(R.id.container, splash, splash::class.java.simpleName)
            .addSharedElement(sharedElem, "ns_logo")
            .commit()
    }

    companion object {
        private val TAG = MainActivity::class.java.simpleName
    }
}