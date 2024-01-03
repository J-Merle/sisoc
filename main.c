#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <sys/poll.h>
#include <unistd.h>

#include "raylib.h"
#include "pulse/context.h"
#include "pulse/mainloop.h"

#define SSC_APP_NAME "Sisoc"

void context_state_callback(pa_context *c, void *userdata);


typedef struct {
    char state_i18n[20];
    Color state_color;
} ssc_userdata;

int main(void) {

    const int screenWidth = 800;
    const int screenHeight = 450;

    pa_context *context = NULL;
    pa_mainloop *mainloop = NULL;
    mainloop = pa_mainloop_new();
    pa_mainloop_api* api = pa_mainloop_get_api(mainloop);


    pa_proplist *proplist = pa_proplist_new();

    InitWindow(screenWidth, screenHeight, SSC_APP_NAME);

    SetTargetFPS(60);

    // Pulseaudio initialization

    assert(api);

    pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, SSC_APP_NAME);
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ID, "org.jmerle.sisoc");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ICON_NAME, "audio-card");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_VERSION, "0.1");

    context = pa_context_new_with_proplist(api, NULL, proplist);
    assert(context);

    ssc_userdata userdata = {
        .state_i18n = "Non connecté",
        .state_color = DARKGRAY
    };

    pa_context_set_state_callback(context, context_state_callback, &userdata);

    if (pa_context_connect(context, NULL, PA_CONTEXT_NOFAIL, NULL) < 0) {
        TraceLog(LOG_ERROR, "Could not connect to pulseaudio context");
        goto end;
    }


    int *mainloop_retval = 0;
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
