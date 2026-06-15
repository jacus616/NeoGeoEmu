package org.neogeoemu.app

import android.app.Application

class NeoGeoEmuApp : Application() {

    companion object {
        const val TAG = "NeoGeoEmu"
    }

    override fun onCreate() {
        super.onCreate()
        instance = this
    }

    companion object {
        lateinit var instance: NeoGeoEmuApp
            private set
    }
}
