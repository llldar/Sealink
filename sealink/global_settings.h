//
//  global_settings.h
//  sealink
//
//  Created by 林理露 on 16/8/30.
//  Copyright © 2016年 林理露. All rights reserved.
//

#ifndef global_settings_h
#define global_settings_h

void init_settings(const gchar* username);

enum{RECEIVE_AND_NOTIFY,RECEIVE_NOT_NOTIFY,BLOCK};

typedef struct{
    int UID;
    int avatar_id;
    gchar* nick_name;
    int notify_method;
    gchar* main_path;
}user_info;

typedef struct{
    int UID;
    GtkWidget *receive_text;
}chat_info;

//目录长度
#ifndef dest_len
#define dest_len 500
#endif
//全局设置
#ifndef in_setting
extern user_info g_s;
extern GList *windows;
extern GList *group_windows;
#endif

#endif /* global_settings_h */
