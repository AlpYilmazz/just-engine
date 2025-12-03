
#include "curl/curl.h"

typedef CURLcode CurlErrorCode;
typedef CURLMcode CurlMultiCode;
typedef CURLMsg CurlMessage;
typedef CURL HttpRequest;
typedef CURLM HttpRequestMulti;
#define HTTP_CURL_NO_DEFINE
#include "httpclient.h"

void just_http_global_init_default() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

void just_http_global_cleanup() {
    curl_global_cleanup();
}

HttpHeaders http_headers_from_static(char* kv_list[][2], usize count) {
    HttpHeaders headers = {0};
    for (usize i = 0; i < count; i++) {
        HttpHeader header = {
            .key = string_from_cstr(kv_list[i][0]),
            .value = string_from_cstr(kv_list[i][1]),
        };
        dynarray_push_back_custom(headers, .headers, header);
    }
    return headers;
}

void http_headers_add_header_static(HttpHeaders* headers, char* key, char* value) {
    HttpHeader header = {
        .key = string_from_cstr(key),
        .value = string_from_cstr(value),
    };
    dynarray_push_back_custom(*headers, .headers, header);
}

HttpRequest* http_request_easy_init() {
    return curl_easy_init();
}

void http_request_set_threaded_use(HttpRequest* req) {
    curl_easy_setopt(req, CURLOPT_NOSIGNAL, 1L);
}

void http_request_set_verbose(HttpRequest* req) {
    curl_easy_setopt(req, CURLOPT_VERBOSE, 1L);
}

void http_request_set_ssl_opt(HttpRequest* req, CurlSSLOpt ssl_opt) {
    curl_easy_setopt(req, CURLOPT_SSL_VERIFYPEER, ssl_opt.verify_peer ? 1 : 0);
    curl_easy_setopt(req, CURLOPT_SSL_VERIFYHOST, ssl_opt.verify_host ? 2 : 0);
    if (ssl_opt.cainfo_file.str != NULL) {
        curl_easy_setopt(req, CURLOPT_CAINFO, ssl_opt.cainfo_file.cstr);
    }
}

void http_request_set_callbacks(HttpRequest* req, CurlCallbacks callbacks) {
    if (callbacks.read_fn != NULL) {
        curl_easy_setopt(req, CURLOPT_READFUNCTION, callbacks.read_fn);
        curl_easy_setopt(req, CURLOPT_READDATA, callbacks.read_arg);
    }
    if (callbacks.write_fn != NULL) {
        curl_easy_setopt(req, CURLOPT_WRITEFUNCTION, callbacks.write_fn);
        curl_easy_setopt(req, CURLOPT_WRITEDATA, callbacks.write_arg);
    }
    if (callbacks.progress_fn != NULL) {
        curl_easy_setopt(req, CURLOPT_NOPROGRESS, 0);
        curl_easy_setopt(req, CURLOPT_XFERINFOFUNCTION, callbacks.progress_fn);
        curl_easy_setopt(req, CURLOPT_XFERINFODATA, callbacks.progress_arg);
    }
}

void http_request_set_version(HttpRequest* req, HttpVersion version) {
    static int32 MAP_CURL[HTTP_VERSION_ENUM_COUNT] = {
        [HTTP_VERSION_1_0] = CURL_HTTP_VERSION_1_0,
        [HTTP_VERSION_1_1] = CURL_HTTP_VERSION_1_1,
        [HTTP_VERSION_2_0] = CURL_HTTP_VERSION_2_0,
    };
    curl_easy_setopt(req, CURLOPT_HTTP_VERSION, MAP_CURL[version]);
}

void http_request_set_method(HttpRequest* req, HttpMethod method) {
    static int32 MAP_CURL[HTTP_METHOD_ENUM_COUNT] = {
        [HTTP_METHOD_GET] = CURLOPT_HTTPGET,
        [HTTP_METHOD_POST] = CURLOPT_POST,
    };
    curl_easy_setopt(req, MAP_CURL[method], 1);
}

