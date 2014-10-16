/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2010,2011  Free Software Foundation, Inc.
 *
 *  GRUB is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GRUB is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GRUB.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GRUB_NET_HEADER
#define GRUB_NET_HEADER	1

#include <grub/types.h>
#include <grub/err.h>
#include <grub/list.h>
#include <grub/fs.h>
#include <grub/file.h>
#include <grub/mm.h>
#include <grub/net/netbuff.h>

enum
  {
    GRUB_NET_MAX_LINK_HEADER_SIZE = 64,
    GRUB_NET_UDP_HEADER_SIZE = 8,
    GRUB_NET_TCP_HEADER_SIZE = 20,
    GRUB_NET_OUR_IPV4_HEADER_SIZE = 20,
    GRUB_NET_OUR_IPV6_HEADER_SIZE = 40,
    GRUB_NET_OUR_MAX_IP_HEADER_SIZE = 40,
    GRUB_NET_TCP_RESERVE_SIZE = GRUB_NET_TCP_HEADER_SIZE 
    + GRUB_NET_OUR_IPV4_HEADER_SIZE
    + GRUB_NET_MAX_LINK_HEADER_SIZE
  };

typedef enum grub_link_level_protocol_id 
{
  GRUB_NET_LINK_LEVEL_PROTOCOL_ETHERNET
} grub_link_level_protocol_id_t;

typedef struct grub_net_link_level_address
{
  grub_link_level_protocol_id_t type;
  union
  {
    grub_uint8_t mac[6];
  };
} grub_net_link_level_address_t;

typedef enum grub_net_interface_flags
  {
    GRUB_NET_INTERFACE_HWADDRESS_IMMUTABLE = 1,
    GRUB_NET_INTERFACE_ADDRESS_IMMUTABLE = 2,
    GRUB_NET_INTERFACE_PERMANENT = 4
  } grub_net_interface_flags_t;

typedef enum grub_net_card_flags
  {
    GRUB_NET_CARD_HWADDRESS_IMMUTABLE = 1,
    GRUB_NET_CARD_NO_MANUAL_INTERFACES = 2
  } grub_net_card_flags_t;

struct grub_net_card;

struct grub_net_card_driver
{
  struct grub_net_card_driver *next;
  struct grub_net_card_driver **prev;
  const char *name;
  grub_err_t (*open) (struct grub_net_card *dev);
  void (*close) (struct grub_net_card *dev);
  grub_err_t (*send) (struct grub_net_card *dev,
		      struct grub_net_buff *buf);
  struct grub_net_buff * (*recv) (struct grub_net_card *dev);
};

typedef struct grub_net_packet
{
  struct grub_net_packet *next;
  struct grub_net_packet *prev;
  struct grub_net_packets *up;
  struct grub_net_buff *nb;
} grub_net_packet_t;

typedef struct grub_net_packets
{
  grub_net_packet_t *first;
  grub_net_packet_t *last;
  grub_size_t count;
} grub_net_packets_t;

#ifdef GRUB_MACHINE_EFI
#include <grub/efi/api.h>
#endif

struct grub_net_slaac_mac_list
{
  struct grub_net_slaac_mac_list *next;
  struct grub_net_slaac_mac_list **prev;
  grub_net_link_level_address_t address;
  int slaac_counter;
  char *name;
};

struct grub_net_link_layer_entry;

struct grub_net_card
{
  struct grub_net_card *next;
  struct grub_net_card **prev;
  const char *name;
  struct grub_net_card_driver *driver;
  grub_net_link_level_address_t default_address;
  grub_net_card_flags_t flags;
  int num_ifaces;
  int opened;
  unsigned idle_poll_delay_ms;
  grub_uint64_t last_poll;
  grub_size_t mtu;
  struct grub_net_slaac_mac_list *slaac_list;
  grub_ssize_t new_ll_entry;
  struct grub_net_link_layer_entry *link_layer_table;
  void *txbuf;
  void *rcvbuf;
  grub_size_t rcvbufsize;
  grub_size_t txbufsize;
  int txbusy;
  union
  {
#ifdef GRUB_MACHINE_EFI
    struct
    {
      struct grub_efi_simple_network *efi_net;
      grub_efi_handle_t efi_handle;
      grub_size_t last_pkt_size;
    };
#endif
    void *data;
    int data_num;
  };
};

