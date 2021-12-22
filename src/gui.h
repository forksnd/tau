#ifndef GUI_H_INCL
#define GUI_H_INCL

#define GUI_VERSION 1

/* input */
enum gui_keys {
  GUI_KEY_ACT,
  GUI_KEY_DEACT,

  GUI_KEY_NEXT_WIDGET,
  GUI_KEY_PREV_WIDGET,

  GUI_KEY_TAB_NEXT,
  GUI_KEY_TAB_PREV,

  /* menu */
  GUI_KEY_MENU_NXT,

  /* spinner */
  GUI_KEY_SPIN_INC,
  GUI_KEY_SPIN_DEC,

  /* list */
  GUI_KEY_LST_LEFT,
  GUI_KEY_LST_RIGHT,
  GUI_KEY_LST_UP,
  GUI_KEY_LST_DN,
  GUI_KEY_LST_SEL,
  GUI_KEY_LST_SEL_ALL,
  GUI_KEY_LST_CPY,
  GUI_KEY_LST_CUT,
  GUI_KEY_LST_PASTE,
  GUI_KEY_LST_DEL,
  GUI_KEY_LST_RET,

  /* tree */
  GUI_KEY_TREE_EXPAND,
  GUI_KEY_TREE_COLLAPSE,
  GUI_KEY_TREE_LEFT,
  GUI_KEY_TREE_RIGHT,

  /* scroll */
  GUI_KEY_SCRL_PGDN,
  GUI_KEY_SCRL_PGUP,
  GUI_KEY_SCRL_BEGIN,
  GUI_KEY_SCRL_END,

  /* edit */
  GUI_KEY_EDIT_CUR_LEFT,
  GUI_KEY_EDIT_CUR_RIGHT,
  GUI_KEY_EDIT_CUR_UP,
  GUI_KEY_EDIT_CUR_DOWN,
  GUI_KEY_EDIT_WORD_LEFT,
  GUI_KEY_EDIT_WORD_RIGHT,
  GUI_KEY_EDIT_START,
  GUI_KEY_EDIT_END,

  GUI_KEY_EDIT_IN_MODE,
  GUI_KEY_EDIT_REPL_MODE,
  GUI_KEY_EDIT_CLR_MODE,

  GUI_KEY_EDIT_SEL_ALL,
  GUI_KEY_EDIT_SEL_CUR_LEFT,
  GUI_KEY_EDIT_SEL_CUR_RIGHT,
  GUI_KEY_EDIT_SEL_CUR_UP,
  GUI_KEY_EDIT_SEL_CUR_DOWN,
  GUI_KEY_EDIT_SEL_WORD_LEFT,
  GUI_KEY_EDIT_SEL_WORD_RIGHT,
  GUI_KEY_EDIT_SEL_START,
  GUI_KEY_EDIT_SEL_END,

  GUI_KEY_EDIT_DELETE,
  GUI_KEY_EDIT_REMOVE,
  GUI_KEY_EDIT_COMMIT,
  GUI_KEY_EDIT_ABORT,
  GUI_KEY_EDIT_COPY,
  GUI_KEY_EDIT_CUT,
  GUI_KEY_EDIT_PASTE,
  GUI_KEY_EDIT_UNDO,
  GUI_KEY_EDIT_REDO,
  GUI_KEY_CNT
};
enum gui_mouse_btn_id {
  GUI_MOUSE_LEFT,
  GUI_MOUSE_RIGHT,
  GUI_MOUSE_MIDDLE,
  GUI_MOUSE_BTN_CNT
};
enum gui_mouse_btn_mask {
  GUI_BTN_LEFT = 1 << GUI_MOUSE_LEFT,
  GUI_BTN_RIGHT = 1 << GUI_MOUSE_RIGHT,
  GUI_BTN_MIDDLE = 1 << GUI_MOUSE_MIDDLE,
};
struct gui_mouse_btn {
  unsigned clk : 1;
  unsigned doubled : 1;
  unsigned pressed : 1;
  unsigned released : 1;
  unsigned down : 1;

  unsigned grabbed : 1;
  unsigned gained_grab : 1;
  unsigned lost_grab : 1;

  unsigned drag_begin : 1;
  unsigned dragged : 1;
  unsigned drag_end : 1;
  int drag_pos[2];
};
struct gui_mouse_btns {
  struct gui_mouse_btn left;
  struct gui_mouse_btn right;
  struct gui_mouse_btn middle;
  struct gui_mouse_btn mouse4;
  struct gui_mouse_btn mouse5;
};
struct gui_mouse {
  int pos[2];
  int pos_rel[2];
  int pos_last[2];
  int pos_delta[2];
  int scroll_delta;
  union {
    struct gui_mouse_btn btns[GUI_MOUSE_BTN_CNT];
    struct gui_mouse_btns btn;
  };
};
struct gui_input {
  unsigned long *keys;
  const char *txt;

  struct gui_mouse mouse;
  unsigned is_hot : 1;
  unsigned entered : 1;
  unsigned exited : 1;
  unsigned is_focused : 1;
  unsigned gained_focus : 1;
  unsigned lost_focus : 1;
};

/* box */
struct gui_bnd {
  int min, mid, max, ext;
};
struct gui_box {
  struct gui_bnd x;
  struct gui_bnd y;
};
enum gui_box_cut_side {
  GUI_BOX_CUT_LHS,
  GUI_BOX_CUT_RHS,
  GUI_BOX_CUT_TOP,
  GUI_BOX_CUT_BOT,
};
#define gui_box_cut(b, s, g) (struct gui_box_cut){b, s, g}
struct gui_box_cut {
  struct gui_box *box;
  enum gui_box_cut_side side;
  int gap;
};
#define gui_unbox(b) (b)->x.min, (b)->y.min, (b)->x.max, (b)->y.max

/* panel */
struct gui_ctx;
#define gui_id(ptr) ((unsigned long long)(ptr))
enum gui_state {
  GUI_HIDDEN,
  GUI_NORMAL,
  GUI_HOVERED,
  GUI_PRESSED,
  GUI_FOCUSED,
  GUI_DISABLED,
};
struct gui_panel {
  struct gui_box box;

  unsigned long long id;
  enum gui_state state;

  int max[2];
  int mouse_pos[2];
  int rel_mouse_pos[2];

  unsigned focusable : 1; /* in */
  unsigned is_hot : 1;
  unsigned is_hov : 1;
  unsigned has_focus : 1;
  unsigned is_focused : 1;
};

