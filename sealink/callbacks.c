#include <gtk/gtk.h>
#include "chat_window.h"
#include "main_window.h"
#include "login_window.h"
#include "register_window.h"
#include "settings_window.h"
#include "global_settings.h"
#include "calendar_window.h"
#include "capture_image.h"
#include "emoji_window.h"
#include "logger.h"
#include "user_dao.h"
#include "avatar_select_window.h"
#include "change_skin.h"

#include "common.h"

extern int g_sockfd;
extern uint8_t* key;
pthread_t g_Receiver;
uint32_t g_uid;

void appendText(GtkWidget* text_view, gchar* text) {
	GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(buffer, &start, &end);
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffer), &end, text, strlen(text));
}

/****   登录窗口    ****/
//登录按钮
void on_login_btn_clicked(GtkWidget *button,gpointer *my_login_info)
{
    const gchar *username = gtk_entry_get_text(GTK_ENTRY(((user_login_info*)my_login_info)->username_entry));
    const gchar *password = gtk_entry_get_text(GTK_ENTRY(((user_login_info*)my_login_info)->password_entry));
    logEvent(LOG_INFO, "login btn result:\n");
    logEvent(LOG_INFO, "Your username is: %s\n",username);
    logEvent(LOG_INFO, "Your password is: %s\n",password);

    int login = -1;

    uint32_t uid = atoi(username);
    PPACKET packet = protocol_BuildLoginPacket(uid, password);
    if(!packet) goto Check;
    
    int retry = 10;
    int clisock;
    uint8_t* result;
    while(retry) {
	    packet->u.logreq.port = (uint16_t)getRandom32();
	    clisock = srv_TCPListenConnectBack(packet->u.logreq.port);
	    if(clisock < 0) {
		    retry--;
		    continue;
	    }
	    break;
    }
    if(clisock >= 0) {
    	pthread_create(&g_Receiver, NULL, client_msg_handler, (void*)clisock);
	login = srv_WritePacket(g_sockfd, packet, sizeof(PACKET), key);
	free(packet);
	if(login >= 0) {
		result = srv_ReadPacket(g_sockfd, MAX_PACKET, key);
		if( result != NULL ) {
			uint32_t logsess;
			login = protocol_ParseLogResultPacket(result, &logsess);
		} else {
			login = -1;
		}
	}
    }

Check:
    //TODO: 进行密码验证
    if (login >= 0) {
        logEvent(LOG_INFO, "Login SUCESS.\n");
	g_uid = uid;
        init_settings(username);
        create_main_window();
        gtk_widget_destroy(((user_login_info*)my_login_info)->window);
        g_free(my_login_info);
        logEvent(LOG_DEBUG, "pointer [my_login_info] FREED, size: %d\n",sizeof(user_login_info));
    }else{
        logEvent(LOG_INFO, "login FAILED.\n");
    }
    
}
//注册按钮
void on_register_btn_clicked(){
    create_register_window();
}

gboolean on_login_window_delete(GtkWidget*widget,GdkEvent *event,gpointer *my_login_info)
{
    g_free(my_login_info);
    logEvent(LOG_DEBUG, "pointer [my_login_info] FREED, size: %d\n",3*sizeof(GtkWidget*));
    gtk_main_quit();
    return FALSE;
}

