#include <pjsua-lib/pjsua.h>
#include <stdio.h>

#define THIS_FILE "ivr_app.c"

static pjsua_call_id original_call_c_id = PJSUA_INVALID_ID;
static pjsua_call_id consultation_call_c_id = PJSUA_INVALID_ID;


/* Function to put the call on hold */
void hold_call(pjsua_call_id call_id) {
    pj_status_t status;
    
    /* Send the hold request using the built-in API */
    status = pjsua_call_set_hold(call_id, NULL);
    if (status != PJ_SUCCESS) {
        PJ_LOG(3, ("hold_call", "Error placing the call on hold"));
    } else {
        PJ_LOG(3, ("hold_call", "Call placed on hold successfully"));
    }
}

void transfer_call(pjsua_call_id call_a_id, const char *party_c_uri) {
    pj_status_t status;
    pj_str_t refer_to_uri = pj_str(party_c_uri);

    // // Parse Party C's URI
    // status = pjsip_parse_uri(party_c_uri, &refer_to_uri);
    // if (status != PJ_SUCCESS) {
    //     PJ_LOG(3, (THIS_FILE, "Error parsing Party C URI"));
    //     return;
    // }

    // Send REFER to Caller A
    status = pjsua_call_xfer(call_a_id, &refer_to_uri, NULL);
    if (status != PJ_SUCCESS) {
        PJ_LOG(3, (THIS_FILE, "Error sending REFER to Caller A"));
    } else {
        PJ_LOG(3, (THIS_FILE, "Call transferred to Party C successfully"));
    }

    // Cleanup
    // pjsip_uri_destroy(refer_to_uri);
}

pjsua_call_id make_call_to_c(const char *dest_uri) {
    pj_status_t status;
    pjsua_acc_id acc_id = pjsua_acc_get_default();
    pjsua_call_id call_c_id;

    /* Prepare the call settings (use the default) */
    pjsua_call_setting call_setting;
    pjsua_call_setting_default(&call_setting);

    /* Initiate a new call to Party C and retrieve call ID */
    pj_str_t pdest_uri = pj_str(dest_uri);
    status = pjsua_call_make_call(acc_id, &pdest_uri, &call_setting, NULL, NULL, &call_c_id);
    if (status != PJ_SUCCESS) {
        PJ_LOG(3, (THIS_FILE, "Error making call to Party C"));
        return PJSUA_INVALID_ID;  // Return invalid call ID on error
    } else {
        PJ_LOG(3, (THIS_FILE, "Call to Party C initiated successfully. Call ID: %d", call_c_id));
    }

    /* Return the call ID of the newly initiated call */
    return call_c_id;
}

// Play a prompt to the user (e.g., audio file)
static void play_prompt(pjsua_call_id call_id, const char* wav_file) {
    pj_status_t status;
    pj_str_t wav_path = pj_str(wav_file);

    // Create player
    pjsua_player_id player_id;
    status = pjsua_player_create(&wav_path, PJMEDIA_FILE_NO_LOOP, &player_id);
    if (status != PJ_SUCCESS) {
        printf("Error: unable to create player for file %s\n", wav_file);
        return;
    }

    // Connect the call's media to the player (send audio to the call)
    pjsua_conf_port_id call_conf_id = pjsua_call_get_conf_port(call_id);
    pjsua_conf_port_id player_conf_id = pjsua_player_get_conf_port(player_id);
    pjsua_conf_connect(player_conf_id, call_conf_id);

    printf("Playing prompt: %s\n", wav_file);
}

// Handle incoming call
static void on_incoming_call(pjsua_acc_id acc_id, pjsua_call_id call_id, pjsip_rx_data *rdata) {
    printf("Incoming call: %d\n", acc_id);
    pj_status_t status;
    pjsua_call_info call_info;

    // Automatically answer incoming call
    pjsua_call_answer(call_id, 200, NULL, NULL);

    // Get call information (such as caller ID)
    status = pjsua_call_get_info(call_id, &call_info);
    if (status == PJ_SUCCESS) {
        printf("Incoming call from %s\n", call_info.remote_info.ptr);
    }

    // Play a prompt (for example, a WAV file saying "Please press 1 for sales, 2 for support")
    play_prompt(call_id, "media/welcom.wav");
}