enum gui_collapse_state {
  GUI_COLLAPSED,
  GUI_EXPANDED,
};

/* Widget: Text */
enum gui_halign {
  GUI_HALIGN_LEFT,
  GUI_HALIGN_MID,
  GUI_HALIGN_RIGHT,
};
enum gui_valign {
  GUI_VALIGN_TOP,
  GUI_VALIGN_MID,
  GUI_VALIGN_BOT,
};
struct gui_align {
  enum gui_halign h;
  enum gui_valign v;
};

/* Widget: Button */
struct gui_btn {
  struct gui_box box;
  struct gui_panel pan;
  /* out */
  struct gui_input in;
  unsigned clk : 1;
  unsigned pressed : 1;
  unsigned released : 1;
};

/* Widget: Icon */
struct gui_icon {
  struct gui_box box;
  struct gui_panel pan;
  /* out */
  struct gui_input in;
  unsigned clk : 1;
  unsigned pressed : 1;
  unsigned released : 1;
};

enum gui_chk_state {
  GUI_CHK_UNSELECTED,
  GUI_CHK_PARTIAL,
  GUI_CHK_SELECTED
};

/* Utility: Layout */
enum gui_orient {
  GUI_HORIZONTAL,
  GUI_VERTICAL,
};
struct gui_lay_sol {
  int fix_siz;
  int fix_cnt;
  int dyn_siz;
  int dyn_cnt;
  float weight;
};
enum gui_lay_type {
  GUI_LAY_ROW,
  GUI_LAY_COL
};
struct gui_lay {
  struct gui_box box; /* in */
  /* internal */
  enum gui_lay_type type;
  struct gui_box sub;
  int idx, cnt;
  int gap[2], item;
};
#define gui_row(gui, lay, row, def, gap, con, sol) \
  for (int lay[cntof((int[])def)],            \
    uniqid__i_) = ((gui)->lay.solve(lay, (row).x.ext, (int[])def, cntof((int[])def), gap, (int[])con, sol), 0); \
    uniqid(_i_) < 1; ++uniqid(_i_))
#define gui_hlay(gui, ctx, items, lay, def, row_h, row_gap, col_gap, con, sol)\
  for (int items[cntof(def)],            \
    uniqid(_i_) = ((gui)->lay.hlay(ctx, lay, items, def, cntof(def), row_h, row_gap, col_gap, con, sol), 0); \
    uniqid(_i_) < 1; ++uniqid(_i_))
#define gui_vlay(gui, ctx, items, lay, def, col_w, row_gap, col_gap, con, sol)\
  for (int items[cntof(def)],            \
    uniqid(_i_) = ((gui)->lay.vlay(ctx, lay, items, def, cntof(def), col_w, row_gap, col_gap, con, sol), 0); \
    uniqid(_i_) < 1; ++uniqid(_i_))

/* Widget: Edit */
#define GUI_EDT_UNDO_CNT 128
#define GUI_EDT_UNDO_CHAR_CNT (4 * 1024)

struct gui_txt_ed_undo_entry {
  int where;
  short in_len;
  short del_len;
  short char_at;
};
struct gui_txt_ed_undo {
  short undo_pnt;
  short redo_pnt;
  short undo_char_pnt;
  short redo_char_pnt;
  struct gui_txt_ed_undo_entry stk[GUI_EDT_UNDO_CNT];
  char buf[GUI_EDT_UNDO_CHAR_CNT];
};
enum gui_txt_ed_mode {
  GUI_EDT_MODE_INSERT,
  GUI_EDT_MODE_REPLACE
};
struct gui_txt_ed {
  int off, cur;
  int sel[2];

  char mode;
  char act;
  char init;

  float pref_x;
  struct gui_txt_ed_undo undo;
};
enum gui_edit_flags {
  GUI_EDIT_DEF              = 0x00,
  GUI_EDIT_SEL_ON_ACT       = 0x01,
  GUI_EDIT_CLR_ON_ACT       = 0x02,
  GUI_EDIT_ENABLE_COMMIT    = 0x04,
  GUI_EDIT_READ_ONLY        = 0x08,
  GUI_EDIT_GOTO_END_ON_ACT  = 0x10,
};
struct gui_edit_box {
  /* in: */
  unsigned flags;
  struct gui_box box;
  /* out: */
  struct gui_panel pan;
  unsigned active : 1;
  unsigned focused : 1;
  unsigned unfocused : 1;
  unsigned commited : 1;
  unsigned aborted : 1;
  unsigned mod : 1;
};

/* Widget: Scroll */
struct gui_scrl {
  struct gui_box box;     /* in */
  int min[2];             /* in */
  double total[2];        /* in */
  double size[2];         /* in */
  double off[2];          /* in-out */
  struct gui_panel pan;   /* out */
  unsigned scrolled : 1;  /* out */
};
struct gui_scrl_bar {
  /* in: */
  struct gui_box box;
  double step;
  int min_size;
  double total;
  double size;
  double off;
  /* out: */
  struct gui_panel pan;
  struct gui_btn btn_inc;
  struct gui_btn btn_dec;
  unsigned scrolled : 1;
};

/* Widget: Spin */
enum gui_spin_val_typ { GUI_SPIN_INT, GUI_SPIN_FLT };
union gui_spin_dat {float f; int i;};
struct gui_spin_val {
  enum gui_spin_val_typ typ;
  union gui_spin_dat val;
  union gui_spin_dat min;
  union gui_spin_dat max;
  union gui_spin_dat inc;
};
enum gui_spin_flag {
  GUI_SPIN_SLIDER = 1 << 0,
};
struct gui_spin {
  /* in */
  struct gui_box box;
  unsigned flags;
  /* out */
  struct gui_panel pan;
  unsigned drag_begin : 1;
  unsigned dragged : 1;
  unsigned drag_end : 1;
  unsigned mod : 1;
};

/* Widget: Group */
struct gui_grp {
  /* in */
  struct gui_box box;
  /* out */
  struct gui_panel pan;
  /* internal */
  int ext[2];
  struct gui_box content;
};

/* Widget: Region */
struct gui_reg {
  /* in */
  struct gui_box box;
  /* out */
  struct gui_panel pan;
  int scrl_wheel;
  double off[2];
  struct gui_box space;
  struct gui_scrl_bar vscrl;
  struct gui_scrl_bar hscrl;
  unsigned scrolled : 1;
  /* internal */
  double max_off[2];
  struct gui_box clip_rect;
  unsigned background;
};

