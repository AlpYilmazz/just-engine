#pragma once

#include "core.h"
#include "memory/juststring.h"

#ifndef HTTP_CURL_NO_DEFINE

/* This is a return code for the read callback that, when returned, will
   signal libcurl to immediately abort the current transfer. */
#define CURL_READFUNC_ABORT 0x10000000
/* This is a return code for the read callback that, when returned, will
   signal libcurl to pause sending data on the current transfer. */
#define CURL_READFUNC_PAUSE 0x10000001

/* This is a return code for the progress callback that, when returned, will
   signal libcurl to continue executing the default progress function */
#define CURL_PROGRESSFUNC_CONTINUE 0x10000001

#define CURLPAUSE_RECV      (1<<0)
#define CURLPAUSE_RECV_CONT (0)

#define CURLPAUSE_SEND      (1<<2)
#define CURLPAUSE_SEND_CONT (0)

#define CURLPAUSE_ALL       (CURLPAUSE_RECV|CURLPAUSE_SEND)
#define CURLPAUSE_CONT      (CURLPAUSE_RECV_CONT|CURLPAUSE_SEND_CONT)

typedef enum {
    CURLE_OK = 0,
    CURLE_UNSUPPORTED_PROTOCOL,    /* 1 */
    CURLE_FAILED_INIT,             /* 2 */
    CURLE_URL_MALFORMAT,           /* 3 */
    CURLE_NOT_BUILT_IN,            /* 4 - [was obsoleted in August 2007 for
                                        7.17.0, reused in April 2011 for 7.21.5] */
    CURLE_COULDNT_RESOLVE_PROXY,   /* 5 */
    CURLE_COULDNT_RESOLVE_HOST,    /* 6 */
    CURLE_COULDNT_CONNECT,         /* 7 */
    CURLE_WEIRD_SERVER_REPLY,      /* 8 */
    CURLE_REMOTE_ACCESS_DENIED,    /* 9 a service was denied by the server
                                        due to lack of access - when login fails
                                        this is not returned. */
    CURLE_FTP_ACCEPT_FAILED,       /* 10 - [was obsoleted in April 2006 for
                                        7.15.4, reused in Dec 2011 for 7.24.0]*/
    CURLE_FTP_WEIRD_PASS_REPLY,    /* 11 */
    CURLE_FTP_ACCEPT_TIMEOUT,      /* 12 - timeout occurred accepting server
                                        [was obsoleted in August 2007 for 7.17.0,
                                        reused in Dec 2011 for 7.24.0]*/
    CURLE_FTP_WEIRD_PASV_REPLY,    /* 13 */
    CURLE_FTP_WEIRD_227_FORMAT,    /* 14 */
    CURLE_FTP_CANT_GET_HOST,       /* 15 */
    CURLE_HTTP2,                   /* 16 - A problem in the http2 framing layer.
                                        [was obsoleted in August 2007 for 7.17.0,
                                        reused in July 2014 for 7.38.0] */
    CURLE_FTP_COULDNT_SET_TYPE,    /* 17 */
    CURLE_PARTIAL_FILE,            /* 18 */
    CURLE_FTP_COULDNT_RETR_FILE,   /* 19 */
    CURLE_OBSOLETE20,              /* 20 - NOT USED */
    CURLE_QUOTE_ERROR,             /* 21 - quote command failure */
    CURLE_HTTP_RETURNED_ERROR,     /* 22 */
    CURLE_WRITE_ERROR,             /* 23 */
    CURLE_OBSOLETE24,              /* 24 - NOT USED */
    CURLE_UPLOAD_FAILED,           /* 25 - failed upload "command" */
    CURLE_READ_ERROR,              /* 26 - could not open/read from file */
    CURLE_OUT_OF_MEMORY,           /* 27 */
    CURLE_OPERATION_TIMEDOUT,      /* 28 - the timeout time was reached */
    CURLE_OBSOLETE29,              /* 29 - NOT USED */
    CURLE_FTP_PORT_FAILED,         /* 30 - FTP PORT operation failed */
    CURLE_FTP_COULDNT_USE_REST,    /* 31 - the REST command failed */
    CURLE_OBSOLETE32,              /* 32 - NOT USED */
    CURLE_RANGE_ERROR,             /* 33 - RANGE "command" did not work */
    CURLE_OBSOLETE34,              /* 34 */
    CURLE_SSL_CONNECT_ERROR,       /* 35 - wrong when connecting with SSL */
    CURLE_BAD_DOWNLOAD_RESUME,     /* 36 - could not resume download */
    CURLE_FILE_COULDNT_READ_FILE,  /* 37 */
    CURLE_LDAP_CANNOT_BIND,        /* 38 */
    CURLE_LDAP_SEARCH_FAILED,      /* 39 */
    CURLE_OBSOLETE40,              /* 40 - NOT USED */
    CURLE_OBSOLETE41,              /* 41 - NOT USED starting with 7.53.0 */
    CURLE_ABORTED_BY_CALLBACK,     /* 42 */
    CURLE_BAD_FUNCTION_ARGUMENT,   /* 43 */
    CURLE_OBSOLETE44,              /* 44 - NOT USED */
    CURLE_INTERFACE_FAILED,        /* 45 - CURLOPT_INTERFACE failed */
    CURLE_OBSOLETE46,              /* 46 - NOT USED */
    CURLE_TOO_MANY_REDIRECTS,      /* 47 - catch endless re-direct loops */
    CURLE_UNKNOWN_OPTION,          /* 48 - User specified an unknown option */
    CURLE_SETOPT_OPTION_SYNTAX,    /* 49 - Malformed setopt option */
    CURLE_OBSOLETE50,              /* 50 - NOT USED */
    CURLE_OBSOLETE51,              /* 51 - NOT USED */
    CURLE_GOT_NOTHING,             /* 52 - when this is a specific error */
    CURLE_SSL_ENGINE_NOTFOUND,     /* 53 - SSL crypto engine not found */
    CURLE_SSL_ENGINE_SETFAILED,    /* 54 - can not set SSL crypto engine as
                                        default */
    CURLE_SEND_ERROR,              /* 55 - failed sending network data */
    CURLE_RECV_ERROR,              /* 56 - failure in receiving network data */
    CURLE_OBSOLETE57,              /* 57 - NOT IN USE */
    CURLE_SSL_CERTPROBLEM,         /* 58 - problem with the local certificate */
    CURLE_SSL_CIPHER,              /* 59 - could not use specified cipher */
    CURLE_PEER_FAILED_VERIFICATION, /* 60 - peer's certificate or fingerprint
                                        was not verified fine */
    CURLE_BAD_CONTENT_ENCODING,    /* 61 - Unrecognized/bad encoding */
    CURLE_OBSOLETE62,              /* 62 - NOT IN USE since 7.82.0 */
    CURLE_FILESIZE_EXCEEDED,       /* 63 - Maximum file size exceeded */
    CURLE_USE_SSL_FAILED,          /* 64 - Requested FTP SSL level failed */
    CURLE_SEND_FAIL_REWIND,        /* 65 - Sending the data requires a rewind
                                        that failed */
    CURLE_SSL_ENGINE_INITFAILED,   /* 66 - failed to initialise ENGINE */
    CURLE_LOGIN_DENIED,            /* 67 - user, password or similar was not
                                        accepted and we failed to login */
    CURLE_TFTP_NOTFOUND,           /* 68 - file not found on server */
    CURLE_TFTP_PERM,               /* 69 - permission problem on server */
    CURLE_REMOTE_DISK_FULL,        /* 70 - out of disk space on server */
    CURLE_TFTP_ILLEGAL,            /* 71 - Illegal TFTP operation */
    CURLE_TFTP_UNKNOWNID,          /* 72 - Unknown transfer ID */
    CURLE_REMOTE_FILE_EXISTS,      /* 73 - File already exists */
    CURLE_TFTP_NOSUCHUSER,         /* 74 - No such user */
    CURLE_OBSOLETE75,              /* 75 - NOT IN USE since 7.82.0 */
    CURLE_OBSOLETE76,              /* 76 - NOT IN USE since 7.82.0 */
    CURLE_SSL_CACERT_BADFILE,      /* 77 - could not load CACERT file, missing
                                        or wrong format */
    CURLE_REMOTE_FILE_NOT_FOUND,   /* 78 - remote file not found */
    CURLE_SSH,                     /* 79 - error from the SSH layer, somewhat
                                        generic so the error message will be of
                                        interest when this has happened */

    CURLE_SSL_SHUTDOWN_FAILED,     /* 80 - Failed to shut down the SSL
                                        connection */
    CURLE_AGAIN,                   /* 81 - socket is not ready for send/recv,
                                        wait till it is ready and try again (Added
                                        in 7.18.2) */
    CURLE_SSL_CRL_BADFILE,         /* 82 - could not load CRL file, missing or
                                        wrong format (Added in 7.19.0) */
    CURLE_SSL_ISSUER_ERROR,        /* 83 - Issuer check failed.  (Added in
                                        7.19.0) */
    CURLE_FTP_PRET_FAILED,         /* 84 - a PRET command failed */
    CURLE_RTSP_CSEQ_ERROR,         /* 85 - mismatch of RTSP CSeq numbers */
    CURLE_RTSP_SESSION_ERROR,      /* 86 - mismatch of RTSP Session Ids */
    CURLE_FTP_BAD_FILE_LIST,       /* 87 - unable to parse FTP file list */
    CURLE_CHUNK_FAILED,            /* 88 - chunk callback reported error */
    CURLE_NO_CONNECTION_AVAILABLE, /* 89 - No connection available, the
                                        session will be queued */
    CURLE_SSL_PINNEDPUBKEYNOTMATCH, /* 90 - specified pinned public key did not
                                        match */
    CURLE_SSL_INVALIDCERTSTATUS,   /* 91 - invalid certificate status */
    CURLE_HTTP2_STREAM,            /* 92 - stream error in HTTP/2 framing layer
                                        */
    CURLE_RECURSIVE_API_CALL,      /* 93 - an api function was called from
                                        inside a callback */
    CURLE_AUTH_ERROR,              /* 94 - an authentication function returned an
                                        error */
    CURLE_HTTP3,                   /* 95 - An HTTP/3 layer problem */
    CURLE_QUIC_CONNECT_ERROR,      /* 96 - QUIC connection error */
    CURLE_PROXY,                   /* 97 - proxy handshake error */
    CURLE_SSL_CLIENTCERT,          /* 98 - client-side certificate required */
    CURLE_UNRECOVERABLE_POLL,      /* 99 - poll/select returned fatal error */
    CURLE_TOO_LARGE,               /* 100 - a value/data met its maximum */
    CURLE_ECH_REQUIRED,            /* 101 - ECH tried but failed */
    CURL_LAST /* never use! */
} CurlErrorCode;

