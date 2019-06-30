package com.mumumusuc.droidpro

import android.content.Context
import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import android.os.Handler
import android.os.Message
import android.util.Log
import com.mumumusuc.droidpro.eventbus.*
import com.mumumusuc.joycon.Joycon
import org.greenrobot.eventbus.EventBus
import org.greenrobot.eventbus.Subscribe
import java.util.*

class JoyconProxy(
    private val context: Context
) {
    private val joycon = Joycon()
    private val hidClient = BtHidClient(context, ProBtGamepad())
    private val sensorManager by lazy { context.getSystemService(Context.SENSOR_SERVICE) as SensorManager }
    private val handler = Handler {
        when (it.what) {
            MSG_INPUT -> {
                val reportId = it.arg1
                makeupInputReport2(reportId)
                val input = joycon.getInputBuffer()
                Log.d(TAG, "${input.first} ${Utils.Array2String("%02x ", input.second.toList())}")
                hidClient.sendInputReport(input.first, input.second)
            }
            MSG_OUTPUT -> {
                val reportId = it.arg1
                val data = it.obj as ByteArray
                replayOutputReport2(reportId, data)
                val input = joycon.getInputBuffer()
                hidClient.sendInputReport(input.first, input.second)
            }
        }
        return@Handler false
    }
    private val timer = JcTimer(handler)

    fun init() {
        EventBus.getDefault().register(this)
        joycon.init(context.assets, SPI_BIN_PATH)
        joycon.setStatusChangeListener(object : Joycon.JcStatusListener {
            override fun onImuEnable(enable: Boolean) {
            }

            override fun onVibratorEnable(enable: Boolean) {
            }

            override fun onStartPush() {
                timer.standardInputReportId = 0x30
                timer.startTimer(16)
            }

            override fun onFetchNfcIr() {
                timer.standardInputReportId = 0x31
                timer.startTimer(16)
            }

            override fun onPlayerChange(index: Int) {
                EventBus.getDefault().post(JcPlayerChangeEvent(index))
            }
        })
        hidClient.register()
    }

    fun release() {
        EventBus.getDefault().unregister(this)
        joycon.nativeFree()
        hidClient.unregister()
    }

    fun resume() {
        hidClient.resume()
        sensorManager.registerListener(
            sensorListener,
            sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER),
            SensorManager.SENSOR_DELAY_GAME
        )
        sensorManager.registerListener(
            sensorListener,
            sensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE),
            SensorManager.SENSOR_DELAY_GAME
        )
    }

    fun pause() {
        hidClient.pause()
        timer.stopTimer()
        sensorManager.unregisterListener(sensorListener)
    }

    fun connect() {
        hidClient.connect(TARGET)
    }

    fun disconnect() {
        hidClient.disconnect()
        timer.stopTimer()
    }

    private fun replayOutputReport2(reportId: Int, data: ByteArray) {
        joycon.setOutputBuffer(reportId, data)
        joycon.nativeReplayOutputReport()
    }

    private fun makeupInputReport2(reportId: Int) {
        joycon.setInputBuffer(reportId)
        joycon.nativeMakeupInputReport()
    }

    @Subscribe
    fun onButtonEvent(ev: JcButtonEvent) {
        joycon.setButtons(Pair(ev.mask, ev.value))
    }

    @Subscribe
    fun onStickEvent(ev: JcStickEvent) {
        if (ev.ls != null)
            joycon.setLStick(ev.ls.first, ev.ls.second)
        if (ev.rs != null)
            joycon.setRStick(ev.rs.first, ev.rs.second)
    }

    @Subscribe
    fun onInputEvent(ev: JcInputEvent) {
        /* take it easy */
        Message.obtain(handler, MSG_INPUT, ev.reportId, 0).sendToTarget()
    }

    @Subscribe
    fun onOutputEvent(ev: JcOutputEvent) {
        /* take it easy */
        Message.obtain(handler, MSG_OUTPUT, ev.reportId, 0, ev.data).sendToTarget()
    }

    @Subscribe
    fun onImuEvent(ev: JcImuEvent) {
        if (ev.accl != null) {
            joycon.setAccelerate(ev.accl[0], ev.accl[1], ev.accl[2])
        }
        if (ev.gyro != null) {
            joycon.setGyroscope(ev.gyro[0], ev.gyro[1], ev.gyro[2])
        }
    }

    private val sensorListener = object : SensorEventListener {
        override fun onAccuracyChanged(p0: Sensor?, p1: Int) {
        }

        override fun onSensorChanged(ev: SensorEvent) {
            if (ev.sensor.type == Sensor.TYPE_ACCELEROMETER) {
                val x = -ev.values[0] / 0.000244
                val y = -ev.values[1] / 0.000244
                val z = -ev.values[2] / 0.000244
                joycon.setAccelerate(x.toInt(), y.toInt(), z.toInt())
                //Log.d(TAG, "[accl] ${ev.values[0]} , ${ev.values[1]} , ${ev.values[2]}")
            } else if (ev.sensor.type == Sensor.TYPE_GYROSCOPE) {
                val x = ev.values[0] / 0.06103 * 10
                val y = ev.values[1] / 0.06103 * 10
                val z = ev.values[2] / 0.06103 * 10
                joycon.setGyroscope(x.toInt(), y.toInt(), z.toInt())
                //Log.d(TAG, "[gyro] ${ev.values[0]} , ${ev.values[1]} , ${ev.values[2]}")
            }
        }
    }

    class JcTimer(val handler: Handler) {
        private var task: TimerTask? = null
        private var timer: Timer? = null
        var standardInputReportId = 0x30

        fun startTimer(period: Long) {
            if (task != null || timer != null) {
                stopTimer()
            }
            val newTask = object : TimerTask() {
                override fun run() {
                    Message.obtain(handler, MSG_INPUT, standardInputReportId, 0).sendToTarget()
                }
            }
            val newTimer = Timer()
            newTimer.schedule(newTask, period, period)
            timer = newTimer
            task = newTask
        }

        fun stopTimer() {
            task?.cancel()
            timer?.cancel()
            task = null
            timer = null
        }
    }

    companion object {
        private val TAG = JoyconProxy::class.java.simpleName
        private const val SPI_BIN_PATH = "pro_spi.bin"
        private const val TARGET = "Nintendo Switch"
        private const val MSG_INPUT = 111
        private const val MSG_OUTPUT = 112
    }
}