/* Utility: List-Layout */
struct gui_lst_lay {
  /* in */
  enum gui_orient orient;
  struct gui_box box;
  int pad[2], gap[2];
  int item[2];
  double offset;
  /* out */
  int space[2];
  int slot[2];
  int cnt[2];
  int page, page_cnt;
  int off, off_idx;
  int view_cnt;
  /* internal */
  int idx;
  struct gui_box lay;
  struct gui_box total;
  struct gui_box elm;
};

/* Utility: List-Control */
enum gui_lst_focus {
  GUI_LST_FOCUS_ON_CLK,
  GUI_LST_FOCUS_ON_HOV,
};
enum gui_lst_scrl {
  GUI_LST_FIT_ITEM_START,
  GUI_LST_FIT_ITEM_END,
  GUI_LST_CLAMP_ITEM
};
struct gui_lst_ctl {
  /* in */
  enum gui_lst_focus focus;
  unsigned show_cursor : 1;
  /* out: scroll event */
  enum gui_lst_scrl scrl;
  int item_idx;
  unsigned was_scrolled : 1;
  /* out: state */
  unsigned mod : 1;
  unsigned has_focus : 1;
  unsigned gained_focus : 1;
  unsigned lost_focus : 1;
  unsigned key_mod : 1;
  unsigned cur_mod : 1;
  unsigned jmp_mod : 1;
  unsigned activated : 1;
  /* internal */
  unsigned cur_vis : 1;
  struct gui_box cur_box;
  unsigned long long focused_node;
  unsigned long long owner_id;
  int idx;
};

/* Utility: List-Selector */
enum gui_lst_sel_mode {
  GUI_LST_SEL_SINGLE,
  GUI_LST_SEL_MULTI,
};
enum gui_lst_sel_on {
  GUI_LST_SEL_ON_CLK,
  GUI_LST_SEL_ON_HOV,
  GUI_LST_SEL_ON_NEVER
};
enum gui_lst_sel_behavior {
  GUI_LST_SEL_BHV_FLEETING,
  GUI_LST_SEL_BHV_TOGGLE,
};
enum gui_lst_sel_src {
  GUI_LST_SEL_SRC_INT,
  GUI_LST_SEL_SRC_EXT
};
enum gui_lst_sel_hov {
  GUI_LST_SEL_HOV_NO,
  GUI_LST_SEL_HOV_YES
};
enum gui_lst_sel_mod {
  GUI_LST_SEL_MOD_REPLACE,
  GUI_LST_SEL_MOD_EMPLACE
};
enum gui_lst_sel_op {
  GUI_LST_SEL_OP_SET,
  GUI_LST_SEL_OP_CLR,
};
struct gui_lst_sel {
  /* in: */
  enum gui_lst_sel_mode mode;
  enum gui_lst_sel_src src;
  enum gui_lst_sel_on on;
  enum gui_lst_sel_hov hov;
  enum gui_lst_sel_behavior bhv;
  unsigned long *bitset;
  unsigned long long *set;
  int cnt;
  /* out: */
  enum gui_lst_sel_mod mut;
  enum gui_lst_sel_op op;
  int begin_idx;
  int end_idx;
  int idx;
  /* out: key event */
  unsigned mod : 1;
  unsigned cpy : 1;
  unsigned cut : 1;
  unsigned del : 1;
  unsigned paste : 1;
  unsigned ret : 1;
  /* internal */
  unsigned has_mod : 1;
};

/* Utility: View */
struct gui_lst_view {
  /* in */
  int total_cnt;
  /* out */
  int begin, end;
  int total[2];
  int at[2], max[2];
  int cnt[2];
};

/* Utility: List */
struct gui_lst_state {
  int focused;
  unsigned long long owner;
  int sel_idx;
  int sel_op;
  int cur_idx;
};
enum gui_lst_fltr_on {
  GUI_LST_FLTR_ON_ZERO,
  GUI_LST_FLTR_ON_ONE
};
struct gui_lst_lay_cfg {
  enum gui_orient orient;
  int pad[2], gap[2];
  int item[2];
  double offset;
  int scrl_mult;
};
struct gui_lst_ctl_cfg {
  enum gui_lst_focus focus;
  unsigned show_cursor : 1;
};
struct gui_lst_sel_cfg {
  enum gui_lst_sel_src src;
  enum gui_lst_sel_behavior bhv;
  enum gui_lst_sel_mode mode;
  enum gui_lst_sel_on on;
  enum gui_lst_sel_hov hov;
  unsigned long *bitset;
  int cnt;
};
struct gui_lst_fltr_cfg {
  enum gui_lst_fltr_on on;
  unsigned long *bitset;
};
struct gui_lst_cfg {
  int total_cnt; /* view */
  struct gui_lst_lay_cfg lay;
  struct gui_lst_ctl_cfg ctl;
  struct gui_lst_sel_cfg sel;
  struct gui_lst_fltr_cfg fltr;
};
struct gui_lst {
  /* in */
  struct gui_box box;
  /* out */
  int begin;
  int end;
  int cnt;
  /* internal */
  struct gui_lst_lay lay;
  struct gui_lst_ctl ctl;
  struct gui_lst_sel sel;
  struct gui_lst_view view;

  int fltr_cnt;
  uintptr_t *fltr_set;
  unsigned long *fltr_bitset;
  enum gui_lst_fltr_on fltr_on;
  unsigned long long id;
};
#define for_gui_lst(i,gui,l)\
  for (int i = (l)->begin; i < (l)->end; i = (gui).lst.nxt(l, i))

/* Widget: List-Area */
struct gui_lst_reg {
  struct gui_box box;
  struct gui_lst lst;
  struct gui_reg reg;
};
#define for_gui_reg_lst(i,gui,r)\
  for (int i = (r)->lst.begin; i < (r)->lst.end; i = (gui).lst.nxt(&(r)->lst, i))

/* Widget: Tree-Node */
enum gui_tree_type {
  GUI_TREE_NODE,
  GUI_TREE_LEAF,
};
struct gui_tree_node {
  enum gui_tree_type type;  /* in */
  unsigned open:1;          /* in-out */
  /* out */
  unsigned changed:1;
  struct gui_box box;
  struct gui_panel pan;
};

