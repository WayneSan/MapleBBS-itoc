/* ----------------------------------------------------- */
/* txt2htm.c	( YZU WindTopBBS Ver 3.XX )		 */
/* ----------------------------------------------------- */
/* target : ANSI text to HTML tag			 */
/* create : 03/06/10					 */
/* update : 03/07/09					 */
/* author : visor.bbs@bbs.yzu.edu.tw			 */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/* ----------------------------------------------------- */


#define LSTRLEN		4096


#define ATTR_HIGHLIGHT	1	/* 高亮度 */
#define ATTR_UNDER	2	/* 底線 */
#define ATTR_BLINK	4	/* 閃動 */
#define ATTR_ITALIC	8	/* 斜體 */


static void
strip_multicolorword(buf)	/* 將雙色字改成單色字 */
  uschar *buf;
{
  uschar *src;
  uschar *des;
  int in_chi = 0;
  int ch;

  src = des = buf;

  for (; ch = *src; src++)
  {
    if (!in_chi)
    {
      if (ch & 0x80)
	in_chi = 1;
    }
    else
    {
      /* 如果是中文字裡面的 ANSI code 就拋棄 */
      if (ch == KEY_ESC)
      {
        while (1)
	{
	  if (!(ch = *++src))
	    goto end_strip;
	  if (ch == 'm')
	    break;
	}
	continue;
      }

      in_chi = 0;
    }
    *des++ = ch;
  }

end_strip:
  *des = '\0';
}


static char *
ansi2color(src, attr, fg, bg)	/* 傳入 ANSI，回傳顏色 */
  char *src;
  int *attr, *fg, *bg;
{
  int color;
  char *str, *end;
  char *ptr, code[ANSILINELEN];	/* 弄這麼長，以免有人暴力 */

  str = src + 2;	/* 跳過 "\033[" */

  if (!(end = (char *) strchr(str, 'm')))
    return src;

  ptr = code;

  do
  {
    if (*str >= '0' && *str <= '7')
    {
      *ptr++ = *str;
      continue;
    }

    *ptr = '\0';
    ptr = code;
    color = atoi(ptr);

    /* 除非是還原碼，否則 ANSI code 有繼承的效果 */

    if (color == 0)
    {
      *fg = 37;
      *bg = 40;
      *attr = 0;
    }
    else if (color >= 30 && color <= 37)
      *fg = color;
    else if (color >= 40 && color <= 47)
      *bg = color;
    else if (color == 1)
      *attr |= ATTR_HIGHLIGHT;
    else if (color == 4)
      *attr |= ATTR_UNDER;
    else if (color == 5)
      *attr |= ATTR_BLINK;
    else if (color == 7)	/* 反白的效果用斜體來代替 */
      *attr |= ATTR_ITALIC;
  } while (++str <= end);

  return end;
}


#define	MOZILLA_SUPPORT		/* 為了支援 Mozilla browser */

#ifdef MOZILLA_SUPPORT
static int fontdirty = 0;		/* 1:在 <font> 裡面 */
#endif


