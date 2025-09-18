// #include <sys/time.h>
#include <pthread_time.h>
#include <stdatomic.h>

#include "justengine.h"

// typedef struct timeval TimeVal;
typedef struct timespec TimeVal;

typedef enum {
    SEND_STATIC_PREFIX,
    SEND_DYNAMIC_SUFFIX,
    SEND_END_TRANSFER,
} SendState;

typedef struct {
    SendState state;
    atomic_bool* dynamic_part_received;
    usize cursor;
    String static_prefix;
    String* dynamic_suffix;
} SendData;

typedef struct {
    bool should_pause;
    TimeVal* last_time;
    SendData send_data;
} ReadFnArg;

usize read_callback(char* buffer, usize size, usize nitems, void* userdata) {
    ReadFnArg* arg = userdata;

    if (arg->should_pause) {
        arg->should_pause = false;
        return CURL_READFUNC_PAUSE;
    }
    // gettimeofday(arg->last_time, NULL);
    clock_gettime(CLOCK_MONOTONIC, arg->last_time);

    usize buffer_size = size * nitems;
    usize sent_count = 0;

    if (arg->send_data.state == SEND_STATIC_PREFIX) {
        String send_string = arg->send_data.static_prefix;
        if (atomic_load(arg->send_data.dynamic_part_received)) {
            if (arg->send_data.cursor < send_string.count) {
                char* str = send_string.str + arg->send_data.cursor;
                usize str_count = send_string.count - arg->send_data.cursor;
                str_count = MIN(buffer_size, str_count);

                if (str_count > 0) {
                    std_memcpy(buffer, str, str_count);
                    buffer += str_count;
                    buffer_size -= str_count;
                    sent_count += str_count;
                }
                
                arg->send_data.cursor += str_count;
                if (arg->send_data.cursor >= send_string.count) {
                    arg->send_data.state = SEND_DYNAMIC_SUFFIX;
                    arg->send_data.cursor = 0;
                }
            }
            else { // already completed prefix
                arg->send_data.state = SEND_DYNAMIC_SUFFIX;
                arg->send_data.cursor = 0;
            }
        }
        else {
            // char ch = arg->send_data.cursor >= send_string.count
            //     ? send_string.str[arg->send_data.cursor++]
            //     : ' ';
            char ch = ' ';
            if (arg->send_data.cursor < send_string.count) {
                ch = send_string.str[arg->send_data.cursor++];
                printf("sending [%d]: '%c'\n", arg->send_data.cursor, ch);
            }
            else {
                static int32 stall_count = 0;
                printf("stall [%d]: '%c'\n", ++stall_count, ch);
            }
            buffer[0] = ch;
            arg->should_pause = true;
            return 1;
        }
    }
    if (arg->send_data.state == SEND_DYNAMIC_SUFFIX) {
        String send_string = *arg->send_data.dynamic_suffix;
        if (arg->send_data.cursor < send_string.count) {
            char* str = send_string.str + arg->send_data.cursor;
            usize str_count = send_string.count - arg->send_data.cursor;
            str_count = MIN(buffer_size, str_count);

            if (str_count > 0) {
                std_memcpy(buffer, str, str_count);
                buffer += str_count;
                buffer_size -= str_count;
                sent_count += str_count;
            }

            arg->send_data.cursor += str_count;
            if (arg->send_data.cursor >= send_string.count) {
                arg->send_data.state = SEND_END_TRANSFER;
                arg->send_data.cursor = 0;
            }
            
            return sent_count;
        }
        else { // already completed prefix
            arg->send_data.state = SEND_END_TRANSFER;
            arg->send_data.cursor = 0;
        }
    }
    // if (arg->send_data.state == SEND_STATIC_SUFFIX) {
    //     String send_string = arg->send_data.static_suffix;
    //     if (arg->send_data.cursor < send_string.count) {
    //         char* str = send_string.str + arg->send_data.cursor;
    //         usize str_count = send_string.count - arg->send_data.cursor;
    //         str_count = MIN(buffer_size, str_count);

    //         std_memcpy(buffer, str, str_count);

    //         arg->send_data.cursor += str_count;
    //         if (arg->send_data.cursor >= send_string.count) {
    //             arg->send_data.state = SEND_END_TRANSFER;
    //             arg->send_data.cursor = 0;
    //         }
            
    //         return str_count;
    //     }
    //     else { // already completed prefix
    //         arg->send_data.state = SEND_END_TRANSFER;
    //         arg->send_data.cursor = 0;
    //     }
    // }
    if (arg->send_data.state == SEND_END_TRANSFER) {
        return 0;
    }

    return sent_count;
}

