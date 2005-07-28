/*-------------------------------------------------------*/
/* web/bhttpd.c		( NTHU CS MapleBBS Ver 3.10 )	 */
/*-------------------------------------------------------*/
/* target : BBS's HTTP interface			 */
/* create : 03/07/09					 */
/* update : 04/02/21					 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#if 0	/* �s���@���� */

  http://my.domain/                            ����
  http://my.domain/boardlist                   �ݪO�C��
  http://my.domain/favorlist                   �ڪ��̷R
  http://my.domain/userlist                    �ϥΪ̦W��
  http://my.domain/brd/brdname&##              �峹�C��A�C�X�ݪO [brdname] �s�� ## �}�l�� 50 �g�峹
  http://my.domain/gem/brdname&folder          ��ذϦC��A�C�X�ݪO [brdname] ��ذϤ� folder �o�Ө��v�U���Ҧ��F��
  http://my.domain/mbox/##                     �H�c�C��A�C�X�H�c���s�� ## �}�l�� 50 �g�峹
  http://my.domain/brdmore/brdname&##          �\Ū�ݪO�峹�A�\Ū�ݪO [brdname] ���� ## �g�峹
  http://my.domain/brdmost/brdname&##          �\Ū�ݪO�峹�A�\Ū�ݪO [brdname] ���Ҧ��W�� ## �g�P���D���峹
  http://my.domain/gemmore/brdname&folder&##   �\Ū��ذϤ峹�A�\Ū�ݪO [brdname] ��ذϤ� folder �o�Ө��v�U���� ## �g�峹
  http://my.domain/mboxmore/##                 �\Ū�H�c�峹�A�\Ū�H�c���� ## �g�峹
  http://my.domain/dopost/brdname              �o��峹��ݪO [brdname]
  http://my.domain/domail/                     �o�e�H��
  http://my.domain/predelpost/brdname&##&###   �߰ݽT�w�R���ݪO [brdname] ���� ## �g�峹 (�� chrono �O ###)
  http://my.domain/delpost/brdname&##&###      �R���ݪO [brdname] ���� ## �g�峹 (�� chrono �O ###)
  http://my.domain/markpost/brdname&##&###     �аO�ݪO [brdname] ���� ## �g�峹 (�� chrono �O ###)
  http://my.domain/predelmail/##&###           �߰ݽT�w�R���H�c���� ## �g�峹 (�� chrono �O ###)
  http://my.domain/delmail/##&###              �R���H�c���� ## �g�峹 (�� chrono �O ###)
  http://my.domain/markmail/##&###             �аO�H�c���� ## �g�峹 (�� chrono �O ###)
  http://my.domain/query/userid                �d�� userid
  http://my.domain/image/filename              ��ܹ���

#endif


#define _MODES_C_

#include "bbs.h"


#define HTML_TALL	50	/* �C��@�� 50 �g */


/*-------------------------------------------------------*/
/* ��檺�C��						 */
/*-------------------------------------------------------*/


#define HCOLOR_BG	"#000000"	/* �I�����C�� */
#define HCOLOR_TEXT	"#ffffff"	/* ��r���C�� */
#define HCOLOR_LINK	"#00ffff"	/* ���s���L�s�����C�� */
#define HCOLOR_VLINK	"#c0c0c0"	/* �w�s���L�s�����C�� */
#define HCOLOR_ALINK	"#ff0000"	/* �s���Q���U�ɪ��C�� */

#define HCOLOR_NECK	"#000070"	/* ��l���C�� */
#define HCOLOR_TIE	"#a000a0"	/* ��a���C�� */

#define HCOLOR_BAR	"#808080"	/* �����C�� */


/* ----------------------------------------------------- */
/* HTML output basic function				 */
/* ----------------------------------------------------- */


#define LEN_COOKIE	(IDLEN + PSWDLEN + 5 + 1)	/* u=userid&p=passwd */

static int setcookie = 0;
static char cookie[LEN_COOKIE];


static char *
Gtime(now)
  time_t *now;
{
  static char datemsg[40];

  strftime(datemsg, sizeof(datemsg), "%a, %d %b %Y %T GMT", gmtime(now));
  return datemsg;
}


/* out_head() ���� <HTML> <BODY> <CENTER> �T�Ӥj�g���ҳe���� html ��
   ���� out_tail() �~�� </HTML> </BODY> </CENTER> �٭� */


static void
out_head(title)
  char *title;
{
  time_t now;

  /* HTTP 1.0 ���Y */
  time(&now);
  printf("HTTP/1.0 200 OK\r\n");
  printf("Date: %s\r\n", Gtime(&now));
  printf("Server: MapleBBS 3.10\r\n");
  if (setcookie)	/* acct_login() ���H��~�ݭn Set-Cookie */
    printf("Set-Cookie: user=%s; path=/\r\n", cookie);
  printf("Content-Type: text/html; charset="MYCHARSET"\r\n");
  printf("\r\n");

  /* html �ɮ׶}�l */
  printf("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\r\n");
  printf("<HTML>\r\n"
    "<style type=text/css>\r\n"
    "  pre {font-size: 15pt; line-height: 15pt; font-weight: lighter; background-color: 000000; color: c0c0c0;}\r\n"
    "  td  {font-size: 15pt; line-height: 15pt; font-weight: lighter;}\r\n"
    "</style>\r\n");

  printf("<head>\r\n"
    "  <meta http-equiv=Content-Type content=\"text/html; charset="MYCHARSET"\">\r\n"
    "  <title>�i"BBSNAME"�j%s</title>\r\n"
    "</head>\r\n", title);
  
  printf("<BODY bgcolor="HCOLOR_BG" text="HCOLOR_TEXT" link="HCOLOR_LINK" vlink="HCOLOR_VLINK" alink="HCOLOR_ALINK"><CENTER>\r\n");

  printf("<a href=/><img src=/image/site.jpg border=0></a><br><br>\r\n"
    "<input type=image src=/image/up.jpg onclick=history.go(-1);return;> / "
    "<a href=/boardlist><img src=/image/boardlist.jpg border=0></a> / "
    "<a href=/favorlist><img src=/image/favorlist.jpg border=0></a> / "
    "<a href=/mbox/0><img src=/image/mboxlist.jpg border=0></a> / "
    "<a href=/userlist><img src=/image/userlist.jpg border=0></a> / "
    "<a href=telnet://"MYHOSTNAME"><img src=/image/telnet.jpg border=0></a> / "
    "<a href=mailto:"STR_SYSOP".bbs@"MYHOSTNAME"><img src=/image/mailsysop.jpg border=0></a>"
    "<br>\r\n");
}


static void
out_mesg(msg)
  char *msg;
{
  printf("<br>%s<br><br>\r\n", msg);
}


static void
out_reload(msg)		/* God.050327: �N�D���� reload �������s�}���� */
  char *msg;
{
  printf("<BODY onLoad='opener.location.reload(); parent.close();'>\n");
  printf("<script language=VBScript>MsgBox \"%s\",vbInformation+vbOKonly,\"�T�{\"</script>\n", msg);
}


static void
out_tail()
{
  printf("</CENTER></BODY></HTML>\r\n");
}


/* ----------------------------------------------------- */
/* �N ANSI plain text �ন HTML file			 */
/* ----------------------------------------------------- */


#include "txt2htm.c"


static void
out_article(fpath)
  char *fpath;
{
  FILE *fp;

  if (fp = fopen(fpath, "r"))
  {
    show_style();
    txt2htm(fp);

    fclose(fp);
  }
}


static void
html_most(folder, title)
  char *folder, *title;
{
  int fd;
  char fpath[64];
  FILE *fp;
  HDR hdr;

  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    show_style();

    while (read(fd, &hdr, sizeof(HDR)) == sizeof(HDR))
    {
#ifdef HAVE_REFUSEMARK
      if (hdr.xmode & POST_RESTRICT)
	continue;
#endif

      if (!strcmp(str_ttl(hdr.title), title))
      {
	hdr_fpath(fpath, folder, &hdr);
	if (fp = fopen(fpath, "r"))
	{
	  txt2htm(fp);
	  fclose(fp);
	}
      }
    }
    close(fd);
  }
}


/* ----------------------------------------------------- */
/* �ѽX							 */
/* ----------------------------------------------------- */


#define is_hex(x)	((x >= '0' && x <= '9') || (x >= 'A' && x <= 'F'))
#define hex2int(x)	((x >= 'A') ? (x - 'A' + 10) : (x - '0'))


static void
str_dehtml(src, dst, size)
  char *src, *dst;
  int size;
{
  int ch, len;
  char *s, *d;

  len = 1;	/* �O�d 1 �� '\0' */
  s = src;
  d = dst;
  while (ch = *s++)
  {
    if (ch == '+')
    {
      ch = ' ';
    }
    else if (ch == '%')
    {
      if (is_hex(s[0]) && is_hex(s[1]))
      {
	ch = (hex2int(s[0]) << 4) + hex2int(s[1]);
	s += 2;
	if (ch == '\r')		/* '\r' �N���n�F */
	  continue;
      }
    }

    *d++ = ch;
    if (++len >= size)
      break;
  }
  *d = '\0';
}


/*-------------------------------------------------------*/
/* ���R�Ѽ�						 */
/*-------------------------------------------------------*/