/****   注册界面   ****/
//注册按钮
void on_reg_confirm_btn_clicked(GtkWidget*button,gpointer *my_reg_info){
    const gchar* username = gtk_entry_get_text(GTK_ENTRY(((user_reg_info*)my_reg_info)->username_entry));
    const gchar* password1 = gtk_entry_get_text(GTK_ENTRY(((user_reg_info*)my_reg_info)->password_entry));
    const gchar* password2 = gtk_entry_get_text(GTK_ENTRY(((user_reg_info*)my_reg_info)->password_confirm_entry));
    
    logEvent(LOG_INFO, "register confirm button clicked.\n");
    logEvent(LOG_INFO, "register btn result:\n");
    logEvent(LOG_INFO, "Your username is: %s\n",username);
    logEvent(LOG_INFO, "Your password is: %s\n",password1);
    logEvent(LOG_INFO, "Your password is: %s\n",password2);

    if(g_strcmp0(password1, password2) != 0) {
	    logEvent(LOG_ERROR, "password1 != password2\n");
	    return;
    }

    int reg = -1;
    PPACKET packet = protocol_BuildRegisterPacket(username, password1);
    if(!packet) goto Check;
    reg = srv_WritePacket(g_sockfd, packet, sizeof(PACKET), key);
    free(packet);
    if( reg >= 0 ) {
	    uint8_t* result = srv_ReadPacket(g_sockfd, MAX_PACKET, key);
	    if( result ) {
		    uint32_t uid;
		    reg = protocol_ParseRegResultPacket(result, &uid);
		    free(result);
	    } else {
		    reg = -1;
	    }
    }
    
Check:
    if (reg >= 0) {
        logEvent(LOG_INFO, "Reg SUCESS.\n");
        gtk_widget_destroy(((user_reg_info*)my_reg_info)->window);
    }else{
        logEvent(LOG_INFO, "Reg FAILED.\n");
    }
}

void on_reg_cancel_btn_clicked(GtkWidget*button,gpointer *myinfo){
    gtk_widget_destroy(((user_reg_info*)myinfo)->window);
    g_free(myinfo);
    logEvent(LOG_DEBUG, "pointer [my_reg_info] FREED, size: %d\n",sizeof(myinfo));
}

gboolean on_reg_window_delete(GtkWidget*widget,GdkEvent *event,gpointer *my_reg_info){
    g_free(my_reg_info);
    logEvent(LOG_DEBUG, "pointer [my_reg_info] FREED, size: %d\n",sizeof(my_reg_info));
    return FALSE;
}

/****   主界面   ****/
gboolean on_main_window_delete(GtkWidget*widget,GdkEvent *event,gpointer data)
{
    gtk_main_quit();
    return FALSE;
}

//主界面 HeadBar
void on_avatar_clicked()
{
    create_settings_window();
}

void on_status_changed(gchar* status)
{
    logEvent(LOG_INFO, "status changed.\n");
}

//主界面 NavBar
void on_dialog_list_btn_clicked(GtkWidget* button)
{
    logEvent(LOG_INFO, "dialog list btn clicked.\n");
}

void on_friend_list_btn_clicked(GtkWidget *widget, GdkEventButton *event, gpointer *u_info)
{
    if (GTK_IS_BUTTON(widget) && (event->type==GDK_2BUTTON_PRESS) ) {
        create_chat_window((simple_user_info*)u_info);
    }
}

void on_group_list_btn_clicked(GtkWidget *widget, GdkEventButton *event, gpointer *g_info){
    if (GTK_IS_BUTTON(widget) && (event->type==GDK_2BUTTON_PRESS) ) {
        create_group_chat_window((group_info*)g_info);
    }
}

//主界面 List
void on_dialog_clicked(gchar* target_name)
{
    
}

void on_friend_clicked(gchar* friend_name)
{
    
}

void on_group_clicked(gchar* group_name)
{
    
}

//主界面 BottomBar
void on_settings_btn_clicked()
{
    create_settings_window();
}

void on_calendar_btn_clicked(GtkWidget* button,GtkWidget* window)
{
    create_calendar_window(window);
}


/****   聊天窗口   ****/
//聊天窗口 HeadBar
void on_file_tranfer_btn_clicked(GtkWidget* button,gpointer *c_i)
{
    GtkWidget *dialog;
    GSList *filenames;
    dialog = gtk_file_chooser_dialog_new ("选择要发送的文件", GTK_WINDOW(
                                                                 ((chat_info*)c_i)->receive_text),
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                          NULL);
    /* Allow the user to choose more than one file at a time. */
    gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (dialog), FALSE);
    gint result = gtk_dialog_run (GTK_DIALOG (dialog));
    if (result == GTK_RESPONSE_ACCEPT)
    {
        filenames = gtk_file_chooser_get_filenames (GTK_FILE_CHOOSER (dialog));
        while (filenames != NULL)
        {
            gchar *file = (gchar*) filenames->data;
            logEvent(LOG_INFO,"%s was selected.\n", file);
            logEvent(LOG_INFO, "target UID: %d.\n",((chat_info*)c_i)->UID);
	    struct stat filestat;
	    if( stat(file, &filestat) < 0 ) 	break;
	    uint32_t filesz = filestat.st_size;
	    char* fname = pathToFileName(file);
	    sendFile((((chat_info*)c_i)->UID), fname, filesz, file);
	    break;
            filenames = filenames->next;
        }
    }
    gtk_widget_destroy (dialog);
}

