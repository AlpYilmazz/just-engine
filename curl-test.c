#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>

#include "justengine.h"

int main() {
    just_http_global_init_default();

    CurlSSLOpt ssl_opt = {
        .verify_peer = true,
        .verify_host = true,
        .cainfo_file = string_from_cstr("test-assets/cacert.pem"),
    };

    HttpRequest* req = http_request_easy_init();
    if(req) {
        http_request_set_ssl_opt(req, ssl_opt);
        http_request_set_url(req, string_from_cstr("https://google.com"));
        HttpResponse res = http_request_easy_send(req);
        
        if(!res.success) {
            fprintf(stderr,
                "http_request_easy_send failed: %s\n",
                res.error_msg
            );
        }
        
        http_request_easy_cleanup(req);
    }
    
    just_http_global_cleanup();
    return 0;
}