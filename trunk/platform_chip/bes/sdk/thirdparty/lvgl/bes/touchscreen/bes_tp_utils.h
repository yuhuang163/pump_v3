#ifndef __BES_TP_UTILS_H__
#define __BES_TP_UTILS_H__

#include <stdint.h>

typedef enum tp_contact_e
{
    CONTACT_NONE = 0,
    CONTACT_DOWN,
    CONTACT_MOVE,
    CONTACT_UP
}tp_contact_t;

typedef struct tp_sample {
  uint8_t    id;
  tp_contact_t contact;
  bool       valid;
  uint16_t   x;
  uint16_t   y;
}tp_sample_t;

#endif