void on_chat_history_btn_clicked(gchar* target_name,GtkWidget *sidebar){
    
}
void on_detail_info_btn_clicked(gchar* target_name,GtkWidget *sidebar){
    
}

//聊天窗口 ToolBar
void on_text_style_btn_clicked (GtkWidget *button,GtkWidget *text_view){
    GtkWidget *dialog;
    dialog = gtk_font_selection_dialog_new("请选择字体:");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gchar *s=gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(dialog));
    logEvent(LOG_INFO,"font selected: %s\n",s);
    gdk_fontset_load(s);
    PangoFontDescription *pfd;
    pfd = pango_font_description_from_string(s);
    gtk_widget_modify_font(text_view, pfd);
    gtk_widget_destroy(dialog);
}

void on_text_color_btn_clicked (GtkWidget* button,GtkWidget* text_view){
    GtkResponseType result;
    GtkColorSelection *colorsel;
    GtkWidget *dialog = gtk_color_selection_dialog_new("字体颜色选择");
    result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_OK)
    {
        GdkColor color;
        colorsel = GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(dialog)->colorsel);
        gtk_color_selection_get_current_color(colorsel,&color);
        gtk_widget_modify_text(text_view,GTK_STATE_NORMAL,&color);
    }
    gtk_widget_destroy(dialog);
}

void on_face_btn_clicked(GtkWidget* button,GtkWidget* main_window){
    create_emoji_window(main_window);
}
void on_screen_shot_btn_clicked(){
    capture_image();
}
void on_send_image_btn_clicked(GtkWidget* button,GtkWidget *window){
    GtkWidget *dialog;
    GSList *filenames;
    dialog = gtk_file_chooser_dialog_new ("选择要发送的图片", GTK_WINDOW(window),
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                          NULL);
    /* Allow the user to choose more than one file at a time. */
    gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (dialog), FALSE);
    gint result = gtk_dialog_run (GTK_DIALOG (dialog));
    if (result == GTK_RESPONSE_ACCEPT)
    {
        filenames = gtk_file_chooser_get_filenames (GTK_FILE_CHOOSER (dialog));
        while (filenames != NULL)
        {
            gchar *file = (gchar*) filenames->data;
            logEvent(LOG_INFO,"%s was selected.\n", file);
/*
	    struct stat filestat;
	    if( stat(file, &filestat) < 0 ) {
		    perror("stat");
		    break;
	    }
	    uint32_t filesz = filestat.st_size;
	    char* filename = pathToFileName(file);
	    sendFile(g_uid, filename, filesz, file);
*/
	    break; 	// one file per transmission
            filenames = filenames->next;
        }
    }
    gtk_widget_destroy (dialog);
}

//表情选择窗口
void on_emoji_confrim_btn_clicked(GtkWidget *button,gpointer* my_emoji_info){
    GList *list;
    list = gtk_icon_view_get_selected_items(GTK_ICON_VIEW(((emoji_info*)my_emoji_info)->icon_view));
    gchar *path = gtk_tree_path_to_string (list->data);
    logEvent(LOG_INFO, "selected item: %s.\n",path);
    gtk_widget_destroy(((emoji_info*)my_emoji_info)->window);
    g_free(my_emoji_info);
    logEvent(LOG_DEBUG, "pointer [my_emoji_info] FREED, size: %d\n",sizeof(emoji_info));
    
}

