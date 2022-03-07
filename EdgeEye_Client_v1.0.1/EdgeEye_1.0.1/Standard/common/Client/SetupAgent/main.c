#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "guipart.h"
#include "version.h"
#include "icapapi.h"
#include "JsonParser.h"
#include "DBInterface.h"

/* global variables */
int             is_cnnt;

GtkWidget       *main_ev;
GtkWidget       *main_win;
GtkWidget       *menu;
GtkWidget       *menu_show;
GtkWidget       *menu_exit;
GtkStatusIcon   *tray_icon;

GATEWAY_INFO     gwInfo;
INTERVAL_INFO    itvlInfo;
STATUS_INFO      stsInfo;
COORDINATE_INFO  coorInfo;
LOG_INFO         logInfo;


/* string */
const char *title    = "Client Service Setting" iCAP_CLIENT_VERSION;
const char *label_g  = "Gateway Address:";
const char *label_i1 = "Upload Interval:";
const char *label_i2 = "[Sec.]";
const char *label_s  = "Service Status:";
const char *label_l  = "Log";
const char *label_c  = "Located Coordinate:";
const char *label_lo = "(Longitude)";
const char *label_la = "(Latitude)";
const char *cnnt_str = "Reconnect";

const char *rtnCnntMsg[] =
{
  "Reconnect successfully !",
  "Reconnect failed !"
};

#if defined(_WIN32)
  const char *tray_menu_str[2] = {"Show", "Exit"};
  const char *tray_icon_hint   = "";
#endif


/* function prototype */
static void gwInit   (void);
static void itvlInit (void);
static void stsInit  (void);
static void coorInit (void);
static void logInit  (void);
static void mainInit (void);

static void clearLog  (void);
static void insertLog (const char *msg);

gboolean ChkStatus(gpointer user_data);

/* gtk callback function */
static void destroyCB (GtkWidget *widget, gpointer user_data);
static void btnCnntCB (GtkWidget *widget, gpointer user_data);
static bool etyKeyCB  (GtkWidget *entry,  GdkEventKey *event, gpointer user_data);
static bool cnntDraw  (GtkWidget *widget, cairo_t *cr, gpointer user_data);

#if defined(_WIN32)
  static void tray_popup_menu (GtkStatusIcon *status_icon, guint button, guint32 activate_time, gpointer popUpMenu);
  static void tray_activate   (GtkStatusIcon *status_icon, gpointer user_data);
  static bool minimizeCB      (GtkWidget *widget, GdkEventWindowState *event, gpointer user_data);
#endif

#if defined(__linux__)
  static void draw_child (cairo_t *cr, GtkWidget *parent);
#endif

void LoadSettingFiles();


/*--------------------------------------------------------------------------------------*/
int main (int argc, char *argv[])
{
  gtk_init(&argc, &argv);

  gwInit();
  itvlInit();
  stsInit();
  coorInit();
  logInit();
  mainInit();
  LoadSettingFiles();

  g_timeout_add(500, ChkStatus, NULL);

  gtk_widget_show_all(main_win);
  gtk_main();
  return 0;

} /* END: main() */



/*--------------------------------------------------------------------------------------*/
static void gwInit (void)
{
  GtkWidget   *label;

  label = gtk_label_new (label_g);

  gwInfo.entry = gtk_entry_new();
  gwInfo.hbox  = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

  gtk_entry_set_alignment  (GTK_ENTRY(gwInfo.entry), 1.0);
  gtk_entry_set_width_chars(GTK_ENTRY(gwInfo.entry), 21);

  gtk_box_pack_start(GTK_BOX(gwInfo.hbox), label, false, false, 5);
  gtk_box_pack_start(GTK_BOX(gwInfo.hbox), gwInfo.entry, false, false, 5);

} /* END: gwInit() */



