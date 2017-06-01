/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.
*/

/** @file
 *
 * minimal example filesystem using high-level API
 *
 * Compile with:
 *
 *     gcc -Wall learn.c `pkg-config fuse3 --cflags --libs` -o learn
 *
 * ## Source code ##
 * \include learn.c
 */


#define FUSE_USE_VERSION 30

#include <config.h>

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>

#include <string.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include "web/webOps.h"
#include "web/courDetail.h"
#include "web/webOps.c"
#include "web/courDetail.c"
//#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
 * Command line options
 *
 * We can't set default values for the char* fields here because
 * fuse_opt_parse would attempt to free() them when the user specifies
 * different values on the command line.
 */

//char test[2];
#define courNumMax 50
int login;
char userBuf[200];
int userBufLen;
char userid[20];
char userpass[50];

int courInfoLen[courNumMax];

char **fileName;
int downloadWords;

char page_buff[200000];

int log;
extern FILE *log_file;

struct course_notice notice_list[courNumMax][100];
int notice_num[courNumMax];
int noticeLen = strlen("发布者：\n发布时间：\n内容：\n");

struct file_list file_lists[courNumMax][5];
int file_list_num[courNumMax];
int fileInfoLen = strlen("简要说明：\n文件大小：\n上载时间：");
char *defaultDownloadPath;

int homework_list_num[courNumMax];
struct homework h_list[courNumMax][50];

static struct options {
	const char *filename;
	const char *contents;
	int show_help;
} options;

#define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("--name=%s", filename),
	OPTION("--contents=%s", contents),
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};

static void *learn_init(struct fuse_conn_info *conn,
			struct fuse_config *cfg)
{
	(void) conn;
	cfg->kernel_cache = 1;
	//test[0] = '0'; test[1] = '\0';
	login = 0;
	sprintf(userBuf,"malf14\ntest12138password\n请将上述两个标签替换为您的学号和密码");
	userBufLen = strlen(userBuf);
	
	fileName = malloc(sizeof(char*)*2);
	fileName[0] = "song0word";
	fileName[1] = "song1word";
	downloadWords = 0;
	defaultDownloadPath = "/home/mlf/桌面/";

	/*login = 1;
	for(int i = 0; i < courNum; i++)
		mkdir(courName[i], S_IFDIR | 0755);*/

	/*log = open("/home/mlf/桌面/log.txt", O_WRONLY);
	log_file = fdopen(log, "a");*/
	fileInit();
	memset(notice_list, 0, sizeof(struct course_notice)*courNumMax*100);
	memset(file_lists, 0, sizeof(struct file_list)*courNumMax*5);
	memset(h_list, 0, sizeof(struct homework)*courNumMax*5);
	return NULL;
}

