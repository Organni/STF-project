#include "webOps.h"

int get_notice_page(int course_id, char* notice_page){
	char URL[256] = "http://learn.tsinghua.edu.cn/MultiLanguage/public/bbs/note_list_student.jsp?bbs_id=8479156&course_id=";
	char num_str[50];
	sprintf(num_str, "%d", course_id);
	strcat(URL, num_str);

	//char header[500];
	char content[5000];
	memset(header, 0, 500);

	send_get(URL, get_cookie(), &content, NULL);

	//printf("HEADER:%s\n",header);
	//printf("CONTENT:%s\n",content);
	memcpy(notice_page, content, strlen(content));
	return 0;
}

int main(int argc, char** argv){
	char username[] = "2014011410";				//在这里输入你的用户名和密码来测试
	char userpass[] = "jtnlyanf4838";
	if(web_get_cookie(username, userpass) != 0)
		return -1;

	char notice_page[50000];
	get_notice_page(143823, &notice_page);
}