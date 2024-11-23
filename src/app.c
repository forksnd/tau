/* std */
#include <assert.h>
#include <stddef.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* app */
#include "sys/cpu.h"
#include "lib/fmt.h"
#include "lib/std.h"
#include "lib/rpq.h"
#include "lib/fnt.h"
#include "lib/sql.h"
#include "sys/gfx.h"
#include "sys/sys.h"
#include "gui/res.h"
#include "gui/gui.h"
#include "app/cfg.h"
#include "app/pck.h"
#include "app/dbs.h"
#include "app.h"

#define APP_VIEW_CNT 8
#define APP_VIEW_PATH_BUF (MAX_FILE_PATH * APP_VIEW_CNT)

enum app_view_state {
  APP_VIEW_STATE_FILE,
  APP_VIEW_STATE_DB,
};
struct app_view {
  enum app_view_state state;
  enum app_view_state last_state;
  struct db_state db;
  struct str file_path;
  int path_buf_off;
};
struct app {
  struct res res;
  struct gui_ctx gui;
  struct file_view fs;
  struct db_view db;

  /* views */
  double tab_lst_off[2];
  unsigned unused;
  unsigned char show_tab_lst;
  unsigned char sel_tab;
  unsigned char tab_cnt;
  unsigned char tabs[APP_VIEW_CNT];
  struct app_view views[APP_VIEW_CNT];

  char path_buf[APP_VIEW_PATH_BUF];
  char db_mem[CFG_DB_MAX_MEMORY];
  char gui_mem[CFG_GUI_MAX_MEMORY];
};

static struct res_api res;
static struct gui_api gui;
static struct pck_api pck;
static struct db_api dbs;

#include "lib/fmt.c"
#include "lib/std.c"
#include "lib/math.c"
#include "lib/rpq.c"
#include "lib/fnt.c"
#include "lib/sql.c"
#include "gui/res.c"
#include "gui/gui.c"
#include "app/pck.c"
#include "app/dbs.c"

/* -----------------------------------------------------------------------------
 *                                  App
 * ---------------------------------------------------------------------------*/