#define MAX_DIGITS 10
// Handle DTMF input
static void on_dtmf_digit(pjsua_call_id call_id, int digit) {
    static char dtmf_buffer[MAX_DIGITS];

    printf("Received DTMF digit: %c\n", digit);

    if (digit == '2') {
        // Transfer to support
        play_prompt(call_id, "media/transfering.wav");
        // hold_call(call_id);
        // consultation_call_c_id = make_call_to_c("sip:phong2@192.168.0.4");
        // if (consultation_call_c_id == PJSUA_INVALID_ID) {
        //     printf("Error making call to Party C\n");
        //     return;
        // }
        // original_call_c_id = call_id;
        pj_str_t forward_to = pj_str("sip:phong2@192.168.0.4");  // Destination to forward the call
        PJ_LOG(3, (THIS_FILE, "Forwarding call to %.*s", (int)forward_to.slen, forward_to.ptr));
        pjsua_call_xfer(call_id, &forward_to, NULL);
        // Call transfer logic here, using pjsua_call_xfer()
    } else if (digit == '3') {
        // Transfer to support
        play_prompt(call_id, "media/transfering.wav");
        // hold_call(call_id);
        // make_call_to_c("sip:phong3@192.168.0.4");
        // pj_str_t forward_to = pj_str("sip:phong3@192.168.0.4");  // Destination to forward the call
        // PJ_LOG(3, (THIS_FILE, "Forwarding call to %.*s", (int)forward_to.slen, forward_to.ptr));
        // pjsua_call_xfer(call_id, &forward_to, NULL);
        // Call transfer logic here, using pjsua_call_xfer()
    } else {
        printf("Invalid input\n");
    }
}

// Main callback handler for SIP events
static void on_call_state(pjsua_call_id call_id, pjsip_event *e) {
    // pjsua_call_info call_info;
    // pjsua_call_get_info(call_id, &call_info);
    // printf("on_call_state %d state=%s\n", call_id, call_info.state_text.ptr);
    //     // Call disconnected
    // if (call_id == consultation_call_c_id) {
    //     // get receiver uri
    //     if (call_info.state == PJSIP_INV_STATE_CONNECTING && consultation_call_c_id != -1) {
    //         printf("Transfer call to phong2\n");
    //         transfer_call(original_call_c_id, "sip:phong2@192.168.0.4");
            
    //         // end call
    //         // printf("End call\n");
    //         // pjsua_call_hangup(consultation_call_c_id, 0, NULL, NULL); // Hang up Receiver B's call if needed
    //     }
    // } else if (call_id == original_call_c_id) {
    //     if (call_info.state == PJSIP_INV_STATE_CONFIRMED) {
    //         PJ_LOG(3, (THIS_FILE, "Call transfer successful: Caller A connected to Party C"));
    //     }
    // }
}

// Handle media state (RTP streams)
static void on_call_media_state(pjsua_call_id call_id) {
    pjsua_call_info call_info;
    pjsua_call_get_info(call_id, &call_info);

    if (call_info.media_status == PJSUA_CALL_MEDIA_ACTIVE) {
        // Call connected and media is active
        pjsua_conf_connect(pjsua_call_get_conf_port(call_id), 0);
        printf("Media connected\n");
    }
}

int main() {
    pj_status_t status;

    // Initialize PJSUA (the main SIP framework)
    status = pjsua_create();
    if (status != PJ_SUCCESS) {
        printf("Error initializing PJSUA\n");
        return 1;
    }

    // Configure PJSUA
    pjsua_config cfg;
    pjsua_config_default(&cfg);
    cfg.cb.on_incoming_call = &on_incoming_call;
    cfg.cb.on_call_media_state = &on_call_media_state;
    cfg.cb.on_call_state = &on_call_state;
    cfg.cb.on_dtmf_digit = &on_dtmf_digit;

    // Configure media
    pjsua_media_config media_cfg;
    pjsua_media_config_default(&media_cfg);

    // Initialize the PJSUA library
    status = pjsua_init(&cfg, NULL, &media_cfg);
    if (status != PJ_SUCCESS) {
        printf("Error initializing PJSUA\n");
        return 1;
    }

    // Add a SIP transport (e.g., UDP)
    pjsua_transport_config transport_cfg;
    pjsua_transport_config_default(&transport_cfg);
    transport_cfg.port = 5555;
    status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &transport_cfg, NULL);
    if (status != PJ_SUCCESS) {
        printf("Error creating transport\n");
        return 1;
    }

    // Start PJSUA
    status = pjsua_start();
    if (status != PJ_SUCCESS) {
        printf("Error starting PJSUA\n");
        return 1;
    }

    // Add an account to receive calls
    pjsua_acc_config acc_cfg;
    pjsua_acc_config_default(&acc_cfg);
    acc_cfg.id = pj_str("sip:phongivr@192.168.0.4");
    acc_cfg.reg_uri = pj_str("sip:192.168.0.4");
    acc_cfg.cred_count = 1;
    acc_cfg.cred_info[0].realm = pj_str("192.168.0.4");
    acc_cfg.cred_info[0].username = pj_str("phongivr");
    acc_cfg.cred_info[0].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
    acc_cfg.cred_info[0].data = pj_str(" ");

    pjsua_acc_id acc_id;
    status = pjsua_acc_add(&acc_cfg, PJ_TRUE, &acc_id);
    if (status != PJ_SUCCESS) {
        printf("Error adding account\n");
        return 1;
    }

    // Wait for calls
    printf("IVR is ready to receive calls...\n");
    for (;;) {
        pj_time_val delay = {0, 10};
        pjsua_handle_events(&delay);
    }

    // Clean up
    pjsua_destroy();

    return 0;
}
