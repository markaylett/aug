/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "augas/ssl.hpp"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#if HAVE_OPENSSL_SSL_H

# include "augas/options.hpp"
# include "augsys/base.h"
# include "augsys/log.h"
# include "augsys/string.h"

# include <sstream>
# include <string>

# include <openssl/err.h>

using namespace aug;
using namespace augas;
using namespace std;

namespace {

    int
    passwdcb_(char* buf, int size, int rwflag, void* password)
    {
        aug_strlcpy(buf, static_cast<const char*>(password), size);
        return static_cast<int>(strlen(buf));
    }

    void
    infocb_(const SSL* ssl, int where, int ret)
    {
        if (aug_loglevel() < AUG_LOGDEBUG0 + 2)
            return;

        const char* str;
        int op(where & ~SSL_ST_MASK);

        if (op & SSL_ST_CONNECT)
            str = "connect";
        else if (op & SSL_ST_ACCEPT)
            str = "accept";
        else
            str = "undefined";

        if (where & SSL_CB_LOOP) {

            // Callback has been called to indicate state change inside a
            // loop.

            aug_debug2("SSL: %s loop: %s", str, SSL_state_string_long(ssl));

        } else if (where & SSL_CB_ALERT) {

            // Callback has been called due to an alert being sent or
            // received.

            str = (where & SSL_CB_READ) ? "read" : "write";
            aug_debug2("SSL: %s %s: %s", str,
                       SSL_alert_type_string_long(ret),
                       SSL_alert_desc_string_long(ret));

        } else if (where & SSL_CB_EXIT) {

            // Callback has been called to indicate error exit of a handshake
            // function. (May be soft error with retry option for non-blocking
            // setups.)

            if (0 == ret) {
                aug_debug2("SSL: %s failed: %s", str,
                           SSL_state_string_long(ssl));
            } else if (ret < 0) {
                aug_debug2("SSL: %s error: %s", str,
                           SSL_state_string_long(ssl));
            }
        }
    }

    int
    verifycb_(int preverify_ok, X509_STORE_CTX* x509_ctx)
    {
        X509* peer(X509_STORE_CTX_get_current_cert(x509_ctx));

        char buf[256];
        X509_NAME_oneline(X509_get_issuer_name(peer), buf, sizeof(buf));
        aug_info("issuer: %s", buf);

        X509_NAME* subject(X509_get_subject_name(peer));
        X509_NAME_oneline(subject, buf, sizeof(buf));
        aug_info("subject: %s", buf);

        buf[0] = '\0';
        X509_NAME_get_text_by_NID
            (subject, NID_countryName, buf, sizeof(buf));
        if (buf[0])
            aug_info("country: %s", buf);

        buf[0] = '\0';
        X509_NAME_get_text_by_NID
            (subject, NID_stateOrProvinceName, buf, sizeof(buf));
        if (buf[0])
            aug_info("state or province: %s", buf);

        buf[0] = '\0';
        X509_NAME_get_text_by_NID
            (subject, NID_localityName, buf, sizeof(buf));
        if (buf[0])
            aug_info("locality: %s", buf);

        buf[0] = '\0';
        X509_NAME_get_text_by_NID
            (subject, NID_organizationName, buf, sizeof(buf));
        if (buf[0])
            aug_info("organization: %s", buf);

        buf[0] = '\0';
        X509_NAME_get_text_by_NID
            (subject, NID_organizationalUnitName, buf, sizeof(buf));
        if (buf[0])
            aug_info("organizational unit: %s", buf);

        buf[0] = '\0';
        X509_NAME_get_text_by_NID
            (subject, NID_commonName, buf, sizeof(buf));
        if (buf[0])
            aug_info("common name: %s", buf);

        buf[0] = '\0';
        X509_NAME_get_text_by_NID
            (subject, NID_pkcs9_emailAddress, buf, sizeof(buf));
        if (buf[0])
            aug_info("email address: %s", buf);

		return preverify_ok;
    }