static int
app_view_new(struct app *app) {
  assert(app);
  assert(app->unused > 0);

  int idx = cpu_bit_ffs32(app->unused);
  app->unused &= ~(1u << idx);
  mset(app->views + idx, 0, szof(app->views[0]));
  return idx;
}
static void
app_view_del(struct app *app, int idx) {
  assert(app);
  assert(idx >= 0);
  assert(idx < cntof(app->views));
  assert(!(app->unused & (1u << idx)));

  struct app_view *view = &app->views[idx];
  app->unused |= (1u << idx);
  view->file_path = str_nil;
}
static void
app_view_setup(struct app *app, int idx) {
  assert(app);
  assert(idx >= 0);
  assert(idx < cntof(app->views));
  assert(!(app->unused & (1u << idx)));

  struct app_view *view = &app->views[idx];
  view->state = APP_VIEW_STATE_FILE;
  view->last_state = view->state;
  view->path_buf_off = idx * MAX_FILE_PATH;
}
static int
app_view_init(struct app *app, int idx, struct str path) {
  assert(app);
  assert(idx >= 0);
  assert(idx < cntof(app->views));
  assert(!(app->unused & (1u << idx)));

  struct app_view *view = &app->views[idx];
  app_view_setup(app, idx);
  view->file_path = str_set(app->path_buf + view->path_buf_off, MAX_FILE_PATH, path);
  if (str_is_inv(view->file_path)) {
    return -1;
  }
  view->state = APP_VIEW_STATE_DB;
  return 0;
}
static int
app_tab_add(struct app *app, int idx) {
  assert(app);
  assert(idx >= 0);
  assert(idx < cntof(app->views));
  assert(app->tab_cnt < cntof(app->tabs));
  assert(!(app->unused & (1u << idx)));

  app->tabs[app->tab_cnt] = castb(idx);
  return app->tab_cnt++;
}
static void
app_tab_rm(struct app *app, int tab_idx) {
  assert(app);
  assert(app->tab_cnt > 0);
  assert(tab_idx >= 0);
  assert(tab_idx < app->tab_cnt);
  assert(tab_idx < cntof(app->tabs));

  arr_rm(app->tabs, tab_idx, app->tab_cnt);
  app->tab_cnt--;
  app->sel_tab = castb(clamp(0, app->sel_tab, app->tab_cnt-1));
}
static void
app_tab_close(struct app *app, int tab_idx) {
  assert(app);
  assert(app->tab_cnt > 0);
  assert(tab_idx >= 0);
  assert(tab_idx < app->tab_cnt);
  assert(tab_idx < cntof(app->tabs));
  assert(!(app->unused & (1u << tab_idx)));

  app_view_del(app, app->tabs[tab_idx]);
  app_tab_rm(app, tab_idx);
}
static void
app_tab_open_files(struct app *app, const struct str *files, int file_cnt) {
  assert(app);
  assert(files);
  assert(file_cnt >= 0);
  /* open each database in new tab */
  for (int i = 0; app->unused && i < file_cnt; ++i) {
    int view = app_view_new(app);
    if (app_view_init(app, view, files[i]) < 0) {
      app_view_del(app, view);
      continue;
    }
    app->sel_tab = castb(app_tab_add(app, view));
    if (!dbs.setup(&app->views[view].db, &app->gui, files[i])) {
      app_tab_close(app, app->sel_tab);
    }
  }
}
static int
app_tab_open(struct app *app) {
  assert(app);
  assert(app->unused > 0);
  assert(app->tab_cnt < cntof(app->tabs));

  int view = app_view_new(app);
  app->sel_tab = castb(app_tab_add(app, view));
  return app->sel_tab;
}
static void
app_tab_swap(struct app *app, int dst_idx, int src_idx) {
  assert(app);
  assert(dst_idx >= 0);
  assert(src_idx >= 0);

  assert(dst_idx < app->tab_cnt);
  assert(src_idx < app->tab_cnt);
  assert(dst_idx < cntof(app->tabs));
  assert(src_idx < cntof(app->tabs));

  assert(!(app->unused & (1u << app->tabs[dst_idx])));
  assert(!(app->unused & (1u << app->tabs[src_idx])));
  iswap(app->tabs[dst_idx], app->tabs[src_idx]);
}
static void
app_init(struct app *app, struct sys *s) {
  assert(s);
  assert(app);
  res.init(&app->res, s);
  {
    struct gui_args args = {0};
    args.scale = s->dpi_scale;
    if (s->has_style){
      args.scm = GUI_COL_SCHEME_SYS;
    } else {
      args.scm = GUI_COL_SCHEME_DARK;
    }
    app->gui.sys = s;
    app->gui.res = &app->res;
    app->gui.vtx_buf = app->gui_mem;
    app->gui.idx_buf = app->gui_mem + CFG_GUI_VTX_MEMORY;
    app->gui.vtx_buf_siz = CFG_GUI_VTX_MEMORY;
    app->gui.idx_buf_siz = CFG_GUI_IDX_MEMORY;
    gui.init(&app->gui, &args);
  }
  if (pck.init(&app->fs, s, &app->gui) < 0) {

  }
  dbs.init(app->db_mem, szof(app->db_mem));

  app->tab_cnt = 0;
  app->unused = ~0u;
  app->tabs[app->tab_cnt++] = castb(app_view_new(app));
  app_view_setup(app, app->tabs[0]);

#ifdef DEBUG_MODE
  ut_str(s);
#endif
}
static void
app_shutdown(struct app *app, struct sys *s) {
  assert(app);
  unsigned used = ~app->unused;
  while (used) {
    int idx = cpu_bit_ffs32(used);
    assert(idx < APP_VIEW_CNT);
    struct app_view *view = &app->views[idx];
    if (view->db.con) {
      dbs.del(&view->db);
    }
    app_view_del(app, idx);
    used = ~app->unused;
  }
  pck.shutdown(&app->fs, s);
  res.shutdown(&app->res);
}