/* Widget: Splitter */
#define GUI_SPLIT_CAP(n) ((n) * 8 + 4)
#define GUI_SPLIT_COL(n) ((n) << 1)
struct gui_split_toc {
  int col_cnt;
  int lay_cnt;
  int *slot;
  int *cons;
  int *seps;
};
enum gui_split_lay_slot_type {
  GUI_LAY_SLOT_FIX,
  GUI_LAY_SLOT_DYN
};
struct gui_split_lay_slot {
  int type;
  int size;
  int con[2];
};
struct gui_split_lay_cfg {
  int total;
  int cnt;
  int off;
  int size;
  const void *slots;
  // optional:
  const int *sort;
  const unsigned long *fltr;
};
struct gui_split_lay {
  struct gui_split_toc toc;
  int cnt, idx;
  int sep_size;
};
enum gui_split_type {
  GUI_SPLIT_FIT,
  GUI_SPLIT_EXP,
};
struct gui_split {
  /* in */
  struct gui_box box;
  enum gui_orient orient;
  enum gui_split_type typ;
  /* out */
  struct gui_panel pan;
  struct gui_box item;
  /* internal */
  struct gui_box lay;
  struct gui_lay_sol sol;
  int idx, sidx, cnt;
};

/* Widget: Table */
#define GUI_TBL_CAP(n) GUI_SPLIT_CAP(n)
#define GUI_TBL_COL(n) GUI_SPLIT_COL(n)
enum gui_sort_order {
  GUI_SORT_ASC,
  GUI_SORT_DESC,
};
struct gui_tbl_sort {
  enum gui_sort_order order;
  int col;
};
struct gui_tbl_lst_lay_cfg {
  int pad[2];
  int gap[2];
  int item_size;
  int scrl_mult;
};
struct gui_tbl_lst_cfg {
  int total_cnt;
  struct gui_tbl_lst_lay_cfg lay;
  struct gui_lst_ctl_cfg ctl;
  struct gui_lst_sel_cfg sel;
  struct gui_lst_fltr_cfg fltr;
};
struct gui_tbl {
  struct gui_box box; /* in */
  /* out: */
  struct gui_panel *hdr;
  unsigned resort : 1;
  struct gui_tbl_sort sort;
  /* out: loop */
  struct gui_panel *pan;
  struct gui_lst lst;
  /* internal */
  int hdr_h;
  struct gui_reg reg;
  struct gui_split spt;
  struct gui_box clip;
  struct gui_box col_clip;
  struct gui_box col_lay;
  int idx, cnt;
};
#define for_gui_tbl_lst(i,gui,t)\
  for (int i = (t)->lst.begin; i < (t)->lst.end; i = (gui).tbl.lst.nxt(&(t)->lst, i))

/* Widget: Tab Control */
struct gui_tab_ctl_hdr {
  struct gui_box box;         /* in */
  struct gui_panel pan;       /* out */
  /* internal */
  unsigned long long id;
  struct gui_box slot;
};
struct gui_tab_ctl_sel {
  int idx;                    /* in-out */
  unsigned mod:1;             /* out */
};
struct gui_tab_ctl_sort {
  unsigned mod : 1;
  int dst, src;
};
struct gui_tab_ctl {
  struct gui_box box;           /* in */
  struct gui_panel pan;         /* out */
  struct gui_box bdy;           /* out */
  struct gui_box hdr;           /* out */
  int cnt;                      /* out */
  struct gui_tab_ctl_sel sel;   /* in-out */
  struct gui_btn btn;           /* out */
  unsigned show_btn : 1;        /* in */
  struct gui_tab_ctl_sort sort; /* out */
  /* internal */
  int tab_w;
  int total, idx;
  int off, at;
};

/* Widget: Grid */
enum gui_grid_flags {
  GUI_GRID_DFLT = 0,
  GUI_GRID_X    = 1 << 0,
  GUI_GRID_Y    = 1 << 1,
  GUI_GRID_XY   = GUI_GRID_X|GUI_GRID_Y,
};
struct gui_grid {
  /* in */
  struct gui_box box;
  unsigned flags;
  /* out */
  struct gui_panel pan;
  struct gui_box space;
  double off[2];
  unsigned scrolled : 1;
  /* internal */
  struct gui_box clip;
};

/* Widget: Graph Node */
struct gui_graph_node {
  struct gui_box box; /* in */
  struct gui_panel pan; /* out */
  /* internal */
  struct gui_box lay;
};
struct gui_graph_node_hdr {
  /* in */
  struct gui_box box;
  /* out */
  struct gui_panel pan;
  struct gui_box content;

  struct gui_input in;
  unsigned mov_begin : 1;
  unsigned moved : 1;
  unsigned mov_end : 1;
  int pos[2];
};

/* Context */
enum gui_pass {
  GUI_INPUT,
  GUI_RENDER,
  /* internal */
  GUI_FINISHED,
};
enum gui_color_id {
  GUI_COL_BEGIN = 0,
  GUI_COL_BG = GUI_COL_BEGIN,
  GUI_COL_CONTENT,
  GUI_COL_CONTENT_HOV,
  GUI_COL_SEL,
  GUI_COL_LIGHT,
  GUI_COL_SHADOW_SEAM,
  GUI_COL_SHADOW,
  GUI_COL_ICO,
  GUI_COL_TXT,
  GUI_COL_TXT_DISABLED,
  GUI_COL_TXT_SELECTED,
  GUI_COLOR_MAX
};
enum gui_col_scheme {
  GUI_COL_SCHEME_SYS,
  GUI_COL_SCHEME_DARK,
  GUI_COL_SCHEME_STEAM,
  GUI_COL_SCHEME_GRAY,
};
struct gui_cfg {
  int depth;
  int item;
  int elm;
  int ico;
  int tab;
  int sep;
  int scrl;
  int grid;
  int btn_pad;
  int grp_off;
  int grp_pad;
  int pad[2];
  int gap[2];
  int lay_pad[2];
  int lst_pad[2];
  int pan_pad[2];
  int pan_gap[2];
  unsigned col[GUI_COLOR_MAX];
};
struct gui_cfg_stk {
  int val;
  int *ptr;
};
enum gui_dnd_paq_state {
  GUI_DND_ENTER,
  GUI_DND_PREVIEW,
  GUI_DND_DELIVERY,
  GUI_DND_LEFT
};
struct gui_dnd_paq {
  enum gui_dnd_paq_state state;
  unsigned long long src_id;
  const char *type;
  void *data;
  int size;
};
struct gui_btn_state {
  unsigned long long active;
  unsigned long long prev_active;
  unsigned long long origin;
  int drag_pos[2];
};
struct gui_txt_buf {
  unsigned long long owner_id;
  char *buf;
};
enum gui_img_id {
  GUI_IMG_CHECK,
  GUI_IMG_TIMES,
  GUI_IMG_NO,
  GUI_IMG_MINUS,
  GUI_IMG_PLUS,
  GUI_IMG_CHEVRONL,
  GUI_IMG_CHEVRONR,
  GUI_IMG_MAX,
};
struct gui_ctx {
  /* sys */
  struct sys *sys;
  struct res *res;

