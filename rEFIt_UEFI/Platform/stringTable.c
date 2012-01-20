/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.1 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * Copyright 1993 NeXT, Inc.
 * All rights reserved.
 */

#import "libsaio.h"
#import "bootstruct.h"
#import <driverkit/configTablePrivate.h>

extern char *Language;
extern char *LoadableFamilies;

static void eatThru(char val, char **table_p);

static inline int isspace(char c)
{
    return (c == ' ' || c == '\t');
}

/*
 * Compare a string to a key with quoted characters
 */
static inline int
keyncmp(char *str, char *key, int n)
{
    int c;
    while (n--) {
	c = *key++;
	if (c == '\\') {
	    switch(c = *key++) {
	    case 'n':
		c = '\n';
		break;
	    case 'r':
		c = '\r';
		break;
	    case 't':
		c = '\t';
		break;
	    default:
		break;
	    }
	} else if (c == '\"') {
	    /* Premature end of key */
	    return 1;
	}
	if (c != *str++) {
	    return 1;
	}
    }
    return 0;
}

static void eatThru(char val, char **table_p)
{
	register char *table = *table_p;
	register BOOL found = NO;

	while (*table && !found)
	{
		if (*table == '\\') table += 2;
		else
		{
			if (*table == val) found = YES;
			table++;
		}
	}
	*table_p = table;
}

char *
newStringFromList(
    char **list,
    int *size
)
{
    char *begin = *list, *end;
    char *newstr;
    int newsize = *size;
    
    while (*begin && newsize && isspace(*begin)) {
	begin++;
	newsize--;
    }
    end = begin;
    while (*end && newsize && !isspace(*end)) {
	end++;
	newsize--;
    }
    if (begin == end)
	return 0;
    newstr = malloc(end - begin + 1);
    strncpy(newstr, begin, end - begin);
    *list = end;
    *size = newsize;
    return newstr;
}

/* 
 * compress == compress escaped characters to one character
 */
int stringLength(char *table, int compress)
{
	int ret = 0;

	while (*table)
	{
		if (*table == '\\')
		{
			table += 2;
			ret += 1 + (compress ? 0 : 1);
		}
		else
		{
			if (*table == '\"') return ret;
			ret++;
			table++;
		}
	}
	return ret;
}


// looks in table for strings of format << "key" = "value"; >>
// or << "key"; >>
BOOL getValueForStringTableKey(char *table, char *key, char **val, int *size)
{
	int keyLength;
	char *tableKey;

	do
	{
		eatThru('\"',&table);
		tableKey = table;
		keyLength = strlen(key);
		if (keyLength &&
		    (stringLength(table,1) == keyLength) &&
		    (keyncmp(key, table, keyLength) == 0))
		{
			int c;
			
			/* found the key; now look for either
			 * '=' or ';'
			 */
			while (c = *table) {
			    ++table;
			    if (c == '\\') {
				++table;
				continue;
			    } else if (c == '=' || c == ';') {
				break;
			    }
			}
			if (c == ';') {
			    table = tableKey;
			} else {
			    eatThru('\"',&table);
			}
			*val = table;
			*size = stringLength(table,0);
			return YES;
		}

		eatThru(';',&table);

	} while (*table);

	return NO;
}


/*
 * Returns a new malloc'ed string if one is found
 * in the string table matching 'key'.  Also translates
 * \n escapes in the string.
 */
char *newStringForStringTableKey(
	char *table,
	char *key
)
{
    char *val, *newstr, *p;
    int size;
    
    if (getValueForStringTableKey(table, key, &val, &size)) {
	newstr = malloc(size+1);
	for (p = newstr; size; size--, p++, val++) {
	    if ((*p = *val) == '\\') {
		switch (*++val) {
		case 'r':
		    *p = '\r';
		    break;
		case 'n':
		    *p = '\n';
		    break;
		case 't':
		    *p = '\t';
		    break;
		default:
		    *p = *val;
		    break;
		}
		size--;
	    }
	}
	*p = '\0';
	return newstr;
    } else {
	return 0;
    }
}

