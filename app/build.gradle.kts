plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

android {
    namespace 'org.neogeoemu.app'
    compileSdk 34
    buildToolsVersion "34.0.0"
    ndkVersion "27.0.0"

    defaultConfig {
        applicationId "org.neogeoemu.app"
        minSdk 26   // Android 8.0
        targetSdk 34
        versionCode 1
        versionName "0.1.0-alpha"

        testInstrumentationRunner "androidx.test.runner.AndroidJUnitRunner"

        ndk {
            abiFilters 'arm64-v8a', 'armeabi-v7a', 'x86_64'
        }

        externalNativeBuild {
            cmake {
                cppFlags '-std=c++17 -O2 -fexceptions'
                arguments '-DANDROID_TOOLCHAIN=clang',
                          '-DANDROID_STL=c++_shared',
                          '-DANDROID_ARM_MODE=arm'
            }
        }
    }

    buildTypes {
        release {
            minifyEnabled true
            proguardFiles getDefaultProguardFile('proguard-android-optimize.txt'), 'proguard-rules.pro'
            signingConfig signingConfigs.debug // TODO: release keystore
        }
        debug {
            isJniDebuggable = true
        }
    }

    compileOptions {
        sourceCompatibility JavaVersion.VERSION_17
        targetCompatibility JavaVersion.VERSION_17
    }

    kotlinOptions {
        jvmTarget = '17'
        freeCompilerArgs += [
            '-opt-in=androidx.compose.ui.ExperimentalComposeUiApi',
            '-opt-in=androidx.compose.material3.ExperimentalMaterial3Api'
        ]
    }

    buildFeatures {
        viewBinding true
        compose true
    }

    composeOptions {
        kotlinCompilerExtensionVersion '1.5.14'
    }

    externalNativeBuild {
        cmake {
            path file('src/main/cpp/CMakeLists.txt')
            version '3.22.1'
        }
    }

    packaging {
        resources {
            excludes += '/META-INF/{AL2.0,LGPL2.1}'
        }
    }
}

dependencies {
    // AndroidX Core
    implementation 'androidx.core:core-ktx:1.12.0'
    implementation 'androidx.appcompat:appcompat:1.6.1'
    implementation 'androidx.activity:activity-ktx:1.8.2'
    implementation 'androidx.fragment:fragment-ktx:1.6.2'

    // Material Design 3
    implementation 'com.google.android.material:material:1.11.0'

    // Compose
    implementation platform('androidx.compose:compose-bom:2024.02.00')
    implementation 'androidx.compose.ui:ui'
    implementation 'androidx.compose.ui:ui-graphics'
    implementation 'androidx.compose.ui:ui-tooling-preview'
    implementation 'androidx.compose.material3:material3'
    implementation 'androidx.compose.material:material-icons-extended'
    implementation 'androidx.activity:activity-compose:1.8.2'
    implementation 'androidx.lifecycle:lifecycle-viewmodel-compose:2.7.0'
    implementation 'androidx.navigation:navigation-compose:2.7.7'

    // Lifecycle
    implementation 'androidx.lifecycle:lifecycle-runtime-ktx:2.7.0'
    implementation 'androidx.lifecycle:lifecycle-viewmodel-ktx:2.7.0'
    implementation 'androidx.lifecycle:lifecycle-livedata-ktx:2.7.0'

    // Preferences / DataStore
    implementation 'androidx.datastore:datastore-preferences:1.0.0'

    // Coroutines
    implementation 'org.jetbrains.kotlinx:kotlinx-coroutines-android:1.8.0'

    // Image loading
    implementation 'io.coil-kt:coil-compose:2.6.0'

    // JSON
    implementation 'com.google.code.gson:gson:2.10.1'

    // Debug
    debugImplementation 'androidx.compose.ui:ui-tooling'
    debugImplementation 'androidx.compose.ui:ui-test-manifest'
}
