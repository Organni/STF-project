
#include "webOps.h"

char user_cookie[500];
FILE* log_file;

struct course_info user_courses[50];
int course_num;

void set_cookie(char new_cookie[]){
	memcpy(user_cookie, new_cookie, strlen(new_cookie));
}

char* get_cookie(){
	return (char*) user_cookie;
}

size_t write_data(void * ptr, size_t size, size_t nmemb, void * stream)
{
	memcpy(stream + strlen(stream), ptr, size * nmemb);
	return size * nmemb;
}

/*int main(int argc, char** argv){
	memset(user_cookie, 0 , 500);
	char username[] = "";				//在这里输入你的用户名和密码来测试
	char userpass[] = "";
	if(web_get_cookie(username, userpass) != 0)
		return -1;

	char course_page[50000];
	memset(course_page, 0 , 50000);
	get_course_page(course_page);

	extract_courses(course_page, &user_courses, &course_num);
	int i = 0;
	for(; i < course_num; i++)
		printf("%s\n", user_courses[i].name);
 	return 0;
}*/

int web_get_cookie(char userid[], char userpass[]){
	char URL[] = "http://learn.tsinghua.edu.cn/MultiLanguage/lesson/teacher/loginteacher.jsp";
	char body[500] = "userid=";
	char content[50000];
	char header[2000];
	memset(content, 0, 50000);
	memset(header, 0, 2000);
	strcat(body,userid);
	strcat(body, "&userpass=");
	strcat(body, userpass);
	strcat(body, "&submiy1: 登录");
	fprintf(log_file, "[login uid]%s\n", userid);
	fprintf(log_file, "[login upass]%s\n", userpass);	
	fflush(log_file);
	int res = send_post(URL, body, NULL, content, header);
	if(res != 0){
		return -1;
	}

	if(log_file == NULL)
		return -2;
/*	fprintf(log_file, "[HEADER]%s\n", header);
	fprintf(log_file, "[CONTENT]%s\n", content);*/
	fflush(log_file);
	char cookies[500];
	memset(cookies, 0, 500);

	extract_cookies(header, cookies);
	set_cookie(cookies);

	fprintf(log_file,"[COOKIES]: %s\n",cookies);
	fflush(log_file);
	if(strstr(cookies,"THNSV2COOKIE") == NULL){
		fprintf(log_file,"[COOKIES]FAIL\n");
		fflush(log_file);
		return -1;
	}
	return 0;
}

int send_post(char URL[], char body[], char cookies[], char* content, char* header){
	
	CURL *curl = curl_easy_init();
	if (NULL == curl)
	{
                  curl_global_cleanup(); 
                  printf("cannot get a CURL\n");
		return -1;
	}
	int rst = 0;
 	// 设置属性 	
 	curl_easy_setopt(curl, CURLOPT_URL, URL);
 	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
 	if(content){
 		curl_easy_setopt(curl, CURLOPT_WRITEDATA, content);   
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); 	
 	}
	if(header){
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, header);   
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_data); 	
	}
	

	if(cookies){

	}

	rst = curl_easy_perform(curl);	
	printf("[error]%s\n",  curl_easy_strerror(rst));
	//printf("%s\n",header);
	//printf("%s\n",content);
 	return rst;
}

int send_get(char URL[], char cookies[], char* content, char* header){
	
	CURL *curl = curl_easy_init();
	if (NULL == curl)
	{
                  curl_global_cleanup(); 
                  printf("cannot get a CURL\n");
		return -1;
	}
 	// 设置属性 	
 	curl_easy_setopt(curl, CURLOPT_URL, URL);
 	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
 	if(content){
 		curl_easy_setopt(curl, CURLOPT_WRITEDATA, content);   
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); 	
 	}
	if(header){
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, header);   
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_data); 
	}
	
	if(cookies){
		curl_easy_setopt(curl, CURLOPT_COOKIE, cookies);
	}

	curl_easy_perform(curl);	
	curl_easy_cleanup(curl);

	//printf("%s\n",header);
	//printf("%s\n",content);
 	return 0;
}

