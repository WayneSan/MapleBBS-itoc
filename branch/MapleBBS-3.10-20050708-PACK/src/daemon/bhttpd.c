/*-------------------------------------------------------*/
/* web/bhttpd.c		( NTHU CS MapleBBS Ver 3.10 )	 */
/*-------------------------------------------------------*/
/* target : BBS's HTTP interface			 */
/* create : 03/07/09					 */
/* update : 04/02/21					 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#if 0	/* 連結一覽表 */

  http://my.domain/                            首頁
  http://my.domain/boardlist                   看板列表
  http://my.domain/favorlist                   我的最愛
  http://my.domain/userlist                    使用者名單
  http://my.domain/brd/brdname&##              文章列表，列出看板 [brdname] 編號 ## 開始的 50 篇文章
  http://my.domain/gem/brdname&folder          精華區列表，列出看板 [brdname] 精華區中 folder 這個卷宗下的所有東西
  http://my.domain/mbox/##                     信箱列表，列出信箱中編號 ## 開始的 50 篇文章
  http://my.domain/brdmore/brdname&##          閱讀看板文章，閱讀看板 [brdname] 的第 ## 篇文章
  http://my.domain/brdmost/brdname&##          閱讀看板文章，閱讀看板 [brdname] 中所有名第 ## 篇同標題的文章
  http://my.domain/gemmore/brdname&folder&##   閱讀精華區文章，閱讀看板 [brdname] 精華區中 folder 這個卷宗下的第 ## 篇文章
  http://my.domain/mboxmore/##                 閱讀信箱文章，閱讀信箱中第 ## 篇文章
  http://my.domain/dopost/brdname              發表文章於看板 [brdname]
  http://my.domain/domail/                     發送信件
  http://my.domain/predelpost/brdname&##&###   詢問確定刪除看板 [brdname] 中第 ## 篇文章 (其 chrono 是 ###)
  http://my.domain/delpost/brdname&##&###      刪除看板 [brdname] 中第 ## 篇文章 (其 chrono 是 ###)
  http://my.domain/markpost/brdname&##&###     標記看板 [brdname] 中第 ## 篇文章 (其 chrono 是 ###)
  http://my.domain/predelmail/##&###           詢問確定刪除信箱中第 ## 篇文章 (其 chrono 是 ###)
  http://my.domain/delmail/##&###              刪除信箱中第 ## 篇文章 (其 chrono 是 ###)
  http://my.domain/markmail/##&###             標記信箱中第 ## 篇文章 (其 chrono 是 ###)
  http://my.domain/query/userid                查詢 userid
  http://my.domain/image/filename              顯示圖檔

#endif


#define _MODES_C_

#include "bbs.h"


#define HTML_TALL	50	/* 列表一頁 50 篇 */


/*-------------------------------------------------------*/
/* 選單的顏色						 */
/*-------------------------------------------------------*/


#define HCOLOR_BG	"#000000"	/* 背景的顏色 */
#define HCOLOR_TEXT	"#ffffff"	/* 文字的顏色 */
#define HCOLOR_LINK	"#00ffff"	/* 未瀏覽過連結的顏色 */
#define HCOLOR_VLINK	"#c0c0c0"	/* 已瀏覽過連結的顏色 */
#define HCOLOR_ALINK	"#ff0000"	/* 連結被壓下時的顏色 */

#define HCOLOR_NECK	"#000070"	/* 脖子的顏色 */
#define HCOLOR_TIE	"#a000a0"	/* 領帶的顏色 */

#define HCOLOR_BAR	"#808080"	/* 光棒顏色 */


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


/* out_head() 中的 <HTML> <BODY> <CENTER> 三個大寫標籤貫穿整個 html 檔
   直到 out_tail() 才由 </HTML> </BODY> </CENTER> 還原 */


