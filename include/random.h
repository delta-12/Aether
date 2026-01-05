#ifndef AETHER_RANDOM_H
#define AETHER_RANDOM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

void a_Random_Seed(void);
uint32_t a_Random_Get32(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AETHER_RANDOM_H */