int extract_cookies(char header[], char cookies[]) {
	char delima[] = "\r\n";
	char *token  = NULL;
	int i  = 0;
	token = strtok(header, delima);
	if(!token)
		return -1;
	for (; i < 4; i++){			// cookies are the 5th and 6th line in header
		token = strtok(NULL, delima);
		if(!token)
			return -1;
	}
	token = token + 12; 		// skip "Set-Cookie: "
	strncat(cookies, token,strlen(token)-6);	//skip "path=/"
	token =  strtok(NULL, delima);
	token = token + 12;
	strncat(cookies, token,strlen(token)-6);
	
	return 0;
}

int get_course_page(char* page_content){
	char URL[] = "http://learn.tsinghua.edu.cn/MultiLanguage/lesson/student/MyCourse.jsp?language=cn";
	char header[500];
	memset(header, 0, 500);
	send_get(URL, get_cookie(), page_content, header);
	//printf("COOKIES: %s\n",get_cookie());
	//printf("HEADER: %s\n",header);
	//printf("CONTENT: %s\n",page_content);
	return 0;
}

int send_download(char URL[], char cookies[], char* header, char* save_path){
	CURL *curl = curl_easy_init();
	if (NULL == curl)
	{
                  curl_global_cleanup(); 
                  printf("cannot get a CURL\n");
		return -1;
	}

	FILE* file = fopen(save_path,"w+");
	if(file == NULL){
		printf("[DownloadFail] cannot open file %s\n", save_path);
		return -1;
	}
 	// 设置属性 	
 	curl_easy_setopt(curl, CURLOPT_URL, URL);
 	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

 	curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);   
 
	if(header){
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, header);   
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_data); 
	}
	
	if(cookies){
		curl_easy_setopt(curl, CURLOPT_COOKIE, cookies);
	}
	curl_easy_perform(curl);	
	curl_easy_cleanup(curl);
	fclose(file);
	//printf("%s\n",header);
	//printf("%s\n",content);
 	return 0;
}

int send_upload(char URL[], char cookies[], char* header,  char* content, struct curl_httppost* formpost, char* form_buff){
	CURL *curl = curl_easy_init();
	if (NULL == curl)
	{
                  curl_global_cleanup(); 
                  printf("cannot get a CURL\n");
		return -1;
	}

 	// 设置属性 	
 	curl_easy_setopt(curl, CURLOPT_URL, URL);
 	curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
 	//printf("[UPLOAD]setting opts\n" );
 	if(content){
 		curl_easy_setopt(curl, CURLOPT_WRITEDATA, content);   
 		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); 
 	}
 	if(form_buff){
 		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, form_buff);
 	}
	if(header){
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, header);   
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_data); 
	}

	if(cookies){
		curl_easy_setopt(curl, CURLOPT_COOKIE, cookies);
	}
	//printf("[UPLOAD]set opts\n" );
	int rst = curl_easy_perform(curl);	
	if(rst != 0){
		printf("[UPLOAD]%s\n", curl_easy_strerror(rst));
	}
	curl_easy_cleanup(curl);
	//printf("[UPLOAD]curl performed\n" );
	//fclose(file);
	//printf("%s\n",header);
	//printf("%s\n",content);
 	return rst;
}

