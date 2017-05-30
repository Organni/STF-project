#ifndef WEB_OPS
#define WEB_OPS

#include <curl/curl.h>
#include <curl/easy.h>
#include <stdio.h>
#include <memory.h>

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

int web_get_cookie(char userid[], char userpass[]);

int send_post(char URL[], char body[], char cookies[], char* content, char* header);

int extract_cookies(char header[], char cookies[]);

int extract_courses(char *raw_html, struct course_info *info_list, int *info_num);

int send_get(char URL[], char cookies[], char* content, char* header);

int send_download(char URL[], char cookies[], char* header, char* save_path);

//find string in string, return the first start location or -1 if can not find
int string_find(const char *pSrc, const char *pDst);

#endif