  /* state */
  enum gui_pass pass;
  struct gui_cfg cfg;
  unsigned long keys[bits_to_long(GUI_KEY_CNT)];
  struct gui_box box;

  unsigned disabled;
  char focus_next;
  char focus_last;

  /* tree */
  struct gui_panel root;
  unsigned long long id;
  unsigned long long hot;
  unsigned long long prev_hot;
  unsigned long long focusable;
  unsigned long long prev_focused;
  unsigned long long focused;
  unsigned long long prev_id;
  unsigned long long first_id;
  unsigned long long cur_id;

  /* drag & drop */
  unsigned dnd_act : 1;
  unsigned dnd_set : 1;
  unsigned dnd_clr : 1;
  unsigned dnd_in : 1;
  struct gui_dnd_paq dnd_paq;

  /* state */
  struct ren_cmd_buf *ren;
  int ico[GUI_IMG_MAX];
  struct gui_box clip;
  struct gui_btn_state btn[GUI_MOUSE_BTN_CNT];
  double drag_state[2];
  struct gui_lst_state lst_state;
  struct gui_txt_ed txt_ed_tmp_state;
  struct gui_txt_ed txt_ed_state;
  struct gui_txt_buf txt_state;
};

/* API */
struct gui_bnd_api {
  struct gui_bnd (*min_max)(int a, int b);
  struct gui_bnd (*min_ext)(int m, int e);
  struct gui_bnd (*max_ext)(int m, int e);
  struct gui_bnd (*mid_min)(int c, int m);
  struct gui_bnd (*mid_max)(int c, int m);
  struct gui_bnd (*mid_ext)(int c, int e);
  struct gui_bnd (*shrink)(const struct gui_bnd *x, int p);
  struct gui_bnd (*div)(const struct gui_bnd *b, int gap, int cnt, int idx);
};
struct gui_box_api {
  struct gui_box (*div_x)(const struct gui_box *b, int gap, int cnt, int idx);
  struct gui_box (*div_y)(const struct gui_box *b, int gap, int cnt, int idx);
  struct gui_box (*div)(const struct gui_box *b, int *gap, int cntx, int cnty, int x, int y);
  struct gui_box (*mid_ext)(const struct gui_box *b, int w, int h);
  struct gui_box (*box)(int x, int y, int w, int h);
  struct gui_box (*pos)(const struct gui_box *b, int x, int y);
  struct gui_box (*mov)(const struct gui_box *b, int x, int y);
  struct gui_box (*clip)(int x0, int y0, int x1, int y1);
  struct gui_box (*pad)(const struct gui_box *b, int padx, int pady);
  struct gui_box (*padv)(const struct gui_box *b, int *pad);
  struct gui_box (*posv)(const struct gui_box *b, int *p);
};
struct gui_cut_api {
  struct gui_box (*lhs)(struct gui_box *b, int a, int gap);
  struct gui_box (*top)(struct gui_box *b, int a, int gap);
  struct gui_box (*rhs)(struct gui_box *b, int a, int gap);
  struct gui_box (*bot)(struct gui_box *b, int a, int gap);
  struct gui_box (*box)(struct gui_box_cut *cut, int a);
};
struct gui_lay_api {
  void (*solve)(int *res, int ext, const int *slots, int cnt, int gap, const int *con, struct gui_lay_sol *sol);
  struct gui_box (*hcut)(struct gui_lay *lay, int row_h);
  struct gui_box (*vcut)(struct gui_lay *lay, int col_w);
  struct gui_box (*hitem)(struct gui_lay *lay, const int *items);
  struct gui_box (*vitem)(struct gui_lay *lay, const int *items);
  struct gui_box (*item)(struct gui_lay *lay, const int *items);
  void (*hlay)(struct gui_ctx *ctx, struct gui_lay *lay, int *items, const int *def, int cnt, int row_h, int row_gap, int col_gap, const int *con, struct gui_lay_sol *sol);
  void (*vlay)(struct gui_ctx *ctx, struct gui_lay *lay, int *items, const int *def, int cnt, int col_w, int row_gap, int col_gap, const int *con, struct gui_lay_sol *sol);
};
struct gui_panel_api {
  void (*hot)(struct gui_ctx *ctx, struct gui_panel *p, struct gui_panel *parent);
  void (*focus)(struct gui_ctx *ctx, struct gui_panel *p);
  void (*input)(struct gui_input *in, struct gui_ctx *ctx, const struct gui_panel *p, unsigned mask);
  enum gui_state (*state)(const struct gui_ctx *ctx, const struct gui_panel *p);
  void (*drw)(struct gui_ctx *ctx, const struct gui_box *b);
  void (*drw_focus)(struct gui_ctx *ctx, const struct gui_box *b, int pad);
  void (*begin)(struct gui_ctx *ctx, struct gui_panel *pan, struct gui_panel *parent);
  void (*end)(struct gui_ctx *ctx, struct gui_panel *pan, struct gui_panel *p);
  void (*open)(struct gui_ctx *ctx, struct gui_panel *pan, struct gui_panel *parent, unsigned long long id);
  void (*close)(struct gui_ctx *ctx, struct gui_panel *pan, struct gui_panel *p);
};
struct gui_cfg_api {
  void (*pushi)(struct gui_cfg_stk *stk, void *ptr, int val);
  void (*pushi_on)(struct gui_cfg_stk *stk, void *ptr, int val, int cond);
  void (*pushu)(struct gui_cfg_stk *stk, void *ptr, unsigned val);
  void (*pushu_on)(struct gui_cfg_stk *stk, void *ptr, unsigned val, int cond);
  void (*pop)(const struct gui_cfg_stk *stk);
  void (*pop_on)(const struct gui_cfg_stk *stk, int cond);
};
struct gui_input_api {
  void (*consume)(struct gui_ctx *ctx);
};
struct gui_txt_api {
  int (*width)(struct gui_ctx *ctx, struct str txt);
  void (*uln)(struct gui_ctx *ctx, struct gui_panel *pan, struct gui_panel *parent, struct str txt, const struct gui_align *align, int uln_pos, int uln_cnt);
  void (*lbl)(struct gui_ctx *ctx, struct gui_panel *pan, struct gui_panel *parent, struct str txt, const struct gui_align *align);
  void (*fmtv)(struct gui_ctx *ctx, struct gui_panel *pan, struct gui_panel *parent, const struct gui_align *align, const char *fmt, va_list args);
  void (*txtf)(struct gui_ctx *ctx, struct gui_panel *pan, struct gui_panel *parent, const struct gui_align *align, const char *fmt, ...);
  void (*tm)(struct gui_ctx *ctx, struct gui_panel *pan, struct gui_panel *parent, const char *fmt, struct tm *tm);
};
struct gui_lbl_api {
  void (*txt)(struct gui_ctx *ctx, struct gui_panel *pan, struct gui_panel *parent, struct gui_box_cut *cut, struct str txt);
  void (*fmtv)(struct gui_ctx *ctx, struct gui_panel *pan, struct gui_panel *parent, struct gui_box_cut *cut, const char *fmt, va_list args);
  void (*fmt)(struct gui_ctx *ctx, struct gui_panel *pan, struct gui_panel *parent, struct gui_box_cut *cut, const char *fmt, ...);
};
struct gui_ico_api {
  void (*icon)(struct gui_ctx *ctx, struct gui_icon *icn, struct gui_panel *parent, const char *icon);
  void (*box)(struct gui_ctx *ctx, struct gui_panel *pan, struct gui_panel *parent, const char *icon, struct str txt);
};
struct gui_btn_api {
  void (*begin)(struct gui_ctx *ctx, struct gui_btn *btn, struct gui_panel *parent);
  void (*end)(struct gui_ctx *ctx, struct gui_btn *btn, struct gui_panel *parent);
  int (*txt)(struct gui_ctx *ctx, struct gui_btn *btn, struct gui_panel *parent, struct str s, const struct gui_align *align);
  int (*lbl)(struct gui_ctx *ctx, struct gui_btn *btn, struct gui_panel *parent, struct gui_box_cut *cut, struct str txt, const struct gui_align *align);
  int (*ico)(struct gui_ctx *ctx, struct gui_btn *btn, struct gui_panel *parent, const char *icon);
  int (*ico_txt)(struct gui_ctx *ctx, struct gui_btn *btn, struct gui_panel *parent, struct str txt, const char *icon, int uline);
};
struct gui_chk_api {
  enum gui_chk_state (*ico)(struct gui_ctx *ctx, struct gui_panel *pan, struct gui_panel *parent, enum gui_chk_state chkd);
  int (*box)(struct gui_ctx *ctx, struct gui_panel *pan, struct gui_panel *parent, struct gui_box_cut *cut, struct str txt, enum gui_chk_state *chkd);
  int (*boxi)(struct gui_ctx *ctx, struct gui_panel *pan, struct gui_panel *parent, struct gui_box_cut *cut, struct str txt, int *chkd);
};
struct gui_tog_api {
  int (*ico)(struct gui_ctx *ctx, struct gui_panel *pan, struct gui_panel *parent, int *act);
  int (*box)(struct gui_ctx *ctx, struct gui_panel *pan, struct gui_panel *parent, struct gui_box_cut *cut, struct str txt, int *is_act);
};
struct gui_scrl_api {
  void (*map)(struct gui_ctx *ctx, struct gui_scrl *s, struct gui_panel *parent);
  void (*barh)(struct gui_ctx *ctx, struct gui_scrl_bar *s, struct gui_panel *parent);
  void (*barv)(struct gui_ctx *ctx, struct gui_scrl_bar *s, struct gui_panel *parent);
};
struct gui_edt_buf {
  void (*init)(struct gui_txt_ed *edt);
  int (*has_sel)(struct gui_txt_ed *edt);
  void (*reset)(struct gui_txt_ed *edt);
  void (*undo)(struct gui_txt_ed *edt, struct sys *sys, char **buf);
  void (*redo)(struct gui_txt_ed *edt, struct sys *sys, char **buf);
  int (*cut)(struct gui_txt_ed *edt, char **buf);
  int (*paste)(struct gui_txt_ed *edt, struct sys *sys, char **buf, struct str txt);
  void (*sel_all)(struct gui_txt_ed *edt, char *buf);
};
struct gui_edt_api {
  struct gui_edt_buf buf;
  void (*drw)(struct gui_ctx *ctx, const struct gui_panel *pan);
  void (*fld)(struct gui_ctx *ctx, struct gui_edit_box *box, struct gui_panel *pan, struct gui_panel *parent, struct gui_txt_ed *edt, char **buf);
  void (*txt)(struct gui_ctx *ctx, struct gui_edit_box *edt, struct gui_panel *parent, struct gui_txt_ed *ed, char **buf);
  void (*box)(struct gui_ctx *ctx, struct gui_edit_box *box, struct gui_panel *parent, char **buf);
};
struct gui_spin_api {
  int (*val)(struct gui_ctx *ctx, struct gui_spin *s, struct gui_panel *parent, struct gui_spin_val *spin);
  int (*flt)(struct gui_ctx *ctx, struct gui_spin *ctl, struct gui_panel *parent, float *n, float min, float max, float inc);
  int (*f)(struct gui_ctx *ctx, struct gui_spin *ctl, struct gui_panel *parent, float *n);
  int (*num)(struct gui_ctx *ctx, struct gui_spin *ctl, struct gui_panel *parent, int *n, int min, int max, int inc);
  int (*i)(struct gui_ctx *ctx, struct gui_spin *ctl, struct gui_panel *parent, int *n);
};
struct gui_grp_api {
  void (*begin)(struct gui_ctx *ctx, struct gui_grp *grp, struct gui_panel *parent, struct str txt);
  void (*end)(struct gui_ctx *ctx, struct gui_grp *grp, struct gui_panel *parent);
};
struct gui_reg_api {
  void (*begin)(struct gui_ctx *ctx, struct gui_reg *d, struct gui_panel *parent, const double *off);
  void (*apply_lst)(struct gui_reg *d, const struct gui_lst *lst, int row_mult);
  void (*end)(struct gui_ctx *ctx, struct gui_reg *d, struct gui_panel *parent, double *off);
};
struct gui_lst_lay_api {
  void (*init)(struct gui_lst_lay *lst);
  void (*gen)(struct gui_box *box, struct gui_lst_lay *lst);
  void (*apply_view)(struct gui_lst_lay *lst, const struct gui_lst_view *v);
  double (*center)(struct gui_lst_lay *lay, int idx);
  double (*fit_start)(struct gui_lst_lay *lay, int idx);
  double (*fit_end)(struct gui_lst_lay *lay, int idx);
  double (*clamps)(struct gui_lst_lay *lay, int idx);
};
struct gui_lst_ctl_api {
  void (*elm)(struct gui_ctx *ctx, struct gui_lst_ctl *ctl, const struct gui_lst_lay *lst, const struct gui_box *box, struct gui_panel *parent, unsigned long long id);
  void (*proc)(struct gui_ctx *ctx, struct gui_lst_ctl *ctl, const struct gui_lst_lay *lay, int total);
};
struct gui_lst_sel_api {
  void (*elm)(struct gui_ctx *ctx, struct gui_lst_sel *sel, const struct gui_lst_lay *lay, const struct gui_lst_ctl *ctl, struct gui_box *box, struct gui_panel *parent, int is_sel, unsigned long long id);
  void (*proc)(struct gui_ctx *ctx, struct gui_lst_sel *sel, const struct gui_lst_ctl *ctl, int total);
};
struct gui_lst_elm_api {
  void (*begin)(struct gui_ctx *ctx, struct gui_lst *lst, struct gui_panel *pan, struct gui_panel *p, unsigned long long id, int sel);
  void (*end)(struct gui_ctx *ctx, struct gui_lst *lst, struct gui_panel *pan, struct gui_panel *parent);
};
struct gui_lst_reg_elm_api {
  void (*begin)(struct gui_ctx *ctx, struct gui_lst_reg *la, struct gui_panel *pan, unsigned long long id, int sel);
  void (*end)(struct gui_ctx *ctx, struct gui_lst_reg *la, struct gui_panel *pan);
  void (*txt)(struct gui_ctx *ctx, struct gui_lst_reg *reg, struct gui_panel *elm, unsigned long long id, int is_sel, struct str txt, const char *icon, const struct gui_align *align);
};
struct gui_lst_reg_api {
  void (*begin)(struct gui_ctx *ctx, struct gui_lst_reg *la, struct gui_panel *parent, const struct gui_lst_cfg *cfg, const double *off);
  int (*nxt)(const struct gui_lst_reg *la, int idx);
  struct gui_lst_reg_elm_api elm;
  void (*center)(struct gui_lst_reg *la, int idx);
  void (*fit_start)(struct gui_lst_reg *la, int idx);
  void (*fit_end)(struct gui_lst_reg *la, int idx);
  void (*clamp)(struct gui_lst_reg *la, int idx);
  void (*end)(struct gui_ctx *ctx, struct gui_lst_reg *la, struct gui_panel *parent, double *off);
};
struct gui_lst_api {
  struct gui_lst_lay_api lay;
  struct gui_lst_ctl_api ctl;
  struct gui_lst_sel_api sel;
  struct gui_lst_elm_api elm;
  struct gui_lst_reg_api reg;