    int
    tomode_(int level)
    {
        int mode;
        switch (level) {
        case 0:
            mode = SSL_VERIFY_NONE;
            break;
        case 1:
            mode = SSL_VERIFY_PEER;
            break;
        default:
            mode = SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
            break;
        }
        return mode;
    }
}

sslctx::sslctx()
    : ctx_(SSL_CTX_new(SSLv23_method()))
{
    if (!ctx_)
        throw ssl_error(__FILE__, __LINE__, ERR_get_error());
}

sslctx::~sslctx() AUG_NOTHROW
{
    SSL_CTX_free(ctx_);
}

void
sslctx::setclient(fdref ref)
{
    SSL* ssl = SSL_new(ctx_);
    BIO* sbio = BIO_new_socket((int)aug_getosfd(ref.get()), BIO_NOCLOSE);
    SSL_set_bio(ssl, sbio, sbio);
    aug_setsslclient(ref.get(), ssl);
}

void
sslctx::setserver(fdref ref)
{
    SSL* ssl = SSL_new(ctx_);
    BIO* sbio = BIO_new_socket((int)aug_getosfd(ref.get()), BIO_NOCLOSE);
    SSL_set_bio(ssl, sbio, sbio);
    aug_setsslserver(ref.get(), ssl);
}

void
augas::initssl()
{
    // Global system initialization.

    SSL_library_init();
    SSL_load_error_strings();
}

sslctxptr
augas::createsslctx(const string& name, const options& options)
{
    string s("ssl.context.");
    s += name;

    const char* certfile(options.get(s + ".certfile", 0));
    const char* keyfile(options.get(s + ".keyfile", 0));
    const char* password(options.get(s + ".password", 0));
    const char* cadir(options.get(s + ".cadir", 0));
    const char* cafile(options.get(s + ".cafile", 0));
    const char* ciphers(options.get(s + ".ciphers", 0));
    int verify(atoi(options.get(s + ".verify", "1")));

    sslctxptr ptr(new sslctx());
    SSL_CTX* ctx(ptr->ctx_);

    // Load our keys and certificates.

    if (certfile && !SSL_CTX_use_certificate_chain_file(ctx, certfile))
        throw ssl_error(__FILE__, __LINE__, ERR_get_error());

    if (password) {
        SSL_CTX_set_default_passwd_cb(ctx, passwdcb_);
        SSL_CTX_set_default_passwd_cb_userdata
            (ctx, const_cast<char*>(password));
    }

    if (keyfile && !SSL_CTX_use_PrivateKey_file(ctx, keyfile,
                                                SSL_FILETYPE_PEM))
        throw ssl_error(__FILE__, __LINE__, ERR_get_error());

    // Load the CAs we trust.

    if ((cafile || cadir)
        && !SSL_CTX_load_verify_locations(ctx, cafile, cadir))
        throw ssl_error(__FILE__, __LINE__, ERR_get_error());

    if (ciphers && !SSL_CTX_set_cipher_list(ctx, ciphers))
        throw ssl_error(__FILE__, __LINE__, ERR_get_error());

    SSL_CTX_set_info_callback(ctx, infocb_);
    SSL_CTX_set_verify(ctx, tomode_(verify), verifycb_);

#if SSLEAY_VERSION_NUMBER >= 0x00906000L
    SSL_CTX_set_mode(ctx, SSL_MODE_ENABLE_PARTIAL_WRITE);
#endif // >= OpenSSL-0.9.6

#if OPENSSL_VERSION_NUMBER < 0x00905100L
    SSL_CTX_set_verify_depth(ctx, 1);
#endif // < OpenSSL-0.9.5

    return ptr;
}

void
augas::createsslctxs(sslctxs& sslctxs, const options& options)
{
    const char* contexts = options.get("ssl.contexts", 0);

    if (contexts) {

        istringstream is(contexts);
        string name;
        while (is >> name)
            sslctxs.insert(make_pair(name, createsslctx(name, options)));
    }
}

#endif // HAVE_OPENSSL_SSL_H
