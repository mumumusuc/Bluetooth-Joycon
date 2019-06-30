package com.mumumusuc.droidpro

import java.lang.StringBuilder

object Utils {
    private val sb by lazy { StringBuilder() }
    fun <TYPE> Array2String(fmt: String, data: Collection<TYPE>): String {
        sb.clear()
        for (i in 0 until data.size) {
            if (i in 61..320)
                continue
            if (i == 60)
                sb.append("... ")
            else
                sb.append(String.format(fmt, data.elementAt(i)))
        }
        return sb.toString()
    }
}