  void (*view)(struct gui_lst_view *v, const struct gui_lst_lay *lay);
  void (*cfg)(struct gui_lst_cfg *cfg, int total_cnt, double off);
  void (*begin)(struct gui_ctx *ctx, struct gui_lst *lst, const struct gui_lst_cfg *cfg);
  void (*begin_def)(struct gui_ctx *ctx, struct gui_lst *lst, int total_cnt, double off);
  int (*nxt)(const struct gui_lst *lst, int idx);
  void (*end)(struct gui_ctx *ctx, struct gui_lst *lst);
  void (*set_sel_idx)(struct gui_ctx *ctx, struct gui_lst *lst, int idx);
  void (*set_cur_idx)(struct gui_ctx *ctx, struct gui_lst *lst, int idx);
};
struct gui_tree_node_api {
  void (*begin)(struct gui_ctx *ctx, struct gui_tree_node *node, struct gui_panel *parent, int depth);
  void (*end)(struct gui_ctx *ctx, struct gui_tree_node *node, struct gui_panel *parent);
  void (*node)(struct gui_ctx *ctx, struct gui_tree_node *node, struct gui_panel *parent, int depth, struct str txt);
};
struct gui_split_lay_api {
  void (*begin)(struct gui_split_lay *bld, int *state, int cnt, int sep_size);
  void (*add)(struct gui_split_lay *bld, int type, int val, const int *con);
  void (*end)(struct gui_split_lay *bld);
  void (*bld)(int *state, const struct gui_ctx *ctx, const struct gui_split_lay_cfg *cfg);
};
struct gui_split_api {
  struct gui_split_lay_api lay;
  void (*begin)(struct gui_ctx *ctx, struct gui_split *spt, struct gui_panel *parent, enum gui_split_type typ, enum gui_orient orient, int *res, int *state);
  void (*sep)(struct gui_ctx *ctx, struct gui_split *spt, const int *lay, int *split_state);
  void (*end)(struct gui_ctx *ctx, struct gui_split *spt, struct gui_panel *parent);
};
struct gui_tbl_hdr_slot_api {
  void (*begin)(struct gui_ctx *ctx, struct gui_tbl *tbl, struct gui_btn *slot);
  void (*end)(struct gui_ctx *ctx, struct gui_tbl *tbl, const int *lay, struct gui_btn *slot, int *state);
  void (*txt)(struct gui_ctx *ctx, struct gui_tbl *tbl, const int *lay, int *state, struct str txt);
};
struct gui_tbl_hdr_api {
  struct gui_tbl_hdr_slot_api slot;
  void (*begin)(struct gui_ctx *ctx, struct gui_tbl *tbl, int *res, int *s);
  void (*end)(struct gui_ctx *ctx, struct gui_tbl *tbl);
};
struct gui_tbl_lst_elm_col_api {
  void (*slot)(struct gui_box *box, struct gui_ctx *ctx, struct gui_tbl *tbl, const int *lay);
  void (*txt)(struct gui_ctx *ctx, struct gui_tbl *tbl, const int *lay, struct gui_panel *elm, struct str txt, const char *icon, const struct gui_align *align);
  void (*txtf)(struct gui_ctx *ctx, struct gui_tbl *tbl, const int *lay, struct gui_panel *elm, const struct gui_align *align, const char *fmt, ...);
  void (*tm)(struct gui_ctx *ctx, struct gui_tbl *tbl, const int *lay, struct gui_panel *elm, const char *fmt, struct tm *tm);
};
struct gui_tbl_lst_elm_api {
  struct gui_tbl_lst_elm_col_api col;
  void (*begin)(struct gui_ctx *ctx, struct gui_tbl *tbl, struct gui_panel *elm, unsigned long long id, int sel);
  void (*end)(struct gui_ctx *ctx, struct gui_tbl *tbl, struct gui_panel *elm);
};
struct gui_tbl_lst_api {
  struct gui_tbl_lst_elm_api elm;
  void (*cfg)(struct gui_ctx *ctx, struct gui_tbl_lst_cfg *cfg, int total_cnt);
  void (*begin)(struct gui_ctx *ctx, struct gui_tbl *tbl, const struct gui_tbl_lst_cfg *cfg);
  void (*begin_def)(struct gui_ctx *ctx, struct gui_tbl *tbl, int total_cnt);
  void (*end)(struct gui_ctx *ctx, struct gui_tbl *tbl);
  int (*nxt)(const struct gui_lst *lst, int idx);
};
struct gui_tbl_api {
  struct gui_tbl_hdr_api hdr;
  struct gui_tbl_lst_api lst;
  void (*begin)(struct gui_ctx *ctx, struct gui_tbl *tbl, struct gui_panel *parent, const double *off, const struct gui_tbl_sort *sort);
  void (*end)(struct gui_ctx *ctx, struct gui_tbl *tbl, struct gui_panel *parent, double *off);
  void (*lay)(int *state, const struct gui_ctx *ctx, const struct gui_split_lay_cfg *cfg);
};
struct gui_tab_ctl_hdr_slot_api {
  void (*begin)(struct gui_ctx *ctx, struct gui_tab_ctl *tab, struct gui_tab_ctl_hdr *hdr, struct gui_panel *slot, unsigned long long id);
  void (*end)(struct gui_ctx *ctx, struct gui_tab_ctl *tab, struct gui_tab_ctl_hdr *hdr, struct gui_panel *slot, struct gui_input *in);
  void (*txt_id)(struct gui_ctx *ctx, struct gui_tab_ctl *tab, struct gui_tab_ctl_hdr *hdr, unsigned long long id, struct str txt);
  void (*txt)(struct gui_ctx *ctx, struct gui_tab_ctl *tab, struct gui_tab_ctl_hdr *hdr, struct str txt);
};
struct gui_tab_ctl_hdr_api {
  void (*begin)(struct gui_ctx *ctx, struct gui_tab_ctl *tab, struct gui_tab_ctl_hdr *hdr);
  void (*end)(struct gui_ctx *ctx, struct gui_tab_ctl *tab, struct gui_tab_ctl_hdr *hdr);
  struct gui_tab_ctl_hdr_slot_api slot;
};
struct gui_tab_ctl_api {
  struct gui_tab_ctl_hdr_api hdr;
  void (*begin)(struct gui_ctx *ctx, struct gui_tab_ctl *tab, struct gui_panel *parent, int cnt, int sel_idx);
  void (*sel)(struct gui_ctx *ctx, struct gui_tab_ctl *tab, int idx);
  void (*end)(struct gui_ctx *ctx, struct gui_tab_ctl *tab, struct gui_panel *parent);
};
struct gui_grid_api {
  void (*begin)(struct gui_ctx *ctx, struct gui_grid *g, struct gui_panel *parent, const double *off);
  void (*end)(struct gui_ctx *ctx, struct gui_grid *g, struct gui_panel *parent, double *off);
};
struct gui_graph_hdr_api {
  void (*begin)(struct gui_ctx *ctx, struct gui_graph_node *n, struct gui_graph_node_hdr *hdr);
  void (*end)(struct gui_ctx *ctx, struct gui_graph_node *n, struct gui_graph_node_hdr *hdr);
};
struct gui_graph_api {
  void (*begin)(struct gui_ctx *ctx, struct gui_graph_node *n, struct gui_panel *parent);
  void (*item)(struct gui_box *ret, struct gui_ctx *ctx, struct gui_graph_node *n, int item_h);
  void (*end)(struct gui_ctx *ctx, struct gui_graph_node *n, struct gui_panel *parent);
  struct gui_graph_hdr_api hdr;
};
struct gui_api {
  int version;
  void (*init)(struct gui_ctx *ctx, struct arena *mem, enum gui_col_scheme scm);
  void (*color_scheme)(struct gui_ctx *ctx, enum gui_col_scheme scm);
  void (*free)(struct gui_ctx *ctx);
  struct gui_panel*(*begin)(struct gui_ctx *ctx);
  void (*end)(struct gui_ctx *ctx);
  void (*scissor)(struct gui_ctx *ctx, int lhs, int top, int rhs, int bot);
  void (*clip_begin)(struct gui_box *tmp, struct gui_ctx *ctx, int lhs, int top, int rhs, int bot);
  void (*clip_end)(struct gui_ctx *ctx, struct gui_box *clip);
  void (*enable)(struct gui_ctx *ctx, int cond);
  void (*disable)(struct gui_ctx *ctx, int cond);

  struct gui_bnd_api bnd;
  struct gui_box_api box;
  struct gui_cut_api cut;
  struct gui_lay_api lay;
  struct gui_panel_api pan;
  struct gui_cfg_api cfg;
  struct gui_input_api in;
  struct gui_txt_api txt;
  struct gui_lbl_api lbl;
  struct gui_ico_api ico;
  struct gui_btn_api btn;
  struct gui_chk_api chk;
  struct gui_tog_api tog;
  struct gui_scrl_api scrl;
  struct gui_edt_api edt;
  struct gui_spin_api spin;
  struct gui_grp_api grp;
  struct gui_reg_api reg;
  struct gui_lst_api lst;
  struct gui_tree_node_api tree;
  struct gui_split_api splt;
  struct gui_tbl_api tbl;
  struct gui_tab_ctl_api tab;
  struct gui_grid_api grid;
  struct gui_graph_api node;
};
static void gui_get_api(void *export, void *import);

#endif

