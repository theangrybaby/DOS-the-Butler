

// Copyright (c) 1999-2006 by Digital Mars
// All Rights Reserved
// written by Walter Bright
// www.digitalmars.com
// License for redistribution is by either the Artistic License
// in artistic.txt, or the GNU General Public License in gnu.txt.
// See the included readme.txt for details.

#ifndef ROOT_H
#define ROOT_H

#include <stdlib.h>
#include <stdarg.h>

#if __DMC__
#pragma once
#endif

typedef size_t hash_t;

#include "dchar.h"

char *wchar2ascii(wchar_t *);
int wcharIsAscii(wchar_t *);
char *wchar2ascii(wchar_t *, unsigned len);
int wcharIsAscii(wchar_t *, unsigned len);

int bstrcmp(unsigned char *s1, unsigned char *s2);
char *bstr2str(unsigned char *b);
void error(const char *format, ...);
void error(const wchar_t *format, ...);
void warning(const char *format, ...);

#ifndef TYPEDEFS
#define TYPEDEFS

#if _MSC_VER
typedef __int64 longlong;
typedef unsigned __int64 ulonglong;
#else
typedef long long longlong;
typedef unsigned long long ulonglong;
#endif

#endif

longlong randomx();

/*
 * Root of our class library.
 */

struct OutBuffer;
struct Array;

struct Object
{
    Object() { }
    virtual ~Object() { }

    virtual int equals(Object *o);

    /**
     * Returns a hash code, useful for things like building hash tables of Objects.
     */
    virtual hash_t hashCode();

    /**
     * Return <0, ==0, or >0 if this is less than, equal to, or greater than obj.
     * Useful for sorting Objects.
     */
    virtual int compare(Object *obj);

    /**
     * Pretty-print an Object. Useful for debugging the old-fashioned way.
     */
    virtual void print();

    virtual char *toChars();
    virtual dchar *toDchars();
    virtual void toBuffer(OutBuffer *buf);

    /**
     * Used as a replacement for dynamic_cast. Returns a unique number
     * defined by the library user. For Object, the return value is 0.
     */
    virtual int dyncast();

    /**
     * Marks pointers for garbage collector by calling mem.mark() for all pointers into heap.
     */
    /*virtual*/		// not used, disable for now
	void mark();
};

struct String : Object
{
    int ref;			// != 0 if this is a reference to someone else's string
    char *str;			// the string itself

    String(char *str, int ref = 1);

    ~String();

    static hash_t calcHash(const char *str, size_t len);
    static hash_t calcHash(const char *str);
    hash_t hashCode();
    unsigned len();
    int equals(Object *obj);
    int compare(Object *obj);
    char *toChars();
    void print();
    void mark();
};

struct FileName : String
{
    FileName(char *str, int ref);
    FileName(char *path, char *name);
    hash_t hashCode();
    int equals(Object *obj);
    int compare(Object *obj);
    static int absolute(const char *name);
    static char *ext(const char *);
    char *ext();
    static char *name(const char *);
    char *name();
    static char *path(const char *);
    static char *replaceName(char *path, char *name);

    static char *combine(char *path, char *name);
    static Array *splitPath(const char *path);
    static FileName *defaultExt(const char *name, const char *ext);
    static FileName *forceExt(const char *name, const char *ext);
    int equalsExt(const char *ext);

    void CopyTo(FileName *to);
    static char *searchPath(Array *path, char *name, int cwd);
    static int exists(const char *name);
    static void ensurePathExists(const char *path);
};

struct File : Object
{
    int ref;			// != 0 if this is a reference to someone else's buffer
    unsigned char *buffer;	// data for our file
    unsigned len;		// amount of data in buffer[]
    void *touchtime;		// system time to use for file

    FileName *name;		// name of our file

    File(char *);
    File(FileName *);
    ~File();

    void mark();

    char *toChars();

    /* Read file, return !=0 if error
     */

