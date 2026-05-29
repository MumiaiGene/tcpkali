#define _GNU_SOURCE
#include <assert.h>

#include "tcpkali_common.h"
#include "tcpkali_connection.h"
#include "tcpkali_ssl.h"

static void
print_ssl_error(const char *action) {
    unsigned long err = ERR_peek_error();
    if(err) {
        char errbuf[256];
        ERR_error_string_n(err, errbuf, sizeof(errbuf));
        fprintf(stderr, "%s: %s\n", action, errbuf);
        ERR_print_errors_fp(stderr);
    } else {
        fprintf(stderr, "%s: no OpenSSL error available\n", action);
    }
}

int
ssl_setup(struct connection UNUSED *conn, int UNUSED sockfd,
          char UNUSED *ssl_cert, char UNUSED *ssl_key,
          char UNUSED *ssl_server_name) {
#ifdef HAVE_OPENSSL
    conn->conn_blocked = 0;
    if(conn->ssl_ctx == NULL) {
        const SSL_METHOD *method = conn->conn_type == CONN_OUTGOING
#if (OPENSSL_VERSION_NUMBER < 0x10100000L)
                                       ? TLSv1_2_client_method()
                                       : TLSv1_2_server_method();
#else
                                       ? TLS_client_method()
                                       : TLS_server_method();
#endif
        if(method == NULL) {
            print_ssl_error("Can not create SSL method");
            exit(1);
        }
        conn->ssl_ctx = SSL_CTX_new(method);
    }
    if(conn->ssl_ctx == NULL) {
        print_ssl_error("Can not create SSL context");
        exit(1);
    } else {
        if(conn->conn_type == CONN_INCOMING) {
#ifdef  HAVE_SSL_CTX_SET_ECDH_AUTO
            SSL_CTX_set_ecdh_auto(conn->ssl_ctx, 1);
#endif
            if(SSL_CTX_use_certificate_file(conn->ssl_ctx, ssl_cert,
                                            SSL_FILETYPE_PEM)
               <= 0) {
                fprintf(stderr, "%s: %s\n", ssl_cert,
                        ERR_error_string(ERR_get_error(), NULL));
                exit(1);
            }
            if(SSL_CTX_use_PrivateKey_file(conn->ssl_ctx, ssl_key,
                                           SSL_FILETYPE_PEM)
               <= 0) {
                fprintf(stderr, "%s: %s\n", ssl_key,
                        ERR_error_string(ERR_get_error(), NULL));
                exit(1);
            }
        }
        if(!conn->ssl_fd) {
            conn->ssl_fd = SSL_new(conn->ssl_ctx);
            if(conn->conn_type == CONN_OUTGOING && ssl_server_name) {
                if(!SSL_set_tlsext_host_name(conn->ssl_fd, ssl_server_name)) {
                    print_ssl_error("Can not set TLS server name");
                    return 0;
                }
            }
            SSL_set_fd(conn->ssl_fd, sockfd);
            switch(conn->conn_type) {
            case CONN_OUTGOING:
                SSL_set_connect_state(conn->ssl_fd);
                break;
            case CONN_INCOMING:
                SSL_set_accept_state(conn->ssl_fd);
                break;
            case CONN_ACCEPTOR:
                assert(!"Unreachable");
                break;
            }
        }
        int status = -1;
        switch(conn->conn_type) {
        case CONN_OUTGOING:
            status = SSL_connect(conn->ssl_fd);
            break;
        case CONN_INCOMING:
            status = SSL_accept(conn->ssl_fd);
            break;
        case CONN_ACCEPTOR:
            assert(!"Unreachable");
            break;
        }
        switch(SSL_get_error(conn->ssl_fd, status)) {
        case SSL_ERROR_NONE:
            assert(status == 1);
            break;
        case SSL_ERROR_WANT_READ:
            assert(status == -1);
            conn->conn_blocked |= CBLOCKED_ON_READ;
            break;
        case SSL_ERROR_WANT_WRITE:
            assert(status == -1);
            conn->conn_blocked |= CBLOCKED_ON_WRITE;
            break;
        default:
            print_ssl_error(conn->conn_type == CONN_OUTGOING
                                ? "Can not establish SSL connection"
                                : "Can not accept SSL connection");
            return 0;
        }
        if(status < 0) {
            conn->conn_blocked |= CBLOCKED_ON_INIT;
        }
    }
    assert(conn->ssl_fd != NULL);
#else
    assert(!"Unreachable");
#endif /* HAVE_OPENSSL */
    return 1;
}
