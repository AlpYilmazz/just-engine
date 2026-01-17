#include "justengine.h"

typedef struct {
    uint32 status_code;
    char* body;
} C_HttpResponse;

__declspec(dllimport) void __cdecl test_start(char* start_string);
__declspec(dllimport) void __cdecl test_complete(char* complete_string);
__declspec(dllimport) bool __cdecl test_poll(C_HttpResponse* set_response);

int main() {
    bool poll_success;
    C_HttpResponse response;
    test_start("start into -> ");

    poll_success = test_poll(&response);
    JUST_LOG_INFO("poll_success: %d\n", poll_success);

    test_complete("complete!");

    while(!(poll_success = test_poll(&response))) {
        JUST_LOG_INFO("poll_success: %d\n", poll_success);
    }
    JUST_LOG_INFO("poll_success: %d\n", poll_success);
    JUST_LOG_INFO("C_HttpResponse: {\n\tstatus_code: %u,\n\tbody: %s,\n}\n", response.status_code, response.body);

    return 0;
}