typedef enum {
    CURLM_CALL_MULTI_PERFORM = -1, /* please call curl_multi_perform() or
                                    curl_multi_socket*() soon */
    CURLM_OK,
    CURLM_BAD_HANDLE,      /* the passed-in handle is not a valid CURLM handle */
    CURLM_BAD_EASY_HANDLE, /* an easy handle was not good/valid */
    CURLM_OUT_OF_MEMORY,   /* if you ever get this, you are in deep sh*t */
    CURLM_INTERNAL_ERROR,  /* this is a libcurl bug */
    CURLM_BAD_SOCKET,      /* the passed in socket argument did not match */
    CURLM_UNKNOWN_OPTION,  /* curl_multi_setopt() with unsupported option */
    CURLM_ADDED_ALREADY,   /* an easy handle already added to a multi handle was
                            attempted to get added - again */
    CURLM_RECURSIVE_API_CALL, /* an api function was called from inside a
                                callback */
    CURLM_WAKEUP_FAILURE,  /* wakeup is unavailable or failed */
    CURLM_BAD_FUNCTION_ARGUMENT, /* function called with a bad parameter */
    CURLM_ABORTED_BY_CALLBACK,
    CURLM_UNRECOVERABLE_POLL,
    CURLM_LAST
} CurlMultiCode;

