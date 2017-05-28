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
	return NULL;
}

static int learn_getattr(const char *path, struct stat *stbuf,
			 struct fuse_file_info *fi)
{
	(void) fi;
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path+1, options.filename) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(options.contents);
	} else if (strcmp(path+1, "login") == 0) {
		stbuf->st_mode = S_IFREG | 0666;
		stbuf->st_nlink = 1;
		stbuf->st_size = userBufLen;
	}
	else
		res = -ENOENT;

	return res;
}

static int learn_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi,
			 enum fuse_readdir_flags flags)
{
	(void) offset;
	(void) fi;
	(void) flags;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0, 0);
	filler(buf, "..", NULL, 0, 0);
	filler(buf, options.filename, NULL, 0, 0);

	if(!login) {
		filler(buf, "login", NULL, 0, 0);
	}
	//test[0] ++;
	//filler(buf, test, NULL, 0, 0);

	return 0;
}

static int learn_open(const char *path, struct fuse_file_info *fi)
{
	if (strcmp(path+1, options.filename) == 0)
	{
		if ((fi->flags & O_ACCMODE) != O_RDONLY)
		return -EACCES;
	} else if(strcmp(path+1, "login") == 0)
	{

	} else
		return -ENOENT;

	
	return 0;
}

static int learn_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;
	if(strcmp(path+1, options.filename) == 0)
	{
		len = strlen(options.contents);
		if (offset < len) {
			if (offset + size > len)
				size = len - offset;
			memcpy(buf, options.contents + offset, size);
		} else
			size = 0;
	} else if (strcmp(path+1,"login") == 0)
	{
		memcpy(buf, userBuf, userBufLen);
		size = userBufLen;
	} else 
	{
		return -ENOENT;
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
	result = strtok(NULL, delims);
	memcpy(userpass, result, strlen(result) > 50 ? 50 : strlen(result));
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
	if(strcmp(path+1,"login") != 0)
		return -1;
	/*userBufLen = strlen(userid);
	memcpy(userBuf, userid, userBufLen);*/
	userBufLen = strlen(userpass);
	memcpy(userBuf, userpass, userBufLen);
	return 0;
}

static struct fuse_operations learn_oper = {
	.init           = learn_init,
	.getattr	= learn_getattr,
	.readdir	= learn_readdir,
	.open		= learn_open,
	.read		= learn_read,
	.write		= learn_write,
	.truncate   = learn_truncate,
	.flush		= learn_flush
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

int main(int argc, char *argv[])/**/
{
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	/* Set defaults -- we have to use strdup so that
	   fuse_opt_parse can free the defaults if other
	   values are specified */
	options.filename = strdup("learn");
	options.contents = strdup("learn World!\n");

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

	return fuse_main(args.argc, args.argv, &learn_oper, NULL);
}