char *
newStringForKey(char *key)
{
    char *val, *newstr;
    int size;
    
    if (getValueForKey(key, &val, &size) && size) {
	newstr = malloc(size + 1);
	strncpy(newstr, val, size);
	return newstr;
    } else {
	return 0;
    }
}

/* parse a command line
 * in the form: [<argument> ...]  [<option>=<value> ...]
 * both <option> and <value> must be either composed of
 * non-whitespace characters, or enclosed in quotes.
 */

static char *getToken(char *line, char **begin, int *len)
{
    if (*line == '\"') {
	*begin = ++line;
	while (*line && *line != '\"')
	    line++;
	*len = line++ - *begin;
    } else {
	*begin = line;
	while (*line && !isspace(*line) && *line != '=')
	    line++;
	*len = line - *begin;
    }
    return line;
}

BOOL getValueForBootKey(char *line, char *match, char **matchval, int *len)
{
    char *key, *value;
    int key_len, value_len;
    
    while (*line) {
	/* look for keyword or argument */
	while (isspace(*line)) line++;

	/* now look for '=' or whitespace */
	line = getToken(line, &key, &key_len);
	/* line now points to '=' or space */
	if (!isspace(*line++)) {
	    line = getToken(line, &value, &value_len);
	    if ((strlen(match) == key_len)
		&& strncmp(match, key, key_len) == 0) {
		*matchval = value;
		*len = value_len;
		return YES;
	    }
	}
    }
    return NO;
}

BOOL getBoolForKey(
    char *key
)
{
    char *val;
    int size;
    
    if (getValueForKey(key, &val, &size) && (size >= 1) &&
	val[0] == 'Y' || val[0] == 'y')
	    return YES;
    return NO;
}

BOOL getValueForKey(
    char *key,
    char **val,
    int *size
)
{
    if (getValueForBootKey(bootArgs->bootString, key, val, size))
	return YES;
    else if (getValueForStringTableKey(bootArgs->config, key, val, size))
	return YES;

    return NO;
}

#define LOCALIZABLE \
    "/usr/Devices/%s.config/%s.lproj/Localizable.strings"

char *
loadLocalizableStrings(
    char *name
)
{
    char buf[256], *config;
    register int count, fd;
    
    sprintf(buf, LOCALIZABLE, name, Language);
    if ((fd = open(buf,0)) < 0) {
	sprintf(buf, LOCALIZABLE, name, "English");
	if ((fd = open(buf,0)) < 0)
	    return 0;
    }
    count = file_size(fd);
    config = malloc(count);
    count = read(fd, config, count);
    close(fd);
    if (count <= 0) {
	free(config);
	return 0;
    }
    return config;
}

char *
bundleLongName(
    char *bundleName
)
{
    char *table, *name, *val;
    int size;
    
    if ((table = loadLocalizableStrings(bundleName)) != 0 &&
	 getValueForStringTableKey(table,"Long Name", &val, &size) == YES) {
	name = malloc(size+1);
	strncpy(name, val, size);
	free(table);
    } else {
	name = newString(bundleName);
    }
    return name;
}

int sysConfigValid;


/*
 * Returns 0 if file loaded OK,
 *        -1 if file was not loaded
 * Does not print error messages.
 * Returns pointer to table in memory in *table.
 */
int
loadConfigFile( char *configFile, char **table, int allocTable)
{
    char *configPtr = bootArgs->configEnd;
    int fd, count;
    
    /* Read config file into memory */
    if ((fd = open(configFile, 0)) >= 0)
    {
	if (allocTable) {
	    configPtr = malloc(file_size(fd)+2);
	} else {
	    if ((configPtr - bootArgs->config) > CONFIG_SIZE) {
		error("No room in memory for config file %s\n",configFile);
		close(fd);
		return -1;
	    }
	    localPrintf("Reading config: %s\n",configFile);	    
	}
	if (table) *table = configPtr;
	count = read(fd, configPtr, IO_CONFIG_DATA_SIZE);
	close(fd);

	configPtr += count;
	*configPtr++ = 0;
	*configPtr = 0;
	if (!allocTable)
	    bootArgs->configEnd = configPtr;

	return 0;
    } else {
	return -1;
    }
}

