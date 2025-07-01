#include "stdlib.h"
#include "stdio.h"
#include "stdbool.h"

#include <openssl/ssl.h>
void test() {
    printf("-- %d\n", __LINE__);
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if (ctx == NULL) {
        printf("Failed to create the SSL_CTX\n");
        goto end;
    }

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);

    /* Use the default trusted certificate store */
    if (!SSL_CTX_set_default_verify_paths(ctx)) {
        printf("Failed to set the default trusted certificate store\n");
        goto end;
    }

    if (!SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION)) {
        printf("Failed to set the minimum TLS protocol version\n");
        goto end;
    }

    SSL* ssl = SSL_new(ctx);
    if (ssl == NULL) {
        printf("Failed to create the SSL object\n");
        goto end;
    }

    char* hostname = "www.openssl.org";
    char* port = "443";
    int family = AF_INET;

    int sock = -1;
    BIO_ADDRINFO *res;
    const BIO_ADDRINFO *ai = NULL;
    printf("-- %d\n", __LINE__);

    /*
    * Lookup IP address info for the server.
    */
    if (!BIO_lookup_ex(hostname, port, BIO_LOOKUP_CLIENT, family, SOCK_STREAM, 0, &res)) {
        printf("Failed BIO_lookup_ex\n");
        goto end;
    }

    /*
    * Loop through all the possible addresses for the server and find one
    * we can connect to.
    */
    for (ai = res; ai != NULL; ai = BIO_ADDRINFO_next(ai)) {
        /*
        * Create a TCP socket. We could equally use non-OpenSSL calls such
        * as "socket" here for this and the subsequent connect and close
        * functions. But for portability reasons and also so that we get
        * errors on the OpenSSL stack in the event of a failure we use
        * OpenSSL's versions of these functions.
        */
        sock = BIO_socket(BIO_ADDRINFO_family(ai), SOCK_STREAM, 0, 0);
        if (sock == -1)
            continue;

        /* Connect the socket to the server's address */
        if (!BIO_connect(sock, BIO_ADDRINFO_address(ai), BIO_SOCK_NODELAY)) {
            BIO_closesocket(sock);
            sock = -1;
            continue;
        }

        /* We have a connected socket so break out of the loop */
        break;
    }

    /* Free the address information resources we allocated earlier */
    BIO_ADDRINFO_free(res);

    printf("-- %d\n", __LINE__);
    /* Create a BIO to wrap the socket */
    BIO* bio = BIO_new(BIO_s_socket());
    if (bio == NULL) {
        BIO_closesocket(sock);
        printf("Failed BIO_new\n");
        goto end;
    }

    /*
    * Associate the newly created BIO with the underlying socket. By
    * passing BIO_CLOSE here the socket will be automatically closed when
    * the BIO is freed. Alternatively you can use BIO_NOCLOSE, in which
    * case you must close the socket explicitly when it is no longer
    * needed.
    */
    BIO_set_fd(bio, sock, BIO_CLOSE);

    SSL_set_bio(ssl, bio, bio);
    printf("-- %d\n", __LINE__);

    /*
    * Tell the server during the handshake which hostname we are attempting
    * to connect to in case the server supports multiple hosts.
    */
    if (!SSL_set_tlsext_host_name(ssl, hostname)) {
        printf("Failed to set the SNI hostname\n");
        goto end;
    }

    /*
    * Ensure we check during certificate verification that the server has
    * supplied a certificate for the hostname that we were expecting.
    * Virtually all clients should do this unless you really know what you
    * are doing.
    */
    if (!SSL_set1_host(ssl, hostname)) {
        printf("Failed to set the certificate verification hostname");
        goto end;
    }
    printf("-- %d\n", __LINE__);

    /* Do the handshake with the server */
    if (SSL_connect(ssl) < 1) {
        printf("Failed to connect to the server\n");
        /*
        * If the failure is due to a verification error we can get more
        * information about it from SSL_get_verify_result().
        */
        if (SSL_get_verify_result(ssl) != X509_V_OK)
            printf("Verify error: %s\n",
                X509_verify_cert_error_string(SSL_get_verify_result(ssl)));
        goto end;
    }

    printf("success\n");
    end:
    printf("end\n");
    return;
}

typedef     unsigned int            uint32;
typedef     unsigned long long      uint64;
typedef     uint64                  usize;
typedef     unsigned char           byte;

typedef struct {
    uint32 a;       // 4
    uint64 b;       // 8
    byte c;         // 1
    uint32 d[9];    // 4*9 = 36 => (4*2)*4 + 4
} Test;

typedef struct {
    uint32 a;       // 4
    byte pad1[4];       // 4
    uint64 b;       // 8
    byte c;         // 1
    byte pad2[3];       // 3
    uint32 d0[1];   // 4
    uint32 d[8];    // 32 = 4*8
} TestPadded;

usize addr_align_up(usize addr, usize align) {
    return (addr + align - 1) & ~(align - 1);
}

void* ptr_align_up(void* ptr, usize align) {
    return (void*) addr_align_up((usize) ptr, align);
}

int main() {

    #define branchless_if(cond, on_true, on_false) ( ( ((cond) != 0) * (on_true) ) + ( ((cond) == 0) * (on_false) ) )

    int val_on_true = 10;
    int val_on_false = 53;

    int on_true = branchless_if(true, val_on_true, val_on_false);
    int on_true_2 = branchless_if(102, val_on_true, val_on_false);
    int on_true_3 = branchless_if(-178, val_on_true, val_on_false);
    
    int on_false = branchless_if(false, val_on_true, val_on_false);
    int on_false_2 = branchless_if(0, val_on_true, val_on_false);

    printf("on_true: %d\n", on_true);
    printf("on_true_2: %d\n", on_true_2);
    printf("on_true_3: %d\n", on_true_3);
    test();
    printf("on_false: %d\n", on_false);
    printf("on_false_2: %d\n", on_false_2);

    return 0;

    printf("Auto padded   - Size: %d, Align: %d\n", sizeof(Test), _Alignof(Test));
    printf("Manual padded - Size: %d, Align: %d\n", sizeof(TestPadded), _Alignof(TestPadded));

    uint32* p = (uint32*)20;
    uint32* pa = ptr_align_up(p, 4);
    usize pa2 = addr_align_up((usize)p, 4);
    printf("pointer: %p\naddr usize: %u\naddr: %u\n", p, (usize)p, p);
    printf("---\n");
    printf("aligned: %p, %u\n", pa, pa);
    printf("aligned: %p, %u\n", pa2, pa2);

    return 0;
}