    int read();

    /* Write file, either succeed or fail
     * with error message & exit.
     */

    void readv();

    /* Read file, return !=0 if error
     */

    int mmread();

    /* Write file, either succeed or fail
     * with error message & exit.
     */

    void mmreadv();

    /* Write file, return !=0 if error
     */

    int write();

    /* Write file, either succeed or fail
     * with error message & exit.
     */

    void writev();

    /* Return !=0 if file exists.
     *	0:	file doesn't exist
     *	1:	normal file
     *	2:	directory
     */

    /* Append to file, return !=0 if error
     */

    int append();

    /* Append to file, either succeed or fail
     * with error message & exit.
     */

    void appendv();

    /* Return !=0 if file exists.
     *	0:	file doesn't exist
     *	1:	normal file
     *	2:	directory
     */

    int exists();

    /* Given wildcard filespec, return an array of
     * matching File's.
     */

    static Array *match(char *);
    static Array *match(FileName *);

    // Compare file times.
    // Return	<0	this < f
    //		=0	this == f
    //		>0	this > f
    int compareTime(File *f);

    // Read system file statistics
    void stat();

    /* Set buffer
     */

    void setbuffer(void *buffer, unsigned len)
    {
	this->buffer = (unsigned char *)buffer;
	this->len = len;
    }

    void checkoffset(size_t offset, size_t nbytes);

    void remove();		// delete file
};

struct OutBuffer : Object
{
    unsigned char *data;
    unsigned offset;
    unsigned size;

    OutBuffer();
    ~OutBuffer();
    void *extractData();
    void mark();

    void reserve(unsigned nbytes);
    void setsize(unsigned size);
    void reset();
    void write(const void *data, unsigned nbytes);
    void writebstring(unsigned char *string);
    void writestring(char *string);
    void writedstring(const char *string);
    void writedstring(const wchar_t *string);
    void prependstring(char *string);
    void writenl();			// write newline
    void writeByte(unsigned b);
    void writebyte(unsigned b) { writeByte(b); }
    void writeUTF8(unsigned b);
    void writedchar(unsigned b);
    void prependbyte(unsigned b);
    void writeword(unsigned w);
    void writeUTF16(unsigned w);
    void write4(unsigned w);
    void write(OutBuffer *buf);
    void write(Object *obj);
    void fill0(unsigned nbytes);
    void align(unsigned size);
    void vprintf(const char *format, va_list args);
    void printf(const char *format, ...);
#if M_UNICODE
    void vprintf(const unsigned short *format, va_list args);
    void printf(const unsigned short *format, ...);
#endif
    void bracket(char left, char right);
    unsigned bracket(unsigned i, char *left, unsigned j, char *right);
    void spread(unsigned offset, unsigned nbytes);
    unsigned insert(unsigned offset, const void *data, unsigned nbytes);
    void remove(unsigned offset, unsigned nbytes);
    char *toChars();
    char *extractString();
};

struct Array : Object
{
    unsigned dim;
    unsigned allocdim;
    void **data;

    Array();
    ~Array();
    void mark();
    char *toChars();

    void reserve(unsigned nentries);
    void setDim(unsigned newdim);
    void fixDim();
    void push(void *ptr);
    void *pop();
    void shift(void *ptr);
    void insert(unsigned index, void *ptr);
    void insert(unsigned index, Array *a);
    void append(Array *a);
    void remove(unsigned i);
    void zero();
    void *tos();
    void sort();
    Array *copy();
};

struct Bits : Object
{
    unsigned bitdim;
    unsigned allocdim;
    unsigned *data;

    Bits();
    ~Bits();
    void mark();

    void resize(unsigned bitdim);

    void set(unsigned bitnum);
    void clear(unsigned bitnum);
    int test(unsigned bitnum);

    void set();
    void clear();
    void copy(Bits *from);
    Bits *clone();

    void sub(Bits *b);
};

#endif