typedef struct {
    HttpRequest* req;
    TimeVal* last_time;
    float64 continue_period_sec;
    bool work_done;
    atomic_bool* dynamic_part_received;
    TimeVal* signal_time;
} ProgressFnArg;

int32 progress_callback(void* clientp, int64 dltotal, int64 dlnow, int64 ultotal, int64 ulnow) {
    ProgressFnArg* arg = clientp;

    if (arg->work_done) {
        return 0;
    }

    if (atomic_load(arg->dynamic_part_received)) {
        arg->work_done = true;
        http_request_easy_pause(arg->req, CURLPAUSE_CONT);
        // gettimeofday(arg->signal_time, NULL);
        clock_gettime(CLOCK_MONOTONIC, arg->signal_time);
        return 0;
    }

    TimeVal now;
    // gettimeofday(&now, NULL);
    clock_gettime(CLOCK_MONOTONIC, &now);

    // float64 elapsed = (now.tv_sec - arg->last_time->tv_sec) + (now.tv_usec - arg->last_time->tv_usec) / 1000000.0;
    float64 elapsed_ms = ((now.tv_sec - arg->last_time->tv_sec) * 1000.0)
                    + (now.tv_nsec - arg->last_time->tv_nsec) / 1000000.0;

    if (elapsed_ms >= arg->continue_period_sec * 1000) {
        http_request_easy_pause(arg->req, CURLPAUSE_CONT);
    }
    
    return 0;
}

typedef struct {
    String response_body;
} WriteFnArg;

usize write_callback(char* ptr, usize size, usize nmemb, void* userdata) {
    WriteFnArg* arg = userdata;

    usize count = size * nmemb;
    string_nappend_cstr(&arg->response_body, ptr, count);

    return count;
}

HttpHeaders get_common_headers() {
    char* headers[][2] = {
        { "accept", "application/json, text/plain, */*" },
        { "accept-language", "en-US,en;q=0.9,tr;q=0.8" },
        { "content-type", "application/json" },
        { "currentculture", "tr-TR" },
        { "sec-ch-ua", "\"Not_A Brand\";v=\"8\", \"Chromium\";v=\"120\", \"Microsoft Edge\";v=\"120\"" },
        { "sec-ch-ua-mobile", "?0" },
        { "sec-ch-ua-platform", "\"Windows\"" },
        { "sec-fetch-dest", "empty" },
        { "sec-fetch-mode", "cors" },
        { "sec-fetch-site", "same-site" },
        { "Referer", "https://www.passo.com.tr/" },
        { "Referrer-Policy", "strict-origin-when-cross-origin" },
        { "User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36 Edg/120.0.0.0" },
        { "Origin", "https://www.passo.com.tr/" },
    };
    return http_headers_from_static(headers, ARRAY_LENGTH(headers));
}

typedef struct {
    atomic_bool* dynamic_part_received;
    String* dynamic_suffix;
    TimeVal signal_time;
    TimeVal request_end_time;
} RequestThreadArg;

