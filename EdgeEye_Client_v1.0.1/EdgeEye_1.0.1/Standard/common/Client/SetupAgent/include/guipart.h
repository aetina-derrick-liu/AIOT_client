#ifndef __GUIPART_H__
#define __GUIPART_H__

#include <gtk/gtk.h>

#define    PI            3.141592
#define    CNNT_RADIUS   8

#if defined(_WIN32)
  #define    WIN_WIDTH     285
  #define    WIN_HEIGHT    260
  #define    BTN_SIZE_X    80
  #define    SWIN_SIZE_X   270
  #define    SWIN_SIZE_Y   100
  #define    CNNT_POS_X    220
  #define    CNNT_POS_Y    90
#else
  #define    WIN_WIDTH     290
  #define    WIN_HEIGHT    260
  #define    BTN_SIZE_X    90
  #define    SWIN_SIZE_X   285
  #define    SWIN_SIZE_Y   120
  #define    CNNT_POS_X    238
  #define    CNNT_POS_Y    103
#endif


#if defined(__linux__)
  #define   ICON_PATH_LINUX   "./icon/cloud.ico"
#endif

#if defined(_WIN32)
  #define   ICON_TRAY         "./icon/cloud.bmp"

  enum
  {
    TRAY_SHOW = 0,
    TRAY_EXIT,
    TRAY_ACTIVATE
  };

#endif



/*----------------------------*/
typedef struct
{
  GtkWidget  *hbox;
  GtkWidget  *entry;

} GATEWAY_INFO;



/*----------------------------*/
typedef struct
{
  GtkWidget  *hbox;
  GtkWidget  *entry;

} INTERVAL_INFO;



/*----------------------------*/
typedef struct
{
  GtkWidget  *hbox;
  GtkWidget  *btn;

} STATUS_INFO;


/*----------------------------*/
enum
{
  COOR_LONG = 0,
  COOR_LAT,
  COOR_ETY_NUM

};


typedef struct
{
  GtkWidget  *hbox;
  GtkWidget  *entry[COOR_ETY_NUM];

} COORDINATE_INFO;



/*----------------------------*/
typedef struct
{
  GtkWidget     *hbox;
  GtkWidget     *tree;
  GtkListStore  *store;

} LOG_INFO;


#endif