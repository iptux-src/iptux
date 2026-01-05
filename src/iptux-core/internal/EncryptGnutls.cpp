#include "Encrypt.h"
#include "iptux-utils/output.h"
#include <gnutls/abstract.h>
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

using namespace std;

std::pair<std::string, std::string> generate_rsa_keypair(int bits) {
  pair<string, string> keypair;
  int ret;

  gnutls_global_init();

  gnutls_privkey_t privkey;
  ret = gnutls_privkey_init(&privkey);
  if (ret < 0) {
    LOG_WARN("gnutls_privkey_init failed: %s", gnutls_strerror(ret));
    return keypair;
  }

  ret = gnutls_privkey_generate(privkey, GNUTLS_PK_RSA, bits, 0);
  if (ret < 0) {
    LOG_WARN("gnutls_privkey_generate failed: %s", gnutls_strerror(ret));
    gnutls_privkey_deinit(privkey);
    return keypair;
  }

  return keypair;
}