/* -----------------------------------------------------------------------------
 *
 *                                  GUI
 *
 * -----------------------------------------------------------------------------
 */
static void
ui_app_file_view(struct app *app, struct app_view *view, struct gui_ctx *ctx,
                struct gui_panel *pan, struct gui_panel *parent) {
  assert(app);
  assert(ctx);
  assert(pan);
  assert(parent);

  view->file_path = pck.ui(app->path_buf + view->path_buf_off, MAX_FILE_PATH, &app->fs, ctx, pan, parent);
  if (str_is_val(view->file_path)) {
    dbs.setup(&view->db, &app->gui, view->file_path);
    view->state = APP_VIEW_STATE_DB;

    int new_view = app_view_new(app);
    app_view_setup(app, new_view);
    app_tab_add(app, new_view);
    ctx->sys->repaint = 1;
  }
}
static void
ui_app_view(struct app *app, struct app_view *view, struct gui_ctx *ctx,
            struct gui_panel *pan, struct gui_panel *parent) {
  assert(app);
  assert(ctx);
  assert(pan);
  assert(parent);

  gui.pan.begin(ctx, pan, parent);
  {
    struct gui_panel bdy = {.box = pan->box};
    switch (view->state) {
    case APP_VIEW_STATE_FILE: {
      ui_app_file_view(app, view, ctx, pan, parent);
    } break;
    case APP_VIEW_STATE_DB: {
      dbs.ui(&view->db, &app->db, ctx, &bdy, pan);
    } break;}
  }
  gui.pan.end(ctx, pan, parent);
}
static void
ui_app_dnd_files(struct app *app, struct gui_ctx *ctx, struct gui_panel *pan) {
  assert(app);
  assert(ctx);
  assert(pan);

  if (gui.dnd.dst.begin(ctx, pan)) {
    struct gui_dnd_paq *paq = gui.dnd.dst.get(ctx, STR_HASH16("[sys:files]"));
    if (paq) { /* file drag & drop */
      const struct str *file_urls = paq->data;
      switch (paq->state) {
      case GUI_DND_DELIVERY: {
        int file_cnt = paq->size;
        app_tab_open_files(app, file_urls, file_cnt);
        paq->response = GUI_DND_ACCEPT;
      } break;
      case GUI_DND_LEFT: break;
      case GUI_DND_ENTER:
      case GUI_DND_PREVIEW: {
        paq->response = GUI_DND_ACCEPT;
      } break;}
    }
    gui.dnd.dst.end(ctx);
  }
}
static int
ui_app_tab_view_lst(struct app *app, struct gui_ctx *ctx, struct gui_panel *pan,
                    struct gui_panel *parent) {
  assert(app);
  assert(ctx);
  assert(pan);
  assert(parent);

  int ret = -1;
  gui.pan.begin(ctx, pan, parent);
  {
    struct gui_lst_cfg cfg = {0};
    int cnt = cpu_bit_cnt(~app->unused);
    gui.lst.cfg(&cfg, cnt, app->tab_lst_off[1]);
    cfg.ctl.focus = GUI_LST_FOCUS_ON_HOV;
    cfg.sel.on = GUI_LST_SEL_ON_HOV;

    struct gui_lst_reg reg = {.box = pan->box};
    gui.lst.reg.begin(ctx, &reg, pan, &cfg, app->tab_lst_off);
    for gui_lst_reg_loop(i,gui,&reg) {

      struct gui_panel elm = {0};
      struct app_view *view = &app->views[app->tabs[i]];
      unsigned long long tab_id = hash_ptr(view);
      gui.lst.reg.elm.txt(ctx, &reg, &elm, tab_id, 0, strv("Open"), 0);

      struct gui_input in = {0};
      gui.pan.input(&in, ctx, &elm, GUI_BTN_LEFT);
      ret = in.mouse.btn.left.clk ? i : ret;
    }
    gui.lst.reg.end(ctx, &reg, pan, app->tab_lst_off);
  }
  gui.pan.end(ctx, pan, parent);
  return ret;
}
static int
ui_app_view_tab_slot_close(struct gui_ctx *ctx, struct gui_panel *pan,
                           struct gui_panel *parent, struct str title,
                           enum res_ico_id ico) {
  assert(ctx);
  assert(pan);
  assert(ico);
  assert(parent);

  int ret = 0;
  gui.pan.begin(ctx, pan, parent);
  {
    struct gui_box lay = pan->box;
    struct gui_icon close = {.box = gui.cut.rhs(&lay, ctx->cfg.item, 0)};
    gui.ico.clk(ctx, &close, pan, RES_ICO_CLOSE);
    ret = close.clk;

    struct gui_panel lbl = {.box = lay};
    gui.ico.box(ctx, &lbl, pan, ico, title);
  }
  gui.pan.end(ctx, pan, parent);
  return ret;
}
static int
ui_app_view_tab_slot(struct app *app, struct app_view *view,
                     struct gui_ctx *ctx, struct gui_tab_ctl *tab,
                     struct gui_tab_ctl_hdr *hdr, struct gui_panel *slot,
                     struct str title, enum res_ico_id ico) {
  assert(app);
  assert(ctx);
  assert(tab);
  assert(hdr);
  assert(ico);
  assert(slot);
  assert(view);

  int ret = 0;
  gui.tab.hdr.slot.begin(ctx, tab, hdr, slot, hash_ptr(view));
  if (app->tab_cnt > 1 && tab->idx == tab->sel.idx) {
    ret = ui_app_view_tab_slot_close(ctx, slot, &hdr->pan, title, ico);
  } else {
    gui.ico.box(ctx, slot, &hdr->pan, ico, title);
  }
  gui.tab.hdr.slot.end(ctx, tab, hdr, slot, 0);
  return ret;
}
static int
ui_app_view_tab(struct app *app, struct app_view *view,
                struct gui_ctx *ctx, struct gui_tab_ctl *tab,
                struct gui_tab_ctl_hdr *hdr, struct gui_panel *slot) {
  assert(app);
  assert(ctx);
  assert(tab);
  assert(hdr);
  assert(slot);
  assert(view);

  enum res_ico_id ico = RES_ICO_FOLDER_OPEN;
  struct str title = strv("Open");
  if (view->state != APP_VIEW_STATE_FILE) {
    title = path_file(view->file_path);
    ico = RES_ICO_DATABASE;
  }
  return ui_app_view_tab_slot(app, view, ctx, tab, hdr, slot, title, ico);
}
static void
ui_app_main(struct app *app, struct gui_ctx *ctx, struct gui_panel *pan,
            struct gui_panel *parent) {

  assert(app);
  assert(ctx);
  assert(pan);
  assert(parent);

  gui.pan.begin(ctx, pan, parent);
  {
    /* tab control */
    struct gui_tab_ctl tab = {.box = pan->box, .show_btn = 1};
    gui.tab.begin(ctx, &tab, pan, app->tab_cnt, app->sel_tab);
    {
      /* tab header */
      int del_tab = 0;
      struct gui_tab_ctl_hdr hdr = {.box = tab.hdr};
      gui.tab.hdr.begin(ctx, &tab, &hdr);
      for loop(i, tab.cnt) {
        /* tab header slots */
        struct gui_panel slot = {0};
        struct app_view *view = &app->views[app->tabs[i]];
        del_tab |= ui_app_view_tab(app, view, ctx, &tab, &hdr, &slot);
      }
      gui.tab.hdr.end(ctx, &tab, &hdr);
      if (tab.sort.mod) {
        app_tab_swap(app, tab.sort.dst, tab.sort.src);
      }
      if (del_tab) {
        app_tab_close(app, app->sel_tab);
      }
      if (app->unused) {
        /* add/open new file view tab */
        struct gui_btn add = {.box = hdr.pan.box};
        add.box.x = gui.bnd.min_ext(tab.off, ctx->cfg.item);
        if (gui.btn.ico(ctx, &add, &hdr.pan, RES_ICO_FOLDER_ADD)) {
          app_tab_open(app);
        }
      }
      /* tab body */
      struct gui_panel bdy = {.box = tab.bdy};
      app->show_tab_lst = tab.btn.clk ? !app->show_tab_lst : app->show_tab_lst;
      if (app->show_tab_lst) {
        /* tab selection */
        int ret = ui_app_tab_view_lst(app, ctx, &bdy, pan);
        if (ret >= 0) {
          app_tab_swap(app, 0, ret);
          app->show_tab_lst = 0;
          app->sel_tab = 0;
        }
      } else {
        assert(app->sel_tab < APP_VIEW_CNT);
        int sel_view = app->tabs[app->sel_tab];
        assert(!(app->unused & (1u << sel_view)));
        ui_app_view(app, app->views + sel_view, ctx, &bdy, pan);
      }
    }
    gui.tab.end(ctx, &tab, pan);
    if (tab.sel.mod) {
      app->sel_tab = castb(tab.sel.idx);
      assert(app->sel_tab < APP_VIEW_CNT);
    }
  }
  gui.pan.end(ctx, pan, parent);
  ui_app_dnd_files(app, ctx, pan);
}