uint32 send_request(TaskArgVoid* arg_void) {
    RequestThreadArg* arg = arg_void;

    HttpRequest* req = http_request_easy_init();
    if (req == NULL) {
        PANIC("HttpRequest could not be initialized\n");
    }
    http_request_set_threaded_use(req);
    // http_request_set_verbose(req);

    CurlSSLOpt ssl_opt = {
        .verify_peer = true,
        .verify_host = true,
        .cainfo_file = string_from_cstr("test-assets/cacert.pem"),
    };

    TimeVal* last_time = std_malloc(sizeof(TimeVal));
    *last_time = (TimeVal) {0};

    ReadFnArg read_arg = {
        .should_pause = false,
        .last_time = last_time,
        .send_data = (SendData) {
            .state = SEND_STATIC_PREFIX,
            .dynamic_part_received = arg->dynamic_part_received,
            .cursor = 0,
            .static_prefix = string_from_cstr(
                "{         "
                "          "
                "          "
                "          "
                "          "
                "          "
                "          "
                "          "
                "          "
                "          "
                "          "
                "          "
                "          "
                "          "
                "          "
                "          "
                "          "
                "          "
                "          "
            ), // total 190
            .dynamic_suffix = arg->dynamic_suffix, // total 10
        },
    };

    ProgressFnArg progress_arg = {
        .req = req,
        .last_time = last_time,
        .continue_period_sec = 0.2,
        .work_done = false,
        .dynamic_part_received = arg->dynamic_part_received,
        .signal_time = &arg->signal_time,
    };

    WriteFnArg write_arg = {
        .response_body = string_new(),
    };

    CurlCallbacks callbacks = {
        .read_fn = read_callback,
        .read_arg = &read_arg,
        .progress_fn = progress_callback,
        .progress_arg = &progress_arg,
        .write_fn = write_callback,
        .write_arg = &write_arg,
    };

    HttpHeaders headers = get_common_headers();
    http_headers_add_header_static(&headers, "Transfer-Encoding", "chunked");

    http_request_set_ssl_opt(req, ssl_opt);
    http_request_set_callbacks(req, callbacks);
    http_request_set_version(req, HTTP_VERSION_1_1);
    http_request_set_method(req, HTTP_METHOD_POST);
    http_request_set_url(req, string_from_cstr("https://ticketingweb.passo.com.tr/api/passoweb/getcaptcha"));
    http_request_set_headers(req, headers);

    TimeVal request_start_time = {0};
    // gettimeofday(&request_start_time, NULL);
    clock_gettime(CLOCK_MONOTONIC, &request_start_time);
    HttpResponse res = http_request_easy_perform(req);
    // gettimeofday(&arg->request_end_time, NULL);
    clock_gettime(CLOCK_MONOTONIC, &arg->request_end_time);

    // float64 elapsed =
    //     (arg->request_end_time.tv_sec - request_start_time.tv_sec)
    //     + (arg->request_end_time.tv_usec - request_start_time.tv_usec) / 1000000.0;
    float64 elapsed_ms = ((arg->request_end_time.tv_sec - request_start_time.tv_sec) * 1000.0)
                    + (arg->request_end_time.tv_nsec - request_start_time.tv_nsec) / 1000000.0;
    JUST_LOG_INFO("Preemptive request took in total [%0.10lf ms]\n", elapsed_ms);

    if(res.success) {
        printf("--- Response Body ---\n");
        print_string(write_arg.response_body);
        printf("\n---------------------\n");
    }
    else {
        JUST_LOG_ERROR(
            "http_request_easy_send failed: %s\n",
            res.error_msg
        );
    }
    
    http_request_easy_cleanup(req);

    end_thread(0);
    return 0;
}


