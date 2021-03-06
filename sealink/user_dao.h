//
//  user_dao.h
//  sealink
//
//  Created by 林理露 on 16/8/31.
//  Copyright © 2016年 林理露. All rights reserved.
//

#ifndef user_dao_h
#define user_dao_h

typedef struct{
    int UID;
    gchar uname[500];
    int avatar_id;
}simple_user_info;

int QueryUserName(int uid, char* buffer);
GList* get_user_list();

#endif /* user_dao_h */
