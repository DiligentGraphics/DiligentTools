/*
 *  Copyright 2019-2022 Diligent Graphics LLC
 *  Copyright 2015-2019 Egor Yusov
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF ANY PROPRIETARY RIGHTS.
 *
 *  In no event and under no legal theory, whether in tort (including negligence), 
 *  contract, or otherwise, unless required by applicable law (such as deliberate 
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental, 
 *  or consequential damages of any character arising as a result of this License or 
 *  out of the use or inability to use the software (including but not limited to damages 
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and 
 *  all other commercial damages or losses), even if such Contributor has been advised 
 *  of the possibility of such damages.
 */

#include "AndroidAppBase.hpp"
#include "Errors.hpp"
#include "Timer.hpp"

#include <android/sensor.h>
//#include <android/log.h>
//#include <android_native_app_glue.h>
#include <android/native_window_jni.h>
//#include <cpu-features.h>

namespace Diligent
{

int AndroidAppBase::InitDisplay()
{
    if (!initialized_resources_)
    {
        Initialize();

        LoadResources();
        initialized_resources_ = true;
    }
    else
    {
        // initialize OpenGL ES and EGL
        if (EGL_SUCCESS != Resume(app_->window))
        {
            UnloadResources();
            LoadResources();
        }
    }

    ShowUI();

    //tap_camera_.SetFlip( 1.f, -1.f, -1.f );
    //tap_camera_.SetPinchTransformFactor( 2.f, 2.f, 8.f );

    return 0;
}

void AndroidAppBase::InitSensors()
{
    sensor_manager_       = ASensorManager_getInstance();
    accelerometer_sensor_ = ASensorManager_getDefaultSensor(sensor_manager_, ASENSOR_TYPE_ACCELEROMETER);
    sensor_event_queue_   = ASensorManager_createEventQueue(sensor_manager_, app_->looper, LOOPER_ID_USER, NULL, NULL);
}

//
// Just the current frame in the display.
//
void AndroidAppBase::DrawFrame()
{
    float fFPS;
    if (monitor_.Update(fFPS))
    {
        UpdateFPS(fFPS);
    }

    static Diligent::Timer Timer;

    static double PrevTime    = Timer.GetElapsedTime();
    auto          CurrTime    = Timer.GetElapsedTime();
    auto          ElapsedTime = CurrTime - PrevTime;

    PrevTime = CurrTime;

    Update(CurrTime, ElapsedTime);

    Render();

    Present();
    //if( EGL_SUCCESS != pRenderDevice_->Present() )
    //{
    //    UnloadResources();
    //    LoadResources();
    //}
}


//
// Process the next input event.
//
int32_t AndroidAppBase::HandleInput(android_app* app, AInputEvent* event)
{
    AndroidAppBase* eng = (AndroidAppBase*)app->userData;
    return eng->HandleInput(event);
}

//
// Process the next main command.
//
void AndroidAppBase::HandleCmd(struct android_app* app, int32_t cmd)
{
    AndroidAppBase* eng = (AndroidAppBase*)app->userData;
    switch (cmd)
    {
        case APP_CMD_INIT_WINDOW:
            LOG_INFO_MESSAGE("APP_CMD_INIT_WINDOW");
            app->window = app->pendingWindow;
            if (app->window &&
                ANativeWindow_getWidth(app->window) &&
                ANativeWindow_getHeight(app->window))
            {
                LOG_INFO_MESSAGE("INIT DISPLAY - HAS SURFACE");
                eng->InitDisplay();
                eng->DrawFrame();
                eng->AddAppStatusFlag(APP_STATUS_FLAG_HAS_REAL_SURFACE);
            }
            else
            {
                LOG_INFO_MESSAGE("NO SURFACE");
                eng->RemoveAppStatusFlag(APP_STATUS_FLAG_HAS_REAL_SURFACE);
            }
            break;

        case APP_CMD_TERM_WINDOW:
            LOG_INFO_MESSAGE("APP_CMD_TERM_WINDOW - LOST SURFACE - TERM DISPLAY");
            {
                eng->RemoveAppStatusFlag(APP_STATUS_FLAG_HAS_REAL_SURFACE);
            }
            eng->TermDisplay();
            break;

        // Note that as of NDK r21b (21.1.6352462), APP_CMD_CONTENT_RECT_CHANGED event is never
        // generated by android_native_app_glue
        case APP_CMD_CONTENT_RECT_CHANGED:
        {
            LOG_INFO_MESSAGE("APP_CMD_CONTENT_RECT_CHANGED");

            int32_t new_window_width =
                app->contentRect.right - app->contentRect.left;
            int32_t new_window_height =
                app->contentRect.bottom - app->contentRect.top;
            eng->WindowResize(new_window_width, new_window_height);
            break;
        }

        // Note that as of NDK r21b (21.1.6352462), APP_CMD_WINDOW_RESIZED event is never generated
        // by android_native_app_glue.
        // Also note that modifying android_native_app_glue to handle onNativeWindowResized
        // callback (as suggested in https://android-developers.googleblog.com/2020/02/handling-device-orientation-efficiently.html)
        // does not work either - the callback is only called once after the window has been created.
        case APP_CMD_WINDOW_RESIZED:
        {
            LOG_INFO_MESSAGE("APP_CMD_WINDOW_RESIZED");
            if (app->window)
            {
                int32_t new_window_width  = ANativeWindow_getWidth(app->window);
                int32_t new_window_height = ANativeWindow_getHeight(app->window);
                if (new_window_width && new_window_height)
                {
                    eng->WindowResize(new_window_width, new_window_height);
                }
            }
            break;
        }

        case APP_CMD_GAINED_FOCUS:
            LOG_INFO_MESSAGE("APP_CMD_GAINED_FOCUS - HAS FOCUS");
            {
                eng->AddAppStatusFlag(APP_STATUS_FLAG_FOCUSED);
            }
            eng->ResumeSensors();
            break;

        case APP_CMD_LOST_FOCUS:
            LOG_INFO_MESSAGE("APP_CMD_LOST_FOCUS - LOST FOCUS");
            {
                eng->RemoveAppStatusFlag(APP_STATUS_FLAG_FOCUSED);
            }
            eng->SuspendSensors();
            break;

        case APP_CMD_RESUME:
            LOG_INFO_MESSAGE("APP_CMD_RESUME - IS ACTIVE");
            {
                eng->AddAppStatusFlag(APP_STATUS_FLAG_ACTIVE);
            }
            break;

        case APP_CMD_START:
            LOG_INFO_MESSAGE("APP_CMD_START");
            break;

        case APP_CMD_PAUSE:
            LOG_INFO_MESSAGE("APP_CMD_PAUSE - IS NOT ACTIVE");
            {
                eng->RemoveAppStatusFlag(APP_STATUS_FLAG_ACTIVE);
            }
            break;

        case APP_CMD_STOP:
            LOG_INFO_MESSAGE("APP_CMD_STOP");
            break;

        case APP_CMD_CONFIG_CHANGED:
            LOG_INFO_MESSAGE("APP_CMD_CONFIG_CHANGED");
            // AConfiguration_fromAssetManager(app->config, app->activity->assetManager);
            // This callback is not reliable for handling orientation changes. Depending on the
            // device, it may be called before or after the surface has been actually resized.
            break;

        case APP_CMD_DESTROY:
            LOG_INFO_MESSAGE("APP_CMD_DESTROY - IS NOT RUNNING");
            {
                eng->RemoveAppStatusFlag(APP_STATUS_FLAG_RUNNING);
            }
            break;

        case APP_CMD_WINDOW_REDRAW_NEEDED:
            LOG_INFO_MESSAGE("APP_CMD_WINDOW_REDRAW_NEEDED");
            if (eng->IsReady()) eng->DrawFrame();
            break;

        case APP_CMD_SAVE_STATE:
            LOG_INFO_MESSAGE("APP_CMD_SAVE_STATE");
            break;

        case APP_CMD_LOW_MEMORY:
            LOG_INFO_MESSAGE("APP_CMD_LOW_MEMORY");
            //Free up GL resources
            eng->TrimMemory();
            break;
    }
}

//-------------------------------------------------------------------------
//Sensor handlers
//-------------------------------------------------------------------------
void AndroidAppBase::ProcessSensors(int32_t id)
{
    // If a sensor has data, process it now.
    if (id == LOOPER_ID_USER)
    {
        if (accelerometer_sensor_ != NULL)
        {
            ASensorEvent event;
            while (ASensorEventQueue_getEvents(sensor_event_queue_, &event, 1) > 0)
            {
            }
        }
    }
}

void AndroidAppBase::ResumeSensors()
{
    // When our app gains focus, we start monitoring the accelerometer.
    if (accelerometer_sensor_ != NULL)
    {
        ASensorEventQueue_enableSensor(sensor_event_queue_, accelerometer_sensor_);
        // We'd like to get 60 events per second (in us).
        ASensorEventQueue_setEventRate(sensor_event_queue_, accelerometer_sensor_,
                                       (1000L / 60) * 1000);
    }
}

void AndroidAppBase::SuspendSensors()
{
    // When our app loses focus, we stop monitoring the accelerometer.
    // This is to avoid consuming battery while not being used.
    if (accelerometer_sensor_ != NULL)
    {
        ASensorEventQueue_disableSensor(sensor_event_queue_, accelerometer_sensor_);
    }
}

//-------------------------------------------------------------------------
//Misc
//-------------------------------------------------------------------------
void AndroidAppBase::SetState(android_app* state, const char* native_activity_class_name)
{
    app_ = state;

    native_activity_class_name_ = native_activity_class_name;
    doubletap_detector_.SetConfiguration(app_->config);
    drag_detector_.SetConfiguration(app_->config);
    pinch_detector_.SetConfiguration(app_->config);
}

bool AndroidAppBase::IsReady()
{
    APP_STATUS_FLAGS app_status = GetAppStatus();
    return ValueHasAppStatusFlag(app_status, APP_STATUS_FLAG_RUNNING) &&
        ValueHasAppStatusFlag(app_status, APP_STATUS_FLAG_ACTIVE) &&
        ValueHasAppStatusFlag(app_status, APP_STATUS_FLAG_FOCUSED) &&
        ValueHasAppStatusFlag(app_status, APP_STATUS_FLAG_HAS_REAL_SURFACE);
}

//void Engine::TransformPosition( ndk_helper::Vec2& vec )
//{
//    vec = ndk_helper::Vec2( 2.0f, 2.0f ) * vec
//        / ndk_helper::Vec2( pDeviceContext_->GetMainBackBufferDesc().Width, pDeviceContext_->GetMainBackBufferDesc().Height )
//        - ndk_helper::Vec2( 1.f, 1.f );
//}

void AndroidAppBase::ShowUI()
{
    JNIEnv* jni;
    app_->activity->vm->AttachCurrentThread(&jni, NULL);

    //Default class retrieval
    jclass    clazz    = jni->GetObjectClass(app_->activity->clazz);
    jmethodID methodID = jni->GetMethodID(clazz, "showUI", "()V");
    jni->CallVoidMethod(app_->activity->clazz, methodID);

    app_->activity->vm->DetachCurrentThread();
    return;
}

void AndroidAppBase::UpdateFPS(float fFPS)
{
    JNIEnv* jni;
    app_->activity->vm->AttachCurrentThread(&jni, NULL);

    //Default class retrieval
    jclass    clazz    = jni->GetObjectClass(app_->activity->clazz);
    jmethodID methodID = jni->GetMethodID(clazz, "updateFPS", "(F)V");
    jni->CallVoidMethod(app_->activity->clazz, methodID, fFPS);

    app_->activity->vm->DetachCurrentThread();
    return;
}

AndroidAppBase::APP_STATUS_FLAGS AndroidAppBase::GetAppStatus()
{
    return app_status_.load();
}

void AndroidAppBase::AddAppStatusFlag(AndroidAppBase::APP_STATUS_FLAGS Flag)
{
    auto LastStoredValue = app_status_.load();
    while (!app_status_.compare_exchange_weak(LastStoredValue, static_cast<APP_STATUS_FLAGS>(LastStoredValue | Flag)))
    {
        // If exchange fails, LastStoredValue will hold the actual value of app_status_.
    }
}

void AndroidAppBase::RemoveAppStatusFlag(AndroidAppBase::APP_STATUS_FLAGS Flag)
{
    auto LastStoredValue = app_status_.load();
    while (!app_status_.compare_exchange_weak(LastStoredValue, static_cast<APP_STATUS_FLAGS>(LastStoredValue & ~Flag)))
    {
        // If exchange fails, LastStoredValue will hold the actual value of app_status_.
    }
}

bool AndroidAppBase::ValueHasAppStatusFlag(AndroidAppBase::APP_STATUS_FLAGS Value, AndroidAppBase::APP_STATUS_FLAGS Flag)
{
    return (Value & Flag) != APP_STATUS_FLAG_NONE;
}

bool AndroidAppBase::HasAppStatusFlag(AndroidAppBase::APP_STATUS_FLAGS Flag)
{
    return ValueHasAppStatusFlag(app_status_.load(), Flag);
}

} // namespace Diligent
