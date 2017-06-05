#ifndef WEB_OPS
#define WEB_OPS

#include <curl/curl.h>
#include <curl/easy.h>
#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h> 

struct course_info {
	char name[255];		// 课程名称
	int id;				// 课程id，在网页源代码的超链接中找它
	int unhanded_work_num;	// 未交作业数
	int unread_notice_num;		// 未读公告数
	int new_file_num;		// 新文件数
};

void set_cookie(char new_cookie[]);

char* get_cookie();

size_t write_data(void * ptr, size_t size, size_t nmemb, void * stream);

/* 	向网络学堂发送一个POST请求以获得COOKIE
	userid	： 	用户名
	userpass  :	用户密码
	返回 ： 成功时0，否则为错误码
*/
int web_get_cookie(char userid[], char userpass[]);

/*	使用curl发送一个POST请求
	URL ： 目标的URL
	body ：POST消息体
	COOKIES：需要嵌入消息头的cookie
	content ： 响应体的写回处，为空时不写回
	header ： 响应头的写回处，为空时不写回
	返回 ： 成功时为0，否则为错误码
*/
int send_post(char URL[], char body[], char cookies[], char* content, char* header);

/* 从响应头中提取出cookie
	header : 响应头
	cookie : cookie保存的位置
	返回 : 成功时为0，否则为错误码
*/
int extract_cookies(char header[], char cookies[]);

/*	提取课程信息页面（html字符串）到指定buffer
	page_content : 储存页面的buffer
	返回 : 成功时0，否则为错误码
*/
int extract_courses(char *raw_html, struct course_info *info_list, int *info_num);

/* 	使用curl发送一个get请求
	URL : 目标URL
	cookies ： 捎带在请求头中的cookie
	content  : 响应体的写回处，为空则不写回
	header ： 响应头的写回处，为空则不写回
	返回 : 成功时为0，否则为错误码
*/
int send_get(char URL[], char cookies[], char* content, char* header);

/* 	使用curl发送一个下载请求
	URL : 目标URL
	cookies ： 捎带在请求头中的cookie
	save_path  :  下载到的文件的存放路径
	header ： 响应头的写回处，为空则不写回
	返回 : 成功时为0，否则为错误码
*/
int send_download(char URL[], char cookies[], char* header, char* save_path);

//find string in string, return the first start location or -1 if can not find
int string_find(const char *pSrc, const char *pDst);

/* 	使用curl发送一个上传请求
	URL : 目标URL
	cookies ： 捎带在请求头中的cookie
	content  : 响应体的写回处，为空则不写回
	header ： 响应头的写回处，为空则不写回
	formpost : 上传请求专用的post表格
	form_buff : 额外的post信息
	返回 : 成功时为0，否则为错误码
*/
int send_upload(char URL[], char cookies[], char* header,  char* content, struct curl_httppost* formpost, char* form_buff);
#endif