static int learn_getattr(const char *path, struct stat *stbuf,
			 struct fuse_file_info *fi)
{
	(void) fi;
	int res = 0;
	char* off;
	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} /*else if (strcmp(path+1, options.filename) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(options.contents);
	}*/ else if (strcmp(path+1, "login") == 0) {
		stbuf->st_mode = S_IFREG | 0666;
		stbuf->st_nlink = 1;
		stbuf->st_size = userBufLen;
	} /*else {
		return lstat(path, stbuf);
	}*/
	 else if (strstr(path+1,"/") == NULL) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 1;
	}
	 else if (strstr(path+1,"课程信息") != NULL) {
	 	int i = getCourseIndexFromPath(path);
	 	if(i >= 0)
	 	{	
	 		//int i = getCourseIndexFromPath(path);
	 		char info[100];
			sprintf(info,"未交作业数：%d\n未读公告数：%d\n新文件数：%d\n",
				user_courses[i].unhanded_work_num, 
				user_courses[i].unread_notice_num,
				user_courses[i].new_file_num);
			courInfoLen[i] = strlen(info);
		}
		else
			i = 0;
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = courInfoLen[i];
	} else {
		off = strstr(path+1,"/");
		if(strstr(off+1, "/") == NULL) {
			stbuf->st_mode = S_IFDIR | 0755;
			stbuf->st_nlink = 1;
		}
		else {
			if(strstr(off+1, "-未读") != NULL
				|| strstr(off+1, "-已读") != NULL)
			{
				stbuf->st_mode = S_IFREG | 0444;
				stbuf->st_nlink = 1;
				stbuf->st_size = noticeLen ;
				int i = getCourseIndexFromPath(path);
				int j = getNoticeIndexFromPath(path, i);
				if(i >= 0 && j >= 0)
					stbuf->st_size += strlen(notice_list[i][j].publisher)
							+ strlen(notice_list[i][j].time)
							+ strlen(notice_list[i][j].content);
			} else {
				off = strstr(off+1,"/");
				if((off = strstr(off+1,"/")) == NULL) {
					stbuf->st_mode = S_IFDIR | 0755;
					stbuf->st_nlink = 1;
				} else if(strstr(off+1,"/") == NULL) {
					stbuf->st_mode = S_IFDIR | 0755;
					stbuf->st_nlink = 1;
				} else if(strstr(off+1, "文件信息") != NULL) {
					stbuf->st_mode = S_IFREG | 0444;
					stbuf->st_nlink = 1;
					stbuf->st_size = fileInfoLen;
					int i = getCourseIndexFromPath(path);
					int j = getListIndexFromPath(path, i);
					int k = getFileIndexFromPath(path, i, j);
					if(i >= 0 && j >= 0 && k >= 0)
					{
						struct course_file *temp = file_lists[i][j].files;
						stbuf->st_size += strlen(temp[k].intro)
								+ strlen(temp[k].file_size)
								+ strlen(temp[k].upload_time);
						if(strstr(temp[k].status, "新文件") != NULL)
							stbuf->st_size += strlen("\n新文件");
					}
					
				} else {
					stbuf->st_mode = S_IFREG | 0666;
					stbuf->st_nlink = 1;
					stbuf->st_size = strlen(defaultDownloadPath);
					int i = getCourseIndexFromPath(path);
					int j = getListIndexFromPath(path, i);
					int k = getFileIndexFromPath(path, i, j);
					if(i >= 0 && j >= 0 && k >= 0)
					{
						struct course_file *temp = file_lists[i][j].files;
						if(temp[k].download_flag == 1)
							stbuf->st_size = strlen(temp[k].save_path);
						else
							stbuf->st_size += strlen(temp[k].title);
					}
				}
			}
			
		}
	}
/*	if((off = strstr(path+1,"/") ==)) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 1;
	}	else {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = 1;
	}*/

	/*else
		res = -ENOENT;*/

	return res;
}

static int learn_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi,
			 enum fuse_readdir_flags flags)
{
	(void) offset;
	(void) fi;
	(void) flags;

	filler(buf, ".", NULL, 0, 0);
	filler(buf, "..", NULL, 0, 0);

	char *off;
	if(strcmp(path,"/") == 0)
	{
		if(!login) {
			filler(buf, "login", NULL, 0, 0);
		} else {
			getCourseInfo();
			if(course_num == 0)
				filler(buf, "bug", NULL, 0, 0);
			else
			for(int i = 0; i < course_num; i++)
				filler(buf, user_courses[i].name, NULL, 0, 0);
		}
	} else if(strstr(path+1,"/") == NULL){
		filler(buf, "课程信息", NULL, 0, 0);
		filler(buf, "公告", NULL, 0, 0);
		filler(buf, "文件", NULL, 0, 0);
		filler(buf, "作业", NULL, 0, 0);
	} else if(strstr(path+1,"公告") != NULL) {
		int i = getCourseIndexFromPath(path);
		getNoticeInfo(i);
		for(int j = 0; j < notice_num[i]; j++) {
			char title[120];
			strcpy(title, notice_list[i][j].title);
			if(strcmp(notice_list[i][j].status, "未读") == 0)
				strcat(title, "-未读");
			else
				strcat(title, "-已读");
			filler(buf, title, NULL, 0, 0);
			//fprintf(log_file, "[notice %d add is ok]\n", j);fflush(log_file);
		}
	} else if(strstr(path+1,"文件") != NULL) {
		//fprintf(log_file, "[read addr]%s\n", path);fflush(log_file);
		off = strstr(path+1,"/");
		int i = getCourseIndexFromPath(path);
		if((off = strstr(off+1,"/")) == NULL) {
			getFileInfo(i);
			for(int j = 0; j < file_list_num[i]; j++)
				filler(buf, file_lists[i][j].name, NULL, 0, 0);
		} else if((off = strstr(off+1,"/")) == NULL) {
			int j = getListIndexFromPath(path, i);
			for(int k = 0; k < file_lists[i][j].file_num; k ++)
				filler(buf, file_lists[i][j].files[k].title, NULL, 0, 0);
		} else {
			filler(buf, "文件信息", NULL, 0, 0);
			int j = getListIndexFromPath(path, i);
			int k = getFileIndexFromPath(path, i, j);
			struct course_file *temp = file_lists[i][j].files;
			char name[100];
			strcpy(name, temp[k].title);
			if(temp[k].download_flag == 0)
				strcat(name, "-未下载");
			else
				strcat(name, "-已下载");
			filler(buf, name, NULL, 0, 0);
		}
	} else if(strstr(path+1,"作业") != NULL) {
		off = strstr(path+1,"/");
		int i = getCourseIndexFromPath(path);
		if((off = strstr(off+1,"/")) == NULL) {
			getHomeworkInfo(i);
			/*for(int j = 0; j < file_list_num[i]; j++)
				filler(buf, file_lists[i][j].name, NULL, 0, 0);*/
		}
	}

	
	