static int			/* 1:���\ */
arg_analyze(argc, str, arg1, arg2, arg3)
  int argc;		/* ���X�ӰѼ� */
  char *str;		/* �޼� */
  char **arg1;		/* �ѼƤ@ */
  char **arg2;		/* �ѼƤG */
  char **arg3;		/* �ѼƤT */
{
  int i;
  char *ptr;

  ptr = str;
  for (i = 1; i <= argc; i++)
  {
    if (!*ptr)
      break;

    if (i == 1)
      *arg1 = ptr;
    else if (i == 2)
      *arg2 = ptr;
    else /* if (i == 3) */	/* �̦h�T�ӰѼ� */
    {
      *arg3 = ptr;
      continue;		/* ���L do-while */
    }

    do
    {
      if (*ptr == '&')
      {
	*ptr++ = '\0';
	break;;
      }
      ptr++;
    } while (*ptr);
  }

  return i > argc;
}   


/*-------------------------------------------------------*/
/* �ϥΪ̵n�J						 */
/*-------------------------------------------------------*/


static void
acct_login(str)
  char *str;
{
  char *userid, *passwd;
  char fpath[64], dst[LEN_COOKIE];
  ACCT acct;

  str_dehtml(str, dst, sizeof(dst));

  /* u=userid&p=passwd */

  if (!arg_analyze(2, dst, &userid, &passwd, NULL))
    return;

  userid += 2;	/* skip "u=" */
  passwd += 2;	/* skip "p=" */

  if (*userid && *passwd && strlen(userid) <= IDLEN && strlen(passwd) <= PSWDLEN)
  {
    usr_fpath(fpath, userid, FN_ACCT);
    if (!rec_get(fpath, &acct, sizeof(ACCT), 0) &&
      !(acct.userlevel & (PERM_DENYLOGIN | PERM_PURGE)) &&
      !chkpasswd(acct.passwd, passwd))				/* �n�J���\ */
    {
      /* itoc.040308: ���� Cookie */
      setcookie = 1;
      str_ncpy(cookie, str, sizeof(cookie));
    }
  }
}


static ACCT cuser;		/* �ϥ� cuser �e�A�O�o�n�� acct_fetch() */


static int		/* 1:�n�J���\ 0:�n�J���� */
acct_fetch()
{
  char *userid, *passwd;
  char fpath[64], dst[LEN_COOKIE];

  if (cookie[0])
  {
    str_dehtml(cookie, dst, sizeof(dst));

    /* u=userid&p=passwd */

    if (!arg_analyze(2, dst, &userid, &passwd, NULL))
      return;

    userid += 2;	/* skip "u=" */
    passwd += 2;	/* skip "p=" */
    
    if (*userid && *passwd && strlen(userid) <= IDLEN && strlen(passwd) <= PSWDLEN)
    {
      usr_fpath(fpath, userid, FN_ACCT);
      if (!rec_get(fpath, &cuser, sizeof(ACCT), 0) &&
	!HAS_PERM(PERM_DENYLOGIN | PERM_PURGE) &&
	!chkpasswd(cuser.passwd, passwd))			/* �n�J���\ */
	return 1;
    }
  }

  /* �S���n�J�B�n�J���� */
  cuser.userno = 0;
  cuser.userlevel = 0;
  strcpy(cuser.userid, STR_GUEST);
  cuser.ufo = 0;
  return 0;
}


/*-------------------------------------------------------*/
/* �i���ϥΪ̶��P pal.c/talk.c/bmw.c �ۮe		 */
/*-------------------------------------------------------*/


static int			/* 1: userno �b pool �W��W */
pertain_pal(pool, max, userno)	/* �Ѧ� pal.c:belong_pal() */
  int *pool;
  int max;
  int userno;
{
  int *up, datum, mid;

  up = pool;
  while (max > 0)
  {
    datum = up[mid = max >> 1];
    if (userno == datum)
    {
      return 1;
    }
    if (userno > datum)
    {
      up += (++mid);
      max -= mid;
    }
    else
    {
      max = mid;
    }
  }
  return 0;
}


static int
is_hisbad(up)			/* 1: ���]�ڬ��a�H */
  UTMP *up;			/* �Ѧ� pal.c:is_obad() */
{
#ifdef HAVE_BADPAL
  return pertain_pal(up->pal_spool, up->pal_max, -cuser.userno);
#else
  return 0;
#endif
}


static int			/* 1:�i�ݨ� 0:���i�ݨ� */
can_seen(up)			/* �Ѧ� bmw.c:can_see() */
  UTMP *up;
{
  usint mylevel, myufo, urufo;

  mylevel = cuser.userlevel;
  myufo = cuser.ufo;
  urufo = up->ufo;

  if (!(mylevel & PERM_SEECLOAK) && ((urufo & UFO_CLOAK) || is_hisbad(up)))
    return 0;

#ifdef HAVE_SUPERCLOAK
  if (!(myufo & UFO_SUPERCLOAK) && (urufo & UFO_SUPERCLOAK))
    return 0;
#endif

  return 1;
}


/*-------------------------------------------------------*/
/* UTMP shm �������P cache.c �ۮe			 */
/*-------------------------------------------------------*/


static UCACHE *ushm;


static void
init_ushm()
{
  ushm = shm_new(UTMPSHM_KEY, sizeof(UCACHE));
}


static UTMP *myusr[MAXACTIVE];
static int usr_num;


static int
userid_cmp(a, b)
  UTMP **a, **b;
{
  return str_cmp((*a)->userid, (*b)->userid);
}


static void
init_myusr()
{
  UTMP *uentp, *uceil;

  init_ushm();
  acct_fetch();

  uentp = ushm->uslot;
  uceil = (void *) uentp + ushm->offset;
  usr_num = 0;

  do
  {
    if (!uentp->pid || !uentp->userno || !can_seen(uentp))
      continue;

    myusr[usr_num] = uentp;
    usr_num++;
  } while (++uentp <= uceil);

  if (usr_num > 1)
    qsort(myusr, usr_num, sizeof(UTMP *), userid_cmp);
}


/* itoc.030711: �[�W�ˬd�ϥΪ̱b���������A�H�K���H�ÿ� */

static int
allow_userid(userid, warn)
  char *userid;
  int warn;
{
  int ch, rc;
  char *str;

  rc = 0;
  ch = strlen(userid);
  if (ch >= 2 && ch <= IDLEN && is_alpha(*userid))
  {
    rc = 1;
    str = userid;
    while (ch = *(++str))
    {
      if (!is_alnum(ch))
      {
	rc = 0;
	break;
      }
    }
  }

  if (warn && !rc)
    out_mesg("�������~�@�̩ҵo���峹");

  return rc;
}


/*-------------------------------------------------------*/
/* �ݪO�v���������P board.c �ۮe			 */
/*-------------------------------------------------------*/


static BCACHE *bshm;


#ifdef HAVE_MODERATED_BOARD
static int		/* !=0:�O�O�n  0:���b�W�椤 */
is_brdgood(bpal)	/* �Ѧ� pal.c:is_bgood() */
  BPAL *bpal;
{
  return pertain_pal(bpal->pal_spool, bpal->pal_max, cuser.userno);
}


static int		/* !=0:�O�O�a  0:���b�W�椤 */
is_brdbad(bpal)		/* �Ѧ� pal.c:is_bbad() */
  BPAL *bpal;
{
#ifdef HAVE_BADPAL
  return pertain_pal(bpal->pal_spool, bpal->pal_max, -cuser.userno);
#else
  return 0;
#endif
}
#endif