uint32 just_send_request(TaskArgVoid* arg_void) {
    void* arg = arg_void;

    TimeVal func_start_time = {0};
    // gettimeofday(&func_start_time, NULL);
    clock_gettime(CLOCK_MONOTONIC, &func_start_time);

    HttpRequest* req = http_request_easy_init();
    if (req == NULL) {
        PANIC("HttpRequest could not be initialized\n");
    }
    http_request_set_threaded_use(req);
    // http_request_set_verbose(req);

    CurlSSLOpt ssl_opt = {
        .verify_peer = true,
        .verify_host = true,
        .cainfo_file = string_from_cstr("test-assets/cacert.pem"),
    };

    WriteFnArg write_arg = {
        .response_body = string_new(),
    };

    CurlCallbacks callbacks = {
        .write_fn = write_callback,
        .write_arg = &write_arg,
    };

    HttpHeaders headers = get_common_headers();
    String body = string_from_cstr(
        "{         "
        "          "
        "          "
        "          "
        "          "
        "          "
        "          "
        "          "
        "          "
        "          "
        "          "
        "          "
        "          "
        "          "
        "          "
        "          "
        "          "
        "          "
        "          "
        "         }"
    ); // total 200

    http_request_set_ssl_opt(req, ssl_opt);
    http_request_set_callbacks(req, callbacks);
    http_request_set_method(req, HTTP_METHOD_POST);
    http_request_set_url(req, string_from_cstr("https://ticketingweb.passo.com.tr/api/passoweb/getcaptcha"));
    http_request_set_body(req, body);
    http_request_set_headers(req, headers);

    TimeVal request_start_time = {0};
    TimeVal request_end_time = {0};
    // gettimeofday(&request_start_time, NULL);
    clock_gettime(CLOCK_MONOTONIC, &request_start_time);
    HttpResponse res = http_request_easy_perform(req);
    // gettimeofday(&request_end_time, NULL);
    clock_gettime(CLOCK_MONOTONIC, &request_end_time);

    // float64 elapsed =
    //     (request_end_time.tv_sec - request_start_time.tv_sec)
    //     + (request_end_time.tv_usec - request_start_time.tv_usec) / 1000000.0;
    
    float64 full_elapsed_ms = ((request_end_time.tv_sec - func_start_time.tv_sec) * 1000.0)
                    + (request_end_time.tv_nsec - func_start_time.tv_nsec) / 1000000.0;
    float64 elapsed_ms = ((request_end_time.tv_sec - request_start_time.tv_sec) * 1000.0)
                    + (request_end_time.tv_nsec - request_start_time.tv_nsec) / 1000000.0;
    JUST_LOG_INFO("Naive request took in total [%0.10lf ms]\n", elapsed_ms);
    JUST_LOG_INFO("Naive request took in total [%0.10lf ms] including setup\n", full_elapsed_ms);

    if(res.success) {
        printf("--- Response Body ---\n");
        print_string(write_arg.response_body);
        printf("\n---------------------\n");
    }
    else {
        JUST_LOG_ERROR(
            "http_request_easy_send failed: %s\n",
            res.error_msg
        );
    }
    
    http_request_easy_cleanup(req);

    end_thread(0);
    return 0;
}

int main() {
    // SET_LOG_LEVEL(LOG_LEVEL_ERROR);
    // SET_LOG_LEVEL(LOG_LEVEL_WARN);
    SET_LOG_LEVEL(LOG_LEVEL_INFO);
    // SET_LOG_LEVEL(LOG_LEVEL_TRACE);
    
    InitWindow(1000, 1000, "Curl Test");
    SetTargetFPS(60);
    
    just_http_global_init_default();

    atomic_bool* dynamic_part_received = std_malloc(sizeof(atomic_bool));
    *dynamic_part_received = ATOMIC_VAR_INIT(false);

    String* dynamic_suffix = std_malloc(sizeof(String));
    *dynamic_suffix = string_new();

    RequestThreadArg* request_arg = std_malloc(sizeof(RequestThreadArg));
    *request_arg = (RequestThreadArg) {
        .dynamic_part_received = dynamic_part_received,
        .dynamic_suffix = dynamic_suffix,
        .request_end_time = {0},
        .signal_time = {0},
    };

    Thread thread = thread_spawn(just_send_request, NULL);
    thread_join(thread);

    Thread request_thread = thread_spawn(send_request, request_arg);

    bool thread_joined = false;
    bool signal_done = false;
    while (!WindowShouldClose()) {

        if (!signal_done && IsKeyPressed(KEY_SPACE)) {
            signal_done = true;
            *dynamic_suffix = string_from_cstr("         }");
            atomic_store(dynamic_part_received, true);
        }

        if (signal_done && !thread_joined) {
            if (thread_try_join(request_thread)) {
                thread_joined = true;
                // float64 elapsed =
                //     (request_arg->request_end_time.tv_sec - request_arg->signal_time.tv_sec)
                //     + (request_arg->request_end_time.tv_usec - request_arg->signal_time.tv_usec) / 1000000.0;
                float64 elapsed_ms = ((request_arg->request_end_time.tv_sec - request_arg->signal_time.tv_sec) * 1000.0)
                    + (request_arg->request_end_time.tv_nsec - request_arg->signal_time.tv_nsec) / 1000000.0;
                // JUST_LOG_INFO("signal_time { %llu s, %llu us}\n", signal_time.tv_sec, signal_time.tv_usec);
                // JUST_LOG_INFO("request_end_time { %llu s, %llu us}\n", request_arg->request_end_time.tv_sec, request_arg->request_end_time.tv_usec);
                JUST_LOG_INFO("Preemptive request took [%0.10lf ms] after the signal\n", elapsed_ms);
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        EndDrawing();
    }
    
    just_http_global_cleanup();
    return 0;
}