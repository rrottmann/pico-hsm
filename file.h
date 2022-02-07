#ifndef _FILE_H_
#define _FILE_H_

#include <stdlib.h>
#include "pico/stdlib.h"

#define FILE_TYPE_UNKNOWN       0x00
#define FILE_TYPE_DF            0x04
#define FILE_TYPE_INTERNAL_EF	0x03
#define FILE_TYPE_WORKING_EF    0x01
#define FILE_TYPE_BSO           0x10

/* EF structures */
#define FILE_EF_UNKNOWN             0x00
#define FILE_EF_TRANSPARENT         0x01
#define FILE_EF_LINEAR_FIXED        0x02
#define FILE_EF_LINEAR_FIXED_TLV    0x03
#define FILE_EF_LINEAR_VARIABLE     0x04
#define FILE_EF_LINEAR_VARIABLE_TLV 0x05
#define FILE_EF_CYCLIC              0x06
#define FILE_EF_CYCLIC_TLV          0x07

#define ACL_OP_DELETE_SELF      0x00
#define ACL_OP_CREATE_DF        0x01
#define ACL_OP_CREATE_EF        0x02
#define ACL_OP_DELETE_CHILD     0x03
#define ACL_OP_WRITE            0x04
#define ACL_OP_UPDATE_ERASE     0x05
#define ACL_OP_READ_SEARCH      0x06

#define SPECIFY_EF 0x1
#define SPECIFY_DF 0x2
#define SPECIFY_ANY 0x3

#define MAX_DEPTH 4

typedef struct file
{
    const uint16_t fid;
    const uint8_t parent; //entry number in the whole table!!
    const uint8_t *name;
    const uint8_t type;
    uint8_t *data; //should include 2 bytes len at begining
    const uint8_t ef_structure;
    const uint8_t acl[7];
} file_t;

extern file_t *currentEF;
extern file_t *currentDF;
extern const file_t *selected_applet;

extern const file_t *MF;
extern const file_t *file_last;
extern const file_t *file_openpgp;
extern const file_t *file_sc_hsm;
extern bool card_terminated;
extern file_t *file_pin1;
extern file_t *file_retries_pin1;
extern file_t *file_sopin;
extern file_t *file_retries_sopin;

extern file_t *search_by_fid(const uint16_t fid, const file_t *parent, const uint8_t sp);
extern file_t *search_by_name(uint8_t *name, uint16_t namelen);
extern file_t *search_by_path(const uint8_t *pe_path, uint8_t pathlen, const file_t *parent);
extern bool authenticate_action(const file_t *ef, uint8_t op);
extern void process_fci(const file_t *pe);

extern file_t file_entries[];

#endif