	//test[0] ++;
	//filler(buf, test, NULL, 0, 0);
	//return -ENOENT;

	return 0;
}

static int learn_open(const char *path, struct fuse_file_info *fi)
{
	if (strcmp(path+1, options.filename) == 0)
	{
		if ((fi->flags & O_ACCMODE) != O_RDONLY)
			return -EACCES;
	}/* else if(strcmp(path+1, "login") == 0)
	{

	} else
		return -ENOENT;*/

	
	return 0;
}

static int learn_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;
	//size = 0;
	/*if((strcmp(path+1, options.filename) == 0 )
		|| strstr(path, options.filename) != NULL)
	{
		len = strlen(options.contents);
		if (offset < len) {
			if (offset + size > len)
				size = len - offset;
			memcpy(buf, options.contents + offset, size);
		} else
			size = 0;
	} else */if (strcmp(path+1,"login") == 0) {
		memcpy(buf, userBuf, userBufLen);
		size = userBufLen;
	} else if (strstr(path, "课程信息") != NULL) {
		int i = getCourseIndexFromPath(path);
		char info[100];
		sprintf(info,"未交作业数：%d\n未读公告数：%d\n新文件数：%d\n",
			user_courses[i].unhanded_work_num, 
			user_courses[i].unread_notice_num,
			user_courses[i].new_file_num);

		size = strlen(info); courInfoLen[i] = size;
		memcpy(buf, info, size);
		//return -ENOENT;
	} else if(strstr(path, "文件信息") != NULL) {
		int i = getCourseIndexFromPath(path);
		int j = getListIndexFromPath(path, i);
		int k = getFileIndexFromPath(path, i, j);
		struct course_file *temp = file_lists[i][j].files;
		char info[500];
		sprintf(info, "简要说明：%s\n文件大小：%s\n上载时间：%s",
			temp[k].intro, temp[k].file_size, temp[k].upload_time);
		if(strstr(temp[k].status, "新文件") != NULL)
			strcat(info,"\n新文件");
		size = strlen(info);
		memcpy(buf, info, size);
	} else if(strstr(path, "-未下载") != NULL) {
		char filePath[100];
		strcpy(filePath, defaultDownloadPath);
		int i = getCourseIndexFromPath(path);
		int j = getListIndexFromPath(path, i);
		int k = getFileIndexFromPath(path, i, j);
		struct course_file *temp = file_lists[i][j].files;
		strcat(filePath, temp[k].title);
		size = strlen(filePath);
		memcpy(buf, filePath, size);
	} else if(strstr(path, "-已下载") != NULL) {
		int i = getCourseIndexFromPath(path);
		int j = getListIndexFromPath(path, i);
		int k = getFileIndexFromPath(path, i, j);
		struct course_file *temp = file_lists[i][j].files;
		size = strlen(temp[k].save_path);
		memcpy(buf, temp[k].save_path, size);
	} else if(strstr(path, "-未读") != NULL
				|| strstr(path, "-已读") != NULL)
	{
		int i = getCourseIndexFromPath(path);
		int j = getNoticeIndexFromPath(path, i);
		char content[3000];
		sprintf(content,"发布者：%s\n发布时间：%s\n内容：\n%s\n",
			notice_list[i][j].publisher, 
			notice_list[i][j].time,
			notice_list[i][j].content);
		size = strlen(content);
		memcpy(buf, content, size);
	} else {
		char *text; text = "file content download from learn";
		size = strlen(text);
		memcpy(buf, text, size);
	}
	return size;
}