static int
Ben_Perm(bno, ulevel)	/* �Ѧ� board.c:Ben_Perm() */
  int bno;
  usint ulevel;
{
  BRD *brd;
  usint readlevel, postlevel, bits;
  char *blist, *bname;
#ifdef HAVE_MODERATED_BOARD
  BPAL *bpal;
  int ftype;	/* 0:�@��ID 1:�O�n 2:�O�a */

  /* itoc.040103: �ݪO�\Ū���Ż�����

  �z�w�w�w�w�s�w�w�w�w�s�w�w�w�w�s�w�w�w�w�{
  �x        �x�@��Τ�x�ݪO�n�͢x�ݪO�a�H�x
  �u�w�w�w�w�q�w�w�w�w�q�w�w�w�w�q�w�w�w�w�t
  �x�@��ݪO�x�v���M�w�x  ����  �x �ݤ��� �x    �ݤ����G�b�ݪO�C���L�k�ݨ�o�ӪO�A�]�i���h
  �u�w�w�w�w�q�w�w�w�w�q�w�w�w�w�q�w�w�w�w�t    �i���h�G�b�ݪO�C���i�H�ݨ�o�ӪO�A���O�i���h
  �x�n�ͬݪO�x �i���h �x  ����  �x  ����  �x    ��  ��G�b�ݪO�C���i�H�ݨ�o�ӪO�A�]�i�o�h�A���O����o��
  �u�w�w�w�w�q�w�w�w�w�q�w�w�w�w�q�w�w�w�w�t    ��  ��G�b�ݪO�C���i�H�ݨ�o�ӪO�A�]�i�o�h�εo��
  �x���K�ݪO�x �ݤ��� �x  ����  �x  ����  �x
  �|�w�w�w�w�r�w�w�w�w�r�w�w�w�w�r�w�w�w�w�}
  */

  static int bit_data[9] =
  {                /* �@��Τ�   �ݪO�n��                           �ݪO�a�H */
    /* ���}�ݪO */    0,         BRD_L_BIT | BRD_R_BIT,             0,
    /* �n�ͬݪO */    BRD_L_BIT, BRD_L_BIT | BRD_R_BIT | BRD_W_BIT, BRD_L_BIT | BRD_R_BIT,
    /* ���K�ݪO */    0,         BRD_L_BIT | BRD_R_BIT | BRD_W_BIT, BRD_L_BIT | BRD_R_BIT,
  };
#endif

  brd = bshm->bcache + bno;
  bname = brd->brdname;
  if (!*bname)
    return 0;

  readlevel = brd->readlevel;

#ifdef HAVE_MODERATED_BOARD
  bpal = bshm->pcache + bno;
  ftype = is_brdgood(bpal) ? 1 : is_brdbad(bpal) ? 2 : 0;

  if (readlevel == PERM_SYSOP)		/* ���K�ݪO */
    bits = bit_data[6 + ftype];
  else if (readlevel == PERM_BOARD)	/* �n�ͬݪO */
    bits = bit_data[3 + ftype];
  else if (ftype)			/* ���}�ݪO�A�Y�b�O�n/�O�a�W�椤 */
    bits = bit_data[ftype];
  else					/* ���}�ݪO�A��L���v���P�w */
#endif

  if (!readlevel || (readlevel & ulevel))
  {
    bits = BRD_L_BIT | BRD_R_BIT;

    postlevel = brd->postlevel;
    if (!postlevel || (postlevel & ulevel))
      bits |= BRD_W_BIT;
  }
  else
  {
    bits = 0;
  }

  /* Thor.980813.����: �S�O�� BM �Ҷq�A�O�D���ӪO���Ҧ��v�� */
  blist = brd->BM;
  if ((ulevel & PERM_BM) && blist[0] > ' ' && str_has(blist, cuser.userid, strlen(cuser.userid)))
    bits = BRD_L_BIT | BRD_R_BIT | BRD_W_BIT | BRD_X_BIT | BRD_M_BIT;

  /* itoc.030515: �ݪO�`�ޭ��s�P�_ */
  else if (ulevel & PERM_ALLBOARD)
    bits = BRD_L_BIT | BRD_R_BIT | BRD_W_BIT | BRD_X_BIT | BRD_M_BIT;

  return bits;
}


/*-------------------------------------------------------*/
/* BRD shm �������P cache.c �ۮe			 */
/*-------------------------------------------------------*/


static void
init_bshm()
{
  /* itoc.030727: �b�}�� bbsd ���e�A���ӴN�n����L account�A
     �ҥH bshm ���Ӥw�]�w�n */

  bshm = shm_new(BRDSHM_KEY, sizeof(BCACHE));

  if (bshm->uptime <= 0)	/* bshm ���]�w���� */
    exit(0);
}


static BRD *mybrd[MAXBOARD];
static int brd_num;


static int
brdtitle_cmp(a, b)
  BRD **a, **b;
{
  /* itoc.010413: ����/�O�W��e��� */
  int k = strcmp((*a)->class, (*b)->class);
  return k ? k : str_cmp((*a)->brdname, (*b)->brdname);
}


static void
init_mybrd()
{
  BRD *bhdr;
  usint ulevel;
  int bno, max;

  init_bshm();
  acct_fetch();

  ulevel = cuser.userlevel;
  bhdr = bshm->bcache;
  bno = 0;
  max = bshm->number;
  brd_num = 0;

  do
  {
    if (bhdr->brdname[0])	/* �ݪO�S�Q�R�� */
    {
      if (Ben_Perm(bno, ulevel) & BRD_R_BIT)
      {
	mybrd[brd_num] = bhdr;
	brd_num++;
      }
    }
    bhdr++;
  } while (++bno < max);

  if (brd_num > 1)
    qsort(mybrd, brd_num, sizeof(BRD *), brdtitle_cmp);
}


static void
init_myfavor()
{
  BRD *bhdr;
  usint ulevel;
  int bno, max;
  char fpath[64];
  FILE *fp;
  MF mf;

  init_bshm();
  acct_fetch();

  ulevel = cuser.userlevel;
  max = bshm->number;
  brd_num = 0;
  usr_fpath(fpath, cuser.userid, "MF/"FN_MF);

  if (fp = fopen(fpath, "r"))
  {
    while (fread(&mf, sizeof(MF), 1, fp) == 1)
    {
      /* �u�䴩�Ĥ@�h���ݪO */
      if (mf.mftype & MF_BOARD)
      {
	bhdr = bshm->bcache;
	bno = 0;

	do
	{
	  if (!strcmp(bhdr->brdname, mf.xname) && (Ben_Perm(bno, ulevel) & BRD_R_BIT))
	  {
	    mybrd[brd_num] = bhdr;
	    brd_num++;
	  }
	  bhdr++;
	} while (++bno < max);
      }
    }
    fclose(fp);
  }

#if 0	/* �ڪ��̷R�̨ϥΪ̦ۤv���Ƨ� */
  if (brd_num > 1)
    qsort(mybrd, brd_num, sizeof(BRD *), brdtitle_cmp);
#endif
}


/* itoc.030708: �[�W�ˬd�ݪO�v���������A�H�K���H�ÿ� */

static BRD *
allow_brdname(brdname, bits, warn)
  char *brdname;
  usint bits;		/* �Y bits != BRD_R_BIT�A�|���K�� acct_fetch() */
  int warn;		/* 1: �Y disallow �n�� warning */
{
  BRD *bcache, *bhdr, *tail;

  bcache = bshm->bcache;
  bhdr = bcache;
  tail = bcache + bshm->number;

  do
  {
    if (!strcmp(bhdr->brdname, brdname))
    {
      /* �Y readlevel == 0�A��� guest �iŪ�A�L�� acct_fetch() */
      if (!bhdr->readlevel && bits == BRD_R_BIT)
	return bhdr;

      if (acct_fetch() && (Ben_Perm(bhdr - bcache, cuser.userlevel) & bits))
	return bhdr;

      break;
    }
  } while (++bhdr < tail);

  if (warn)
    out_mesg("�ާ@���~�A�i���]���G�L���ݪO�B�z�|���n�J�B�z���v������");

  return NULL;
}


/*-------------------------------------------------------*/
/* �o��s�峹						 */
/*-------------------------------------------------------*/


static inline void
outgo_post(hdr, board)
  HDR *hdr;
  char *board;
{
  bntp_t bntp;

  memset(&bntp, 0, sizeof(bntp_t));
  strcpy(bntp.board, board);
  strcpy(bntp.xname, hdr->xname);
  strcpy(bntp.owner, hdr->owner);
  strcpy(bntp.nick, hdr->nick);
  strcpy(bntp.title, hdr->title);
  rec_add("innd/out.bntp", &bntp, sizeof(bntp_t));
}


static int		/* 1:���\ */
add_post(str, size)
  char *str;
  int size;
{
  char *dst;
  char *brdname, *title, *content;
  char *from;
  char folder[64], fpath[64];
  HDR hdr;
  BRD *brd;
  FILE *fp;
  struct sockaddr_in sin;
  struct hostent *hp;
  int len;

  dst = (char *) malloc(size);
  str_dehtml(str, dst, size);

  /* b=brdname&t=title&c=content */

  if (arg_analyze(3, dst, &brdname, &title, &content))
  {
    brdname += 2;	/* skip "b=" */
    title += 2;		/* skip "t=" */
    content += 2;	/* skip "c=" */

    if (*brdname && *title && *content)
    {
      init_bshm();

      if (brd = allow_brdname(brdname, BRD_W_BIT, 1))
      {
	/* ���o�ӷ� */
	len = sizeof(sin);
	getpeername(0, (struct sockaddr *) &sin, &len);
	from = (hp = gethostbyaddr((char *) &sin.sin_addr, sizeof(struct in_addr), sin.sin_family)) ? 
	  (char *) hp->h_name : inet_ntoa(sin.sin_addr);

	brd_fpath(folder, brdname, FN_DIR);

	fp = fdopen(hdr_stamp(folder, 'A', &hdr, fpath), "w");
	fprintf(fp, "%s %s (%s) %s %s\n",
	  STR_AUTHOR1, cuser.userid, cuser.username,
	  STR_POST2, brdname);
	fprintf(fp, "���D: %s\n�ɶ�: %s\n\n", title, Now());
	fprintf(fp, "%s\n", content);
	fprintf(fp, EDIT_BANNER, cuser.userid, from);
	fclose(fp);

	hdr.xmode = (brd->battr & BRD_NOTRAN) ? 0 : POST_OUTGO;
	strcpy(hdr.owner, cuser.userid);
	strcpy(hdr.nick, cuser.username);
	strcpy(hdr.title, title);
	rec_bot(folder, &hdr, sizeof(HDR));

	brd->btime = -1;
	if (hdr.xmode & POST_OUTGO)
	  outgo_post(&hdr, brdname);

	free(dst);
	return 1;
      }
    }
  }
  free(dst);
  return 0;
}


