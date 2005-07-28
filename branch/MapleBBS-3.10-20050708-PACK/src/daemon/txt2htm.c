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


#define ATTR_HIGHLIGHT	1	/* °ª«G«× */
#define ATTR_UNDER	2	/* ©³½u */
#define ATTR_BLINK	4	/* °{°Ê */
#define ATTR_ITALIC	8	/* ±×Åé */


static void
strip_multicolorword(buf)	/* ±NÂù¦â¦r§ï¦¨³æ¦â¦r */
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
      /* ¦pªG¬O¤¤¤å¦r¸Ì­±ªº ANSI code ´N©ß±ó */
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
ansi2color(src, attr, fg, bg)	/* ¶Ç¤J ANSI¡A¦^¶ÇÃC¦â */
  char *src;
  int *attr, *fg, *bg;
{
  int color;
  char *str, *end;
  char *ptr, code[ANSILINELEN];	/* §Ë³o»òªø¡A¥H§K¦³¤H¼É¤O */

  str = src + 2;	/* ¸õ¹L "\033[" */

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

    /* °£«D¬OÁÙ­ì½X¡A§_«h ANSI code ¦³Ä~©Óªº®ÄªG */

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
    else if (color == 7)	/* ¤Ï¥Õªº®ÄªG¥Î±×Åé¨Ó¥N´À */
      *attr |= ATTR_ITALIC;
  } while (++str <= end);

  return end;
}


#define	MOZILLA_SUPPORT		/* ¬°¤F¤ä´© Mozilla browser */

#ifdef MOZILLA_SUPPORT
static int fontdirty = 0;		/* 1:¦b <font> ¸Ì­± */
#endif


static char *
ansi2tag(dstr, sstr)
  char *dstr, *sstr;
{
  int ch;
  char *src, *dst;

  static int oldattr = 0, attr = 0;
  static int oldfg = 37, fgcolor = 37;	/* ¹w³]«e´ºÃC¦â */
  static int oldbg = 40, bgcolor = 40;	/* ¹w³]­I´ºÃC¦â */


  strip_multicolorword(sstr);

  src = sstr;
  dst = dstr;

  for (; ch = *src; src++)
  {
    if (ch != '\033' || *(src + 1) != '[')	/* ¤£¬O ANSI */
    {
      if (ch == '%')	/* itoc.030717: printf ¸Ì­±¦pªG¦³ '%' ªº¸Ü¡A±o´«¦¨ "%%" */
	*dst++ = '%';

      *dst++ = ch;
      continue;
    }

    /* ------------------------------------------------- */
    /* ¶}©l³B²z ANSI -> HTML				 */
    /* ------------------------------------------------- */

    /* ¤ÀªRÃC¦â */
    src = ansi2color(src, &attr, &fgcolor, &bgcolor);

#ifndef MOZILLA_SUPPORT
    /* ÃC¦â¤£¦P¤~»Ý­n¦L¥X */
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
    /* itoc.030720: ¬°¤F Mozilla ­n¨D¤@©w­n¥Î </font>¡A©Ò¥H¥u±o³o¼Ë°µ */

    /* ÃC¦â¤£¦P¤~»Ý­n¦L¥X */
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

    /* ÄÝ©Ê¤£¦P¤~»Ý­n¦L¥X */
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

  /* ¦pªG¬O¤Þ¨¥¡A´N²¤¹L©Ò¦³ªº ANSI ½X */
  if (ch2 == ' ' && (ch1 == QUOTE_CHAR1 || ch1 == QUOTE_CHAR2))	/* ¤Þ¨¥ */
  {
    ch1 = buf[2];
    str_ansi(tmp, buf, sizeof(tmp) - 8);
    sprintf(buf, "\033[3%cm%s\033[m\n", (ch1 == QUOTE_CHAR1 || ch1 == QUOTE_CHAR2) ? '3' : '6', tmp);	/* ¤Þ¥Î¤@¼h/¤G¼h¤£¦PÃC¦â */
  }
  else if (ch1 == '¡' && ch2 == '°')	/* ¡° ¤Þ¨¥ªÌ */
  {
    str_ansi(tmp, buf, sizeof(tmp) - 10);
    sprintf(buf, "\033[1;36m%s\033[m\n", tmp);
  }

  return buf;
}


#define LINE_HEADER	3	/* ÀÉÀY¦³¤T¦æ */


static void
txt2htm(fp)
  FILE *fp;
{
  int i;
  char buf[ANSILINELEN], encodebuf[LSTRLEN];
  char header1[LINE_HEADER][LEN_AUTHOR1] = {"§@ªÌ",   "¼ÐÃD",   "®É¶¡"};
  char header2[LINE_HEADER][LEN_AUTHOR2] = {"µo«H¤H", "¼Ð  ÃD", "µo«H¯¸"};
  char *headvalue, *pbrd, board[128];

  pbrd = NULL;

  printf("<table width=800 cellspacing=0 cellpadding=0 border=0>\r\n");

  /* ³B²zÀÉÀY */
  for (i = 0; i < LINE_HEADER; i++)
  {
    if (!fgets(buf, sizeof(buf), fp))	/* ÁöµM³sÀÉÀY³£ÁÙ¨S¦L§¹¡A¦ý¬OÀÉ®×¤w¸gµ²§ô¡Aª½±µÂ÷¶} */
    {
      printf("</table>\r\n");
      return;
    }

    if (memcmp(buf, header1[i], LEN_AUTHOR1 - 1) && memcmp(buf, header2[i], LEN_AUTHOR2 - 1))	/* ¤£¬OÀÉÀY */
      break;

    /* §@ªÌ/¬ÝªO ÀÉÀY¦³¤GÄæ¡A¯S§O³B²z */
    if (i == 0 && (strstr(buf, "¬ÝªO:") || strstr(buf, "¯¸¤º:")))
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

  if (i < LINE_HEADER)		/* ³Ì«á¤@¦æ¤£¬OÀÉÀY */
  {
    ansi2tag(encodebuf, quotebuf(buf));
    printf(encodebuf);
  }

  /* ³B²z¤º¤å */
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