struct grub_net_network_level_interface;

typedef enum grub_network_level_protocol_id 
{
  GRUB_NET_NETWORK_LEVEL_PROTOCOL_DHCP_RECV,
  GRUB_NET_NETWORK_LEVEL_PROTOCOL_IPV4,
  GRUB_NET_NETWORK_LEVEL_PROTOCOL_IPV6
} grub_network_level_protocol_id_t;

typedef enum
{
  DNS_OPTION_IPV4,
  DNS_OPTION_IPV6,
  DNS_OPTION_PREFER_IPV4,
  DNS_OPTION_PREFER_IPV6
} grub_dns_option_t;

typedef struct grub_net_network_level_address
{
  grub_network_level_protocol_id_t type;
  union
  {
    grub_uint32_t ipv4;
    grub_uint64_t ipv6[2];
  };
  grub_dns_option_t option;
} grub_net_network_level_address_t;

typedef struct grub_net_network_level_netaddress
{
  grub_network_level_protocol_id_t type;
  union
  {
    struct {
      grub_uint32_t base;
      int masksize; 
    } ipv4;
    struct {
      grub_uint64_t base[2];
      int masksize; 
    } ipv6;
  };
} grub_net_network_level_netaddress_t;

#define FOR_PACKETS(cont,var) for (var = (cont).first; var; var = var->next)

static inline grub_err_t
grub_net_put_packet (grub_net_packets_t *pkts, struct grub_net_buff *nb)
{
  struct grub_net_packet *n;

  n = grub_malloc (sizeof (*n));
  if (!n)
    return grub_errno;

  n->nb = nb;
  n->next = NULL;
  n->prev = NULL;
  n->up = pkts;
  if (pkts->first)
    {
      pkts->last->next = n;
      pkts->last = n;
      n->prev = pkts->last;
    }
  else
    pkts->first = pkts->last = n;

  pkts->count++;

  return GRUB_ERR_NONE;
}

static inline void
grub_net_remove_packet (grub_net_packet_t *pkt)
{
  pkt->up->count--;

  if (pkt->prev)
    pkt->prev->next = pkt->next;
  else
    pkt->up->first = pkt->next;
  if (pkt->next)
    pkt->next->prev = pkt->prev;
  else
    pkt->up->last = pkt->prev;
  grub_free (pkt);
}

typedef struct grub_net_app_protocol *grub_net_app_level_t;

typedef struct grub_net_socket *grub_net_socket_t;

struct grub_net_app_protocol 
{
  struct grub_net_app_protocol *next;
  struct grub_net_app_protocol **prev;
  const char *name;
  grub_err_t (*dir) (grub_device_t device, const char *path,
		     int (*hook) (const char *filename,
				  const struct grub_dirhook_info *info));
  grub_err_t (*open) (struct grub_file *file, const char *filename);
  grub_err_t (*seek) (struct grub_file *file, grub_off_t off);
  grub_err_t (*close) (struct grub_file *file);
  grub_err_t (*packets_pulled) (struct grub_file *file);
};

typedef struct grub_net
{
  char *server;
  char *name;
  grub_net_app_level_t protocol;
  grub_net_packets_t packs;
  grub_off_t offset;
  grub_fs_t fs;
  int eof;
  int stall;
} *grub_net_t;

extern grub_net_t (*EXPORT_VAR (grub_net_open)) (const char *name);

struct grub_net_network_level_interface
{
  struct grub_net_network_level_interface *next;
  struct grub_net_network_level_interface **prev;
  char *name;
  struct grub_net_card *card;
  grub_net_network_level_address_t address;
  grub_net_link_level_address_t hwaddress;
  grub_net_interface_flags_t flags;
  struct grub_net_bootp_packet *dhcp_ack;
  grub_size_t dhcp_acklen;
  void *data;
};

struct grub_net_session;

struct grub_net_session_level_protocol
{
  void (*close) (struct grub_net_session *session);
  grub_ssize_t (*recv) (struct grub_net_session *session, void *buf,
		       grub_size_t size);
  grub_err_t (*send) (struct grub_net_session *session, void *buf,
		      grub_size_t size);
};

struct grub_net_session
{
  struct grub_net_session_level_protocol *protocol;
  void *data;
};