/*
*    将网页中的课程列表提取出来
*    <ram_html>  字符串形式的网页源代码，为登陆后的网络学堂首页中iframe内的内容
*    <info_list>      课程信息列表，其结构参考头文件，你需要根据课程数建立并填充一个数组，并使用memcpy
		把内容拷贝到这个变量所在的地址
      <info_num>   课程数量，你需要根据你的结果进行设置（*info_num = x）
      返回值 ： 成功时为0, 否则为其他的数值，你可以自行约定，并打引出对应的错误信息
*/
int extract_courses(char *raw_html, struct course_info *info_list, int *info_num){
	struct course_info  temp_list[50];
	memset(&temp_list, 0, sizeof(struct course_info)*50);
	int course_num = 0;

	/*
		YOUR  CODE  TO FILL THE LIST
	struct course_info {
	char name[255];		// 课程名称
	int id;				// 课程id，在网页源代码的超链接中找它
	int unhanded_work_num;	// 未交作业数
	int unread_notice_num;		// 未读公告数
	int new_file_num;		// 新文件数
	};
	*/
	//printf("twl html: %s\n",raw_html);	
	int head = 0;
	int i=0,j=0,end;
	char buf[5];
	//int tail = 0;
	char * p = raw_html;
	head = string_find(p,"!--td");
	while(head>=i)
	{
		
		p = p+head;
		//id   ?course_id=143823"
		i = string_find(p,"course_id=")+10;
		//printf("i= %d\n",i);
		int id = 0;
		while(p[i]!='\"')//遇到“id结束，可增加是否是数字的判断
		{
			id = id*10 + p[i] - '0';
			i++ ;
		}
		temp_list[course_num].id = id;
		//name   ="_blank">搜索引擎技术基础(0)(2016-2017春季学期)</a>
		i = i + string_find(p+i,"target=\"_blank\">")+26;
		//printf("i= %d\n",i);
		j = i + string_find(p+i,"</a>");
		memcpy(temp_list[course_num].name,p+i,j-i);
		//未交作业数   ">1</span>个未交作业</td>
		i = i + string_find(p+i,"\"red_text\">")+strlen("\"red_text\">");
		end = string_find(p+i,"</span>");
		strncpy(buf, p + i, end);
		buf[end] = "\0";
		temp_list[course_num].unhanded_work_num = atoi(buf);
		//未读公告数   ">2</span>个未读公告</td
		i = i + string_find(p+i,"\"red_text\">")+strlen("\"red_text\">");
		end = string_find(p+i,"</span>");
		strncpy(buf, p + i, end);
		buf[end] = "\0";
		temp_list[course_num].unread_notice_num = atoi(buf);
		//新文件数     ">0</span>个新文件</td>
		i = i + string_find(p+i,"\"red_text\">")+strlen("\"red_text\">");
		end = string_find(p+i,"</span>");
		strncpy(buf, p + i, end);
		buf[end] = "\0";
		temp_list[course_num].new_file_num = atoi(buf);
		
		//printf("num: %d\n",course_num);
		//printf("id: %d\n",temp_list[course_num].id);
		//printf("name: %s\n",temp_list[course_num].name);
		//printf("未交: %d\n",temp_list[course_num].unhanded_work_num);
		//printf("未读: %d\n",temp_list[course_num].unread_notice_num);
		//printf("新: %d\n",temp_list[course_num].new_file_num);
		
		
		course_num++;
		//tail += head;
		//printf("tail %d\n",tail);
		head = i + string_find(p+i,"!--td");
		//printf("head %d\n",head);

	}
	
	
	memcpy(info_list, temp_list, sizeof(struct course_info)*course_num);
	*info_num = course_num;
	return 0;
}


int string_find(const char *pSrc, const char *pDst)  
{  
    char* ans = NULL;
    int i, j;  
    for (i=0; pSrc[i]!='\0'; i++)  
    {  
        if(pSrc[i]!=pDst[0])  
            continue;         
        j = 0;  
        while(pDst[j]!='\0' && pSrc[i+j]!='\0')  
        {  
            j++;  
            if(pDst[j]!=pSrc[i+j])  
            break;  
        }  
        if(pDst[j]=='\0')
			return i;
	}  
    return -1; 
}  

void fileInit()
{
	int log = open("/home/mlf/桌面/log.txt", O_WRONLY);
	if(log < 0) {
		printf("[LOG_INIT]%s\n",  strerror(errno));
	}
	log_file = fdopen(log, "a");
	if(!log_file){
		printf("[LOG_INIT]%s\n",  strerror(errno));
	}
}