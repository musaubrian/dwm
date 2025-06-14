/* See LICENSE file for copyright and license details. */
#include <X11/XF86keysym.h>
#include "fibonacci.c"

/* appearance */
static const unsigned int borderpx  = 1;        /* border pixel of windows */
static const unsigned int snap      = 25;       /* snap pixel */
static const unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayonleft = 0;    /* 0: systray in the right corner, >0: systray on left of status text */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static const int showsystray        = 1;        /* 0 means no systray */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char *fonts[]          = { "Iosevka:size=12" };

static const char col_fg[]        = "#888D94";
static const char col_bg[]        = "#1B1D1E";
static const char col_border[]    = "#232628";
static const char col_selbg[]     = "#262D32";
static const char col_selfg[]     = "#B0B8C0";
static const char col_selborder[] = "#3B4252";

static const char *colors[][3] = {
    [SchemeNorm] = { col_fg,    col_bg,    col_border },
    [SchemeSel]  = { col_selfg, col_selbg, col_selborder },
};


/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5"};


static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{ "firefox",  NULL,       NULL,       1 << 1,       0,           -1 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 0;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

static const Layout layouts[] = {
	/* first entry is default */
	/* symbol     arrange function */
	{ "[\\]",      dwindle },
	{ "[]=",      tile },
	{ "[f]",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle }, // Kinda fullscreen-ish
	{ "[@]",      spiral },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *launcherCmd[] = {"rofi", "-show", "drun", NULL};
static const char *termcmd[]  = { "st", NULL };
static const char *browserCmd[] = {"firefox", NULL};

static const Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_d,      spawn,          {.v = launcherCmd } },
	{ MODKEY,                       XK_t,      spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_b,      spawn,          {.v = browserCmd } },
	{ MODKEY|ShiftMask,             XK_d,      spawn,          SHCMD("xrandr --output HDMI-1 --auto --right-of eDP-1") },
	{ MODKEY|ShiftMask,             XK_x,      spawn,          SHCMD("xrandr --auto") },

	{ MODKEY|ShiftMask,             XK_f,      fullscreen,     {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_equal,  incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_minus,  incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask,             XK_c,      killclient,     {0} },
	{ MODKEY|ShiftMask,             XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[3]} },
	{ MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_5,      tagmon,         {.i = 1} },
	//
	{ 0,   XF86XK_AudioRaiseVolume,    spawn,     {.v = (const char*[]){"wpctl", "set-volume", "@DEFAULT_SINK@", ".1+", NULL} } },
	{ 0,   XF86XK_AudioLowerVolume,    spawn,     {.v = (const char*[]){"wpctl", "set-volume", "@DEFAULT_SINK@", ".1-", NULL} } },
	{ 0,   XF86XK_AudioMute,           spawn,     {.v = (const char*[]){"wpctl", "set-mute", "@DEFAULT_SINK@", "toggle", NULL} } },
	{ 0,   XF86XK_AudioMicMute,        spawn,     {.v = (const char*[]){"wpctl", "set-mute", "@DEFAULT_SOURCE@", "toggle", NULL} } },
	{ 0,   XF86XK_MonBrightnessDown,   spawn,     {.v = (const char*[]){"brightnessctl", "s", "1-%", NULL} } },
	{ 0,   XF86XK_MonBrightnessUp,     spawn,     {.v = (const char*[]){"brightnessctl", "s", "+1%", NULL} } },
	{ 0,   XK_Print,                   spawn,     {.v = (const char*[]){"flameshot", "gui", NULL} } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	{ MODKEY|ShiftMask,             XK_q,      quit,           {0} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