static inline void
grub_net_session_close (struct grub_net_session *session)
{
  session->protocol->close (session);
}

static inline grub_err_t
grub_net_session_send (struct grub_net_session *session, void *buf,
		       grub_size_t size)
{
  return session->protocol->send (session, buf, size);
}

static inline grub_ssize_t
grub_net_session_recv (struct grub_net_session *session, void *buf,
		       grub_size_t size)
{
  return session->protocol->recv (session, buf, size);
}

struct grub_net_network_level_interface *
grub_net_add_addr (const char *name,
		   struct grub_net_card *card,
		   const grub_net_network_level_address_t *addr,
		   const grub_net_link_level_address_t *hwaddress,
		   grub_net_interface_flags_t flags);

extern struct grub_net_network_level_interface *grub_net_network_level_interfaces;
#define FOR_NET_NETWORK_LEVEL_INTERFACES(var) for (var = grub_net_network_level_interfaces; var; var = var->next)
#define FOR_NET_NETWORK_LEVEL_INTERFACES_SAFE(var,next) for (var = grub_net_network_level_interfaces, next = (var ? var->next : 0); var; var = next, next = (var ? var->next : 0))


extern grub_net_app_level_t grub_net_app_level_list;

#ifndef GRUB_LST_GENERATOR
static inline void
grub_net_app_level_register (grub_net_app_level_t proto)
{
  grub_list_push (GRUB_AS_LIST_P (&grub_net_app_level_list),
		  GRUB_AS_LIST (proto));
}
#endif

static inline void
grub_net_app_level_unregister (grub_net_app_level_t proto)
{
  grub_list_remove (GRUB_AS_LIST (proto));
}

#define FOR_NET_APP_LEVEL(var) FOR_LIST_ELEMENTS((var), \
						 (grub_net_app_level_list))

extern struct grub_net_card *grub_net_cards;

static inline void
grub_net_card_register (struct grub_net_card *card)
{
  grub_list_push (GRUB_AS_LIST_P (&grub_net_cards),
		  GRUB_AS_LIST (card));
}

void
grub_net_card_unregister (struct grub_net_card *card);

#define FOR_NET_CARDS(var) for (var = grub_net_cards; var; var = var->next)
#define FOR_NET_CARDS_SAFE(var, next) for (var = grub_net_cards, next = (var ? var->next : 0); var; var = next, next = (var ? var->next : 0))


struct grub_net_session *
grub_net_open_tcp (char *address, grub_uint16_t port);

grub_err_t
grub_net_resolve_address (const char *name,
			  grub_net_network_level_address_t *addr);

grub_err_t
grub_net_resolve_net_address (const char *name,
			      grub_net_network_level_netaddress_t *addr);

grub_err_t
grub_net_route_address (grub_net_network_level_address_t addr,
			grub_net_network_level_address_t *gateway,
			struct grub_net_network_level_interface **interf);


grub_err_t
grub_net_add_route (const char *name,
		    grub_net_network_level_netaddress_t target,
		    struct grub_net_network_level_interface *inter);

grub_err_t
grub_net_add_route_gw (const char *name,
		       grub_net_network_level_netaddress_t target,
		       grub_net_network_level_address_t gw);


#define GRUB_NET_BOOTP_MAC_ADDR_LEN	16

typedef grub_uint8_t grub_net_bootp_mac_addr_t[GRUB_NET_BOOTP_MAC_ADDR_LEN];

struct grub_net_bootp_packet
{
  grub_uint8_t opcode;
  grub_uint8_t hw_type;		/* hardware type.  */
  grub_uint8_t hw_len;		/* hardware addr len.  */
  grub_uint8_t gate_hops;	/* zero it.  */
  grub_uint32_t ident;		/* random number chosen by client.  */
  grub_uint16_t seconds;	/* seconds since did initial bootstrap.  */
  grub_uint16_t flags;
  grub_uint32_t	client_ip;
  grub_uint32_t your_ip;
  grub_uint32_t	server_ip;
  grub_uint32_t	gateway_ip;
  grub_net_bootp_mac_addr_t mac_addr;
  char server_name[64];
  char boot_file[128];
  grub_uint8_t vendor[0];
} GRUB_PACKED;

