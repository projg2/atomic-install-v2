/**
 * atomic-install
 * (c) 2013 Michał Górny
 * Released under the 2-clause BSD license
 */

#pragma once

#ifndef POSIXIO_HXX
#define POSIXIO_HXX 1

#include <exception>
#include <iterator>
#include <string>
#include <cstdio>

extern "C"
{
#	include <sys/stat.h>
#	include <sys/types.h>
#	include <dirent.h>

#	include <openssl/md5.h>

#	include <libcopyfile.h>
};

namespace atomic_install
{
	class POSIXIOException : public std::exception
	{
		char _formatted_message[512];

	public:
		int sys_errno;

		POSIXIOException(const char* message, const std::string& fn);

		virtual const char* what() const throw();
	};

	class CopyFile
	{
		std::string& _from;
		std::string& _to;

		void wrap_error(copyfile_error_t ret);

	public:
		CopyFile(std::string& from, std::string& to);

		void move();
		void link_or_copy();
		void copy();
		void copy_metadata();
	};

	class DirectoryScanner : public std::iterator<
				std::input_iterator_tag, struct dirent>
	{
		DIR* _dir;
		std::string _path;
		dirent* _curr_entry;

		bool _omit_special;

	public:
		DirectoryScanner(const std::string& name, bool omit_special = false);
		~DirectoryScanner();

		bool eof() const;
		dirent& operator*() const;
		dirent* operator->() const;

		DirectoryScanner& operator++();
	};

	class StdIOFile
	{
	protected: // for AtomicIOFile
		const std::string& _path;
		FILE* _f;

		StdIOFile(const std::string& path, int dummy);

	public:
		StdIOFile(const std::string& path, const char* mode = "rb");
		~StdIOFile();

		size_t read(void* buf, size_t len);
		void read_exact(void* buf, size_t len);
		void write(const void* buf, size_t len);

		void read_string(std::string& out);
		void write_string(const std::string& s);

		operator FILE*();
	};

	class AtomicIOFile : public StdIOFile
	{
		std::string _tmp_path;

	public:
		AtomicIOFile(const std::string& path, const char* mode = "wb");
		~AtomicIOFile();
	};

	class FileType
	{
	public:
		typedef enum
		{
			regular_file,
			directory,

			// TODO: more types

			n_types
		} enum_type;

	private:
		enum_type _val;

	public:
		FileType();
		FileType(enum_type val);
		operator enum_type() const;
	};

	struct BinMD5
	{
		unsigned char data[16];

		std::string as_hex();
	};

	class MD5Counter
	{
		const std::string& _path;
		MD5_CTX _ctx;

	public:
		MD5Counter(const std::string& path);

		void feed(const char* buf, unsigned long len);
		void finish(BinMD5& out);
	};

	class FileStat : public stat
	{
		BinMD5 _md5;

	public:
		FileStat(const std::string& path);

		FileType file_type();
		BinMD5 data_md5();
	};

	void remove_file(const std::string& path, bool ignore_nonexist = false);
	void remove_dir(const std::string& path, bool ignore_nonexist = false,
			bool ignore_nonempty = false);
};

#endif /*POSIXIO_HXX*/