static int learn_write(const char *path, const char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	if(strcmp(path, "/login") == 0)
	{
		memcpy(userBuf, buf + offset, size);
		userBufLen = size;
		char delims[] = "\n";
		char *result = NULL;
		result = strtok(userBuf, delims);
		memcpy(userid, result, strlen(result) > 20 ? 20 : strlen(result));
		//fprintf(log_file, "[learn_write]%s\n", userid);
		result = strtok(NULL, delims);
		memcpy(userpass, result, strlen(result) > 50 ? 50 : strlen(result));
		//fprintf(log_file, "[learn_write]%s\n", userpass);
		login = 2;
	} else if(strstr(path,"-未下载") != NULL) {
		int i = getCourseIndexFromPath(path);
		int j = getListIndexFromPath(path, i);
		int k = getFileIndexFromPath(path, i, j);
		struct course_file *temp = file_lists[i][j].files;
		strncpy(temp[k].save_path, buf + offset, size);
		
		temp[k].save_path[size - 1] = '\0';
		fprintf(log_file, "[i give you]%s????\n", temp[k].save_path);fflush(log_file);
		temp[k].download_flag = 1;
		download_course_file(user_courses[i].id, temp[k].file_id, temp[k].file_path, temp[k].save_path);
	}
	return size;
}

static int learn_truncate(const char *path, off_t size, struct fuse_file_info *fi)
{
	(void) fi;
	if (strcmp(path+1,"login") == 0)
		return size;
	else if(strstr(path,"-未下载") != NULL)
		return size;
	else
		return -EINVAL;

	return size;
}

static int learn_flush(const char *path, struct fuse_file_info *fi)
{
	if(strcmp(path+1,"login") == 0 && login == 2)
	{
		/*if(web_get_cookie(userid, userpass) != 0);
			return -1;*/
		/*if(log_file == NULL)
			write(log, "testBuf\n", strlen("testBuf\n"));*/
		//fprintf(log_file, "%s\n", "flushing");
		//fflush(log_file);
		int rst = web_get_cookie(userid, userpass);
		if(rst < 0)
			return 0;
		login = 1;
		/*for(int i = 0; i < courNum; i++)
			mkdir(courName[i], S_IFDIR | 0755);*/
		/*if(chdir("..")==-1)
		{
			char* info = "Couldn't change current working directory.";
			userBufLen = strlen(info);
			memcpy(userBuf, info, userBufLen);
		}
		else{
			char* info = "changed current working directory.";
			userBufLen = strlen(info);
			memcpy(userBuf, info, userBufLen);
		}*/
	}	
	/*userBufLen = strlen(userid);
	memcpy(userBuf, userid, userBufLen);*/
	/*userBufLen = strlen(userpass);
	memcpy(userBuf, userpass, userBufLen);*/
	return 0;
}

/*int learn_opendir(const char *path, struct fuse_file_info *fi)
{
	char tpath[100];
	strcpy(tpath,"/home/mlf/桌面/fuse-3.0.2/build/example/STF-project");
	strcat(tpath, path);
	return opendir(tpath);
}*/

/*static int learn_mkdir(const char *path, mode_t mode)
{
	int res;

	res = mkdir(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}*/

static struct fuse_operations learn_oper = {
	.init           = learn_init,
	.getattr	= learn_getattr,
	.readdir	= learn_readdir,
	.open		= learn_open,
	.read		= learn_read,
	.write		= learn_write,
	.truncate   = learn_truncate,
	.flush		= learn_flush,/*
	.opendir	= learn_opendir,
	.mkdir		= learn_mkdir*/
};

static void show_help(const char *progname)
{
	printf("usage: %s [options] <mountpoint>\n\n", progname);
	printf("File-system specific options:\n"
	       "    --name=<s>          Name of the \"learn\" file\n"
	       "                        (default: \"learn\")\n"
	       "    --contents=<s>      Contents \"learn\" file\n"
	       "                        (default \"learn, World!\\n\")\n"
	       "\n");
}