static void
out_head(title)
  char *title;
{
  time_t now;

  /* HTTP 1.0 檔頭 */
  time(&now);
  printf("HTTP/1.0 200 OK\r\n");
  printf("Date: %s\r\n", Gtime(&now));
  printf("Server: MapleBBS 3.10\r\n");
  if (setcookie)	/* acct_login() 完以後才需要 Set-Cookie */
    printf("Set-Cookie: user=%s; path=/\r\n", cookie);
  printf("Content-Type: text/html; charset="MYCHARSET"\r\n");
  printf("\r\n");

  /* html 檔案開始 */
  printf("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\r\n");
  printf("<HTML>\r\n"
    "<style type=text/css>\r\n"
    "  pre {font-size: 15pt; line-height: 15pt; font-weight: lighter; background-color: 000000; color: c0c0c0;}\r\n"
    "  td  {font-size: 15pt; line-height: 15pt; font-weight: lighter;}\r\n"
    "</style>\r\n");

  printf("<head>\r\n"
    "  <meta http-equiv=Content-Type content=\"text/html; charset="MYCHARSET"\">\r\n"
    "  <title>【"BBSNAME"】%s</title>\r\n"
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
out_reload(msg)		/* God.050327: 將主視窗 reload 並關掉新開視窗 */
  char *msg;
{
  printf("<BODY onLoad='opener.location.reload(); parent.close();'>\n");
  printf("<script language=VBScript>MsgBox \"%s\",vbInformation+vbOKonly,\"確認\"</script>\n", msg);
}


static void
out_tail()
{
  printf("</CENTER></BODY></HTML>\r\n");
}


/* ----------------------------------------------------- */
/* 將 ANSI plain text 轉成 HTML file			 */
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
/* 解碼							 */
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

  len = 1;	/* 保留 1 給 '\0' */
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
	if (ch == '\r')		/* '\r' 就不要了 */
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
/* 分析參數						 */
/*-------------------------------------------------------*/


static int			/* 1:成功 */
arg_analyze(argc, str, arg1, arg2, arg3)
  int argc;		/* 有幾個參數 */
  char *str;		/* 引數 */
  char **arg1;		/* 參數一 */
  char **arg2;		/* 參數二 */
  char **arg3;		/* 參數三 */
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
    else /* if (i == 3) */	/* 最多三個參數 */
    {
      *arg3 = ptr;
      continue;		/* 跳過 do-while */
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
/* 使用者登入						 */
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
      !chkpasswd(acct.passwd, passwd))				/* 登入成功 */
    {
      /* itoc.040308: 產生 Cookie */
      setcookie = 1;
      str_ncpy(cookie, str, sizeof(cookie));
    }
  }
}


static ACCT cuser;		/* 使用 cuser 前，記得要先 acct_fetch() */


static int		/* 1:登入成功 0:登入失敗 */
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
	!chkpasswd(cuser.passwd, passwd))			/* 登入成功 */
	return 1;
    }
  }

  /* 沒有登入、登入失敗 */
  cuser.userno = 0;
  cuser.userlevel = 0;
  strcpy(cuser.userid, STR_GUEST);
  cuser.ufo = 0;
  return 0;
}


/*-------------------------------------------------------*/
/* 可見使用者須與 pal.c/talk.c/bmw.c 相容		 */
/*-------------------------------------------------------*/


static int			/* 1: userno 在 pool 名單上 */
pertain_pal(pool, max, userno)	/* 參考 pal.c:belong_pal() */
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
is_hisbad(up)			/* 1: 對方設我為壞人 */
  UTMP *up;			/* 參考 pal.c:is_obad() */
{
#ifdef HAVE_BADPAL
  return pertain_pal(up->pal_spool, up->pal_max, -cuser.userno);
#else
  return 0;
#endif
}


static int			/* 1:可看見 0:不可看見 */
can_seen(up)			/* 參考 bmw.c:can_see() */
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
/* UTMP shm 部分須與 cache.c 相容			 */
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


/* itoc.030711: 加上檢查使用者帳號的部分，以免有人亂踹 */

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
    out_mesg("此為站外作者所發表的文章");

  return rc;
}


/*-------------------------------------------------------*/
/* 看板權限部分須與 board.c 相容			 */
/*-------------------------------------------------------*/


static BCACHE *bshm;


#ifdef HAVE_MODERATED_BOARD
static int		/* !=0:是板好  0:不在名單中 */
is_brdgood(bpal)	/* 參考 pal.c:is_bgood() */
  BPAL *bpal;
{
  return pertain_pal(bpal->pal_spool, bpal->pal_max, cuser.userno);
}