/* Returns 0 if requested config files were loaded,
 *         1 if default files were loaded,
 *        -1 if no files were loaded.
 * Prints error message if files cannot be loaded.
 */
 
int
loadConfigDir(
    char *bundleName,	// bundle directory name (e.g. "System.config")
    int useDefault,	// use Default.table instead of instance tables
    char **table,	// returns pointer to config table
    int allocTable	// malloc the table and return in *table
)
{
    char *buf;
    int i, ret;
    
    buf = malloc(256);
    ret = 0;
    
    // load up to 99 instance tables
    for (i=0; i < 99; i++) {
	sprintf(buf, "%s%s.config/Instance%d.table", IO_CONFIG_DIR,
		bundleName, i);
	if (useDefault || (loadConfigFile(buf, table, allocTable) != 0)) {
	    if (i == 0) {
		// couldn't load first instance table;
		// try the default table
		sprintf(buf, "%s%s.config/%s", IO_CONFIG_DIR, bundleName,
			IO_DEFAULT_TABLE_FILENAME);
		if (loadConfigFile(buf, table, allocTable) == 0) {
		    ret = 1;
		} else {
		    if (!allocTable)
			error("Config file \"%s\" not found\n", buf);
		    ret = -1;
		}
	    }
	    // we must be done.
	    break;
	}
    }
    free(buf);
    return ret;
}


#define SYSTEM_DEFAULT_FILE \
	IO_SYSTEM_CONFIG_DIR IO_DEFAULT_TABLE_FILENAME
#define SYSTEM_CONFIG "System"
#define LP '('
#define RP ')'

/* Returns 0 if requested config files were loaded,
 *         1 if default files were loaded,
 *        -1 if no files were loaded.
 * Prints error message if files cannot be loaded.
 */
int
loadSystemConfig(
    char *which,
    int size
)
{
    char *buf, *bp, *cp;
    int ret, len, doDefault=0;

    buf = bp = malloc(256);
    if (which && size)
    {
	for(cp = which, len = size; len && *cp && *cp != LP; cp++, len--) ;
	if (*cp == LP) {
	    while (len-- && *cp && *cp++ != RP) ;
	    /* cp now points past device */
	    strncpy(buf,which,cp - which);
	    bp += cp - which;
	} else {
	    cp = which;
	    len = size;
	}
	if (*cp != '/') {
	    strcpy(bp, IO_SYSTEM_CONFIG_DIR);
	    strncat(bp, cp, len);
	    if (strncmp(cp + len - strlen(IO_TABLE_EXTENSION),
		       IO_TABLE_EXTENSION, strlen(IO_TABLE_EXTENSION)) != 0)
		strcat(bp, IO_TABLE_EXTENSION);
	} else {
	    strncpy(bp, cp, len);
	    bp[size] = '\0';
	}
	if (strcmp(bp, SYSTEM_DEFAULT_FILE) == 0)
	    doDefault = 1;
	ret = loadConfigFile(bp = buf, 0, 0);
    } else {
	ret = loadConfigDir((bp = SYSTEM_CONFIG), 0, 0, 0);
    }
    if (ret < 0) {
	error("System config file '%s' not found\n", bp);
    } else
	sysConfigValid = 1;
    free(buf);
    return (ret < 0 ? ret : doDefault);
}


int
loadOtherConfigs(
    int useDefault
)
{
    char *val, *table;
    int count;
    char *string;
    int fd, ret;

    if (getValueForKey( "Boot Drivers", &val, &count))
    {
	while (string = newStringFromList(&val, &count)) {
	    ret = loadConfigDir(string, useDefault, &table, 0);
	    if (ret >= 0) {
		if ((fd = openDriverReloc(string)) >= 0) {
		    localPrintf("Loading binary for %s\n",string);
		    if (loadDriver(string, fd) < 0)
			error("Error loading driver %s\n",string);
		    close(fd);
		}
		driverWasLoaded(string, table);
		free(string);
	    } else {
		driverIsMissing(string);
	    }
	    if (ret == 1)
		useDefault = 1;	// use defaults from now on
	}
    } else {
	error("Warning: No active drivers specified in system config\n");
    }

    bootArgs->first_addr0 =
	    (int)bootArgs->configEnd + 1024;
    return 0;
}