/*-------------------------------------------------------*/
/* �o�e�s�H						 */
/*-------------------------------------------------------*/


static int		/* 1:���\ */
add_mail(str, size)
  char *str;
  int size;
{
  char *dst;
  char *userid, *title, *content;
  char *from;
  char folder[64], fpath[64];
  HDR hdr;
  FILE *fp;
  struct sockaddr_in sin;
  struct hostent *hp;
  int len;

  dst = (char *) malloc(size);
  str_dehtml(str, dst, size);

  /* u=userid&t=title&c=content */

  if (arg_analyze(3, dst, &userid, &title, &content))
  {
    userid += 2;	/* skip "u=" */
    title += 2;		/* skip "t=" */
    content += 2;	/* skip "c=" */

    if (*userid && *title && *content && allow_userid(userid, 0))
    {
      usr_fpath(fpath, userid, FN_ACCT);
      if (dashf(fpath) && acct_fetch())
      {
	/* ���o�ӷ� */
	len = sizeof(sin);
	getpeername(0, (struct sockaddr *) &sin, &len);
	from = (hp = gethostbyaddr((char *) &sin.sin_addr, sizeof(struct in_addr), sin.sin_family)) ? 
	  (char *) hp->h_name : inet_ntoa(sin.sin_addr);

	usr_fpath(folder, userid, FN_DIR);

	fp = fdopen(hdr_stamp(folder, 0, &hdr, fpath), "w");
	fprintf(fp, "%s %s (%s)\n",
	  STR_AUTHOR1, cuser.userid, cuser.username);
	fprintf(fp, "���D: %s\n�ɶ�: %s\n\n", title, Now());
	fprintf(fp, "%s\n", content);
	fprintf(fp, EDIT_BANNER, cuser.userid, from);
	fclose(fp);

	strcpy(hdr.owner, cuser.userid);
	strcpy(hdr.nick, cuser.username);
	strcpy(hdr.title, title);
	rec_add(folder, &hdr, sizeof(HDR));

	free(dst);
	return 1;
      }
    }
  }
  free(dst);
  return 0;
}


/*-------------------------------------------------------*/
/* �ݪO�C��						 */
/*-------------------------------------------------------*/


static void
boardlist_neck()
{
  printf("<br>\r\n"
    "<table cellspacing=0 cellpadding=1 border=0 width=80%%>\r\n"
    "<tr>\r\n"
    "  <td width=100%% align=center bgcolor="HCOLOR_NECK">�ثe���W�� %d �ӪO</td>\r\n"
    "</tr>\r\n"
    "</table>\r\n"
    "<br>\r\n",
    brd_num);
}


static void
html_boardlist(str)
  char *str;
{
  int i;
  BRD *brd;

  init_mybrd();

  out_head("�ݪO�C��");

  boardlist_neck();

  printf("<table cellspacing=0 cellpadding=4 border=0>\r\n"
    "<tr>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=40>�s��</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=80>�ݪO</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=40>���O</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=25>��</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=350>����ԭz</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=75>�O�D</td>\r\n"
    "</tr>\r\n");

  for (i = 0; i < brd_num; i++)
  {
    brd = mybrd[i];
    printf("<tr onMouseOver=\"this.bgColor='"HCOLOR_BAR"'\" onMouseOut=\"this.bgColor='"HCOLOR_BG"'\">\r\n"
      "  <td>%d</td>\r\n"
      "  <td><a href=/brd/%s&0>%s</a></td>\r\n"
      "  <td>%s</td>\r\n"
      "  <td>%s</td>\r\n"
      "  <td>%.33s</td>\r\n"
      "  <td>%.13s</td>\r\n"
      "</tr>\r\n",
      i + 1, 
      brd->brdname, brd->brdname,
      brd->class, 
      (brd->battr & BRD_NOTRAN) ? ICON_NOTRAN_BRD : ICON_TRAN_BRD, 
      brd->title, 
      brd->BM);
  }

  printf("</table>\r\n");

  boardlist_neck();
}


/*-------------------------------------------------------*/
/* �ڪ��̷R						 */
/*-------------------------------------------------------*/


static void
favorlist_neck()
{
  printf("<br>\r\n"
    "<table cellspacing=0 cellpadding=1 border=0 width=80%%>\r\n"
    "<tr>\r\n"
    "  <td width=100%% align=center bgcolor="HCOLOR_NECK">�ڪ��̷R</td>\r\n"
    "</tr>\r\n"
    "</table>\r\n"
    "<br>\r\n");
}


static void
html_favorlist(str)
  char *str;
{
  int i;
  BRD *brd;

  init_myfavor();

  out_head("�ڪ��̷R");

  favorlist_neck();

  printf("<table cellspacing=0 cellpadding=4 border=0>\r\n"
    "<tr>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=40>�s��</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=80>�ݪO</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=40>���O</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=25>��</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=350>����ԭz</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=75>�O�D</td>\r\n"
    "</tr>\r\n");

  for (i = 0; i < brd_num; i++)
  {
    brd = mybrd[i];
    printf("<tr onMouseOver=\"this.bgColor='"HCOLOR_BAR"'\" onMouseOut=\"this.bgColor='"HCOLOR_BG"'\">\r\n"
      "  <td>%d</td>\r\n"
      "  <td><a href=/brd/%s&0>%s</a></td>\r\n"
      "  <td>%s</td>\r\n"
      "  <td>%s</td>\r\n"
      "  <td>%.33s</td>\r\n"
      "  <td>%.13s</td>\r\n"
      "</tr>\r\n",
      i + 1, 
      brd->brdname, brd->brdname,
      brd->class, 
      (brd->battr & BRD_NOTRAN) ? ICON_NOTRAN_BRD : ICON_TRAN_BRD, 
      brd->title, 
      brd->BM);
  }

  printf("</table>\r\n");

  favorlist_neck();
}


/*-------------------------------------------------------*/
/* �H�c�C��						 */
/*-------------------------------------------------------*/