static int		/* !=0:是板壞  0:不在名單中 */
is_brdbad(bpal)		/* 參考 pal.c:is_bbad() */
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
Ben_Perm(bno, ulevel)	/* 參考 board.c:Ben_Perm() */
  int bno;
  usint ulevel;
{
  BRD *brd;
  usint readlevel, postlevel, bits;
  char *blist, *bname;
#ifdef HAVE_MODERATED_BOARD
  BPAL *bpal;
  int ftype;	/* 0:一般ID 1:板好 2:板壞 */

  /* itoc.040103: 看板閱讀等級說明表

  ┌────┬────┬────┬────┐
  │        │一般用戶│看板好友│看板壞人│
  ├────┼────┼────┼────┤
  │一般看板│權限決定│  水桶  │ 看不見 │    看不見：在看板列表中無法看到這個板，也進不去
  ├────┼────┼────┼────┤    進不去：在看板列表中可以看到這個板，但是進不去
  │好友看板│ 進不去 │  完整  │  水桶  │    水  桶：在看板列表中可以看到這個板，也進得去，但是不能發文
  ├────┼────┼────┼────┤    完  整：在看板列表中可以看到這個板，也進得去及發文
  │秘密看板│ 看不見 │  完整  │  水桶  │
  └────┴────┴────┴────┘
  */

  static int bit_data[9] =
  {                /* 一般用戶   看板好友                           看板壞人 */
    /* 公開看板 */    0,         BRD_L_BIT | BRD_R_BIT,             0,
    /* 好友看板 */    BRD_L_BIT, BRD_L_BIT | BRD_R_BIT | BRD_W_BIT, BRD_L_BIT | BRD_R_BIT,
    /* 秘密看板 */    0,         BRD_L_BIT | BRD_R_BIT | BRD_W_BIT, BRD_L_BIT | BRD_R_BIT,
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

  if (readlevel == PERM_SYSOP)		/* 秘密看板 */
    bits = bit_data[6 + ftype];
  else if (readlevel == PERM_BOARD)	/* 好友看板 */
    bits = bit_data[3 + ftype];
  else if (ftype)			/* 公開看板，若在板好/板壞名單中 */
    bits = bit_data[ftype];
  else					/* 公開看板，其他依權限判定 */
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

  /* Thor.980813.註解: 特別為 BM 考量，板主有該板的所有權限 */
  blist = brd->BM;
  if ((ulevel & PERM_BM) && blist[0] > ' ' && str_has(blist, cuser.userid, strlen(cuser.userid)))
    bits = BRD_L_BIT | BRD_R_BIT | BRD_W_BIT | BRD_X_BIT | BRD_M_BIT;

  /* itoc.030515: 看板總管重新判斷 */
  else if (ulevel & PERM_ALLBOARD)
    bits = BRD_L_BIT | BRD_R_BIT | BRD_W_BIT | BRD_X_BIT | BRD_M_BIT;

  return bits;
}


/*-------------------------------------------------------*/
/* BRD shm 部分須與 cache.c 相容			 */
/*-------------------------------------------------------*/


static void
init_bshm()
{
  /* itoc.030727: 在開啟 bbsd 之前，應該就要執行過 account，
     所以 bshm 應該已設定好 */

  bshm = shm_new(BRDSHM_KEY, sizeof(BCACHE));

  if (bshm->uptime <= 0)	/* bshm 未設定完成 */
    exit(0);
}


static BRD *mybrd[MAXBOARD];
static int brd_num;


static int
brdtitle_cmp(a, b)
  BRD **a, **b;
{
  /* itoc.010413: 分類/板名交叉比對 */
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
    if (bhdr->brdname[0])	/* 看板沒被刪除 */
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
      /* 只支援第一層的看板 */
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

#if 0	/* 我的最愛依使用者自己的排序 */
  if (brd_num > 1)
    qsort(mybrd, brd_num, sizeof(BRD *), brdtitle_cmp);
#endif
}


/* itoc.030708: 加上檢查看板權限的部分，以免有人亂踹 */

static BRD *
allow_brdname(brdname, bits, warn)
  char *brdname;
  usint bits;		/* 若 bits != BRD_R_BIT，會順便做 acct_fetch() */
  int warn;		/* 1: 若 disallow 要給 warning */
{
  BRD *bcache, *bhdr, *tail;

  bcache = bshm->bcache;
  bhdr = bcache;
  tail = bcache + bshm->number;

  do
  {
    if (!strcmp(bhdr->brdname, brdname))
    {
      /* 若 readlevel == 0，表示 guest 可讀，無需 acct_fetch() */
      if (!bhdr->readlevel && bits == BRD_R_BIT)
	return bhdr;

      if (acct_fetch() && (Ben_Perm(bhdr - bcache, cuser.userlevel) & bits))
	return bhdr;

      break;
    }
  } while (++bhdr < tail);

  if (warn)
    out_mesg("操作錯誤，可能原因有：無此看板、您尚未登入、您的權限不足");

  return NULL;
}


/*-------------------------------------------------------*/
/* 發表新文章						 */
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


static int		/* 1:成功 */
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
	/* 取得來源 */
	len = sizeof(sin);
	getpeername(0, (struct sockaddr *) &sin, &len);
	from = (hp = gethostbyaddr((char *) &sin.sin_addr, sizeof(struct in_addr), sin.sin_family)) ? 
	  (char *) hp->h_name : inet_ntoa(sin.sin_addr);

	brd_fpath(folder, brdname, FN_DIR);

	fp = fdopen(hdr_stamp(folder, 'A', &hdr, fpath), "w");
	fprintf(fp, "%s %s (%s) %s %s\n",
	  STR_AUTHOR1, cuser.userid, cuser.username,
	  STR_POST2, brdname);
	fprintf(fp, "標題: %s\n時間: %s\n\n", title, Now());
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
/* 發送新信						 */
/*-------------------------------------------------------*/


static int		/* 1:成功 */
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
	/* 取得來源 */
	len = sizeof(sin);
	getpeername(0, (struct sockaddr *) &sin, &len);
	from = (hp = gethostbyaddr((char *) &sin.sin_addr, sizeof(struct in_addr), sin.sin_family)) ? 
	  (char *) hp->h_name : inet_ntoa(sin.sin_addr);

	usr_fpath(folder, userid, FN_DIR);

	fp = fdopen(hdr_stamp(folder, 0, &hdr, fpath), "w");
	fprintf(fp, "%s %s (%s)\n",
	  STR_AUTHOR1, cuser.userid, cuser.username);
	fprintf(fp, "標題: %s\n時間: %s\n\n", title, Now());
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
/* 看板列表						 */
/*-------------------------------------------------------*/


static void
boardlist_neck()
{
  printf("<br>\r\n"
    "<table cellspacing=0 cellpadding=1 border=0 width=80%%>\r\n"
    "<tr>\r\n"
    "  <td width=100%% align=center bgcolor="HCOLOR_NECK">目前站上有 %d 個板</td>\r\n"
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

  out_head("看板列表");

  boardlist_neck();

  printf("<table cellspacing=0 cellpadding=4 border=0>\r\n"
    "<tr>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=40>編號</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=80>看板</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=40>類別</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=25>轉</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=350>中文敘述</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=75>板主</td>\r\n"
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
/* 我的最愛						 */
/*-------------------------------------------------------*/


static void
favorlist_neck()
{
  printf("<br>\r\n"
    "<table cellspacing=0 cellpadding=1 border=0 width=80%%>\r\n"
    "<tr>\r\n"
    "  <td width=100%% align=center bgcolor="HCOLOR_NECK">我的最愛</td>\r\n"
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

  out_head("我的最愛");

  favorlist_neck();

  printf("<table cellspacing=0 cellpadding=4 border=0>\r\n"
    "<tr>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=40>編號</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=80>看板</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=40>類別</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=25>轉</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=350>中文敘述</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=75>板主</td>\r\n"
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
/* 信箱列表						 */
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
    printf("  <td width=33%% align=center bgcolor="HCOLOR_NECK"><a href=/mbox/%d>上%d篇</a></td>\r\n",
      start - HTML_TALL, HTML_TALL);
  }
  else
  {
    printf("  <td width=33%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  start += HTML_TALL;
  if (start <= total)
  {
    printf("  <td width=33%% align=center bgcolor="HCOLOR_NECK"><a href=/mbox/%d>下%d篇</a></td>\r\n",
      start, HTML_TALL);
  }
  else
  {
    printf("  <td width=33%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  printf("  <td width=34%% align=center bgcolor="HCOLOR_NECK"><a href=/domail/ target=_blank>發送信件</a></td>\r\n"
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

  out_head("信箱列表");

  if (!acct_fetch())
  {
    out_mesg("您尚未登入");
    return;
  }

  if (!arg_analyze(1, str, &number, NULL, NULL))
    return;

  usr_fpath(folder, cuser.userid, FN_DIR);

  start = atoi(number);
  total = rec_num(folder, sizeof(HDR));
  if (start <= 0 || start > total)	/* 超過範圍的話，直接到最後一頁 */
    start = (total - 1) / HTML_TALL * HTML_TALL + 1;

  mboxlist_neck(start, total);

  printf("<table cellspacing=0 cellpadding=4 border=0>\r\n"
    "<tr>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=15>刪</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=15>標</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=50>編號</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=10>m</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=50>日期</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=100>作者</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=400>標題</td>\r\n"
    "</tr>\r\n");

  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    int i, end;

    /* 秀出信箱的第 start 篇開始的 HTML_TALL 篇 */
    i = start;
    end = i + HTML_TALL;

    lseek(fd, (off_t) (sizeof(HDR) * (i - 1)), SEEK_SET);

    while (i < end && read(fd, &hdr, sizeof(HDR)) == sizeof(HDR))
    {
      if (ptr1 = strchr(hdr.owner, '.'))	/* 站外作者 */
	*(ptr1 + 1) = '\0';
      if (ptr2 = strchr(hdr.owner, '@'))	/* 站外作者 */
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
	(ptr1 || ptr2 ? "站外作者" : hdr.owner), hdr.owner,
	i, hdr.title);

      i++;
    }
    close(fd);
  }

  printf("</table>\r\n");

  mboxlist_neck(start, total);
}


/*-------------------------------------------------------*/
/* 使用者名單						 */
/*-------------------------------------------------------*/


static void
userlist_neck()
{
  printf("<br>\r\n"
    "<table cellspacing=0 cellpadding=1 border=0 width=80%%>\r\n"
    "<tr>\r\n"
    "  <td width=100%% align=center bgcolor="HCOLOR_NECK">目前站上有 %d 個人</td>\r\n"
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

  out_head("使用者名單");

  userlist_neck();

  printf("<table cellspacing=0 cellpadding=4 border=0>\r\n"
    "<tr>\r\n"
    "  <td bgcolor="HCOLOR_TIE">編號</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE">網友代號</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE">網友暱稱</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE">客途故鄉</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE">網友動態</td>\r\n"
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
/* 發表文章						 */
/*-------------------------------------------------------*/


static void
html_dopost(str)
  char *str;
{
  char *brdname;

  out_head("發表文章");

  if (!arg_analyze(1, str, &brdname, NULL, NULL))
    return;

  init_bshm();

  if (!allow_brdname(brdname, BRD_W_BIT, 1))
    return;

  printf("<form method=post>\r\n"
    "  <input type=hidden name=dopost>\r\n"
    "  <input type=hidden name=b value=%s>\r\n"
    "  請輸入標題：<br>\r\n"
    "  <input type=text name=t size=%d maxlength=%d><br><br>\r\n"
    "  請輸入內容：<br>\r\n"
    "  <textarea name=c rows=10 cols=%d></textarea><br><br>\r\n"
    "  <input type=hidden name=end>\r\n"
    "  <input type=submit value=送出文章> "
    "  <input type=reset value=重新填寫>"
    "</form>\r\n",
    brdname,
    TTLEN, TTLEN,
    SCR_WIDTH);
}


/*-------------------------------------------------------*/
/* 發送信件						 */
/*-------------------------------------------------------*/


static void
html_domail(str)
  char *str;
{
  out_head("發送信件");

  printf("<form method=post>\r\n"
    "  <input type=hidden name=domail>\r\n"
    "  請輸入ＩＤ：<br>\r\n"
    "  <input type=text name=u size=%d maxlength=%d><br><br>\r\n"
    "  請輸入標題：<br>\r\n"
    "  <input type=text name=t size=%d maxlength=%d><br><br>\r\n"
    "  請輸入內容：<br>\r\n"
    "  <textarea name=c rows=10 cols=%d></textarea><br><br>\r\n"
    "  <input type=hidden name=end>\r\n"
    "  <input type=submit value=送出信件> "
    "  <input type=reset value=重新填寫>"
    "</form>\r\n",
    IDLEN, IDLEN,
    TTLEN, TTLEN,
    SCR_WIDTH);
}


/*-------------------------------------------------------*/
/* 刪除文章						 */
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

  out_head("刪除文章");

  if (!arg_analyze(3, str, &brdname, &number, &stamp))
    return;

  if ((pos = atoi(number) - 1) < 0)
    return;
  if ((currchrono = atoi(stamp)) < 0)
    return;

  init_bshm();

  isBM = allow_brdname(brdname, BRD_X_BIT, 0);

  if (isBM || cuser.userlevel)	/* guest 不能砍自己文章 */
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
    out_mesg("已執行刪除指令，若未刪除表示此文章被標記了");
    printf("<a href=/brd/%s&%d>回文章列表</a>\r\n", brdname, (pos - 1) / HTML_TALL * HTML_TALL + 1);
  }
}


static void
html_predelpost(str)
  char *str;
{
  char *brdname, *number, *stamp;

  out_head("確認刪除文章");

  if (!arg_analyze(3, str, &brdname, &number, &stamp))
    return;

  out_mesg("若確定要刪除此篇文章，請再次點選以下連結；若要取消刪除，請按 [上一頁]");
  printf("<a href=/delpost/%s&%s&%s>刪除 [%s] 板第 %s 篇文章</a><br>\r\n", 
    brdname, number, stamp, brdname, number);
}


static void
html_delmail(str)
  char *str;
{
  char folder[64], fpath[64], *number, *stamp;
  int pos;
  HDR hdr;

  out_head("刪除信件");

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
  out_mesg("已執行刪除指令，若未刪除表示此信件被標記了");
  printf("<a href=/mbox/0>回信箱列表</a>\r\n");
}


static void
html_predelmail(str)
  char *str;
{
  char *number, *stamp;

  out_head("確認刪除信件");

  if (!arg_analyze(2, str, &number, &stamp, NULL))
    return;

  out_mesg("若確定要刪除此篇信件，請再次點選以下連結；若要取消刪除，請按 [上一頁]");
  printf("<a href=/delmail/%s&%s>刪除信箱第 %s 篇信件</a><br>\r\n", 
    number, stamp, number);
}


/*-------------------------------------------------------*/
/* 標記文章						 */
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

  out_head("標記文章");

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
    out_mesg("已執行(取消)標記指令");
    printf("<a href=/brd/%s&%d>回文章列表</a>\r\n", brdname, (pos - 1) / HTML_TALL * HTML_TALL + 1);
  }
}


static void
html_markmail(str)
  char *str;
{
  char folder[64], *number, *stamp;
  int pos;

  out_head("標記信件");

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
  out_mesg("已執行(取消)標記指令");
  printf("<a href=/mbox/0>回信箱列表</a>\r\n");
}


/*-------------------------------------------------------*/
/* 查詢使用者						 */
/*-------------------------------------------------------*/


static void
html_query(str)
  char *str;
{
  int fd;
  ACCT acct;
  char fpath[64], *userid;

  out_head("查詢使用者");

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
      "%s通過認證，共上站 %d 次，發表過 %d 篇文章<br>\r\n"
      "最近(%s)從[%s]上站<br>\r\n"
      "</pre>\r\n",
      acct.userid, MYHOSTNAME, acct.userid, acct.username,
      (acct.userlevel & PERM_VALID) ? "已" : "未", acct.numlogins, acct.numposts,
      Btime(&(acct.lastlogin)), acct.lasthost);

    usr_fpath(fpath, acct.userid, FN_PLANS);
    out_article(fpath);
  }
  else
  {
    out_mesg("沒有這個帳號");
  }
}


/* ----------------------------------------------------- */
/* 文章列表						 */
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
    printf("  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/brd/%s&%d>上%d篇</a></td>\r\n",
      brdname, start - HTML_TALL, HTML_TALL);
  }
  else
  {
    printf("  <td width=20%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  start += HTML_TALL;
  if (start <= total)
  {
    printf("  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/brd/%s&%d>下%d篇</a></td>\r\n",
      brdname, start, HTML_TALL);
  }
  else
  {
    printf("  <td width=20%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  printf("  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/dopost/%s target=_blank>發表文章</a></td>\r\n"
    "  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/gem/%s&"FN_DIR">精華區</a></td>\r\n"
    "  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/boardlist>看板列表</a></td>\r\n"
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

  out_head("文章列表");

  if (!arg_analyze(2, str, &brdname, &number, NULL))
    return;

  init_bshm();

  if (!allow_brdname(brdname, BRD_R_BIT, 1))
    return;

  brd_fpath(folder, brdname, FN_DIR);

  start = atoi(number);
  total = rec_num(folder, sizeof(HDR));
  if (start <= 0 || start > total)	/* 超過範圍的話，直接到最後一頁 */
    start = (total - 1) / HTML_TALL * HTML_TALL + 1;

  postlist_neck(brdname, start, total);

  printf("<table cellspacing=0 cellpadding=4 border=0>\r\n"
    "<tr>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=15>刪</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=15>標</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=50>編號</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=10>m</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=10>%%</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=50>日期</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=100>作者</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=400>標題</td>\r\n"
    "</tr>\r\n");

  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    int i, end;
#ifdef HAVE_SCORE
    char score;
#endif

    /* 秀出看板的第 start 篇開始的 HTML_TALL 篇 */
    i = start;
    end = i + HTML_TALL;

    lseek(fd, (off_t) (sizeof(HDR) * (i - 1)), SEEK_SET);

    while (i < end && read(fd, &hdr, sizeof(HDR)) == sizeof(HDR))
    {
      if (ptr1 = strchr(hdr.owner, '.'))	/* 站外作者 */
	*(ptr1 + 1) = '\0';
      if (ptr2 = strchr(hdr.owner, '@'))	/* 站外作者 */
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
	(ptr1 || ptr2 ? "站外作者" : hdr.owner), hdr.owner,
	brdname, i, hdr.title);

      i++;
    }
    close(fd);
  }

  printf("</table>\r\n");

  postlist_neck(brdname, start, total);
}


/* ----------------------------------------------------- */
/* 精華區列表						 */
/* ----------------------------------------------------- */


static void
gemlist_neck(brdname)
  char *brdname;
{
  printf("<br>\r\n"
    "<table cellspacing=0 cellpadding=2 border=0 width=80%%>\r\n"
    "<tr>\r\n"
    "  <td width=50%% align=center bgcolor="HCOLOR_NECK"><a href=/brd/%s&0>回到看板</a></td>\r\n"
    "  <td width=50%% align=center bgcolor="HCOLOR_NECK"><a href=/boardlist>看板列表</a></td>\r\n"
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

  out_head("精華區");

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
    "  <td bgcolor="HCOLOR_TIE" width=50>編號</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=400>標題</td>\r\n"
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
	  "  <td>[唯讀] 資料保密</td>\r\n"
	  "</tr>\r\n", 
	  i);
      }
      else if (hdr.xname[0] == 'A')	/* 文章 */
      {
	printf("<tr onMouseOver=\"this.bgColor='"HCOLOR_BAR"'\" onMouseOut=\"this.bgColor='"HCOLOR_BG"'\">\r\n"
	  "  <td>%d</td>\r\n"
	  "  <td><a href=/gemmore/%s&%s&%d>[文章] %s</a></td>\r\n"
	  "</tr>\r\n",
	  i, 
	  brdname, xname, i, hdr.title);
      }
      else if (hdr.xname[0] == 'F')	/* 卷宗 */
      {
	printf("<tr onMouseOver=\"this.bgColor='"HCOLOR_BAR"'\" onMouseOut=\"this.bgColor='"HCOLOR_BG"'\">\r\n"
	  "  <td>%d</td>\r\n"
	  "  <td><a href=/gem/%s&%s>[卷宗] %s</a></td>\r\n"
	  "</tr>\r\n",
	  i, 
	  brdname, hdr.xname, hdr.title);
      }
      else				/* 其他類別就不秀了 */
      {
	printf("<tr onMouseOver=\"this.bgColor='"HCOLOR_BAR"'\" onMouseOut=\"this.bgColor='"HCOLOR_BG"'\">\r\n"
	  "  <td>%d</td>\r\n"
	  "  <td>[唯讀] 其他資料</td>\r\n"
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
/* 閱讀看板文章						 */
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
    printf("  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/brdmore/%s&%d>上一篇</a></td>\r\n",
      brdname, pos - 1);
  }
  else
  {
    printf("  <td width=20%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  if (pos < total)
  {
    printf("  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/brdmore/%s&%d>下一篇</a></td>\r\n",
      brdname, pos + 1);
  }
  else
  {
    printf("  <td width=20%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  printf("  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/brdmost/%s&%d target=_blank>同標題</a></td>\r\n",
    brdname, pos);

  printf("  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/dopost/%s target=_blank>發表文章</a></td>\r\n",
    brdname);

  printf("  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/brd/%s&0>文章列表</a></td>\r\n"
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

  out_head("閱\讀看板文章");

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
	out_mesg("此為加密文章，您無法閱\讀");
      }
#endif
    }
    else
    {
      out_mesg("您所選取的文章編號已超過本看板所有文章");
    }
  }

  brdmore_neck(brdname, pos, total);
}


/* ----------------------------------------------------- */
/* 閱讀看板同標題文章					 */
/* ----------------------------------------------------- */


static void
brdmost_neck(brdname)
  char *brdname;
{
  printf("<br>\r\n"
    "<table cellspacing=0 cellpadding=4 border=0 width=80%%>\r\n"
    "<tr>\r\n");

  printf("  <td width=100%% align=center bgcolor="HCOLOR_NECK"><a href=/brd/%s&0>文章列表</a></td>\r\n"
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

  out_head("閱\讀看板同標題文章");

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
      out_mesg("您所選取的文章編號已超過本看板所有文章");
  }

  brdmost_neck(brdname);
}


/* ----------------------------------------------------- */
/* 閱讀精華區文章					 */
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
    printf("  <td width=30%% align=center bgcolor="HCOLOR_NECK"><a href=/gemmore/%s&%s&%d>上一篇</a></td>\r\n",
      brdname, xname, pos - 1);
  }
  else
  {
    printf("  <td width=30%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  if (pos < total)
  {
    printf("  <td width=30%% align=center bgcolor="HCOLOR_NECK"><a href=/gemmore/%s&%s&%d>下一篇</a></td>\r\n",
      brdname, xname, pos + 1);
  }
  else
  {
    printf("  <td width=30%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  printf("  <td width=40%% align=center bgcolor="HCOLOR_NECK"><a href=/gem/%s&%s>回到卷宗</a></td>\r\n"
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

  out_head("閱\讀精華區文章");

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
	  out_mesg("此為保密精華區，您無法閱\讀");
	}
      }
      else
      {
	out_mesg("這是卷宗或唯讀資料，您必須由精華區列表來讀取");
      }
    }
    else
    {
      out_mesg("您所選取的文章編號已超過本卷宗所有文章");
    }
  }

  gemmore_neck(brdname, xname, pos, total);
}


/* ----------------------------------------------------- */
/* 閱讀信箱文章						 */
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
    printf("  <td width=30%% align=center bgcolor="HCOLOR_NECK"><a href=/mboxmore/%d>上一篇</a></td>\r\n",
      pos - 1);
  }
  else
  {
    printf("  <td width=30%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  if (pos < total)
  {
    printf("  <td width=30%% align=center bgcolor="HCOLOR_NECK"><a href=/mboxmore/%d>下一篇</a></td>\r\n",
      pos + 1);
  }
  else
  {
    printf("  <td width=30%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  printf("  <td width=40%% align=center bgcolor="HCOLOR_NECK"><a href=/mbox/0>信箱列表</a></td>\r\n"
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

  out_head("閱\讀信箱文章");

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
/* 顯示圖片						 */
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

  /* 只支援 gif 及 jpg 格式 */
  if (!str_cmp(ptr, ".gif"))
    ptr = "gif";
  else if (!str_cmp(ptr, ".jpg") || !str_cmp(ptr, ".jpeg"))
    ptr = "jpeg";
  else
    return;

  /* 只可以讀取 run/html/ 底下的檔案 */
  sprintf(fpath, "run/html/%.30s", fname);
  if (stat(fpath, &st))
    return;

  size = st.st_size;

  /* itoc.040413: 還是乾脆就都不要秀檔頭了？ */
  if (size > 1024)	/* size 小於 1KB 則不需要檔頭 */
  {
    /* HTTP 1.0 檔頭 */
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
    write(1, ptr, size);	/* 1 是 stdout */
    free(ptr);
  }

  exit(0);		/* 無需 out_tail */
}


/*-------------------------------------------------------*/
/* 首頁							 */
/*-------------------------------------------------------*/


static void
mainpage_neck(logined)
  int logined;
{
  printf("<br>\r\n"
    "<table cellspacing=0 cellpadding=1 border=0 width=80%%>\r\n"
    "<tr>\r\n"
    "  <td width=100%% align=center bgcolor="HCOLOR_NECK">%s%s歡迎光臨</td>\r\n"
    "</tr>\r\n"
    "</table>\r\n"
    "<br>\r\n", 
    logined ? cuser.userid : "",
    logined ? "，" : "");
}


static void
html_mainpage()
{
  int logined;
  char fpath[64];

  out_head("開頭畫面");

  logined = acct_fetch();

  mainpage_neck(logined);

  /* 開頭畫面 */
  sprintf(fpath, "gem/@/@opening.%d", time(0) % 3);
  out_article(fpath);

  /* 登入 */
  if (!logined)
  {
    printf("<form method=post>\r\n"
      "  <input type=hidden name=log>\r\n"
      "  <input type=hidden name=in>\r\n"
      "  帳號 <input type=text name=u size=12 maxlength=12> "
      "  密碼 <input type=password name=p size=12 maxlength=8> "
      "  <input type=hidden name=end>\r\n"
      "  <input type=submit value=登入> "
      "  <input type=reset value=清除>"
      "</form>\r\n");
  }

  mainpage_neck(logined);
}


/* ----------------------------------------------------- */
/* 主程式						 */
/* ----------------------------------------------------- */


typedef struct
{
  char *key;
  void (*func) ();
}	WebKeyFunc;


static char *
readline(len, stream)
  int *len;			/* 回傳字串長度 */
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
    /* itoc.040717: 因為 Mozilla 的 POST 結尾不會加 \r\n，只好耍笨招，偵測 & 來當結束 */
    if (ch == '&')
    {
      if (++and >= 4)	/* login 和 dopost 都只有四個 & */
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
      fimage = (char *) malloc(*len + 1);	/* 留 1-byte 補 '\0' */
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

    if (buf[0])		/* 檔頭開始 */
      cc = 1;
    else if (cc)	/* 檔頭結束 */
      break;
    else		/* 檔頭開始前的空行 */
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

	out_head("文章發表");
	out_reload(cc ? "您的文章發表成功\" : "您的文章發表失敗，也許\是因為您尚未登入");
      }
      else if (!strncmp(line, "domail=&", 8))
      {
	cc = add_mail(line + 8, cc);

	out_head("信件發送");
	out_reload(cc ? "您的信件發送成功\" : "您的信件發送失敗，也許\是因為您尚未登入或是查無此使用者");
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

  /* 進入 Web-BBS 站 */

  for (cb = cmd;; cb++)
  {
    if (!(ptr = cb->key))		/* 其餘錯誤的都秀出首頁 */
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
