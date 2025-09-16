#include <sys/time.h>
#include <stdatomic.h>

#include "justengine.h"

typedef struct timeval TimeVal;

typedef enum {
    SEND_STATIC_PREFIX,
    SEND_DYNAMIC_MIDDLE,
    SEND_STATIC_SUFFIX,
    SEND_END_TRANSFER,
} SendState;

typedef struct {
    SendState state;
    atomic_bool* dynamic_part_received;
    usize cursor;
    String static_prefix;
    String* dynamic_middle;
    String static_suffix;
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
    gettimeofday(arg->last_time, NULL);

    usize buffer_size = size * nitems;

    if (arg->send_data.state == SEND_STATIC_PREFIX) {
        String send_string = arg->send_data.static_prefix;
        if (atomic_load(arg->send_data.dynamic_part_received)) {
            if (arg->send_data.cursor < send_string.count) {
                char* str = send_string.str + arg->send_data.cursor;
                usize str_count = send_string.count - arg->send_data.cursor;
                str_count = MIN(buffer_size, str_count);

                std_memcpy(buffer, str, str_count);

                arg->send_data.cursor += str_count;
                if (arg->send_data.cursor >= send_string.count) {
                    arg->send_data.state = SEND_DYNAMIC_MIDDLE;
                    arg->send_data.cursor = 0;
                }
                
                return str_count;
            }
            else { // already completed prefix
                arg->send_data.state = SEND_DYNAMIC_MIDDLE;
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
    if (arg->send_data.state == SEND_DYNAMIC_MIDDLE) {
        String send_string = *arg->send_data.dynamic_middle;
        if (arg->send_data.cursor < send_string.count) {
            char* str = send_string.str + arg->send_data.cursor;
            usize str_count = send_string.count - arg->send_data.cursor;
            str_count = MIN(buffer_size, str_count);

            std_memcpy(buffer, str, str_count);

            arg->send_data.cursor += str_count;
            if (arg->send_data.cursor >= send_string.count) {
                arg->send_data.state = SEND_STATIC_SUFFIX;
                arg->send_data.cursor = 0;
            }
            
            return str_count;
        }
        else { // already completed prefix
            arg->send_data.state = SEND_STATIC_SUFFIX;
            arg->send_data.cursor = 0;
        }
    }
    if (arg->send_data.state == SEND_STATIC_SUFFIX) {
        String send_string = arg->send_data.static_suffix;
        if (arg->send_data.cursor < send_string.count) {
            char* str = send_string.str + arg->send_data.cursor;
            usize str_count = send_string.count - arg->send_data.cursor;
            str_count = MIN(buffer_size, str_count);

            std_memcpy(buffer, str, str_count);

            arg->send_data.cursor += str_count;
            if (arg->send_data.cursor >= send_string.count) {
                arg->send_data.state = SEND_END_TRANSFER;
                arg->send_data.cursor = 0;
            }
            
            return str_count;
        }
        else { // already completed prefix
            arg->send_data.state = SEND_END_TRANSFER;
            arg->send_data.cursor = 0;
        }
    }
    if (arg->send_data.state == SEND_END_TRANSFER) {
        return 0;
    }

    return 0;
}

typedef struct {
    HttpRequest* req;
    TimeVal* last_time;
    float64 continue_period_sec;
    bool work_done;
    atomic_bool* dynamic_part_received;
} ProgressFnArg;

int32 progress_callback(void* clientp, int64 dltotal, int64 dlnow, int64 ultotal, int64 ulnow) {
    ProgressFnArg* arg = clientp;

    if (arg->work_done) {
        return 0;
    }

    if (atomic_load(arg->dynamic_part_received)) {
        arg->work_done = true;
        http_request_easy_pause(arg->req, CURLPAUSE_CONT);
        return 0;
    }

    TimeVal now;
    gettimeofday(&now, NULL);

    float64 elapsed = (now.tv_sec - arg->last_time->tv_sec) + (now.tv_usec - arg->last_time->tv_usec) / 1000000.0;

    if (elapsed >= arg->continue_period_sec) {
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
    String* dynamic_middle;
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
            .static_prefix = string_from_cstr("{         "),    // total 10
            .dynamic_middle = arg->dynamic_middle,              // total 10 (all spaces)
            .static_suffix = string_from_cstr("         }"),    // total 10
        },
    };

    ProgressFnArg progress_arg = {
        .req = req,
        .last_time = last_time,
        .continue_period_sec = 1,
        .work_done = false,
        .dynamic_part_received = arg->dynamic_part_received,
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
    gettimeofday(&request_start_time, NULL);
    HttpResponse res = http_request_easy_perform(req);
    gettimeofday(&arg->request_end_time, NULL);

    float64 elapsed =
        (arg->request_end_time.tv_sec - request_start_time.tv_sec)
        + (arg->request_end_time.tv_usec - request_start_time.tv_usec) / 1000000.0;
    JUST_LOG_INFO("Request took [%0.10lf s]\n", elapsed);

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

    String* dynamic_middle = std_malloc(sizeof(String));
    *dynamic_middle = string_new();

    RequestThreadArg* request_arg = std_malloc(sizeof(RequestThreadArg));
    *request_arg = (RequestThreadArg) {
        .dynamic_part_received = dynamic_part_received,
        .dynamic_middle = dynamic_middle,
        .request_end_time = {0},
    };

    ThreadEntry thread_entry = {
        .handler = send_request,
        .arg = request_arg,
    };
    Thread request_thread = thread_spawn(thread_entry);

    bool thread_joined = false;
    bool signal_done = false;
    TimeVal signal_time = {0};
    while (!WindowShouldClose()) {

        if (!signal_done && IsKeyPressed(KEY_SPACE)) {
            signal_done = true;
            *dynamic_middle = string_from_cstr("          ");
            gettimeofday(&signal_time, NULL);
            atomic_store(dynamic_part_received, true);
        }

        if (signal_done && !thread_joined) {
            if (thread_try_join(request_thread)) {
                thread_joined = true;
                float64 elapsed =
                    (request_arg->request_end_time.tv_sec - signal_time.tv_sec)
                    + (request_arg->request_end_time.tv_usec - signal_time.tv_usec) / 1000000.0;
                // JUST_LOG_INFO("signal_time { %llu s, %llu us}\n", signal_time.tv_sec, signal_time.tv_usec);
                // JUST_LOG_INFO("request_end_time { %llu s, %llu us}\n", request_arg->request_end_time.tv_sec, request_arg->request_end_time.tv_usec);
                JUST_LOG_INFO("Request took [%0.10lf s] after the signal\n", elapsed);
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        EndDrawing();
    }
    
    just_http_global_cleanup();
    return 0;
}