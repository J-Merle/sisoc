#include <assert.h>
#include <pulse/subscribe.h>
#include <stdio.h>
#include <string.h>
#include <sys/poll.h>
#include <unistd.h>

#include "raylib.h"
#include "pulse/context.h"
#include "pulse/mainloop.h"
#include "pulse/pulseaudio.h"

#define SSC_APP_NAME "Sisoc"

void context_state_callback(pa_context *c, void *userdata);
void subscribe_cb(pa_context *c, pa_subscription_event_type_t t, uint32_t index, void *userdata);
void sink_cb(pa_context *c, const pa_sink_info *i, int eol, void *userdata);


typedef struct {
    char state_i18n[20];
    Color state_color;
} ssc_userdata;

int main(void) {

    const int screenWidth = 450;
    const int screenHeight = 800;

    pa_mainloop *mainloop = pa_mainloop_new();
    assert(mainloop);
    int *mainloop_retval = 0;
    pa_mainloop_api* api = pa_mainloop_get_api(mainloop);
    assert(api);

    pa_proplist *proplist = pa_proplist_new();
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, SSC_APP_NAME);
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ID, "org.jmerle.sisoc");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ICON_NAME, "audio-card");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_VERSION, "0.1");

    pa_context *context = pa_context_new_with_proplist(api, NULL, proplist);
    assert(context);

    InitWindow(screenWidth, screenHeight, SSC_APP_NAME);
    SetTargetFPS(60);

    ssc_userdata userdata = {
        .state_i18n = "Non connecté",
        .state_color = DARKGRAY
    };

    pa_context_set_state_callback(context, context_state_callback, &userdata);

    if (pa_context_connect(context, NULL, PA_CONTEXT_NOFAIL, NULL) < 0) {
        TraceLog(LOG_ERROR, "Could not connect to pulseaudio context");
        goto end;
    }


    while (!WindowShouldClose())
    {
        pa_mainloop_iterate(mainloop, 0, mainloop_retval);
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyReleased(KEY_R)) {
            TraceLog(LOG_INFO, "Key pressed");
        };
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawCircle(15, 15, 10, userdata.state_color);
        DrawText(userdata.state_i18n, 30, 5, 18, userdata.state_color);

        EndDrawing();
    }

end:
    CloseWindow();

    return 0;
}


void context_state_callback(pa_context *c, void *userdata) {
    switch (pa_context_get_state(c)) {
        case PA_CONTEXT_UNCONNECTED:
            TraceLog(LOG_INFO, "Context is UNCONNECTED");
            break;
        case PA_CONTEXT_CONNECTING:
            TraceLog(LOG_INFO, "Context is CONNECTING");
            break;
        case PA_CONTEXT_AUTHORIZING:
            TraceLog(LOG_INFO, "Context is AUTHORIZING");
            break;
        case PA_CONTEXT_SETTING_NAME:
            TraceLog(LOG_INFO, "Context is SETTING NAME");
            break;
        case PA_CONTEXT_READY: 
            TraceLog(LOG_INFO, "Context is READY");
            ((ssc_userdata*) userdata)->state_color = GREEN;
            strcpy(((ssc_userdata*) userdata)->state_i18n, "Connecté");

            pa_context_set_subscribe_callback(c, subscribe_cb, userdata);
            pa_operation *o;

            if (!(o = pa_context_get_sink_info_list(c, sink_cb, userdata))) {
                return;
            }
            pa_operation_unref(o);
            break;
        case PA_CONTEXT_FAILED:
            TraceLog(LOG_ERROR, "Context state is FAILED");
            ((ssc_userdata*) userdata)->state_color = RED;
            strcpy(((ssc_userdata*) userdata)->state_i18n, "Erreur");
            c = NULL;
            return;
        case PA_CONTEXT_TERMINATED:
        default:
            TraceLog(LOG_ERROR, "Context state is TERMINATED");
            return;
    }
}

void subscribe_cb(pa_context *c, pa_subscription_event_type_t t, uint32_t index, void *userdata) {
    switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) {
        case PA_SUBSCRIPTION_EVENT_SINK :
            TraceLog(LOG_INFO, "EVENT SINK");
            break;
        case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
            TraceLog(LOG_INFO, "EVENT SINK");
            break;
        case PA_SUBSCRIPTION_EVENT_CLIENT:
            TraceLog(LOG_INFO, "EVENT CLIENT");
            break;
        default :
            TraceLog(LOG_INFO, "OTHER EVENT");
            break;
    }

}

void sink_cb(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
    TraceLog(LOG_INFO, "New sink registered");
}