/*--------------------------------------------------------------------------------------*/
static void itvlInit (void)
{
  GtkWidget   *label1;
  GtkWidget   *label2;

  label1 = gtk_label_new (label_i1);
  label2 = gtk_label_new (label_i2);

  itvlInfo.entry = gtk_entry_new();
  itvlInfo.hbox  = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

  gtk_entry_set_alignment  (GTK_ENTRY(itvlInfo.entry), 1.0);
  gtk_entry_set_max_length (GTK_ENTRY(itvlInfo.entry), 5);
  gtk_entry_set_width_chars(GTK_ENTRY(itvlInfo.entry), 5 + 1);

  gtk_box_pack_start(GTK_BOX(itvlInfo.hbox), label1, false, false, 5);
  gtk_box_pack_start(GTK_BOX(itvlInfo.hbox), itvlInfo.entry, false, false, 17);
  gtk_box_pack_start(GTK_BOX(itvlInfo.hbox), label2, false, false, 0);

  g_signal_connect(G_OBJECT(itvlInfo.entry), "key-press-event", G_CALLBACK(etyKeyCB), NULL);


} /* END: itvlInit() */



/*--------------------------------------------------------------------------------------*/
static void stsInit (void)
{
  GtkWidget   *label;
  GtkWidget   *sep;

  //is_cnnt = false;

  label = gtk_label_new (label_s);
  sep   = gtk_separator_new(GTK_ORIENTATION_VERTICAL);

  stsInfo.hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  stsInfo.btn  = gtk_button_new_with_label(cnnt_str);
  
  gtk_button_set_focus_on_click(GTK_BUTTON(stsInfo.btn), false);
  gtk_widget_set_size_request(GTK_WIDGET(stsInfo.btn), BTN_SIZE_X, -1);

  gtk_box_pack_start (GTK_BOX(stsInfo.hbox), stsInfo.btn, false, false, 5);
  gtk_box_pack_start (GTK_BOX(stsInfo.hbox), sep,   false, false, 5);
  gtk_box_pack_start (GTK_BOX(stsInfo.hbox), label, false, false, 5);

  g_signal_connect(stsInfo.btn, "clicked", G_CALLBACK(btnCnntCB), NULL);


} /* END: stsInit() */



/*--------------------------------------------------------------------------------------*/
static void coorInit (void)
{
  int          i;

  GtkWidget   *vbox_air;
  GtkWidget   *vbox0;
  GtkWidget   *vbox1;
  GtkWidget   *vbox2;
  GtkWidget   *label;
  GtkWidget   *label1;
  GtkWidget   *label2;

  vbox_air = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  vbox0    = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  vbox1    = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  vbox2    = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  coorInfo.hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  coorInfo.entry[COOR_LONG] = gtk_entry_new();
  coorInfo.entry[COOR_LAT]  = gtk_entry_new();

  label  = gtk_label_new (label_c);
  label1 = gtk_label_new (label_lo);
  label2 = gtk_label_new (label_la);

  for(i=0; i<COOR_ETY_NUM; i++)
  {
    gtk_entry_set_alignment  (GTK_ENTRY(coorInfo.entry[i]), 1.0);
    gtk_entry_set_max_length (GTK_ENTRY(coorInfo.entry[i]), 8);
    gtk_entry_set_width_chars(GTK_ENTRY(coorInfo.entry[i]), 8);
  }

  gtk_box_pack_start (GTK_BOX(vbox0), label,    false, false, 0);
  gtk_box_pack_start (GTK_BOX(vbox0), vbox_air, false, false, 0);

  gtk_box_pack_start (GTK_BOX(vbox1), coorInfo.entry[COOR_LONG], false, false, 2);
  gtk_box_pack_start (GTK_BOX(vbox1), label1, false, false, 2);

  gtk_box_pack_start (GTK_BOX(vbox2), coorInfo.entry[COOR_LAT],  false, false, 2);
  gtk_box_pack_start (GTK_BOX(vbox2), label2, false, false, 2);

  gtk_box_pack_start (GTK_BOX(coorInfo.hbox), vbox0, false, false, 5);
  gtk_box_pack_start (GTK_BOX(coorInfo.hbox), vbox1, false, false, 5);
  gtk_box_pack_start (GTK_BOX(coorInfo.hbox), vbox2, false, false, 5);

  g_signal_connect(G_OBJECT(coorInfo.entry[COOR_LONG]), "key-press-event", G_CALLBACK(etyKeyCB), NULL);
  g_signal_connect(G_OBJECT(coorInfo.entry[COOR_LAT]),  "key-press-event", G_CALLBACK(etyKeyCB), NULL);


} /* END: coorInit() */




