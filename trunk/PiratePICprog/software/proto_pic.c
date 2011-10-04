
#include <stdio.h>

#include "pic12.h"
#include "pic16.h"
#include "pic18.h"
#include "pic24.h"

#include "proto_pic.h"

struct proto_ops_t proto_ops[] = {
	[PROTO_PIC12] = {
	},
	[PROTO_PIC16] = {
		.EnterICSP = PIC16_EnterICSP,
		.ExitICSP = PIC16_ExitICSP,
		.ReadID = PIC16_ReadID,
		.Read = PIC16_Read,
		.Write = PIC16_Write,
		.Erase = PIC16_Erase,
		.WriteFlash = PIC16_WriteFlash,
		.ReadFlash = PIC16_ReadFlash,
	},
	[PROTO_PIC18] = {
		.EnterICSP = PIC18_EnterICSP,
		.ExitICSP = PIC18_ExitICSP,
		.ReadID = PIC18_ReadID,
		.Read = PIC18_Read,
		.Write = PIC18_Write,
		.Erase = PIC18_Erase,
		.WriteFlash = PIC18_WriteFlash,
		.ReadFlash = PIC18_ReadFlash,
	},
	[PROTO_PIC24] = {
		.EnterICSP = PIC24_EnterICSP,
		.ExitICSP = PIC24_ExitICSP,
		.ReadID = PIC24_ReadID,
		.Read = PIC24_Read,
		.Write = PIC24_Write,
		.Erase = PIC24_Erase,
		.WriteFlash = PIC24_WriteFlash,
		.ReadFlash = PIC24_ReadFlash,
	}
};

struct proto_ops_t *Proto_GetOps(enum proto_t protocol) {
	if (protocol >= PROTO_LAST)
		return NULL;

	return &proto_ops[protocol];
}
