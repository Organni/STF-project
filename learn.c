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
int login;
char userBuf[200];
int userBufLen;
char userid[20];
char userpass[50];

int courInfoLen[50];

int test;
char *testBuf;

int noticeNum;
char **noticeTitle;
int *noticeStatus;
int *noticeContent;

int fileNum;
char **fileTitle;
char *fileInfo;
int *fileStatus;
char **fileName;
int downloadWords;

int log;
FILE *log_file;

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
	sprintf(userBuf,"<学号>\n<密码>\n请将上述两个标签替换为您的学号和密码");
	userBufLen = strlen(userBuf);
	
	noticeNum = 2;
	noticeTitle = malloc(sizeof(char*)*2);
	noticeTitle[0] = "notice0title";
	noticeTitle[1] = "notice1title";
	noticeStatus = malloc(sizeof(int)*2);
	noticeStatus[0] = 0;//unread
	noticeStatus[1] = 1;
	noticeContent = "发布时间：一分钟后\n发布者：一只兔子\n内容：\nbalabala";
	
	fileNum = 2;
	fileTitle = malloc(sizeof(char*)*2);
	fileTitle[0] = "file0name";
	fileTitle[1] = "file1name";
	fileInfo = "简要说明：潍坊的爱\n文件大小：666K\n上载时间：公元前";
	fileStatus = malloc(sizeof(int)*2);
	fileStatus[0] = 0;//new file
	fileStatus[1] = 1;
	fileName = malloc(sizeof(char*)*2);
	fileName[0] = "song0word";
	fileName[1] = "song1word";
	downloadWords = 0;

	test = 0;
	testBuf = "output:xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
	/*login = 1;
	for(int i = 0; i < courNum; i++)
		mkdir(courName[i], S_IFDIR | 0755);*/

	log = open("/home/mlf/桌面/log.txt", O_WRONLY);
	log_file = fdopen(log, "a");
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
	 	int i = getIforPath(path);
	 	/*fprintf(log_file,"%s\n", path);
	 	fflush(log_file);*/
	 	if(i >= 0)
	 	{	
	 		int i = getIforPath(path);
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
				stbuf->st_size = strlen(noticeContent);
			} else {
				off = strstr(off+1,"/");
				if(strstr(off+1,"/") == NULL) {
					stbuf->st_mode = S_IFDIR | 0755;
					stbuf->st_nlink = 1;
				} else if(strstr(off+1, "文件信息") != NULL) {
					stbuf->st_mode = S_IFREG | 0444;
					stbuf->st_nlink = 1;
					stbuf->st_size = strlen(fileInfo);
					for(int i = 0; i < fileNum; i ++)
					if(strstr(path, fileTitle[i]) != NULL)
					{
						if(fileStatus[i] == 0)
							stbuf->st_size += strlen("\n新文件");
						break;
					}
				} else {
					stbuf->st_mode = S_IFREG | 0444;
					stbuf->st_nlink = 1;
					stbuf->st_size = downloadWords;
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

	/*options.contLen = strlen(path);
	memset(options.contents, path, options.contLen);
	options.contents[options.contLen] = '\0';*/
	
	//memcpy(options.contents+7,path,strlen(path));

	filler(buf, ".", NULL, 0, 0);
	filler(buf, "..", NULL, 0, 0);
	//filler(buf, options.filename, NULL, 0, 0);
	//filler(buf, testBuf, NULL, 0, 0);

	char *off;
	if(strcmp(path,"/") == 0)
	{
		//filler(buf, options.filename, NULL, 0, 0);
		if(!login) {
			filler(buf, "login", NULL, 0, 0);
		} else {
			getCourseInfo();
			for(int i = 0; i < course_num; i++)
				filler(buf, user_courses[i].name, NULL, 0, 0);
		}
	} else if(strstr(path+1,"/") == NULL){
		filler(buf, "课程信息", NULL, 0, 0);
		filler(buf, "公告", NULL, 0, 0);
		filler(buf, "文件", NULL, 0, 0);
		filler(buf, "作业", NULL, 0, 0);
	} else if(strstr(path+1,"公告") != NULL) {
		for(int i = 0; i < noticeNum; i++) {
			char title[120];
			strcpy(title, noticeTitle[i]);
			if(noticeStatus[i] == 0)
				strcat(title, "-未读");
			else
				strcat(title, "-已读");
			filler(buf, title, NULL, 0, 0);
		}
	} else {
		off = strstr(path+1,"/");
		if(strstr(off+1,"/") == NULL) {
			for(int i = 0; i < fileNum; i++)
				filler(buf, fileTitle[i], NULL, 0, 0);
		} else {
			filler(buf, "文件信息", NULL, 0, 0);
			for(int i = 0; i < fileNum; i++) {
				if(strstr(path, fileTitle[i]) != NULL)
				{
					char name[100];
					strcpy(name, fileName[i]);
					if(downloadWords == 0)
						strcat(name, "-未下载");
					filler(buf, name, NULL, 0, 0);
					break;
				}
			}
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
		int i = getIforPath(path);
		char info[100];
		sprintf(info,"未交作业数：%d\n未读公告数：%d\n新文件数：%d\n",
			user_courses[i].unhanded_work_num, 
			user_courses[i].unread_notice_num,
			user_courses[i].new_file_num);

		size = strlen(info); courInfoLen[i] = size;
		memcpy(buf, info, size);
		//return -ENOENT;
	} else if(strstr(path, "文件信息") != NULL) {
		char info[500];
		strcpy(info,fileInfo);
		for(int i = 0; i < fileNum; i ++)
			if(strstr(path, fileTitle[i]) != NULL)
			{
				if(fileStatus[i] == 0)
					strcat(info,"\n新文件");
				break;
			}
		size = strlen(info);
		memcpy(buf, info, size);
	} else if(strstr(path, "-未下载") != NULL) {
		char *text; text = "file content download from learn";
		size = strlen(text);
		memcpy(buf, text, size);
		downloadWords = size;
	} else if(strstr(path, "-未读") != NULL
				|| strstr(path, "-已读") != NULL)
	{
		size = strlen(noticeContent);
		memcpy(buf, noticeContent, size);
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
	memcpy(userBuf, buf + offset, size);
	userBufLen = size;
	char delims[] = "\n";
	char *result = NULL;
	result = strtok(userBuf, delims);
	memcpy(userid, result, strlen(result) > 20 ? 20 : strlen(result));
	fprintf(log_file, "[learn_write]%s\n", userid);
	result = strtok(NULL, delims);
	memcpy(userpass, result, strlen(result) > 50 ? 50 : strlen(result));
	fprintf(log_file, "[learn_write]%s\n", userpass);
	return size;
}

static int learn_truncate(const char *path, off_t size, struct fuse_file_info *fi)
{
	(void) fi;
	if (strcmp(path+1,"login") != 0)
		return -EINVAL;

	return size;
}

static int learn_flush(const char *path, struct fuse_file_info *fi)
{
	if(strcmp(path+1,"login") == 0 && login == 0)
	{
		/*if(web_get_cookie(userid, userpass) != 0);
			return -1;*/
/*		if(log_file == NULL)
			write(log, "testBuf\n", strlen("testBuf\n"));*/
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
	course_num = 0;
	extract_courses(course_page, &user_courses, &course_num);
	for(int i = 0; i < course_num; i ++) {
		fprintf(log_file, "%s %d %d %d\n", user_courses[i].name,
			user_courses[i].unhanded_work_num,
			user_courses[i].unread_notice_num,
			user_courses[i].new_file_num);
	}
	fflush(log_file);
}

int getIforPath(const char* path)
{
	for(int i = 0; i < course_num; i++)
	{
		if(strstr(path, user_courses[i].name) != NULL)
			return i;
	}
	return -1;
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