/*--------------------------------------------------------------------------------------*/
static void logInit (void)
{
  GtkWidget        *swin;
  GtkWidget        *vbox;
  GtkCellRenderer  *renderer;


  logInfo.hbox  = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  logInfo.tree  = gtk_tree_view_new();
  logInfo.store = NULL;

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  swin = gtk_scrolled_window_new (NULL, NULL);

  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(logInfo.tree), -1, label_l, renderer, "text", 0, NULL);
  gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(logInfo.tree), true);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(swin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_size_request(GTK_WIDGET(swin), SWIN_SIZE_X, SWIN_SIZE_Y);

  gtk_container_add(GTK_CONTAINER(swin), logInfo.tree);
  gtk_box_pack_start(GTK_BOX(vbox), swin, false, false, 0);
  gtk_container_add(GTK_CONTAINER(logInfo.hbox), vbox);

  g_signal_connect(logInfo.tree, "button-press-event", G_CALLBACK(gtk_true), NULL);


} /* END: logInit() */



/*--------------------------------------------------------------------------------------*/
static void mainInit (void)
{
  #if defined(__linux__) /* main icon */
    GdkPixbuf *pixbuf = NULL;
    pixbuf = gdk_pixbuf_new_from_file (ICON_PATH_LINUX, NULL);

    if(pixbuf)
    {
      gtk_window_set_default_icon(pixbuf);
      g_object_unref(pixbuf);
    }
    else
      printf("Get icon from %s failed !\n", ICON_PATH_LINUX);
  #endif

  GtkWidget   *main_vbox;

  /* setting */
  main_ev   = gtk_event_box_new();
  main_win  = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  main_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

  gtk_widget_set_size_request(GTK_WIDGET(main_win), WIN_WIDTH, WIN_HEIGHT);
  gtk_window_set_resizable   (GTK_WINDOW(main_win), false);
  gtk_window_set_title       (GTK_WINDOW(main_win), title);
  gtk_window_set_position    (GTK_WINDOW(main_win), GTK_WIN_POS_CENTER);

  gtk_container_set_border_width(GTK_CONTAINER(main_vbox), 5);

  /* pack */
  gtk_box_pack_start(GTK_BOX(main_vbox), gwInfo.hbox,   false, false, 5);
  gtk_box_pack_start(GTK_BOX(main_vbox), itvlInfo.hbox, false, false, 5);
  gtk_box_pack_start(GTK_BOX(main_vbox), stsInfo.hbox,  false, false, 5);
  gtk_box_pack_start(GTK_BOX(main_vbox), coorInfo.hbox, false, false, 5);
  gtk_box_pack_start(GTK_BOX(main_vbox), logInfo.hbox,  false, false, 0);
  gtk_container_add (GTK_CONTAINER(main_ev),  main_vbox);
  gtk_container_add (GTK_CONTAINER(main_win), main_ev);

  /* signal */
  g_signal_connect(main_win, "destroy", G_CALLBACK(destroyCB), NULL);
  g_signal_connect(main_ev,  "draw",    G_CALLBACK(cnntDraw),  NULL);


  #if defined(_WIN32) /* tray icon */
    tray_icon = gtk_status_icon_new_from_file(ICON_TRAY);
    menu      = gtk_menu_new();
    menu_show = gtk_menu_item_new_with_label(tray_menu_str[0]);
    menu_exit = gtk_menu_item_new_with_label(tray_menu_str[1]);

    gtk_status_icon_set_tooltip_text(tray_icon, tray_icon_hint);
    gtk_status_icon_set_visible(tray_icon, false);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_show);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_exit); 

    g_signal_connect(main_win,  "window-state-event", G_CALLBACK(minimizeCB),     NULL);
    g_signal_connect(menu_show, "activate",           G_CALLBACK(tray_activate),  (gpointer) TRAY_SHOW);
    g_signal_connect(menu_exit, "activate",           G_CALLBACK(tray_activate),  (gpointer) TRAY_EXIT);
    g_signal_connect(tray_icon, "activate",           G_CALLBACK(tray_activate),  (gpointer) TRAY_ACTIVATE);
    g_signal_connect(tray_icon, "popup-menu",         G_CALLBACK(tray_popup_menu), menu);
  #endif

} /* END: mainInit() */