static void
mboxlist_neck(start, total)
  int start, total;
{
  printf("<br>\r\n"
    "<table cellspacing=0 cellpadding=4 border=0 width=80%%>\r\n"
    "<tr>\r\n");

  if (start > HTML_TALL)
  {
    printf("  <td width=33%% align=center bgcolor="HCOLOR_NECK"><a href=/mbox/%d>�W%d�g</a></td>\r\n",
      start - HTML_TALL, HTML_TALL);
  }
  else
  {
    printf("  <td width=33%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  start += HTML_TALL;
  if (start <= total)
  {
    printf("  <td width=33%% align=center bgcolor="HCOLOR_NECK"><a href=/mbox/%d>�U%d�g</a></td>\r\n",
      start, HTML_TALL);
  }
  else
  {
    printf("  <td width=33%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  printf("  <td width=34%% align=center bgcolor="HCOLOR_NECK"><a href=/domail/ target=_blank>�o�e�H��</a></td>\r\n"
    "</tr>\r\n"
    "</table>\r\n"
    "<br>\r\n");
}


static void
html_mboxlist(str)
  char *str;
{
  int fd, start, total;
  char folder[64], *number, *ptr1, *ptr2;
  HDR hdr;

  out_head("�H�c�C��");

  if (!acct_fetch())
  {
    out_mesg("�z�|���n�J");
    return;
  }

  if (!arg_analyze(1, str, &number, NULL, NULL))
    return;

  usr_fpath(folder, cuser.userid, FN_DIR);

  start = atoi(number);
  total = rec_num(folder, sizeof(HDR));
  if (start <= 0 || start > total)	/* �W�L�d�򪺸ܡA������̫�@�� */
    start = (total - 1) / HTML_TALL * HTML_TALL + 1;

  mboxlist_neck(start, total);

  printf("<table cellspacing=0 cellpadding=4 border=0>\r\n"
    "<tr>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=15>�R</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=15>��</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=50>�s��</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=10>m</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=50>���</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=100>�@��</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=400>���D</td>\r\n"
    "</tr>\r\n");

  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    int i, end;

    /* �q�X�H�c���� start �g�}�l�� HTML_TALL �g */
    i = start;
    end = i + HTML_TALL;

    lseek(fd, (off_t) (sizeof(HDR) * (i - 1)), SEEK_SET);

    while (i < end && read(fd, &hdr, sizeof(HDR)) == sizeof(HDR))
    {
      if (ptr1 = strchr(hdr.owner, '.'))	/* ���~�@�� */
	*(ptr1 + 1) = '\0';
      if (ptr2 = strchr(hdr.owner, '@'))	/* ���~�@�� */
	*ptr2 = '\0';

      printf("<tr onMouseOver=\"this.bgColor='"HCOLOR_BAR"'\" onMouseOut=\"this.bgColor='"HCOLOR_BG"'\">\r\n"
	"  <td><a href=/predelmail/%d&%d><img src=/image/delpost.gif border=0></a></td>\r\n"
	"  <td><a href=/markmail/%d&%d><img src=/image/markpost.gif border=0></a></td>\r\n"
	"  <td>%d</td>\r\n"
	"  <td>%c</td>\r\n"
	"  <td>%s</td>\r\n"
	"  <td><a href=/query/%s>%s</a></td>\r\n"
	"  <td><a href=/mboxmore/%d>%.50s</a></td>\r\n"
	"</tr>\r\n",
	i, hdr.chrono,
	i, hdr.chrono,
	i,
	hdr.xmode & POST_MARKED ? 'm' : ' ',
	hdr.date + 3,
	(ptr1 || ptr2 ? "���~�@��" : hdr.owner), hdr.owner,
	i, hdr.title);

      i++;
    }
    close(fd);
  }

  printf("</table>\r\n");

  mboxlist_neck(start, total);
}


/*-------------------------------------------------------*/
/* �ϥΪ̦W��						 */
/*-------------------------------------------------------*/


static void
userlist_neck()
{
  printf("<br>\r\n"
    "<table cellspacing=0 cellpadding=1 border=0 width=80%%>\r\n"
    "<tr>\r\n"
    "  <td width=100%% align=center bgcolor="HCOLOR_NECK">�ثe���W�� %d �ӤH</td>\r\n"
    "</tr>\r\n"
    "</table>\r\n"
    "<br>\r\n",
    usr_num);
}


static void
html_userlist(str)
  char *str;
{
  int i;
  UTMP *up;

  init_myusr();

  out_head("�ϥΪ̦W��");

  userlist_neck();

  printf("<table cellspacing=0 cellpadding=4 border=0>\r\n"
    "<tr>\r\n"
    "  <td bgcolor="HCOLOR_TIE">�s��</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE">���ͥN��</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE">���ͼʺ�</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE">�ȳ~�G�m</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE">���ͰʺA</td>\r\n"
    "</tr>\r\n");

  for (i = 0; i < usr_num; i++)
  {
    up = myusr[i];
    printf("<tr onMouseOver=\"this.bgColor='"HCOLOR_BAR"'\" onMouseOut=\"this.bgColor='"HCOLOR_BG"'\">\r\n"
      "  <td>%d</td>\r\n"
      "  <td><a href=/query/%s>%s</a></td>\r\n"
      "  <td>%s</td>\r\n"
      "  <td>%s</td>\r\n"
      "  <td>%s</td>\r\n"
      "</tr>\r\n",
      i + 1, 
      up->userid, up->userid, 
      up->username, 
      up->from, 
      ModeTypeTable[up->mode]);
  }

  printf("</table>\r\n");

  userlist_neck();
}


/*-------------------------------------------------------*/
/* �o��峹						 */
/*-------------------------------------------------------*/


static void
html_dopost(str)
  char *str;
{
  char *brdname;

  out_head("�o��峹");

  if (!arg_analyze(1, str, &brdname, NULL, NULL))
    return;

  init_bshm();

  if (!allow_brdname(brdname, BRD_W_BIT, 1))
    return;

  printf("<form method=post>\r\n"
    "  <input type=hidden name=dopost>\r\n"
    "  <input type=hidden name=b value=%s>\r\n"
    "  �п�J���D�G<br>\r\n"
    "  <input type=text name=t size=%d maxlength=%d><br><br>\r\n"
    "  �п�J���e�G<br>\r\n"
    "  <textarea name=c rows=10 cols=%d></textarea><br><br>\r\n"
    "  <input type=hidden name=end>\r\n"
    "  <input type=submit value=�e�X�峹> "
    "  <input type=reset value=���s��g>"
    "</form>\r\n",
    brdname,
    TTLEN, TTLEN,
    SCR_WIDTH);
}


/*-------------------------------------------------------*/
/* �o�e�H��						 */
/*-------------------------------------------------------*/


static void
html_domail(str)
  char *str;
{
  out_head("�o�e�H��");

  printf("<form method=post>\r\n"
    "  <input type=hidden name=domail>\r\n"
    "  �п�J�עҡG<br>\r\n"
    "  <input type=text name=u size=%d maxlength=%d><br><br>\r\n"
    "  �п�J���D�G<br>\r\n"
    "  <input type=text name=t size=%d maxlength=%d><br><br>\r\n"
    "  �п�J���e�G<br>\r\n"
    "  <textarea name=c rows=10 cols=%d></textarea><br><br>\r\n"
    "  <input type=hidden name=end>\r\n"
    "  <input type=submit value=�e�X�H��> "
    "  <input type=reset value=���s��g>"
    "</form>\r\n",
    IDLEN, IDLEN,
    TTLEN, TTLEN,
    SCR_WIDTH);
}


/*-------------------------------------------------------*/
/* �R���峹						 */
/*-------------------------------------------------------*/


static time_t currchrono;


static void
html_delpost(str)
  char *str;
{
  char folder[64], fpath[64], *brdname, *number, *stamp;
  int pos;
  BRD *isBM;
  HDR hdr;

  out_head("�R���峹");

  if (!arg_analyze(3, str, &brdname, &number, &stamp))
    return;

  if ((pos = atoi(number) - 1) < 0)
    return;
  if ((currchrono = atoi(stamp)) < 0)
    return;

  init_bshm();

  isBM = allow_brdname(brdname, BRD_X_BIT, 0);

  if (isBM || cuser.userlevel)	/* guest �����ۤv�峹 */
  {
    brd_fpath(folder, brdname, FN_DIR);

    if (!rec_get(folder, &hdr, sizeof(HDR), pos) &&
      (hdr.chrono == currchrono) && !(hdr.xmode & POST_MARKED) &&
      (!strcmp(cuser.userid, hdr.owner) || isBM))
    {
      rec_del(folder, sizeof(HDR), pos, NULL);

      hdr_fpath(fpath, folder, &hdr);
      unlink(fpath);
    }
    out_mesg("�w����R�����O�A�Y���R����ܦ��峹�Q�аO�F");
    printf("<a href=/brd/%s&%d>�^�峹�C��</a>\r\n", brdname, (pos - 1) / HTML_TALL * HTML_TALL + 1);
  }
}


static void
html_predelpost(str)
  char *str;
{
  char *brdname, *number, *stamp;

  out_head("�T�{�R���峹");

  if (!arg_analyze(3, str, &brdname, &number, &stamp))
    return;

  out_mesg("�Y�T�w�n�R�����g�峹�A�ЦA���I��H�U�s���F�Y�n�����R���A�Ы� [�W�@��]");
  printf("<a href=/delpost/%s&%s&%s>�R�� [%s] �O�� %s �g�峹</a><br>\r\n", 
    brdname, number, stamp, brdname, number);
}


static void
html_delmail(str)
  char *str;
{
  char folder[64], fpath[64], *number, *stamp;
  int pos;
  HDR hdr;

  out_head("�R���H��");

  if (!arg_analyze(2, str, &number, &stamp, NULL))
    return;

  if ((pos = atoi(number) - 1) < 0)
    return;
  if ((currchrono = atoi(stamp)) < 0)
    return;

  if (!acct_fetch())
    return;

  usr_fpath(folder, cuser.userid, FN_DIR);

  if (!rec_get(folder, &hdr, sizeof(HDR), pos) &&
    (hdr.chrono == currchrono) && !(hdr.xmode & POST_MARKED))
  {
    rec_del(folder, sizeof(HDR), pos, NULL);

    hdr_fpath(fpath, folder, &hdr);
    unlink(fpath);
  }
  out_mesg("�w����R�����O�A�Y���R����ܦ��H��Q�аO�F");
  printf("<a href=/mbox/0>�^�H�c�C��</a>\r\n");
}


static void
html_predelmail(str)
  char *str;
{
  char *number, *stamp;

  out_head("�T�{�R���H��");

  if (!arg_analyze(2, str, &number, &stamp, NULL))
    return;

  out_mesg("�Y�T�w�n�R�����g�H��A�ЦA���I��H�U�s���F�Y�n�����R���A�Ы� [�W�@��]");
  printf("<a href=/delmail/%s&%s>�R���H�c�� %s �g�H��</a><br>\r\n", 
    number, stamp, number);
}


/*-------------------------------------------------------*/
/* �аO�峹						 */
/*-------------------------------------------------------*/


static int
cmp_chrono(hdr)
  HDR *hdr;
{
  return (hdr->chrono == currchrono);
}


static void
markpost(hdd, ram)
  HDR *hdd, *ram;
{
  hdd->xmode ^= POST_MARKED;
}


static void
html_markpost(str)
  char *str;
{
  char folder[64], *brdname, *number, *stamp;
  int pos;

  out_head("�аO�峹");

  if (!arg_analyze(3, str, &brdname, &number, &stamp))
    return;

  if ((pos = atoi(number) - 1) < 0)
    return;
  if ((currchrono = atoi(stamp)) < 0)
    return;

  init_bshm();

  if (allow_brdname(brdname, BRD_X_BIT, 1))
  {
    brd_fpath(folder, brdname, FN_DIR);
    rec_ref(folder, NULL, sizeof(HDR), pos, cmp_chrono, markpost);
    out_mesg("�w����(����)�аO���O");
    printf("<a href=/brd/%s&%d>�^�峹�C��</a>\r\n", brdname, (pos - 1) / HTML_TALL * HTML_TALL + 1);
  }
}


static void
html_markmail(str)
  char *str;
{
  char folder[64], *number, *stamp;
  int pos;

  out_head("�аO�H��");

  if (!arg_analyze(2, str, &number, &stamp, NULL))
    return;

  if ((pos = atoi(number) - 1) < 0)
    return;
  if ((currchrono = atoi(stamp)) < 0)
    return;

  if (!acct_fetch())
    return;

  usr_fpath(folder, cuser.userid, FN_DIR);
  rec_ref(folder, NULL, sizeof(HDR), pos, cmp_chrono, markpost);
  out_mesg("�w����(����)�аO���O");
  printf("<a href=/mbox/0>�^�H�c�C��</a>\r\n");
}


/*-------------------------------------------------------*/
/* �d�ߨϥΪ�						 */
/*-------------------------------------------------------*/


static void
html_query(str)
  char *str;
{
  int fd;
  ACCT acct;
  char fpath[64], *userid;

  out_head("�d�ߨϥΪ�");

  if (!arg_analyze(1, str, &userid, NULL, NULL))
    return;

  if (!allow_userid(userid, 1))
    return;

  usr_fpath(fpath, userid, FN_ACCT);
  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    read(fd, &acct, sizeof(ACCT));
    close(fd);

    printf("<pre>\r\n"
      "<a href=mailto:%s.bbs@%s>%s (%s)</a><br>\r\n"
      "%s�q�L�{�ҡA�@�W�� %d ���A�o��L %d �g�峹<br>\r\n"
      "�̪�(%s)�q[%s]�W��<br>\r\n"
      "</pre>\r\n",
      acct.userid, MYHOSTNAME, acct.userid, acct.username,
      (acct.userlevel & PERM_VALID) ? "�w" : "��", acct.numlogins, acct.numposts,
      Btime(&(acct.lastlogin)), acct.lasthost);

    usr_fpath(fpath, acct.userid, FN_PLANS);
    out_article(fpath);
  }
  else
  {
    out_mesg("�S���o�ӱb��");
  }
}


/* ----------------------------------------------------- */
/* �峹�C��						 */
/* ----------------------------------------------------- */


static void
postlist_neck(brdname, start, total)
  char *brdname;
  int start, total;
{
  printf("<br>\r\n"
    "<table cellspacing=0 cellpadding=4 border=0 width=80%%>\r\n"
    "<tr>\r\n");

  if (start > HTML_TALL)
  {
    printf("  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/brd/%s&%d>�W%d�g</a></td>\r\n",
      brdname, start - HTML_TALL, HTML_TALL);
  }
  else
  {
    printf("  <td width=20%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  start += HTML_TALL;
  if (start <= total)
  {
    printf("  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/brd/%s&%d>�U%d�g</a></td>\r\n",
      brdname, start, HTML_TALL);
  }
  else
  {
    printf("  <td width=20%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  printf("  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/dopost/%s target=_blank>�o��峹</a></td>\r\n"
    "  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/gem/%s&"FN_DIR">��ذ�</a></td>\r\n"
    "  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/boardlist>�ݪO�C��</a></td>\r\n"
    "</tr>\r\n"
    "</table>\r\n"
    "<br>\r\n",
    brdname, 
    brdname, 
    brdname);
}


static void
html_postlist(str)
  char *str;
{
  int fd, start, total;
  char folder[64], *brdname, *number, *ptr1, *ptr2;
  HDR hdr;

  out_head("�峹�C��");

  if (!arg_analyze(2, str, &brdname, &number, NULL))
    return;

  init_bshm();

  if (!allow_brdname(brdname, BRD_R_BIT, 1))
    return;

  brd_fpath(folder, brdname, FN_DIR);

  start = atoi(number);
  total = rec_num(folder, sizeof(HDR));
  if (start <= 0 || start > total)	/* �W�L�d�򪺸ܡA������̫�@�� */
    start = (total - 1) / HTML_TALL * HTML_TALL + 1;

  postlist_neck(brdname, start, total);

  printf("<table cellspacing=0 cellpadding=4 border=0>\r\n"
    "<tr>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=15>�R</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=15>��</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=50>�s��</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=10>m</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=10>%%</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=50>���</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=100>�@��</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=400>���D</td>\r\n"
    "</tr>\r\n");

  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    int i, end;
#ifdef HAVE_SCORE
    char score;
#endif

    /* �q�X�ݪO���� start �g�}�l�� HTML_TALL �g */
    i = start;
    end = i + HTML_TALL;

    lseek(fd, (off_t) (sizeof(HDR) * (i - 1)), SEEK_SET);

    while (i < end && read(fd, &hdr, sizeof(HDR)) == sizeof(HDR))
    {
      if (ptr1 = strchr(hdr.owner, '.'))	/* ���~�@�� */
	*(ptr1 + 1) = '\0';
      if (ptr2 = strchr(hdr.owner, '@'))	/* ���~�@�� */
	*ptr2 = '\0';

#ifdef HAVE_SCORE
      if (hdr.xmode & POST_SCORE)
      {
	score = abs(hdr.score);
	score = score < 10 ? (score + '0') : (score - 10 + 'A');
      }
      else
	score = ' ';
#endif

      printf("<tr onMouseOver=\"this.bgColor='"HCOLOR_BAR"'\" onMouseOut=\"this.bgColor='"HCOLOR_BG"'\">\r\n"
	"  <td><a href=/predelpost/%s&%d&%d><img src=/image/delpost.gif border=0></a></td>\r\n"
	"  <td><a href=/markpost/%s&%d&%d><img src=/image/markpost.gif border=0></a></td>\r\n"
	"  <td>%d</td>\r\n"
	"  <td>%c</td>\r\n"
#ifdef HAVE_SCORE
	"  <td><font color='%s'>%c</font></td>\r\n"
#endif
	"  <td>%s</td>\r\n"
	"  <td><a href=/query/%s>%s</a></td>\r\n"
	"  <td><a href=/brdmore/%s&%d>%.50s</td>\r\n"
	"</tr>\r\n",
	brdname, i, hdr.chrono,
	brdname, i, hdr.chrono,
	i,
	hdr.xmode & POST_MARKED ? 'm' : ' ',
#ifdef HAVE_SCORE
	hdr.score >= 0 ? "red" : "green", score,
#endif
	hdr.date + 3,
	(ptr1 || ptr2 ? "���~�@��" : hdr.owner), hdr.owner,
	brdname, i, hdr.title);

      i++;
    }
    close(fd);
  }

  printf("</table>\r\n");

  postlist_neck(brdname, start, total);
}


/* ----------------------------------------------------- */
/* ��ذϦC��						 */
/* ----------------------------------------------------- */


static void
gemlist_neck(brdname)
  char *brdname;
{
  printf("<br>\r\n"
    "<table cellspacing=0 cellpadding=2 border=0 width=80%%>\r\n"
    "<tr>\r\n"
    "  <td width=50%% align=center bgcolor="HCOLOR_NECK"><a href=/brd/%s&0>�^��ݪO</a></td>\r\n"
    "  <td width=50%% align=center bgcolor="HCOLOR_NECK"><a href=/boardlist>�ݪO�C��</a></td>\r\n"
    "</tr>\r\n"
    "</table>\r\n"
    "<br>\r\n",
    brdname);
}


static void
html_gemlist(str)
  char *str;
{
  int fd, i;
  char folder[64], *brdname, *xname;
  HDR hdr;

  out_head("��ذ�");

  if (!arg_analyze(2, str, &brdname, &xname, NULL))
    return;

  if (*xname != 'F' && strlen(xname) != 8 && strcmp(xname, FN_DIR))
    return;

  init_bshm();

  if (!allow_brdname(brdname, BRD_R_BIT, 1))
    return;

  gemlist_neck(brdname);

  printf("<table cellspacing=0 cellpadding=4 border=0>\r\n"
    "<tr onMouseOver=\"this.bgColor='"HCOLOR_BAR"'\" onMouseOut=\"this.bgColor='"HCOLOR_BG"'\">\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=50>�s��</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=400>���D</td>\r\n"
    "</tr>\r\n");

  if (*xname == '.')
    sprintf(folder, "gem/brd/%s/%s", brdname, FN_DIR);
  else /* if (*xname == 'F') */
    sprintf(folder, "gem/brd/%s/%c/%s", brdname, xname[7], xname);

  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    i = 1;
    while (read(fd, &hdr, sizeof(HDR)) == sizeof(HDR))
    {
      if (hdr.xmode & GEM_RESTRICT)
      {
	printf("<tr onMouseOver=\"this.bgColor='"HCOLOR_BAR"'\" onMouseOut=\"this.bgColor='"HCOLOR_BG"'\">\r\n"
	  "  <td>%d</td>\r\n"
	  "  <td>[��Ū] ��ƫO�K</td>\r\n"
	  "</tr>\r\n", 
	  i);
      }
      else if (hdr.xname[0] == 'A')	/* �峹 */
      {
	printf("<tr onMouseOver=\"this.bgColor='"HCOLOR_BAR"'\" onMouseOut=\"this.bgColor='"HCOLOR_BG"'\">\r\n"
	  "  <td>%d</td>\r\n"
	  "  <td><a href=/gemmore/%s&%s&%d>[�峹] %s</a></td>\r\n"
	  "</tr>\r\n",
	  i, 
	  brdname, xname, i, hdr.title);
      }
      else if (hdr.xname[0] == 'F')	/* ���v */
      {
	printf("<tr onMouseOver=\"this.bgColor='"HCOLOR_BAR"'\" onMouseOut=\"this.bgColor='"HCOLOR_BG"'\">\r\n"
	  "  <td>%d</td>\r\n"
	  "  <td><a href=/gem/%s&%s>[���v] %s</a></td>\r\n"
	  "</tr>\r\n",
	  i, 
	  brdname, hdr.xname, hdr.title);
      }
      else				/* ��L���O�N���q�F */
      {
	printf("<tr onMouseOver=\"this.bgColor='"HCOLOR_BAR"'\" onMouseOut=\"this.bgColor='"HCOLOR_BG"'\">\r\n"
	  "  <td>%d</td>\r\n"
	  "  <td>[��Ū] ��L���</td>\r\n"
	  "</tr>\r\n", 
	  i);
      }

      i++;
    }
    close(fd);
  }

  printf("</table>\r\n");

  gemlist_neck(brdname);
}


/* ----------------------------------------------------- */
/* �\Ū�ݪO�峹						 */
/* ----------------------------------------------------- */


static void
brdmore_neck(brdname, pos, total)
  char *brdname;
  int pos, total;
{
  printf("<br>\r\n"
    "<table cellspacing=0 cellpadding=5 border=0 width=80%%>\r\n"
    "<tr>\r\n");

  if (pos > 1)
  {
    printf("  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/brdmore/%s&%d>�W�@�g</a></td>\r\n",
      brdname, pos - 1);
  }
  else
  {
    printf("  <td width=20%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  if (pos < total)
  {
    printf("  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/brdmore/%s&%d>�U�@�g</a></td>\r\n",
      brdname, pos + 1);
  }
  else
  {
    printf("  <td width=20%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  printf("  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/brdmost/%s&%d target=_blank>�P���D</a></td>\r\n",
    brdname, pos);

  printf("  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/dopost/%s target=_blank>�o��峹</a></td>\r\n",
    brdname);

  printf("  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/brd/%s&0>�峹�C��</a></td>\r\n"
    "</tr>\r\n"
    "</table>\r\n"
    "<br>\r\n",
    brdname);
}


static void
html_brdmore(str)
  char *str;
{
  int fd, pos, total;
  char folder[64], *brdname, *number;
  HDR hdr;

  out_head("�\\Ū�ݪO�峹");

  if (!arg_analyze(2, str, &brdname, &number, NULL))
    return;

  init_bshm();

  if (!allow_brdname(brdname, BRD_R_BIT, 1))
    return;

  brd_fpath(folder, brdname, FN_DIR);

  if ((pos = atoi(number)) <= 0)
    pos = 1;
  total = rec_num(folder, sizeof(HDR));

  brdmore_neck(brdname, pos, total);

  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    int find;

    lseek(fd, (off_t) (sizeof(HDR) * (pos - 1)), SEEK_SET);
    find = read(fd, &hdr, sizeof(HDR)) == sizeof(HDR);
    close(fd);

    if (find)
    {
#ifdef HAVE_REFUSEMARK
      if (!(hdr.xmode & POST_RESTRICT))
#endif
      {
	sprintf(folder, "brd/%s/%c/%s", brdname, hdr.xname[7], hdr.xname);
	out_article(folder);
      }
#ifdef HAVE_REFUSEMARK
      else
      {
	out_mesg("�����[�K�峹�A�z�L�k�\\Ū");
      }
#endif
    }
    else
    {
      out_mesg("�z�ҿ�����峹�s���w�W�L���ݪO�Ҧ��峹");
    }
  }

  brdmore_neck(brdname, pos, total);
}


/* ----------------------------------------------------- */
/* �\Ū�ݪO�P���D�峹					 */
/* ----------------------------------------------------- */


static void
brdmost_neck(brdname)
  char *brdname;
{
  printf("<br>\r\n"
    "<table cellspacing=0 cellpadding=4 border=0 width=80%%>\r\n"
    "<tr>\r\n");

  printf("  <td width=100%% align=center bgcolor="HCOLOR_NECK"><a href=/brd/%s&0>�峹�C��</a></td>\r\n"
    "</tr>\r\n"
    "</table>\r\n"
    "<br>\r\n",
    brdname);
}


static void
html_brdmost(str)
  char *str;
{
  int fd, pos;
  char folder[64], *brdname, *number;
  HDR hdr;

  out_head("�\\Ū�ݪO�P���D�峹");

  if (!arg_analyze(2, str, &brdname, &number, NULL))
    return;

  init_bshm();

  if (!allow_brdname(brdname, BRD_R_BIT, 1))
    return;

  brd_fpath(folder, brdname, FN_DIR);

  if ((pos = atoi(number)) <= 0)
    pos = 1;

  brdmost_neck(brdname);

  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    int find;

    lseek(fd, (off_t) (sizeof(HDR) * (pos - 1)), SEEK_SET);
    find = read(fd, &hdr, sizeof(HDR)) == sizeof(HDR);
    close(fd);

    if (find)
      html_most(folder, str_ttl(hdr.title));
    else
      out_mesg("�z�ҿ�����峹�s���w�W�L���ݪO�Ҧ��峹");
  }

  brdmost_neck(brdname);
}


/* ----------------------------------------------------- */
/* �\Ū��ذϤ峹					 */
/* ----------------------------------------------------- */


static void
gemmore_neck(brdname, xname, pos, total)
  char *brdname, *xname;
  int pos, total;
{
  printf("<br>\r\n"
    "<table cellspacing=0 cellpadding=4 border=0 width=80%%>\r\n"
    "<tr>\r\n");

  if (pos > 1)
  {
    printf("  <td width=30%% align=center bgcolor="HCOLOR_NECK"><a href=/gemmore/%s&%s&%d>�W�@�g</a></td>\r\n",
      brdname, xname, pos - 1);
  }
  else
  {
    printf("  <td width=30%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  if (pos < total)
  {
    printf("  <td width=30%% align=center bgcolor="HCOLOR_NECK"><a href=/gemmore/%s&%s&%d>�U�@�g</a></td>\r\n",
      brdname, xname, pos + 1);
  }
  else
  {
    printf("  <td width=30%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  printf("  <td width=40%% align=center bgcolor="HCOLOR_NECK"><a href=/gem/%s&%s>�^����v</a></td>\r\n"
    "</tr>\r\n"
    "</table>\r\n"
    "<br>\r\n",
    brdname, xname);
}


static void
html_gemmore(str)
  char *str;
{
  int fd, pos, total;
  char folder[64], *brdname, *xname, *number;
  HDR hdr;

  out_head("�\\Ū��ذϤ峹");

  if (!arg_analyze(3, str, &brdname, &xname, &number))
    return;

  if (*xname != 'F' && strlen(xname) != 8 && strcmp(xname, FN_DIR))
    return;

  init_bshm();

  if (!allow_brdname(brdname, BRD_R_BIT, 1))
    return;

  if (*xname == '.')
    gem_fpath(folder, brdname, FN_DIR);
  else /* if (*xname == 'F') */
    sprintf(folder, "gem/brd/%s/%c/%s", brdname, xname[7], xname);

  if ((pos = atoi(number)) <= 0)
    pos = 1;
  total = rec_num(folder, sizeof(HDR));

  gemmore_neck(brdname, xname, pos, total);

  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    int find;

    lseek(fd, (off_t) (sizeof(HDR) * (pos - 1)), SEEK_SET);
    find = read(fd, &hdr, sizeof(HDR)) == sizeof(HDR);
    close(fd);

    if (find)
    {
      if (hdr.xname[0] == 'A')
      {
	if (!(hdr.xmode & GEM_RESTRICT))
	{
	  sprintf(folder, "gem/brd/%s/%c/%s", brdname, hdr.xname[7], hdr.xname);
	  out_article(folder);
	}
	else
	{
	  out_mesg("�����O�K��ذϡA�z�L�k�\\Ū");
	}
      }
      else
      {
	out_mesg("�o�O���v�ΰ�Ū��ơA�z�����Ѻ�ذϦC���Ū��");
      }
    }
    else
    {
      out_mesg("�z�ҿ�����峹�s���w�W�L�����v�Ҧ��峹");
    }
  }

  gemmore_neck(brdname, xname, pos, total);
}


/* ----------------------------------------------------- */
/* �\Ū�H�c�峹						 */
/* ----------------------------------------------------- */


static void
mboxmore_neck(pos, total)
  int pos, total;
{
  printf("<br>\r\n"
    "<table cellspacing=0 cellpadding=4 border=0 width=80%%>\r\n"
    "<tr>\r\n");

  if (pos > 1)
  {
    printf("  <td width=30%% align=center bgcolor="HCOLOR_NECK"><a href=/mboxmore/%d>�W�@�g</a></td>\r\n",
      pos - 1);
  }
  else
  {
    printf("  <td width=30%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  if (pos < total)
  {
    printf("  <td width=30%% align=center bgcolor="HCOLOR_NECK"><a href=/mboxmore/%d>�U�@�g</a></td>\r\n",
      pos + 1);
  }
  else
  {
    printf("  <td width=30%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  printf("  <td width=40%% align=center bgcolor="HCOLOR_NECK"><a href=/mbox/0>�H�c�C��</a></td>\r\n"
    "</tr>\r\n"
    "</table>\r\n"
    "<br>\r\n");
}


static void
html_mboxmore(str)
  char *str;
{
  int fd, pos, total;
  char folder[64], fpath[64], *number;
  HDR hdr;

  out_head("�\\Ū�H�c�峹");

  if (!arg_analyze(1, str, &number, NULL, NULL))
    return;

  if (!acct_fetch())
    return;

  usr_fpath(folder, cuser.userid, FN_DIR);

  if ((pos = atoi(number)) <= 0)
    pos = 1;
  total = rec_num(folder, sizeof(HDR));
  if (pos > total)
    pos = total;

  mboxmore_neck(pos, total);

  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    int find;

    lseek(fd, (off_t) (sizeof(HDR) * (pos - 1)), SEEK_SET);
    find = read(fd, &hdr, sizeof(HDR)) == sizeof(HDR);
    close(fd);

    if (find)
    {
      hdr_fpath(fpath, folder, &hdr);
      out_article(fpath);
    }
  }

  mboxmore_neck(pos, total);
}


/* ----------------------------------------------------- */
/* ��ܹϤ�						 */
/* ----------------------------------------------------- */


static int
valid_path(str)
  char *str;
{
  int ch;

  if (!*str)
    return;

  while (ch = *str++)
  {
    if (!is_alnum(ch) && ch != '.' && ch != '-' && ch != '_')
      return 0;
  }
  return 1;
}


static void
html_image(str)
  char *str;
{
  int fd, size;
  char fpath[64], *fname, *ptr;
  time_t now;
  struct stat st;

  if (!arg_analyze(1, str, &fname, NULL, NULL))
    return;

  if (!valid_path(fname) || !(ptr = strrchr(fname, '.')))
    return;

  /* �u�䴩 gif �� jpg �榡 */
  if (!str_cmp(ptr, ".gif"))
    ptr = "gif";
  else if (!str_cmp(ptr, ".jpg") || !str_cmp(ptr, ".jpeg"))
    ptr = "jpeg";
  else
    return;

  /* �u�i�HŪ�� run/html/ ���U���ɮ� */
  sprintf(fpath, "run/html/%.30s", fname);
  if (stat(fpath, &st))
    return;

  size = st.st_size;

  /* itoc.040413: �٬O���ܴN�����n�q���Y�F�H */
  if (size > 1024)	/* size �p�� 1KB �h���ݭn���Y */
  {
    /* HTTP 1.0 ���Y */
    time(&now);
    printf("HTTP/1.0 200 OK\r\n");
    printf("Date: %s\r\n", Gtime(&now));
    printf("Server: MapleBBS 3.10\r\n");
    printf("Last-Modified: %s\r\n", Gtime(&st.st_mtime));
    printf("Content-Length: %d\r\n", size);
    printf("Content-Type: image/%s\r\n", ptr);
    printf("\r\n");
  }

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    ptr = (char *) malloc(size);
    read(fd, ptr, size);
    close(fd);
    write(1, ptr, size);	/* 1 �O stdout */
    free(ptr);
  }

  exit(0);		/* �L�� out_tail */
}


/*-------------------------------------------------------*/
/* ����							 */
/*-------------------------------------------------------*/


static void
mainpage_neck(logined)
  int logined;
{
  printf("<br>\r\n"
    "<table cellspacing=0 cellpadding=1 border=0 width=80%%>\r\n"
    "<tr>\r\n"
    "  <td width=100%% align=center bgcolor="HCOLOR_NECK">%s%s�w����{</td>\r\n"
    "</tr>\r\n"
    "</table>\r\n"
    "<br>\r\n", 
    logined ? cuser.userid : "",
    logined ? "�A" : "");
}


static void
html_mainpage()
{
  int logined;
  char fpath[64];

  out_head("�}�Y�e��");

  logined = acct_fetch();

  mainpage_neck(logined);

  /* �}�Y�e�� */
  sprintf(fpath, "gem/@/@opening.%d", time(0) % 3);
  out_article(fpath);

  /* �n�J */
  if (!logined)
  {
    printf("<form method=post>\r\n"
      "  <input type=hidden name=log>\r\n"
      "  <input type=hidden name=in>\r\n"
      "  �b�� <input type=text name=u size=12 maxlength=12> "
      "  �K�X <input type=password name=p size=12 maxlength=8> "
      "  <input type=hidden name=end>\r\n"
      "  <input type=submit value=�n�J> "
      "  <input type=reset value=�M��>"
      "</form>\r\n");
  }

  mainpage_neck(logined);
}


/* ----------------------------------------------------- */
/* �D�{��						 */
/* ----------------------------------------------------- */


typedef struct
{
  char *key;
  void (*func) ();
}	WebKeyFunc;


static char *
readline(len, stream)
  int *len;			/* �^�Ǧr����� */
  FILE *stream;
{
  int ch, and;
  char *fimage, fpath[64];
  FILE *fp;
  struct stat st;

  sprintf(fpath, "tmp/web.%d", getpid());
  if (!(fp = fopen(fpath, "w")))
    return NULL;

  and = 0;
  while ((ch = getc(stream)) >= 0)
  {
    /* itoc.040717: �]�� Mozilla �� POST �������|�[ \r\n�A�u�n�A�©ۡA���� & �ӷ��� */
    if (ch == '&')
    {
      if (++and >= 4)	/* login �M dopost ���u���|�� & */
	break;
    }

    fputc(ch, fp);
    if (ch == '\n')
      break;
  }
  fclose(fp);

  fimage = NULL;
  if ((ch = open(fpath, O_RDONLY)) >= 0)
  {
    if (!fstat(ch, &st) && (*len = st.st_size) > 0)
    {
      fimage = (char *) malloc(*len + 1);	/* �d 1-byte �� '\0' */
      read(ch, fimage, *len);
      fimage[*len] = '\0';
    }
    close(ch);
  }
  unlink(fpath);

  return fimage;
}


int
main()
{
  int cc;
  char *ptr, *line;
  char method[10], url[128], protocol[20], buf[80];
  WebKeyFunc *cb;

  WebKeyFunc cmd[] =
  {
    "/userlist",    html_userlist,
    "/boardlist",   html_boardlist,
    "/favorlist",   html_favorlist,

    "/brd/",        html_postlist,
    "/gem/",        html_gemlist,
    "/mbox/",       html_mboxlist,

    "/brdmore/",    html_brdmore,
    "/brdmost/",    html_brdmost,
    "/gemmore/",    html_gemmore,
    "/mboxmore/",   html_mboxmore,

    "/dopost/",     html_dopost,
    "/domail/",     html_domail,

    "/delpost/",    html_delpost,
    "/predelpost/", html_predelpost,
    "/delmail/",    html_delmail,
    "/predelmail/", html_predelmail,
    "/markpost/",   html_markpost,
    "/markmail/",   html_markmail,

    "/query/",      html_query,

    "/image/",      html_image,

    NULL,           html_mainpage
  };  

  chdir(BBSHOME);

  fscanf(stdin, "%8s %100s %15s", method, url, protocol);

  cc = 0;
  while (fgets(buf, sizeof(buf), stdin))
  {
    if (ptr = strchr(buf, '\r'))
      *ptr = '\0';
    if (ptr = strchr(buf, '\n'))
      *ptr = '\0';

    if (buf[0])		/* ���Y�}�l */
      cc = 1;
    else if (cc)	/* ���Y���� */
      break;
    else		/* ���Y�}�l�e���Ŧ� */
      continue;

    if (!str_ncmp(buf, "Cookie: user=", 13))
    {
      str_ncpy(cookie, buf + 13, sizeof(cookie));
      break;
    }
  }

  if (!str_cmp(method, "POST"))
  {
    while (line = readline(&cc, stdin))
    {
      if (ptr = strchr(line, '\r'))
	*ptr = '\0';
      if (ptr = strchr(line, '\n'))
	*ptr = '\0';

      if (!strncmp(line, "log=&in=&", 9))
      {
	acct_login(line + 9);

	html_mainpage();
      }
      else if (!strncmp(line, "dopost=&", 8))
      {
	cc = add_post(line + 8, cc);

	out_head("�峹�o��");
	out_reload(cc ? "�z���峹�o���\\" : "�z���峹�o���ѡA�]�\\�O�]���z�|���n�J");
      }
      else if (!strncmp(line, "domail=&", 8))
      {
	cc = add_mail(line + 8, cc);

	out_head("�H��o�e");
	out_reload(cc ? "�z���H��o�e���\\" : "�z���H��o�e���ѡA�]�\\�O�]���z�|���n�J�άO�d�L���ϥΪ�");
      }
      else
      {
	free(line);
	continue;
      }
      free(line);
      break;
    }

    out_tail();
    return 0;
  }

  /* �i�J Web-BBS �� */

  for (cb = cmd;; cb++)
  {
    if (!(ptr = cb->key))		/* ��l���~�����q�X���� */
    {
      html_mainpage();
      break;
    }

    cc = strlen(ptr);
    if (!strncmp(url, ptr, cc))
    {
      (*(cb->func)) (url + cc);
      break;
    }
  }

  out_tail();
  return 0;
}