void http_request_set_url(HttpRequest* req, String url) {
    curl_easy_setopt(req, CURLOPT_URL, url.cstr);
}

void http_request_set_headers(HttpRequest* req, HttpHeaders headers) {
    struct curl_slist* curl_headers = NULL;
    for (usize i = 0; i < headers.count; i ++) {
        HttpHeader* header = &headers.headers[i];

        String header_line = string_new();
        usize count_hint = header->key.count + header->value.count + 2 + 1;
        string_hinted_append_format(header_line, count_hint, "%s: %s", header->key.cstr, header->value.cstr);

        curl_headers = curl_slist_append(curl_headers, header_line.cstr);
    }
    curl_easy_setopt(req, CURLOPT_HTTPHEADER, curl_headers);
}

void http_request_set_body(HttpRequest* req, String body) {
    curl_easy_setopt(req, CURLOPT_POSTFIELDS, body.str);
    curl_easy_setopt(req, CURLOPT_POSTFIELDSIZE, (int32) body.count);
}

HttpEasyResult http_request_easy_perform(HttpRequest* req) {
    CURLcode curl_response = curl_easy_perform(req);
    
    HttpEasyResult result;
    if (curl_response == CURLE_OK) {
        result = (HttpEasyResult) {
            .success = true,
        };
    }
    else {
        result = (HttpEasyResult) {
            .success = false,
            .error_code = curl_response,
            .error_msg = curl_easy_strerror(curl_response),
        };
    }
    return result;
}

void http_request_easy_cleanup(HttpRequest* req) {
    curl_easy_cleanup(req);
}

HttpRequestMulti* http_request_multi_init() {
    return curl_multi_init();
}

void http_request_multi_add_request(HttpRequestMulti* reqset, HttpRequest* req) {
    curl_multi_add_handle(reqset, req);
}

void http_request_multi_remove_request(HttpRequestMulti* reqset, HttpRequest* req) {
    curl_multi_remove_handle(reqset, req);
}

static HttpMultiResult curlm_multi_result(CURLMcode curlm_code) {
    if (curlm_code == CURLE_OK) {
        return (HttpMultiResult) {
            .success = true,
        };
    }
    else {
        return (HttpMultiResult) {
            .success = false,
            .error_code = curlm_code,
            .error_msg = curl_multi_strerror(curlm_code),
        };
    }
}

HttpMultiResult http_request_multi_perform(HttpRequestMulti* reqset, int32* running_handles) {
    CURLMcode curlm_response = curl_multi_perform(reqset, running_handles);
    return curlm_multi_result(curlm_response);
}

HttpMultiResult http_request_multi_poll(HttpRequestMulti* reqset, int32 timeout_ms) {
    CURLMcode curlm_response = curl_multi_poll(reqset, NULL, 0, timeout_ms, NULL);
    return curlm_multi_result(curlm_response);
}

HttpMultiResult http_request_multi_wakeup(HttpRequestMulti* reqset) {
    CURLMcode curlm_response = curl_multi_wakeup(reqset);
    return curlm_multi_result(curlm_response);
}

CurlMessage* http_request_multi_info_read(HttpRequestMulti* reqset, int32* msgs_in_queue) {
    return curl_multi_info_read(reqset, msgs_in_queue);
}

void http_request_multi_cleanup(HttpRequestMulti* reqset) {
    HttpRequest** reqarr = curl_multi_get_handles(reqset);
    if (reqarr != NULL) {
        for (HttpRequest* req = reqarr[0]; req != NULL; req++) {
            curl_easy_cleanup(req);
        }
    }
    curl_multi_cleanup(reqset);
}

CurlErrorCode http_request_easy_pause(HttpRequest* req, int32 bitmask) {
    return curl_easy_pause(req, bitmask);
}