/*--------------------------------------------------------------------------------------*/
static void clearLog (void)
{
  if(logInfo.store)
    gtk_list_store_clear(logInfo.store);

  logInfo.store = NULL;

} /* END: clearLog() */


/*--------------------------------------------------------------------------------------*/
static void insertLog (const char *msg)
{
  GtkTreeIter  iter;

  if(logInfo.store == NULL)
  {
    logInfo.store = gtk_list_store_new(1, G_TYPE_STRING);
    gtk_tree_view_set_model (GTK_TREE_VIEW(logInfo.tree), GTK_TREE_MODEL(logInfo.store));
  }

  gtk_list_store_append(logInfo.store, &(iter));
  gtk_list_store_set   (logInfo.store, &(iter), 0, msg, -1);

} /* END: insertLog() */



/*--------------------------------------------------------------------------------------*/
static void destroyCB (GtkWidget *widget, gpointer user_data)
{
  /* Do something release ! */

  gtk_main_quit();

} /* END: destroyCB() */



/*--------------------------------------------------------------------------------------*/
static void btnCnntCB (GtkWidget *widget, gpointer user_data)
{
  int           rtn;
  const char   *ip;
  const char   *itvl;
  const char   *lo;
  const char   *la;
  API_INFO      info;


  ip   = gtk_entry_get_text(GTK_ENTRY(gwInfo.entry));
  itvl = gtk_entry_get_text(GTK_ENTRY(itvlInfo.entry));
  lo   = gtk_entry_get_text(GTK_ENTRY(coorInfo.entry[COOR_LONG]));
  la   = gtk_entry_get_text(GTK_ENTRY(coorInfo.entry[COOR_LAT]));

  info.ipAddr = ip;
  info.itvl   = (double) atof(itvl);
  info.lo     = (double) atof(lo);
  info.la     = (double) atof(la);

  rtn = iCAPReCnnt(info);

  /*if(rtn == 0)
  {
    is_cnnt = true;
  }
  else
  {
    is_cnnt = false;
  }*/

  clearLog();
  insertLog(rtnCnntMsg[rtn]);


  gtk_widget_queue_draw(main_ev);

} /* END: btnCnntCB() */




/*--------------------------------------------------------------------------------------*/
static bool etyKeyCB (GtkWidget *entry, GdkEventKey *event, gpointer user_data)
{
  bool rtn = true;

  switch(event->keyval)
  {
    case GDK_KEY_KP_0...GDK_KEY_KP_9:    
    case GDK_KEY_0...GDK_KEY_9:
    case GDK_KEY_KP_Decimal:
    case GDK_KEY_BackSpace:
    case GDK_KEY_period:
    case GDK_KEY_Delete:
    case GDK_KEY_Right:
    case GDK_KEY_Left:
    case GDK_KEY_Tab:

              rtn = false;
              break;
    default:
              rtn = true;
              break;
  }


  return rtn;

} /* END: etyKeyCB() */



/*--------------------------------------------------------------------------------------*/
static bool cnntDraw (GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
  switch(is_cnnt)
  {
    case 0:
      cairo_set_source_rgba(cr, 0, 1, 0, 1);
      break;
    case 4:
      cairo_set_source_rgba(cr, 1, 0, 0, 1);
      break;
    default:
      cairo_set_source_rgba(cr, 1, 1, 0, 1);
      break;
  }
 
  cairo_line_to(cr, CNNT_POS_X, CNNT_POS_Y);
  cairo_arc    (cr, CNNT_POS_X, CNNT_POS_Y, CNNT_RADIUS, 0, 2*PI);
  cairo_line_to(cr, CNNT_POS_X, CNNT_POS_Y);
  cairo_fill   (cr);
  cairo_stroke (cr);

  #if defined(__linux__)
    draw_child(cr, widget);
    return true;
  #else
    return false;
  #endif

} /* END: cnntDraw() */