//聊天窗口 BottomBar
void on_chat_send_btn_clicked(GtkWidget* button, gpointer* m_info){
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(((msg_info*)m_info)->send_text_view));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    gchar* msg = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffer), &start, &end, FALSE);
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(buffer), "", 0);
    logEvent(LOG_INFO, "message: %s\n",msg);
    logEvent(LOG_INFO, "uid: %u\n", ((msg_info*)m_info)->u_info->UID);

    if(strlen(msg) == 0) 	return;
    PPACKET packet = protocol_BuildMsgPacket(((msg_info*)m_info)->u_info->UID, strlen(msg));
    if(!packet) return;
    int w = srv_WritePacket(g_sockfd, packet, sizeof(PACKET), key);
    free(packet);
    if( w >= 0 )
	    w = srv_WritePacket(g_sockfd, msg, strlen(msg), key);

    msg_info* info = m_info;

    PPACKET returned = srv_ReadPacket(g_sockfd, MAX_PACKET, key);
    if(!returned) {
	    return;
    }
    uint32_t v = protocol_ParseMsgResultPacket(returned);
    free(returned);
    if( v == 0 ) {
	    time_t t = time(0);
	    appendText(info->receive_text_view, "Me & ");
	    appendText(info->receive_text_view, ctime(&t));
	    appendText(info->receive_text_view, msg);
	    appendText(info->receive_text_view, "\n");
	    logEvent(LOG_INFO, "SUCCESS\n");

	    addChatHistory(g_uid, info->u_info->UID, 1, msg);
    }
}

void on_group_chat_send_btn_clicked(GtkWidget* button,gpointer* g_m_info){
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(((group_msg_info*)g_m_info)->send_text_view));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    gchar* msg = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffer), &start, &end, FALSE);
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(buffer), "", 0);
    logEvent(LOG_INFO, "message: %s\n",msg);
    logEvent(LOG_INFO, "GID: %d\n",((group_msg_info*)g_m_info)->g_info->GID);

    if(strlen(msg) == 0) 	return;
    uint32_t gid = ((group_msg_info*)g_m_info)->g_info->GID;
    GtkTextView* r = ((group_msg_info*)g_m_info)->receive_text_view;
    int result = groupMsg(gid, msg);
    if( result >= 0 ) {
	    logEvent(LOG_INFO, "SUCCESS\n");
	    time_t t = time(0);
	    appendText(r, "Me & ");
	    appendText(r, ctime(&t));
	    appendText(r, msg);
	    appendText(r, "\n");

	    addGroupHistory(gid, g_uid, 0, 1, msg);
    }
    return ;
}


void on_chat_close_btn_clicked(GtkWidget* button,GtkWidget* window){
    gtk_widget_destroy(window);
}

void on_file_rec_btn_clicked(GtkWidget* button,GtkWidget *window){
    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new ("选择文件保存目录", GTK_WINDOW(window),
                                          GTK_FILE_CHOOSER_ACTION_SAVE,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                          NULL);
    gint result = gtk_dialog_run (GTK_DIALOG (dialog));
    if (result == GTK_RESPONSE_ACCEPT)
    {
        gchar* filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        logEvent(LOG_INFO,"%s was selected.\n", filename);
	getFile(filename);
    }
    gtk_widget_destroy (dialog);
}

/****   设置窗口    ****/
void on_set_save_btn_clicked(GtkWidget* button,gpointer* n_s){
    //TODO: 设置一个头像选择器
//    g_s.avatar_id =
    const gchar *name = gtk_entry_get_text(GTK_ENTRY(((set_info_widgets*)n_s)->nickname_entry));
    const gchar *path = gtk_entry_get_text(GTK_ENTRY(((set_info_widgets*)n_s)->file_save_path_entry));

    int cname = -1;
    PPACKET packet = protocol_BuildChangeNamePacket(name);
    if(packet) {
	    cname = srv_WritePacket(g_sockfd, packet, sizeof(PACKET), key);
	    if( cname >= 0 ) {
		    uint8_t* result = srv_ReadPacket(g_sockfd, MAX_PACKET, key);
		    if(result) {
			    cname = protocol_ParseCNResultPacket(result);
			    free(result);
		    } else {
			    cname = -1;
		    }
	    }
	    free(packet);
    }
    if(cname >= 0) {
   	 g_strlcpy(g_s.nick_name, name,dest_len);

    	logEvent(LOG_INFO, "nick name changed to: %s.\n",g_s.nick_name);
    }


    g_s.notify_method = gtk_combo_box_get_active(GTK_COMBO_BOX(((set_info_widgets*)n_s)->combo_box));
    logEvent(LOG_INFO, "notify method changed to: %d.\n",g_s.notify_method);
    g_strlcpy(g_s.main_path, path,dest_len);
    logEvent(LOG_INFO, "file name changed to: %s.\n",g_s.main_path);
    gtk_widget_destroy(((set_info_widgets*)n_s)->window);
    g_free(n_s);
    logEvent(LOG_DEBUG, "pointer [my_set_info] FREED, size: %d\n",sizeof(set_info_widgets));
}

