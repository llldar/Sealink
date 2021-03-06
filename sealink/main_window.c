#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "callbacks.h"
#include "resize_image.h"
#include "global_settings.h"
#include "logger.h"
#include "user_dao.h"
#include "group_dao.h"

#include "common.h"

extern uint32_t g_uid;
extern uint8_t* key;
extern int g_sockfd;


//对创建菜单，创建工具条函数进行声明
//这两个函数被前一个函数引用，外部没有用到，所以在interface.h中只声明了一个窗口创建函数
GtkWidget *create_headbar(GtkWidget *window);
GtkWidget *create_notebook(GtkWidget *window);
GtkWidget *create_friend_list(GtkWidget* window);
GtkWidget *create_bottombar(GtkWidget* window);
GtkWidget *create_group_list(GtkWidget* window);
//创建窗口函数
GtkWidget *create_main_window(void)
{
	//窗口
	GtkWidget *window;
	//主面板指针
    GtkWidget *mainBox;
    //声明顶部栏
    GtkWidget *headbar;
    GtkWidget *separator;
    GtkWidget *notebook;
    GtkWidget *bottombar;

	//创建窗口，窗口的类型为最上层的主窗口
	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window),"SeaLink v1.1.0");
    gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window),250,600);
    gtk_window_move(GTK_WINDOW(window),gdk_screen_width()-250-100,gdk_screen_height()-600-100);
    
    headbar = create_headbar(window);
    notebook = create_notebook(window);
    bottombar = create_bottombar(window);
    separator = gtk_hseparator_new();
    
    mainBox = gtk_vbox_new(FALSE, 0);
	//创建一个纵向的面板。参数1：表示是否均匀排放，参数2：容器中控件有无间隔
    gtk_box_pack_start(GTK_BOX(mainBox), headbar, 0, 0, 5);
    gtk_box_pack_start(GTK_BOX(mainBox), separator, 0, 0, 3);
    gtk_box_pack_start(GTK_BOX(mainBox), notebook, 0, 1, 5);
    gtk_box_pack_end(GTK_BOX(mainBox), bottombar, 0, 0, 5);
    
    gtk_container_set_border_width(GTK_CONTAINER(mainBox), 20);
	gtk_container_add(GTK_CONTAINER(window),mainBox);
    
	//显示所有的窗口
	gtk_widget_show_all(window);
    g_signal_connect(G_OBJECT(window),"delete_event",G_CALLBACK(on_main_window_delete),NULL);
	//返回窗口
	return window;
}

GtkWidget *create_headbar(GtkWidget *window){
    GtkWidget *image;
    GtkWidget *avatar_button;
    GtkWidget *topBox;
    GtkWidget *top_leftBox;
    GtkWidget *top_rightBox;
    GtkWidget *label;
    GtkWidget *combo_box;

    //头像按钮
    avatar_button = gtk_button_new();

    getAvatar(g_uid, &(g_s.avatar_id));
    
    //头像图片
    gchar avatar_path[dest_len]={0};
    sprintf(avatar_path,"%s/avatars/%d.jpg",g_s.main_path, g_s.avatar_id);
    image = gtk_image_new_from_file(avatar_path);
    logEvent(LOG_INFO, "Reading avatar file from path: %s\n",avatar_path);
    resize_image(image, 64, 64);
    
    //头像图片嵌入按钮
    gtk_container_add(GTK_CONTAINER(avatar_button), image);
   
    //按钮加入左上部盒子
    topBox = gtk_hbox_new(FALSE, 0);
    top_leftBox = gtk_vbox_new(TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(top_leftBox),avatar_button,0,0,20);
    g_signal_connect(G_OBJECT(avatar_button),"clicked",G_CALLBACK(on_avatar_clicked),NULL);
    
    //定义右上部分盒子
    top_rightBox = gtk_vbox_new(TRUE, 0);
    
    //昵称
    /* get name */
   logEvent(LOG_DEBUG, "getting name\n"); 
    PPACKET packet = protocol_BuildGetNamePacket(g_uid);
    if(packet) {
	    logEvent(LOG_DEBUG, "req sent\n");
	    int w = srv_WritePacket(g_sockfd, packet, sizeof(PACKET), key);
	    free(packet);
	    if( w >= 0 ) {
		    uint8_t* result = srv_ReadPacket(g_sockfd, MAX_PACKET, key);
		    if(result) {
			    packet = (PPACKET)result; 
			    logEvent(LOG_DEBUG, "My name : %s\n", packet->u.gnres.username);
			    strncpy(g_s.nick_name, packet->u.gnres.username, 31);
			    free(result);
		    }
	    }
    }
    label = gtk_label_new(g_s.nick_name);
    gtk_box_pack_start(GTK_BOX(top_rightBox), label, 0, 0, 5);
    
    //下拉选择框
    combo_box = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_box), "在线");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_box), "离开");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_box), "忙碌");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_box), "隐身");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_box), 0);
    
    gtk_box_pack_start(GTK_BOX(top_rightBox), combo_box, 0, 0, 5);
    
    //右上部分盒子加入上部盒子
    gtk_box_pack_start(GTK_BOX(topBox),top_leftBox, 0, 0, 5);
    gtk_box_pack_start(GTK_BOX(topBox),top_rightBox, 0, 0, 5);
    
    return topBox;
}

