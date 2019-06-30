package com.mumumusuc.droidpro

import android.content.pm.PackageManager
import de.robv.android.xposed.*
import de.robv.android.xposed.callbacks.XC_InitPackageResources
import de.robv.android.xposed.callbacks.XC_LoadPackage

class XposedUtil : IXposedHookInitPackageResources {

    @Throws(Throwable::class)
    override fun handleInitPackageResources(resparam: XC_InitPackageResources.InitPackageResourcesParam) {
        if (resparam.packageName != TARGET_PKG)
            return
        PROFILE_CONFIGS.forEach {
            resparam.res.setReplacement(TARGET_PKG, "bool", it.first, it.second)
        }
    }
/*
    @Throws(Throwable::class)
    override fun handleLoadPackage(lpparam: XC_LoadPackage.LoadPackageParam) {
        if (lpparam.packageName != "com.android.server.pm"){
            XposedBridge.log("${lpparam.packageName} loaded")
            return
        }
        XposedBridge.log("com.android.server.pm.permission HOOKED")

        XposedHelpers.findAndHookMethod(
            "com.android.server.pm.permission.PermissionManagerService",
            lpparam.classLoader,
            "checkPermission",
            String::class.java, String::class.java, Int::class.java, Int::class.java,
            object : XC_MethodHook() {
                override fun afterHookedMethod(param: MethodHookParam) {
                    super.afterHookedMethod(param)
                    XposedBridge.log("before: ${param.args[0]} ${param.args[1]} ${param.args[2]} ${param.args[3]}")
                }

                override fun beforeHookedMethod(param: MethodHookParam) {
                    super.beforeHookedMethod(param)
                    XposedBridge.log("after:")
                }
            }
        )
    }
    */

    companion object {
        private val TARGET_PKG = "com.android.bluetooth"
        private val PROFILE_CONFIGS = arrayListOf(
            Pair("profile_supported_a2dp", false),
            Pair("profile_supported_a2dp_sink", false),
            Pair("profile_supported_hdp", false),
            Pair("profile_supported_hs_hfp", false),
            Pair("profile_supported_hfpclient", false),
            Pair("profile_supported_hid_host", true),
            Pair("profile_supported_opp", false),
            Pair("profile_supported_pan", false),
            Pair("profile_supported_pbap", false),
            Pair("profile_supported_gatt", false),
            Pair("profile_supported_map", false),
            Pair("profile_supported_avrcp_target", false),
            Pair("profile_supported_avrcp_controller", false),
            Pair("profile_supported_sap", false),
            Pair("profile_supported_pbapclient", false),
            Pair("profile_supported_mapmce", false),
            Pair("profile_supported_hearing_aid", false)
        )
    }
}