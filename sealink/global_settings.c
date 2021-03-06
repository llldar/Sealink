//
//  global_settings.c
//  sealink
//
//  Created by 林理露 on 16/8/30.
//  Copyright © 2016年 林理露. All rights reserved.
//
#include <gtk/gtk.h>
#include <unistd.h>
#define in_setting 1
#include "global_settings.h"
#include "settings_window.h"
#include "logger.h"

user_info g_s;
GList *windows = NULL;
GList *group_windows = NULL;

void init_settings(const gchar* username){
    gchar path[dest_len] = {0};
    if (getcwd(path, sizeof(path)) != NULL)
    {
        logEvent(LOG_INFO, "Path set to: %s\n",path);
    }else{
        logEvent(LOG_ERROR, "Path setting encounter ERROR.\n");
        g_strlcpy(path, "", 0);
        logEvent(LOG_ERROR, "current path: %s.\n",path);
    }
    
    g_s.avatar_id = 1;
    g_s.main_path = g_malloc(dest_len*sizeof(gchar));
    g_strlcpy(g_s.main_path, path, sizeof(path));
    g_s.nick_name = g_malloc(dest_len*sizeof(gchar));
    g_strlcpy(g_s.nick_name, "北理第一提莫", sizeof("北理第一提莫"));
    g_s.notify_method = RECEIVE_AND_NOTIFY;
    
    logEvent(LOG_INFO, "Global setting inited.\n");
    logEvent(LOG_INFO, "avatar id: %d.\n",g_s.avatar_id);
    logEvent(LOG_INFO, "main path: %s.\n",g_s.main_path);
    logEvent(LOG_INFO, "notify_method: %d.\n",g_s.notify_method);
    
}
