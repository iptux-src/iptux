#pragma once

#include "gio/gio.h"
#include "iptux-config.h"
#include "iptux-pal.h"
#include <glib-object.h>

G_BEGIN_DECLS

#define IPTUX_TYPE_SERVICE (iptux_service_get_type())
G_DECLARE_FINAL_TYPE(IptuxService, iptux_service, IPTUX, SERVICE, GObject)

IptuxService* iptux_service_new(IptuxConfig* config);

bool iptux_service_start(IptuxService* self);
bool iptux_service_stop(IptuxService* self);

/**
 * iptux_service_get_pals:
 * @self: An #IptuxService instance.
 *
 * Gets the list of pals.
 *
 * Returns: (element-type IptuxPal) (transfer container): A #GArray of pals.
 */
GArray* iptux_service_get_pals(IptuxService* self);

/**
 * iptux_service_send_message:
 * @self: An #IptuxService instance.
 * @pal: An #IptuxPal recipient.
 * @message: The message string to send.
 * @error: (nullable): a #GError location to store the error.
 *
 * Sends a message to @pal.
 *
 * Returns: %TRUE on success, %FALSE on error.
 */
gboolean iptux_service_send_message(IptuxService* self,
                                    IptuxPal* pal,
                                    const gchar* message,
                                    GError** error);

/**
 * iptux_service_send_message_to:
 * @self: An #IptuxService instance.
 * @ipv4: The IPv4 address of the recipient.
 * @message: The message string to send.
 * @error: (nullable): a #GError location to store the error.
 *
 * Sends a message to the pal with IPv4 address @ipv4. The pal must be
 * known to this service (e.g. after discovery or a detect packet).
 *
 * Returns: %TRUE on success, %FALSE if the pal is unknown or on error.
 */
gboolean iptux_service_send_message_to(IptuxService* self,
                                       const gchar* ipv4,
                                       const gchar* message,
                                       GError** error);

/**
 * iptux_service_send_detect_packet:
 * @self: An #IptuxService instance.
 * @ipv4: The IPv4 address to probe.
 *
 * Sends a detect (entry probe) packet to @ipv4, so that the peer
 * announces itself to this service.
 */
void iptux_service_send_detect_packet(IptuxService* self, const gchar* ipv4);

/**
 * iptux_service_send_file_to:
 * @self: An #IptuxService instance.
 * @ipv4: The IPv4 address of the recipient.
 * @path: The local path of the file (or directory) to send.
 * @error: (nullable): a #GError location to store the error.
 *
 * Announces @path to the pal with IPv4 address @ipv4, like dragging a
 * file into a chat window in the GUI. The pal must be known to this
 * service. The actual transfer starts when the peer accepts the file.
 *
 * Returns: %TRUE on success, %FALSE on error.
 */
gboolean iptux_service_send_file_to(IptuxService* self,
                                    const gchar* ipv4,
                                    const gchar* path,
                                    GError** error);

/**
 * iptux_service_recv_file:
 * @self: An #IptuxService instance.
 * @index: The index of the announced file, as delivered by the
 *   #IptuxService::file-received signal.
 * @save_dir: The directory to save the received file into.
 * @error: (nullable): a #GError location to store the error.
 *
 * Accepts a file announced by a pal and starts receiving it into
 * @save_dir asynchronously.
 *
 * Returns: %TRUE on success, %FALSE on error.
 */
gboolean iptux_service_recv_file(IptuxService* self,
                                 guint index,
                                 const gchar* save_dir,
                                 GError** error);

/**
 * iptux_service_send_message_async:
 * @self: An #IptuxService instance.
 * @pal: An #IptuxPal recipient.
 * @message: The message string to send.
 * @cancellable: (nullable): A #GCancellable or %NULL.
 * @callback: (scope async) (closure user_data): A #GAsyncReadyCallback to call
 * when the request is satisfied.
 * @user_data: The data to pass to callback function.
 *
 * Asynchronously sends a message to a pal.
 */
void iptux_service_send_message_async(IptuxService* self,
                                      IptuxPal* pal,
                                      const gchar* message,
                                      GCancellable* cancellable,
                                      GAsyncReadyCallback callback,
                                      gpointer user_data);

/**
 * iptux_service_send_message_finish:
 * @self: An #IptuxService instance.
 * @result: A #GAsyncResult provided in the callback.
 * @error: A #GError location to store the error, or %NULL.
 *
 * Finishes the send message operation.
 *
 * Returns: %TRUE if the message was sent successfully, %FALSE otherwise.
 */
gboolean iptux_service_send_message_finish(IptuxService* self,
                                           GAsyncResult* result,
                                           GError** error);

void iptux_service_set_log_level(IptuxService* self, GLogLevelFlags level);

G_END_DECLS
