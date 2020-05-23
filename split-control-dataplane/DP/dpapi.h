#ifndef DPI_API_H
#define DPI_API_H

/* INCLUDE */
#include <inttypes.h>

#include "dptables.h"

/* DEFINE */

/* FUNCTIONS */
int32_t updateAcl_ipv4(struct dp_ipv4Tuple *param, struct dp_resultTuple *reslt);
int32_t updateAcl_ipv4Tcp(struct dp_ipv4Tuple *param, struct dp_resultTuple *reslt);
int32_t updateAcl_ipv4Udp(struct dp_ipv4Tuple *param, struct dp_resultTuple *reslt);
int32_t updateAcl_ipv4Icmp(struct dp_ipv4Tuple *param, struct dp_resultTuple *reslt);
int32_t updateAcl_ipv4Http(struct dp_ipv4Tuple *param, struct dp_resultTuple *reslt);
int32_t updateAcl_ipv4Ssl(struct dp_ipv4Tuple *param, struct dp_resultTuple *reslt);
int32_t updateAcl_ipv4Tls(struct dp_ipv4Tuple *param, struct dp_resultTuple *reslt);
int32_t updateAcl_ipv4Smtp(struct dp_ipv4Tuple *param, struct dp_resultTuple *reslt);
int32_t updateAcl_ipv4Ftp(struct dp_ipv4Tuple *param, struct dp_resultTuple *reslt);
int32_t updateAcl_ipv4Sctp(struct dp_ipv4Tuple *param, struct dp_resultTuple *reslt);
int32_t updateAcl_ipv4Gre(struct dp_ipv4Tuple *param, struct dp_resultTuple *reslt);
int32_t updateAcl_ipv6(struct dp_ipv6Tuple *param, struct dp_resultTuple *reslt);
int32_t updateAcl_ipv6Tcp(struct dp_ipv6Tuple *param, struct dp_resultTuple *reslt);
int32_t updateAcl_ipv6Udp(struct dp_ipv6Tuple *param, struct dp_resultTuple *reslt);
int32_t updateAcl_ipv6Icmp(struct dp_ipv6Tuple *param, struct dp_resultTuple *reslt);
int32_t updateAcl_ipv6Http(struct dp_ipv6Tuple *param, struct dp_resultTuple *reslt);
int32_t updateAcl_ipv6Ssl(struct dp_ipv6Tuple *param, struct dp_resultTuple *reslt);
int32_t updateAcl_ipv6Tls(struct dp_ipv6Tuple *param, struct dp_resultTuple *reslt);
int32_t updateAcl_ipv6Smtp(struct dp_ipv6Tuple *param, struct dp_resultTuple *reslt);
int32_t updateAcl_ipv6Ftp(struct dp_ipv6Tuple *param, struct dp_resultTuple *reslt);
int32_t updateAcl_ipv6Sctp(struct dp_ipv6Tuple *param, struct dp_resultTuple *reslt);
int32_t updateAcl_ipv6Gre(struct dp_ipv6Tuple *param, struct dp_resultTuple *reslt);

#endif /* DPI_API_H */
