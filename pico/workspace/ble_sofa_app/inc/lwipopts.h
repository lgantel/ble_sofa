#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Generally you would define your own explicit list of lwIP options
// (see https://www.nongnu.org/lwip/2_1_x/group__lwip__opts.html)
//
// This example uses a common include to avoid repetition
#include "lwipopts_examples_common.h"

// The following is needed to test mDns
#define LWIP_MDNS_RESPONDER 0
#define LWIP_IGMP 0
#define LWIP_NUM_NETIF_CLIENT_DATA 0
#define MDNS_RESP_USENETIF_EXTCALLBACK  0
#define MEMP_NUM_SYS_TIMEOUT (LWIP_NUM_SYS_TIMEOUT_INTERNAL + 3)
#define MEMP_NUM_TCP_PCB 12

// Enable cgi and ssi
#define LWIP_HTTPD_CGI 1
#define LWIP_HTTPD_SSI 1
#define LWIP_HTTPD_SSI_MULTIPART 0

#if !NO_SYS
#define TCPIP_THREAD_STACKSIZE 2048 // mDNS needs more stack
#define DEFAULT_THREAD_STACKSIZE 1024
#define DEFAULT_RAW_RECVMBOX_SIZE 8
#define TCPIP_MBOX_SIZE 8
#define LWIP_TIMEVAL_PRIVATE 0

// not necessary, can be done either way
#define LWIP_TCPIP_CORE_LOCKING_INPUT 1
#endif

// Use generated fsdata
//#define LWIP_HTTPD_CUSTOM_FILES 0
#define HTTPD_USE_CUSTOM_FSDATA 1
//#define HTTPD_FSDATA_FILE "fsdata.c"

#endif