typedef enum {
  CURLMSG_NONE, /* first, not used */
  CURLMSG_DONE, /* This easy handle has completed. 'result' contains
                   the CURLcode of the transfer */
  CURLMSG_LAST /* last, not used */
} CURLMSG;

typedef void HttpRequest;

typedef void HttpRequestMulti;

typedef struct {
  CURLMSG msg;       /* what this message means */
  HttpRequest* easy_handle; /* the handle it concerns */
  union {
    void* whatever;    /* message-specific data */
    CurlErrorCode result;   /* return code for transfer */
  } data;
} CurlMessage;

#endif

typedef enum {
    HTTP_VERSION_1_0 = 0,
    HTTP_VERSION_1_1,
    HTTP_VERSION_2_0,
    HTTP_VERSION_ENUM_COUNT,
} HttpVersion;

typedef enum {
    HTTP_METHOD_GET = 0,
    HTTP_METHOD_POST,
    HTTP_METHOD_ENUM_COUNT,
} HttpMethod;

typedef struct {
    String key;
    String value;
} HttpHeader;

typedef struct {
    usize count;
    usize capacity;
    HttpHeader* headers;
} HttpHeaders;

typedef struct {
    bool success;
    CurlErrorCode error_code;
    const char* error_msg;
} HttpEasyResult;

