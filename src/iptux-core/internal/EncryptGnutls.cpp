// SPDX-License-Identifier: GPL-2.0-or-later
#include "Encrypt.h"
#include "iptux-utils/output.h"
#include <gnutls/abstract.h>
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

using namespace std;

namespace iptux {

std::string Encrypt::genRsaPrivPem(int bits) {
  int ret;
  gnutls_datum_t out = {NULL, 0};

  gnutls_global_init();

  gnutls_x509_privkey_t privkey;
  ret = gnutls_x509_privkey_init(&privkey);
  if (ret < 0) {
    LOG_WARN("gnutls_x509_privkey_init failed: %s", gnutls_strerror(ret));
    return "";
  }

  ret = gnutls_x509_privkey_generate(privkey, GNUTLS_PK_RSA, bits, 0);
  if (ret < 0) {
    LOG_WARN("gnutls_x509_privkey_generate failed: %s", gnutls_strerror(ret));
    gnutls_x509_privkey_deinit(privkey);
    return "";
  }

  ret = gnutls_x509_privkey_export2_pkcs8(privkey, GNUTLS_X509_FMT_PEM, NULL, 0,
                                          &out);
  if (ret < 0) {
    LOG_WARN("gnutls_x509_privkey_export2_pkcs8 failed: %s",
             gnutls_strerror(ret));
    gnutls_x509_privkey_deinit(privkey);
    return "";
  }

  std::string keypair((char*)out.data, out.size);
  gnutls_free(out.data);
  gnutls_x509_privkey_deinit(privkey);
  return keypair;
}

}  // namespace iptux