void getCourseInfo()
{
	char course_page[50000];
	memset(course_page, 0 , 50000);
	get_course_page(course_page);
	/*fprintf(log_file, "%s\n", course_page);
	fflush(log_file);*/
	course_num = 0;
	extract_courses(course_page, &user_courses, &course_num);
	/*for(int i = 0; i < course_num; i ++) {
		fprintf(log_file, "%s %d %d %d\n", user_courses[i].name,
			user_courses[i].unhanded_work_num,
			user_courses[i].unread_notice_num,
			user_courses[i].new_file_num);
	}
	fflush(log_file);*/
}

int getCourseIndexFromPath(const char* path)
{
	for(int i = 0; i < course_num; i++)
	{
		if(strstr(path, user_courses[i].name) != NULL)
			return i;
	}
	return -1;
}

void getNoticeInfo(int i)
{
	memset(page_buff, 0, sizeof(page_buff));
	get_notice_page(user_courses[i].id, page_buff);
	extract_notice_list(page_buff, user_courses[i].id, notice_list[i], &(notice_num[i]));
	/*fprintf(log_file, "%[notice!!!!     ]        course id %d\n", i);
	fprintf(log_file, "%[notice!!!!     ]        %d\n", notice_num[i]);
	for(int j = 0; j < notice_num[i]; j++)
	{
		fprintf(log_file, "[notice!!!!     ]        %s\n", notice_list[i][j].title);
	}
	fflush(log_file);*/
}

int getNoticeIndexFromPath(const char* path, int courseIndex)
{
	for(int i = 0; i < notice_num[courseIndex]; i++)
	{
		if(strstr(path, notice_list[courseIndex][i].title) != NULL)
			return i;
	}
	return -1;
}

void getFileInfo(int i)
{
	memset(page_buff, 0, sizeof(page_buff));
	get_file_page(user_courses[i].id, page_buff);
	//fprintf(log_file, "[filepage!!!!     ]        %s\n", page_buff);fflush(log_file);
	file_list_num[i] = 0;
	extract_file_lists(page_buff, file_lists[i], &(file_list_num[i]));
	fprintf(log_file, "[file!!!!     ]        %d\n", file_list_num[i]);
	fflush(log_file);
	for(int j = 0; j < file_list_num[i]; j++)
	{
		fprintf(log_file, "[file!!!!     ]        %s\n", file_lists[i][j].name);
	}
	fflush(log_file);
}

int getListIndexFromPath(const char* path, int courseIndex)
{
	for(int i = 0; i < file_list_num[courseIndex]; i++)
		if(strstr(path, file_lists[courseIndex][i].name) != NULL)
			return i;
	return -1;
}

int getFileIndexFromPath(const char* path, int courseIndex, int ListIndex)
{
	if(courseIndex < 0 || ListIndex < 0)
		return -1;
	struct course_file *temp = file_lists[courseIndex][ListIndex].files;
	for(int i = 0; i < file_lists[courseIndex][ListIndex].file_num; i++)
		if(strstr(path, temp[i].title) != NULL)
			return i;
	return -1;
}

void getHomeworkInfo(int i)
{
	memset(page_buff, 0, sizeof(page_buff));
	get_homework_page(user_courses[i].id, page_buff);
	homework_list_num[i] = 0;
	extract_file_lists(page_buff, h_list[i], &homework_list_num[i]);
	fprintf(log_file, "[homework num!!!!     ]        %d\n", homework_list_num[i]);
	fflush(log_file);
	for(int j = 0; j < homework_list_num[i]; j++)
	{
		fprintf(log_file, "[homework!!!!     ]        %s\n", h_list[i][j].title);
	}
	fflush(log_file);
}

int main(int argc, char *argv[])/**/
{
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	/* Set defaults -- we have to use strdup so that
	   fuse_opt_parse can free the defaults if other
	   values are specified */
	options.filename = strdup("learn");
	options.contents = strdup("hello,world!\n");

	/* Parse options */
	if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
		return 1;

	/* When --help is specified, first print our own file-system
	   specific help text, then signal fuse_main to show
	   additional help (by adding `--help` to the options again)
	   without usage: line (by setting argv[0] to the empty
	   string) */
	if (options.show_help) {
		show_help(argv[0]);
		assert(fuse_opt_add_arg(&args, "--help") == 0);
		args.argv[0] = (char*) "";
	}

	//CURL *curl = curl_easy_init();

	return fuse_main(args.argc, args.argv, &learn_oper, NULL);
}