void LoadSettingFiles()
{
  char c;
  int i = 0;
  FILE* fp;
  struct json_object  *jobj,
            *jobj_addr,
            *jobj_longitude,
            *jobj_latitude,
            *jobj_Interval;
  char *readSetting;

  fp = fopen(SETTING_FILE_PATH, "r");

  readSetting = (char*)malloc(sizeof(char) * MAX_SETTING_STR_SIZE);

  if(fp)
  {
    while((c = fgetc(fp)) != EOF)
    {
      *(readSetting+(i++)) = c;
    }
    fclose(fp);

    jobj = json_tokener_parse(readSetting);


    jobj_addr = get_json_object(jobj, "ServerAddr");

    jobj_longitude = get_json_object(jobj, "Long");

    jobj_latitude = get_json_object(jobj, "Lati");

    jobj_Interval = get_json_object(jobj, "Interval");

    gtk_entry_set_text(GTK_ENTRY(gwInfo.entry), json_object_get_string(jobj_addr));

    gtk_entry_set_text(GTK_ENTRY(coorInfo.entry[COOR_LONG]), json_object_get_string(jobj_longitude));

    gtk_entry_set_text(GTK_ENTRY(coorInfo.entry[COOR_LAT]), json_object_get_string(jobj_latitude));

    gtk_entry_set_text(GTK_ENTRY(itvlInfo.entry), json_object_get_string(jobj_Interval));

    json_object_put(jobj_addr);
    json_object_put(jobj_longitude);
    json_object_put(jobj_latitude);
    json_object_put(jobj_Interval);
    json_object_put(jobj);

  }

    
}


/*--------------------------------------------------------------------------------------*/
#if defined(_WIN32)
  static void tray_popup_menu (GtkStatusIcon *status_icon, guint button, guint32 activate_time, gpointer popUpMenu)
  {
    gtk_widget_show_all(menu);
    gtk_menu_popup(GTK_MENU(popUpMenu), NULL, NULL, NULL, status_icon, button, activate_time);

  } /* END: tray_popup_menu() */
#endif


/*--------------------------------------------------------------------------------------*/
#if defined(_WIN32)
  static void tray_activate (GtkStatusIcon *status_icon, gpointer user_data)
  {
      int idx = (int) user_data;
      switch(idx)
      {
         case TRAY_ACTIVATE:
                             gtk_status_icon_set_visible(tray_icon, false);
                             gtk_window_deiconify(GTK_WINDOW(main_win));
                             gtk_widget_show_all(gtk_widget_get_toplevel(main_win));
                             break;
        case TRAY_SHOW:
                             gtk_status_icon_set_visible(tray_icon, false);
                             gtk_window_deiconify(GTK_WINDOW(main_win));
                             gtk_widget_show_all(gtk_widget_get_toplevel(main_win));
                             break;
        case TRAY_EXIT:
                             gtk_status_icon_set_visible(tray_icon, false);
                             gtk_main_quit();
                             break;
       }

  } /* END: tray_activate() */
#endif



/*--------------------------------------------------------------------------------------*/
#if defined(_WIN32)
  bool minimizeCB (GtkWidget *widget, GdkEventWindowState *event, gpointer user_data)
  {
    if(event->new_window_state & GDK_WINDOW_STATE_ICONIFIED)
    {
      gtk_status_icon_set_visible (tray_icon, true);
      gtk_window_iconify (GTK_WINDOW(gtk_widget_get_toplevel(main_win)));
      gtk_widget_hide (gtk_widget_get_toplevel(main_win));
    }

    return true;

  } /* END: minimizeCB() */
#endif



/*--------------------------------------------------------------------------------------*/
#if defined(__linux__)
  static void draw_child (cairo_t *cr, GtkWidget *parent)
  {
    int        i, cnt;
    GtkWidget *child;
    GList     *list;

    if(GTK_IS_CONTAINER(parent))
    {
      list = gtk_container_get_children(GTK_CONTAINER(parent));
      cnt  = g_list_length(list);

      for(i=0; i<cnt; i++)
      {
        child = GTK_WIDGET(list->data);
        gtk_container_propagate_draw(GTK_CONTAINER(parent), child, cr);
        list = list->next;
      }
    }

  } /* END: draw_child() */
#endif

gboolean ChkStatus(gpointer user_data)
{
  is_cnnt = ReadStatus();
  gtk_widget_queue_draw(main_ev);
  return TRUE;
}