GtkWidget *create_notebook(GtkWidget *window){
    GtkWidget *notebook = gtk_notebook_new();

    GTK_NOTEBOOK(notebook)->homogeneous = TRUE;
    GTK_NOTEBOOK(notebook)->scrollable = TRUE;
    GTK_NOTEBOOK(notebook)->tab_hborder = 5;
    
    GtkWidget *friend_label = gtk_label_new("好友");
    GtkWidget *group_label = gtk_label_new("群组");
    
    GtkWidget *friend_list = create_friend_list(window);
    GtkWidget *group_list = create_group_list(window);
    
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),friend_list,friend_label);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),group_list,group_label);
    
    gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(notebook),friend_list , TRUE, TRUE, GTK_PACK_START);
    gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(notebook),group_list , TRUE, TRUE, GTK_PACK_START);

    return notebook;
}

GtkWidget *create_friend_list(GtkWidget* window){
    //GtkWidget *sw;
    GtkWidget *mainBox;
    GtkWidget *tempBox;
    GtkWidget *image;
    GtkWidget *label;
    GtkWidget *button;
    
    mainBox = gtk_vbutton_box_new();
    //sw = gtk_scrolled_window_new(NULL, NULL);
    
    //GTK_SCROLLED_WINDOW(sw)->hscrollbar_policy = GTK_POLICY_AUTOMATIC;
    //GTK_SCROLLED_WINDOW(sw)->vscrollbar_policy = GTK_POLICY_AUTOMATIC;
    //gtk_container_add(GTK_CONTAINER(sw), mainBox);
    
    GList *users = get_user_list();
    gchar path[dest_len];
    while (users != NULL) {
        tempBox = gtk_hbox_new(FALSE, 0);
        button = gtk_button_new();
        label = gtk_label_new(((simple_user_info*)(users->data))->uname);
        sprintf(path, "%s/avatars/%d.jpg",g_s.main_path,((simple_user_info*)(users->data))->avatar_id);
        image = gtk_image_new_from_file(path);
        resize_image(image, 48, 48);
        gtk_box_pack_start(GTK_BOX(tempBox), image, 0, 0, 5);
        gtk_box_pack_start(GTK_BOX(tempBox), label, 0, 0, 5);
        gtk_container_add(GTK_CONTAINER(button), tempBox);
        gtk_box_pack_start(GTK_BOX(mainBox), button, 0, 0, 5);
        g_signal_connect(G_OBJECT(button),"button_press_event",G_CALLBACK(on_friend_list_btn_clicked),((users->data)));
        g_signal_connect(G_OBJECT(button),
                         "button_release_event",
                         G_CALLBACK(on_friend_list_btn_clicked),
                         ((users->data)));
        users = g_list_next(users);
    }

    return mainBox;
}

GtkWidget *create_bottombar(GtkWidget* window){
    GtkWidget *settings_button,*calendar_button;
    GtkWidget *bbox;
    GtkWidget *frame;
    frame = gtk_frame_new("");
    bbox = gtk_hbutton_box_new();
    gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox),GTK_BUTTONBOX_SPREAD);
    gtk_container_set_border_width(GTK_CONTAINER(bbox),5);
    gtk_container_add(GTK_CONTAINER(frame),bbox);
    
    settings_button = gtk_button_new_with_label("设置");
    g_signal_connect(G_OBJECT(settings_button), "clicked", G_CALLBACK(on_settings_btn_clicked), NULL);
    calendar_button = gtk_button_new_with_label("日历");
    g_signal_connect(G_OBJECT(calendar_button), "clicked", G_CALLBACK(on_calendar_btn_clicked), window);
    gtk_box_pack_start(GTK_BOX(bbox), settings_button, 0, 0, 2);
    gtk_box_pack_start(GTK_BOX(bbox), calendar_button, 0, 0, 2);
    return frame;
}

GtkWidget *create_group_list(GtkWidget* window){
    GtkWidget *mainBox;
    GtkWidget *tempBox;
    GtkWidget *button;
    
    mainBox = gtk_vbutton_box_new();
    gtk_button_box_set_layout(GTK_BUTTON_BOX(mainBox),GTK_BUTTONBOX_SPREAD);
    
    GList *groups = get_group_list();
    gchar id_s[dest_len];
    while (groups != NULL) {
        tempBox = gtk_hbox_new(FALSE, 0);
        sprintf(id_s, "Group %d",((group_info*)(groups->data))->GID);
        button = gtk_button_new_with_label(id_s);
        gtk_box_pack_start(GTK_BOX(mainBox), button, 0, 0, 5);
        g_signal_connect(G_OBJECT(button),"button_press_event",G_CALLBACK(on_group_list_btn_clicked),((groups->data)));
        g_signal_connect(G_OBJECT(button),
                         "button_release_event",
                         G_CALLBACK(on_group_list_btn_clicked),
                         ((groups->data)));
        groups = g_list_next(groups);
    }
    
    return mainBox;
}
