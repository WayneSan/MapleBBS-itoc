/*-------------------------------------------------------*/
/* bbs.h	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : all header files			 	 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/


#ifndef	_BBS_H_
#define	_BBS_H_


#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/time.h>


#ifdef  SYSV

#ifndef LOCK_EX
#define LOCK_EX		F_LOCK
#define LOCK_UN		F_ULOCK
#endif

#define getdtablesize()         (64)

#define usleep(usec)            {               \
    struct timeval t;                           \
    t.tv_sec = usec / 1000000;                  \
    t.tv_usec = usec % 1000000;                 \
    select( 0, NULL, NULL, NULL, &t);           \
}

#endif				/* SYSV */


#define	BMIN(a,b)	((a<b)?a:b)
#define	BMAX(a,b)	((a>b)?a:b)


/* Flags to getdata input function */
#define NOECHO		0x0000		/* ����ܡA�Ω�K�X���o */
#define DOECHO		0x0100		/* �@����� */
#define LCECHO		0x0200		/* low case echo�A�����p�g */
#define GCARRY		0x0400		/* �|��ܤW�@��/�ثe���� */

#define	GET_LIST	0x1000		/* ���o Link List */
#define	GET_USER	0x2000		/* ���o user id */
#define	GET_BRD		0x4000		/* ���o board id */


#include "config.h"		/* User-configurable stuff */
#include "dao.h"
#include "perm.h"		/* user/board permission */
#include "ufo.h"		/* user flag option */
#include "battr.h"		/* board attribution */
#include "modes.h"		/* The list of valid user modes */
#include "struct.h"		/* data structure */
#include "global.h"		/* global variable & definition */
#include "theme.h"		/* custom theme */
#include "proto.h"		/* prototype of functions */

#endif				/* _BBS_H_ */