static char *
ansi2tag(dstr, sstr)
  char *dstr, *sstr;
{
  int ch;
  char *src, *dst;

  static int oldattr = 0, attr = 0;
  static int oldfg = 37, fgcolor = 37;	/* 預設前景顏色 */
  static int oldbg = 40, bgcolor = 40;	/* 預設背景顏色 */


  strip_multicolorword(sstr);

  src = sstr;
  dst = dstr;

  for (; ch = *src; src++)
  {
    if (ch != '\033' || *(src + 1) != '[')	/* 不是 ANSI */
    {
      if (ch == '%')	/* itoc.030717: printf 裡面如果有 '%' 的話，得換成 "%%" */
	*dst++ = '%';

      *dst++ = ch;
      continue;
    }

    /* ------------------------------------------------- */
    /* 開始處理 ANSI -> HTML				 */
    /* ------------------------------------------------- */

    /* 分析顏色 */
    src = ansi2color(src, &attr, &fgcolor, &bgcolor);

#ifndef MOZILLA_SUPPORT
    /* 顏色不同才需要印出 */
    if (oldfg != fgcolor)
    {
      sprintf(dst, "<font class=col%d%d>", attr & ATTR_HIGHLIGHT, fgcolor);
      dst += 19;
      oldfg = fgcolor;
    }
    if (oldbg != bgcolor)
    {
      sprintf(dst, "<font class=col0%d>", bgcolor);
      dst += 19;
      oldbg = bgcolor;
    }
#else
    /* itoc.030720: 為了 Mozilla 要求一定要用 </font>，所以只得這樣做 */

    /* 顏色不同才需要印出 */
    if (oldfg != fgcolor || oldbg != bgcolor)
    {
      if (fontdirty)
      {
	sprintf(dst, "</font></font>");
	dst += 14;
      }
      sprintf(dst, "<font class=col%d%d><font class=col0%d>", attr & ATTR_HIGHLIGHT, fgcolor, bgcolor);
      dst += 38;
      oldfg = fgcolor;
      oldbg = bgcolor;
      fontdirty = 1;
    }
#endif

    /* 屬性不同才需要印出 */
    if (oldattr != attr)
    {
      if ((attr & ATTR_ITALIC) && !(oldattr & ATTR_ITALIC))
      {
	strcpy(dst, "<I>");
	dst += 3;
      }
      else if (!(attr & ATTR_ITALIC) && (oldattr & ATTR_ITALIC))
      {
	strcpy(dst, "</I>");
	dst += 4;
      }
      if ((attr & ATTR_UNDER) && !(oldattr & ATTR_UNDER))
      {
	strcpy(dst, "<U>");
	dst += 3;
      }
      else if (!(attr & ATTR_UNDER) && (oldattr & ATTR_UNDER))
      {
	strcpy(dst, "</U>");
	dst += 4;
      }
      if ((attr & ATTR_BLINK) && !(oldattr & ATTR_BLINK))
      {
	strcpy(dst, "<BLINK>");
	dst += 7;
      }
      else if (!(attr & ATTR_BLINK) && (oldattr & ATTR_BLINK))
      {
	strcpy(dst, "</BLINK>");
	dst += 8;
      }
      oldattr = attr;
    }
  }		/* end for() */

  *dst = '\0';

  return dstr;
}


static char *
quotebuf(buf)
  char *buf;
{
  int ch1, ch2;
  char tmp[ANSILINELEN];

  ch1 = buf[0];
  ch2 = buf[1];

  /* 如果是引言，就略過所有的 ANSI 碼 */
  if (ch2 == ' ' && (ch1 == QUOTE_CHAR1 || ch1 == QUOTE_CHAR2))	/* 引言 */
  {
    ch1 = buf[2];
    str_ansi(tmp, buf, sizeof(tmp) - 8);
    sprintf(buf, "\033[3%cm%s\033[m\n", (ch1 == QUOTE_CHAR1 || ch1 == QUOTE_CHAR2) ? '3' : '6', tmp);	/* 引用一層/二層不同顏色 */
  }
  else if (ch1 == '�' && ch2 == '�')	/* ※ 引言者 */
  {
    str_ansi(tmp, buf, sizeof(tmp) - 10);
    sprintf(buf, "\033[1;36m%s\033[m\n", tmp);
  }

  return buf;
}


#define LINE_HEADER	3	/* 檔頭有三行 */