typedef struct {
    bool success;
    CurlMultiCode error_code;
    const char* error_msg;
} HttpMultiResult;

typedef usize (*CurlReadFn)(char* buffer, usize size, usize nitems, void* userdata);
typedef usize (*CurlWriteFn)(char* ptr, usize size, usize nmemb, void* userdata);
typedef int32 (*CurlProgressFn)(void* clientp, int64 dltotal, int64 dlnow, int64 ultotal, int64 ulnow);

typedef struct {
    CurlReadFn read_fn;
    void* read_arg;
    CurlWriteFn write_fn;
    void* write_arg;
    CurlProgressFn progress_fn;
    void* progress_arg;
} CurlCallbacks;

typedef struct {
    bool verify_peer;
    bool verify_host;
    String cainfo_file;
} CurlSSLOpt;

void just_http_global_init_default();
void just_http_global_cleanup();

HttpHeaders http_headers_from_static(char* kv_list[][2], usize count);
void http_headers_add_header_static(HttpHeaders* headers, char* key, char* value);

HttpRequest* http_request_easy_init();

void http_request_set_threaded_use(HttpRequest* req);
void http_request_set_verbose(HttpRequest* req);

void http_request_set_ssl_opt(HttpRequest* req, CurlSSLOpt ssl_opt);
void http_request_set_callbacks(HttpRequest* req, CurlCallbacks callbacks);

void http_request_set_version(HttpRequest* req, HttpVersion version);
void http_request_set_method(HttpRequest* req, HttpMethod method);
void http_request_set_url(HttpRequest* req, String url);
void http_request_set_headers(HttpRequest* req, HttpHeaders headers);
void http_request_set_body(HttpRequest* req, String body);

HttpEasyResult http_request_easy_perform(HttpRequest* req);
void http_request_easy_cleanup(HttpRequest* req);

HttpRequestMulti* http_request_multi_init();

void http_request_multi_add_request(HttpRequestMulti* reqset, HttpRequest* req);
void http_request_multi_remove_request(HttpRequestMulti* reqset, HttpRequest* req);

HttpMultiResult http_request_multi_perform(HttpRequestMulti* reqset, int32* running_handles);
HttpMultiResult http_request_multi_poll(HttpRequestMulti* reqset, int32 timeout_ms);
HttpMultiResult http_request_multi_wakeup(HttpRequestMulti* reqset);
CurlMessage* http_request_multi_info_read(HttpRequestMulti* reqset, int32* msgs_in_queue);
void http_request_multi_cleanup(HttpRequestMulti* reqset);

CurlErrorCode http_request_easy_pause(HttpRequest* req, int32 bitmask);