void on_set_cancel_btn_clicked(GtkWidget* button,GtkWidget* window){
    gtk_widget_destroy(window);
}

void on_set_avatar_btn_clicked(GtkWidget* button,GtkWidget* set_window){
    create_avatar_select_window(set_window);
}

void on_set_avatar_confrim_btn_clicked(GtkWidget *button,gpointer* avatar_info){
    GList *list;

    list = gtk_icon_view_get_selected_items(GTK_ICON_VIEW(((emoji_info*)avatar_info)->icon_view));
    gchar *path = gtk_tree_path_to_string (list->data);
    logEvent(LOG_INFO, "selected avatar: %s.\n",path);
    logEvent(LOG_WARNING, "avatar shouldn't be more than 10.\n");
    logEvent(LOG_WARNING, "Otherwise this hack would FAIL.\n");
    g_s.avatar_id = path[0]-'0'+1;
    if (g_s.avatar_id > 9 || g_s.avatar_id < 0) {
        logEvent(LOG_ERROR, "avatar setting ERROR.\n");
    }
    logEvent(LOG_INFO, "avtar changed to %d.\n",g_s.avatar_id);
    setAvatar(g_s.avatar_id);
    gtk_widget_destroy(((emoji_info*)avatar_info)->window);
    g_free(avatar_info);
    logEvent(LOG_DEBUG, "pointer [avatar_info] FREED, size: %d\n",sizeof(emoji_info));
}

void on_add_friend_btn_clicked(GtkWidget* button,GtkWidget* entry){
    const gchar* targetID = gtk_entry_get_text(GTK_ENTRY(entry));
    int target_id = atoi(targetID);
    logEvent(LOG_INFO, "Target add user ID: %d\n",target_id);
    beFriend(target_id);
}

void on_del_friend_btn_clicked(GtkWidget* button,GtkWidget* entry){
    const gchar* targetID = gtk_entry_get_text(GTK_ENTRY(entry));
    int target_id = atoi(targetID);
    logEvent(LOG_INFO, "Target del user ID: %d\n",target_id);
    delFriend(target_id);
}

void on_add_group_btn_clicked(GtkWidget *button,GtkWidget* entry){
    const gchar* targetID = gtk_entry_get_text(GTK_ENTRY(entry));
    int target_id = atoi(targetID);
    logEvent(LOG_INFO, "Target group ID: %d\n",target_id);
}

void on_skin_change_btn_clicked(GtkWidget *button, GtkWidget* window) {
	logEvent(LOG_INFO, "skin change\n");
GtkWidget *dialog;
    GSList *filenames;
    gint result;
    gchar *file=NULL;
    dialog = gtk_file_chooser_dialog_new ("选择皮肤文件", GTK_WINDOW(window),
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                          NULL);

    /* Allow the user to choose more than one file at a time. */
    gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (dialog), FALSE);
    result = gtk_dialog_run (GTK_DIALOG (dialog));
    if (result == GTK_RESPONSE_ACCEPT)
    {

        filenames = gtk_file_chooser_get_filenames (GTK_FILE_CHOOSER (dialog));
        while (filenames != NULL)
        {
            file = (gchar*) filenames->data;
            filenames = filenames->next;
        }
    }
    change_background(window, 600, 450,file);
    gtk_widget_destroy (dialog);
	return;
}