static void
txt2htm(fp)
  FILE *fp;
{
  int i;
  char buf[ANSILINELEN], encodebuf[LSTRLEN];
  char header1[LINE_HEADER][LEN_AUTHOR1] = {"作者",   "標題",   "時間"};
  char header2[LINE_HEADER][LEN_AUTHOR2] = {"發信人", "標  題", "發信站"};
  char *headvalue, *pbrd, board[128];

  pbrd = NULL;

  printf("<table width=800 cellspacing=0 cellpadding=0 border=0>\r\n");

  /* 處理檔頭 */
  for (i = 0; i < LINE_HEADER; i++)
  {
    if (!fgets(buf, sizeof(buf), fp))	/* 雖然連檔頭都還沒印完，但是檔案已經結束，直接離開 */
    {
      printf("</table>\r\n");
      return;
    }

    if (memcmp(buf, header1[i], LEN_AUTHOR1 - 1) && memcmp(buf, header2[i], LEN_AUTHOR2 - 1))	/* 不是檔頭 */
      break;

    /* 作者/看板 檔頭有二欄，特別處理 */
    if (i == 0 && (strstr(buf, "看板:") || strstr(buf, "站內:")))
    {
      if (pbrd = strrchr(buf, ':'))
      {
	*pbrd = '\0';
	strcpy(board, pbrd + 2);
	pbrd -= 5;
	*pbrd++ = '\0';
      }
    }

    if (!(headvalue = strchr(buf, ':')))
      break;

    *headvalue = '\0';
    headvalue = ansi2tag(encodebuf, headvalue + 2);

    if (i == 0 && pbrd)
    {
      printf("<tr>\r\n"
	"  <td align=center width=10%% class=col047><font class=col034>%s</font></td>\r\n"
	"  <td width=60%% class=col044><font class=col037>&nbsp;%s</font></td>\r\n"
	"  <td align=center width=10%% class=col047><font class=col034>%s</font></td>\r\n"
	"  <td width=20%% class=col044><font class=col037>&nbsp;%s</font></td>\r\n"
	"</tr>\r\n",
	header1[i],
	headvalue,
	pbrd,
	board);
    }
    else
    {
      printf("<tr>\r\n"
	"  <td align=center width=10%% class=col047><font class=col034>%s</font></td>\r\n"
	"  <td width=90%% COLSPAN=3 class=col044><font class=col037>&nbsp;%s</font></td>\r\n"
	"</tr>\r\n",
	header1[i],
	headvalue);
    }
  }

  printf("<tr>\r\n"
    "<td colspan=4><pre>\r\n");

  if (i < LINE_HEADER)		/* 最後一行不是檔頭 */
  {
    ansi2tag(encodebuf, quotebuf(buf));
    printf(encodebuf);
  }

  /* 處理內文 */
  while (fgets(buf, sizeof(buf), fp))
  {
    ansi2tag(encodebuf, quotebuf(buf));
    printf(encodebuf);
  }

#ifdef MOZILLA_SUPPORT
  if (fontdirty)
    printf("</font></font>");
#endif

  printf("</pre></td>\r\n"
    "</table>\r\n");  
}


static void 
show_style()
{
  printf("<style type=text/css>\r\n"
    "  PRE         {font-size: 15pt; line-height: 15pt; font-weight: lighter; background-color: 000000; COLOR: c0c0c0;}\r\n"
    "  TD          {font-size: 15pt; line-height: 15pt; font-weight: lighter;}\r\n"
    "  FONT.col030 {COLOR: 000000;}\r\n"
    "  FONT.col031 {COLOR: a00000;}\r\n"
    "  FONT.col032 {COLOR: 00a000;}\r\n"
    "  FONT.col033 {COLOR: a0a000;}\r\n"
    "  FONT.col034 {COLOR: 0000a0;}\r\n"
    "  FONT.col035 {COLOR: a000a0;}\r\n"
    "  FONT.col036 {COLOR: 00a0a0;}\r\n"
    "  FONT.col037 {COLOR: c0c0c0;}\r\n"
    "  FONT.col040 {background-color: 000000;}\r\n"
    "  FONT.col041 {background-color: a00000;}\r\n"
    "  FONT.col042 {background-color: 00a000;}\r\n"
    "  FONT.col043 {background-color: a0a000;}\r\n"
    "  FONT.col044 {background-color: 0000a0;}\r\n"
    "  FONT.col045 {background-color: a000a0;}\r\n"
    "  FONT.col046 {background-color: 00a0a0;}\r\n"
    "  FONT.col047 {background-color: c0c0c0;}\r\n"
    "  FONT.col130 {COLOR: 606060;}\r\n"
    "  FONT.col131 {COLOR: ff0000;}\r\n"
    "  FONT.col132 {COLOR: 00ff00;}\r\n"
    "  FONT.col133 {COLOR: ffff00;}\r\n"
    "  FONT.col134 {COLOR: 0000ff;}\r\n"
    "  FONT.col135 {COLOR: ff00ff;}\r\n"
    "  FONT.col136 {COLOR: 00ffff;}\r\n"
    "  FONT.col137 {COLOR: e0e0e0;}\r\n"
    "  TD.col044   {background-color: 0000a0;}\r\n"
    "  TD.col047   {background-color: c0c0c0;}\r\n"
    "</style>\r\n");
}