#define	GRUB_NET_BOOTP_RFC1048_MAGIC_0	0x63
#define	GRUB_NET_BOOTP_RFC1048_MAGIC_1	0x82
#define	GRUB_NET_BOOTP_RFC1048_MAGIC_2	0x53
#define	GRUB_NET_BOOTP_RFC1048_MAGIC_3	0x63

enum
  {
    GRUB_NET_BOOTP_PAD = 0x00,
    GRUB_NET_BOOTP_NETMASK = 0x01,
    GRUB_NET_BOOTP_ROUTER = 0x03,
    GRUB_NET_BOOTP_DNS = 0x06,
    GRUB_NET_BOOTP_HOSTNAME = 0x0c,
    GRUB_NET_BOOTP_DOMAIN = 0x0f,
    GRUB_NET_BOOTP_ROOT_PATH = 0x11,
    GRUB_NET_BOOTP_EXTENSIONS_PATH = 0x12,
    GRUB_NET_BOOTP_END = 0xff
  };

struct grub_net_network_level_interface *
grub_net_configure_by_dhcp_ack (const char *name,
				struct grub_net_card *card,
				grub_net_interface_flags_t flags,
				const struct grub_net_bootp_packet *bp,
				grub_size_t size,
				int is_def, char **device, char **path);

grub_err_t
grub_net_add_ipv4_local (struct grub_net_network_level_interface *inf,
			 int mask);

void
grub_net_process_dhcp (struct grub_net_buff *nb,
		       struct grub_net_card *card);

int
grub_net_hwaddr_cmp (const grub_net_link_level_address_t *a,
		     const grub_net_link_level_address_t *b);
int
grub_net_addr_cmp (const grub_net_network_level_address_t *a,
		   const grub_net_network_level_address_t *b);


/*
  Currently supported adresses:
  IPv4:   XXX.XXX.XXX.XXX
  IPv6:   XXXX:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX
 */
#define GRUB_NET_MAX_STR_ADDR_LEN sizeof ("XXXX:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX")

/*
  Currently suppoerted adresses:
  ethernet:   XX:XX:XX:XX:XX:XX
 */

#define GRUB_NET_MAX_STR_HWADDR_LEN (sizeof ("XX:XX:XX:XX:XX:XX"))

void
grub_net_addr_to_str (const grub_net_network_level_address_t *target,
		      char *buf);
void
grub_net_hwaddr_to_str (const grub_net_link_level_address_t *addr, char *str);

grub_err_t
grub_env_set_net_property (const char *intername, const char *suffix,
                           const char *value, grub_size_t len);

void
grub_net_poll_cards (unsigned time, int *stop_condition);

void grub_bootp_init (void);
void grub_bootp_fini (void);

void grub_dns_init (void);
void grub_dns_fini (void);

static inline void
grub_net_network_level_interface_unregister (struct grub_net_network_level_interface *inter)
{
  inter->card->num_ifaces--;
  *inter->prev = inter->next;
  if (inter->next)
    inter->next->prev = inter->prev;
  inter->next = 0;
  inter->prev = 0;
}

void
grub_net_tcp_retransmit (void);

void
grub_net_link_layer_add_address (struct grub_net_card *card,
				 const grub_net_network_level_address_t *nl,
				 const grub_net_link_level_address_t *ll,
				 int override);
int
grub_net_link_layer_resolve_check (struct grub_net_network_level_interface *inf,
				   const grub_net_network_level_address_t *proto_addr);
grub_err_t
grub_net_link_layer_resolve (struct grub_net_network_level_interface *inf,
			     const grub_net_network_level_address_t *proto_addr,
			     grub_net_link_level_address_t *hw_addr);
grub_err_t
grub_net_dns_lookup (const char *name,
		     const struct grub_net_network_level_address *servers,
		     grub_size_t n_servers,
		     grub_size_t *naddresses,
		     struct grub_net_network_level_address **addresses,
		     int cache);
grub_err_t
grub_net_add_dns_server (const struct grub_net_network_level_address *s);
void
grub_net_remove_dns_server (const struct grub_net_network_level_address *s);


extern char *grub_net_default_server;

#define GRUB_NET_TRIES 40
#define GRUB_NET_INTERVAL 400
#define GRUB_NET_INTERVAL_ADDITION 20

#endif /* ! GRUB_NET_HEADER */