/* -----------------------------------------------------------------------------
 *                                  Main
 * -----------------------------------------------------------------------------
 */
static int
app_on_mod(unsigned mod, unsigned keymod) {
  if (mod == 0) {
    return 1;
  } else if (mod == (unsigned)-1) {
    return keymod == 0;
  } else {
    return keymod == mod;
  }
}
static void
usage(const char *app) {
  printf("\n"
      "usage: %s [options] file0 file1\n"
      "\n"
      "   arguments:\n"
      "\n"
      "   file,         Input sqlite database\n"
      "\n"
      "   options:\n"
      "   -h            help message\n"
      "\n",
      app
  );
  exit(1);
}
extern void
app_run(struct sys *s) {
  assert(s);
  switch (s->op) {
  case SYS_SETUP: {
    s->win.title = "Tau";
    s->win.x = -1, s->win.y = -1;
    s->win.w = 800, s->win.h = 600;
    s->win.min_w = CFG_WIN_MIN_WIDTH;
    s->win.min_h = CFG_WIN_MIN_HEIGHT;
    s->win.max_w = CFG_WIN_MAX_WIDTH;
    s->win.max_h = CFG_WIN_MAX_HEIGHT;
    s->gfx.clear_color = col_black;
  } break;

  case SYS_RUN: {
    if (s->app == 0) {
      /* init */
      res_api(&res, 0);
      gui_api(&gui, 0);
      pck_api(&pck, 0);
      db_api(&dbs, 0);

      struct app *app = calloc(1, sizeof(struct app));
      app_init(app, s);
      s->app = app;

      const char *exe = 0;
      cmd_arg_begin(exe, s->argc, s->argv){
      case 'h': default: usage(exe); break;
      } cmd_arg_end
    }
    /* user interface */
    assert(s->app);
    struct app *app = (struct app*)s->app;
    if (s->style_mod) {
      gui.color_scheme(&app->gui, CFG_COLOR_SCHEME);
    }
    for arr_loopv(i, app_ui_key_tbl) {
      /* map system keys to ui shortcuts */
      const struct app_ui_shortcut *k = app_ui_key_tbl + i;
      struct gui_ctx *ctx = &app->gui;
      int keymod = app_on_mod(k->key.mod, s->keymod);
      if (bit_tst(s->keys, k->key.code) && keymod) {
        bit_set(ctx->keys, i);
      } else if (bit_tst(s->keys, k->alt.code) && keymod) {
        bit_set(ctx->keys, i);
      }
    }
    /* run app ui */
    while (gui.begin(&app->gui)) {
      struct gui_panel pan = {.box = app->gui.box};
      ui_app_main(app, &app->gui, &pan, &app->gui.root);
      gui.end(&app->gui);
    }
  } break;

  case SYS_QUIT: {
    /* shutdown */
    app_shutdown(s->app, s);
    free(s->app);
    s->app = 0;
  } break;
  }
}

