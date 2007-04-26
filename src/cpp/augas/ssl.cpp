/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "augas/ssl.hpp"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#if HAVE_OPENSSL_SSL_H

# include "augas/conn.hpp"
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

    void
    lognid_(const char* s, X509_name_st* name, int nid)
    {
        char buf[256];
        buf[0] = '\0';

        X509_NAME_get_text_by_NID(name, nid, buf, sizeof(buf));
        if (buf[0])
            aug_info("%s=[%s]", s, buf);
    }

    int
    verifycb_(int preverify_ok, X509_STORE_CTX* x509_ctx)
    {
        X509* peer(X509_STORE_CTX_get_current_cert(x509_ctx));
        X509_name_st* name(X509_get_subject_name(peer));

        char subject[256];
        X509_NAME_oneline(name, subject, sizeof(subject));
        aug_info("subject: %s", subject);

        char issuer[256];
        X509_NAME_oneline(X509_get_issuer_name(peer), issuer, sizeof(issuer));
        aug_info("issuer: %s", issuer);

        lognid_("country", name, NID_countryName);
        lognid_("state or province", name, NID_stateOrProvinceName);
        lognid_("locality", name, NID_localityName);
        lognid_("organisation", name, NID_organizationName);
        lognid_("organisational unit", name, NID_organizationalUnitName);
        lognid_("common name", name, NID_commonName);
        lognid_("email address", name, NID_pkcs9_emailAddress);

        if (!preverify_ok) {
            aug_warn("verification failed: %s",
                     X509_verify_cert_error_string(x509_ctx->error));
            return 0;
        }

        SSL* ssl(static_cast<SSL*>
                 (X509_STORE_CTX_get_ex_data
                  (x509_ctx, SSL_get_ex_data_X509_STORE_CTX_idx())));
        conn_base* conn(static_cast<conn_base*>(SSL_get_app_data(ssl)));

        return conn->authcert(subject, issuer) ? 1 : 0;
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

sslctx::sslctx(int verify)
    : verify_(verify),
      ctx_(SSL_CTX_new(SSLv23_method()))
{
    if (!ctx_)
        throw ssl_error(__FILE__, __LINE__, ERR_get_error());
}

sslctx::~sslctx() AUG_NOTHROW
{
    SSL_CTX_free(ctx_);
}

void
sslctx::setclient(conn_base& conn)
{
    SSL* ssl = SSL_new(ctx_);
    BIO* sbio = BIO_new_socket((int)aug_getosfd(conn.sfd().get()),
                               BIO_NOCLOSE);
    SSL_set_bio(ssl, sbio, sbio);
    SSL_set_app_data(ssl, &conn);
    aug_setsslclient(conn.sfd().get(), ssl);
}

void
sslctx::setserver(conn_base& conn)
{
    SSL* ssl = SSL_new(ctx_);
    BIO* sbio = BIO_new_socket((int)aug_getosfd(conn.sfd().get()),
                               BIO_NOCLOSE);
    SSL_set_bio(ssl, sbio, sbio);
    SSL_set_app_data(ssl, &conn);
    aug_setsslserver(conn.sfd().get(), ssl);
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
    const char* crlfile(options.get(s + ".crlfile", 0));
    const char* ciphers(options.get(s + ".ciphers", 0));
    int depth(atoi(options.get(s + ".depth", "1")));
    int verify(atoi(options.get(s + ".verify", "1")));

    sslctxptr ptr(new sslctx(verify));
    SSL_CTX* ctx(ptr->ctx_);

    // Load our keys and certificates.

    if (certfile && !SSL_CTX_use_certificate_chain_file(ctx, certfile))
        throw ssl_error(__FILE__, __LINE__, ERR_get_error());

    if (password) {
        SSL_CTX_set_default_passwd_cb(ctx, passwdcb_);
        SSL_CTX_set_default_passwd_cb_userdata
            (ctx, const_cast<char*>(password));
    }

    if ((certfile || keyfile)
        && !SSL_CTX_use_PrivateKey_file(ctx, keyfile ? keyfile : certfile,
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

    SSL_CTX_set_mode(ctx, SSL_MODE_ENABLE_PARTIAL_WRITE);
    SSL_CTX_set_verify_depth(ctx, verify);

    if (crlfile) {

        X509_STORE* store(SSL_CTX_get_cert_store(ctx));
        X509_LOOKUP* lookup;

        if (!(lookup = X509_STORE_add_lookup(store, X509_LOOKUP_file())))
            throw ssl_error(__FILE__, __LINE__, ERR_get_error());

        if (X509_load_crl_file(lookup, crlfile, X509_FILETYPE_PEM) != 1)
            throw ssl_error(__FILE__, __LINE__, ERR_get_error());

        X509_STORE_set_flags(store, X509_V_FLAG_CRL_CHECK
                             | X509_V_FLAG_CRL_CHECK_ALL);
    }

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
