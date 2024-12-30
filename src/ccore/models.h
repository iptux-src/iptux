/* SPDX-License-Identifier: LGPL-2.1-or-later */
#pragma once

#include <glib.h>
#include <netinet/in.h>
#include <stdint.h>

G_BEGIN_DECLS

typedef enum IptuxMsgSrcType {
  IPTUX_MSG_SRC_PAL,
  IPTUX_MSG_SRC_SELF,
  IPTUX_MSG_SRC_ERROR,
} IptuxMsgSrcType;

typedef enum IptuxMsgContentType {
  IPTUX_MSG_CONTENT_STRING,
  IPTUX_MSG_CONTENT_PICTURE,
} IptuxMsgContentType;

typedef enum IptuxGroupBelongType {
  IPTUX_GROUP_BELONG_REGULAR,
  IPTUX_GROUP_BELONG_SEGMENT,
  IPTUX_GROUP_BELONG_GROUP,
  IPTUX_GROUP_BELONG_BROADCAST,
} IptuxGroupBelongType;

typedef struct IptuxPalKey {
  struct in_addr ipv4;
  int port;
} IptuxPalKey;

IptuxPalKey iptux_pal_key(in_addr_t ipv4, int port);
gboolean iptux_pal_key_equal(const IptuxPalKey* a, const IptuxPalKey* b);
in_addr iptux_pal_key_ipv4(const IptuxPalKey* key);
int iptux_pal_key_port(const IptuxPalKey* key);
GString* iptux_pal_key_to_ipv4_string(const IptuxPalKey* key);
GString* iptux_pal_key_to_string(const IptuxPalKey* key);

struct IptuxPalInfo {
  IptuxPalKey key;
  GString* name;
  GString* user;
  GString* host;
  GString* version;
  GString* encode;
  GString* group;
  GString* icon_file;
  GString* segdes;
  GString* photo;
  GString* sign;
  uint32_t packetn;
  uint32_t rpacketn;
  uint8_t compatible : 1;
  uint8_t online : 1;
  uint8_t changed : 1;
  uint8_t in_blacklist : 1;
};

G_END_DECLS
