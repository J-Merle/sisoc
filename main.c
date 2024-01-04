#include <assert.h>
#include <pulse/subscribe.h>
#include <stdio.h>
#include <string.h>
#include <sys/poll.h>
#include <unistd.h>

#include "raylib.h"
#include <pulse/context.h>
#include <pulse/mainloop.h>
#include <pulse/pulseaudio.h>

#define SSC_APP_NAME "Sisoc"
#define SIZE_SINK_ARRAY 20

void context_state_callback(pa_context *c, void *data);
void subscribe_cb(pa_context *c, pa_subscription_event_type_t t, uint32_t index, void *data);
void sink_cb(pa_context *c, const pa_sink_info *i, int eol, void *data);


typedef struct {
    int index;
    char description[200];
} ssc_sink_info;



typedef struct {
    char state_i18n[20]; // TODO should be an enum 
    Color state_color;
    ssc_sink_info sinks[SIZE_SINK_ARRAY];
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
        .state_color = DARKGRAY,
        .sinks = {
            {-1, ""}, {-1, ""}, {-1, ""}, {-1, ""}, {-1, ""},
            {-1, ""}, {-1, ""}, {-1, ""}, {-1, ""}, {-1, ""},
            {-1, ""}, {-1, ""}, {-1, ""}, {-1, ""}, {-1, ""},
            {-1, ""}, {-1, ""}, {-1, ""}, {-1, ""}, {-1, ""},
        } // TODO this could be done with a macro or something else
    };

    // Init sinks with empty sinks

    

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
        int vertical_offset = 5;

        for(int i = 0; i < SIZE_SINK_ARRAY; i++) {

            if(userdata.sinks[i].index >= 0) {
                vertical_offset += 30;
                DrawText(userdata.sinks[i].description, 30, vertical_offset, 14, BLACK);
            }
        }

        EndDrawing();
    }

end:
    CloseWindow();
    // TODO free userdata ?

    return 0;
}


void context_state_callback(pa_context *c, void *data) {
    ssc_userdata *userdata = ((ssc_userdata*) data);
    switch (pa_context_get_state(c)) {
        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
            break;
        case PA_CONTEXT_READY: 
            TraceLog(LOG_INFO, "Context is READY");
            userdata->state_color = GREEN;
            strcpy(userdata->state_i18n, "Connecté");

            pa_context_set_subscribe_callback(c, subscribe_cb, userdata);
            pa_operation *o;

            if (!(o = pa_context_subscribe(c,
                                           (pa_subscription_mask_t)(PA_SUBSCRIPTION_MASK_SINK |
                                             PA_SUBSCRIPTION_MASK_SINK_INPUT |
                                             PA_SUBSCRIPTION_MASK_CLIENT
                                             ),
                                           NULL,
                                           NULL))) {
                TraceLog(LOG_ERROR, "Error will subscribing");
                return;
            }
            pa_operation_unref(o);

            if (!(o = pa_context_get_sink_info_list(c, sink_cb, userdata))) {
                return;
            }
            pa_operation_unref(o);
            break;
        case PA_CONTEXT_FAILED:
            TraceLog(LOG_ERROR, "Context state is FAILED");
            userdata->state_color = RED;
            strcpy(userdata->state_i18n, "Erreur");
            c = NULL;
            return;
        case PA_CONTEXT_TERMINATED:
        default:
            TraceLog(LOG_ERROR, "Context state is TERMINATED");
            return;
    }
}

void subscribe_cb(pa_context *c, pa_subscription_event_type_t t, uint32_t index, void *data) {
    ssc_userdata *userdata = ((ssc_userdata*) data);
    switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) {
        case PA_SUBSCRIPTION_EVENT_SINK :
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
                ssc_sink_info empty_sink = {-1, ""};
                userdata->sinks[index] = empty_sink;
            } else {
                pa_operation *o;
                if (!(o = pa_context_get_sink_info_by_index(c, index, sink_cb, userdata))) {
                    TraceLog(LOG_ERROR, "Error while retrieving sink info");
                    return;
                }
                pa_operation_unref(o);
                break;
            }
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

void sink_cb(pa_context *c, const pa_sink_info *info, int eol, void *userdata) {
    if(info) {
        if(eol != 0) return;
        if(info->index >= SIZE_SINK_ARRAY) {
            TraceLog(LOG_ERROR, "Sink index overflow");
            // TODO implement hash map to recover this
        }
        ssc_sink_info *sinks = ((ssc_userdata*) userdata)->sinks;
        ssc_sink_info new_sink_info = {.index = info->index};
        strcpy(new_sink_info.description, info->description);
        sinks[info->index] = new_sink_info;
        TraceLog(LOG_INFO, "Registered new sink [%d %s]", info->index, sinks[info->index].description);
        return;
    }
}
