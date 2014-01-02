#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>


#define TRUE 1
#define FALSE 0

#define TAGNUM 8
#define PAIR 2



static int startsWith(lua_State* L) {
	int res;

	const char* str = luaL_checkstring(L, 1);
	const char* prefix = luaL_checkstring(L, 2);
	
	lua_settop(L, 2);
	lua_pop(L, 2);

	//size_t *len;
	int str_len = strlen (str);
	int prefix_len = strlen (prefix);
		
	if (str_len < prefix_len)
		lua_pushboolean(L, 0);
	else {
		res = strncmp (str, prefix, prefix_len) == 0;
		lua_pushboolean(L, res);
	}

	return 1;
}

static int endsWith(lua_State* L) {
	int res;

	const char* str = luaL_checkstring(L, 1);
	const char* suffix = luaL_checkstring(L, 2);
	
	lua_settop(L, 2);
	lua_pop(L, 2);

	//size_t *len;
	int str_len = strlen (str);
	int suffix_len = strlen (suffix);
		
	if (str_len < suffix_len)
		lua_pushboolean(L, 0);
	else {
		res = strcmp (str + str_len - suffix_len, suffix) == 0;
		lua_pushboolean(L, res);
	}

	return 1;

}

/*
// @param 1  original string
// @param 2  starti
// @param 3  endi
static int bsub(lua_State* L) {
	int res_flag = 0;
	char *p;
	int i;
	size_t str_len;
	
	const char * str = luaL_checklstring(L, 1, &str_len);
	lua_Integer starti = luaL_checkinteger(L, 2);
	lua_Integer endi = luaL_checkinteger(L, 3);
	lua_settop(L, 3);
	lua_pop(L, 3);

	printf("%d\n", str_len);
	if (starti < 0) { starti += str_len + 1; }
	if (endi < 0) { endi += str_len + 1; }

	if (starti > endi) { lua_pushlstring(L, "", 0); return 1; }	
	printf("%d %d\n", starti, endi);
	printf("%d %d\n", starti, endi);	
	lua_pushlstring(L, str + starti - 1, endi - starti + 1);

	return 1;
}
*/

// @param 1  original string
// @param 2  the index of needle
static int rfind(lua_State* L) {
	int res_flag = 0;
	char *p;
	int i;
  	size_t needle_len, str_len;
	
	const char* str = luaL_checklstring(L, 1, &str_len);
	const char* needle = luaL_checklstring(L, 2, &needle_len);
	lua_settop(L, 2);

	lua_pop(L, 2);

	if (needle_len == 0) {
  		p = (char *)str;
	}
	else if (str_len < needle_len) {
		p = NULL;
	}
	else {
		p = (char *)(str + str_len - needle_len);
		while (p >= str) {
			for (i = 0; i < needle_len; i++) {
				if (p[i] != needle[i])
					goto next;
			}
			break;
next:
			p--;
		}
	}
	
	// push the first return, the start offset
	if (p == NULL || p < str) {
		lua_pushnil(L);
		lua_pushnil(L);		
	}
	else {
		lua_pushnumber(L, (int)(p - str + 1));
		lua_pushnumber(L, (int)(p - str + needle_len));		
	}
	
	return 2;
}


// remove whitespace
// @param 1  str
static int ltrim(lua_State* L) {
	char *start;
	const char* str = luaL_checkstring(L, 1);
	lua_settop(L, 1);

	for (start = (char *)str; *start && isspace (*start); start++) 
		;

	if (start != str) {
		lua_pop(L, 1);
		lua_pushstring(L, start);
	} 
	
	return 1;
}

// remove whitespace
// @param 1  str
static int rtrim(lua_State* L) {
	const char* str = luaL_checkstring(L, 1);
	lua_settop(L, 1);

	int len = strlen (str);
	while (len--) {
		if (!isspace (str[len]))
			break;
	}

	int newlen = strlen(str);

	if (newlen != len) {
		// XXX: some odd, may improve
		lua_pop(L, 1);
		lua_pushlstring(L, str, len);
	} 
	
	return 1;
}

// remove whitespace
// @param 1  str
static int trim(lua_State* L) {
	char *start;
	const char* str = luaL_checkstring(L, 1);
	lua_settop(L, 1);

	// trailing whitespace
	int len = strlen (str);
	while (len--) {
		if (!isspace (str[len]))
			break;
	}
	// leading whitespace
	for (start = (char *)str; *start && isspace (*start); start++) 
		;
	int newlen = strlen(str);

	if (start != str || newlen != len) {
		lua_pop(L, 1);
		lua_pushlstring(L, start, len - (start - str) + 1);
	} 
	
	return 1;
}


#define MAXINT  100000

// split string to table string list
// @param 1  str
static int split(lua_State* L) {
	unsigned int n = 1;
	const char *remainder;
	char *s;

	const char* str = luaL_checkstring(L, 1);
	const char* delimiter = luaL_checkstring(L, 2);
	int max_tokens = luaL_optint(L, 3, MAXINT);
	lua_settop(L, 2);	

	// create a new table at stack top
	lua_newtable(L);

	remainder = str;
	s = strstr (remainder, delimiter);
	if (s) {
		int delimiter_len = strlen (delimiter);

		while (--max_tokens && s) {
			int len;

			len = s - remainder;
			// add to lua string
			lua_pushlstring(L, remainder, len);
			// add to return table
			lua_rawseti(L, -2, n);
			n++;
			remainder = s + delimiter_len;
			s = strstr (remainder, delimiter);
		}
	}
	
	if (*str) {
		// add to lua string
		lua_pushstring(L, remainder);
		// add to return table
		lua_rawseti(L, -2, n);
    }

	// return table value
	return 1;
}


static int splitset(lua_State* L) {
	int delim_table[256];
	int n_tokens;
	const char *s;
	const char *current;
	char *token;

	const char* str = luaL_checkstring(L, 1);
	const char* delimiters = luaL_checkstring(L, 2);
	int max_tokens = luaL_optint(L, 3, MAXINT);
	lua_settop(L, 2);	
	lua_pop(L, 2);

	// create a new table at stack top
	lua_newtable(L);
	
	if (*str == '\0') {
		//lua_pop(L, 2);
		lua_pushstring(L, "");
		lua_rawseti(L, -2, 1);
		// return immediately
		return 1;
	}

	memset (delim_table, FALSE, sizeof (delim_table));
	for (s = delimiters; *s != '\0'; ++s)
		delim_table[*(char *)s] = TRUE;

	n_tokens = 1;

	s = current = str;
	while (*s != '\0') {
		if (delim_table[*(char *)s] && n_tokens + 1 <= max_tokens) {
			lua_pushlstring(L, current, s - current);
			// add to return table
			lua_rawseti(L, -2, n_tokens);
			++n_tokens;

			current = s + 1;
		}
		++s;
	}

	lua_pushlstring(L, current, s - current);
	// add to return table
	lua_rawseti(L, -2, n_tokens);

	return 1;
}


const char tagset[TAGNUM][PAIR][3] = {
	{ "{{", "}}" },
	{ "{_", "_}" },
	{ "{%", "%}" },
	{ "{(", ")}" },
	{ "{^", "^}" },
	{ "{<", ">}" },
	{ "{-", "-}" },
	{ "{*", "*}" },	
};


static int get_next_char (const char* str, int* ip, char* ncp) {
	if (!str) {
		printf("Error, the string is missing.\n");
		exit(1);
	} 
	
	if (str[*ip] != '\0') {
		//printf("-- %d %d", *ip, *ncp);
		*ip += 1;
		*ncp = str[*ip];
		//printf("== %d %d", *ip, *ncp);
		return TRUE;
	}
	else 
		return FALSE;
}

static int has_key (const char *str, int *which) {
	int i;
	for (i=0; i<TAGNUM; i++) {
		if (!strcmp(str, tagset[i][0])) {
			*which = i;
			return TRUE;
		}
	}
	
	return FALSE;
	
}
	
static int l_matchtagset(lua_State *L) {
	
	char nc, rc;
	char tagbuf[3] = "", rtagbuf[3] = "";
	int which = 0;
	
	size_t ls;
	const char *str_passin = lua_tolstring(L, lua_upvalueindex(1), &ls);
	int i = lua_tointeger(L, lua_upvalueindex(2));
	if (i < 0) i = 0;
	lua_Number point = i;
	
	nc = str_passin[i];
	do {
		lua_Number start, end;
		
		if (nc == '{') {
			// store the start position
			start = i;
			get_next_char (str_passin, &i, &nc);
			 
			sprintf(tagbuf, "{%c\0", nc);
			if (has_key(tagbuf, &which)) {
				strcpy(rtagbuf, tagset[which][1]);
				
				get_next_char (str_passin, &i, &nc);
				while ( nc != 0) {
					if (nc == rtagbuf[0]) {
						get_next_char (str_passin, &i, &rc);
						if (rc == rtagbuf[1]) {
							// store the end position
							end = i;
							lua_pushlstring(L, str_passin + (int)point, start-point);
							lua_pushlstring(L, str_passin + (int)start, end-start+1);
							lua_pushinteger(L, end + 1);				
							lua_pushvalue(L, -1); //copy the end point value
							lua_replace(L, lua_upvalueindex(2)); // move the end point value to searching start point
							//break;
							return 3;
						}
					}
					get_next_char (str_passin, &i, &nc);
				}
			}
		}
	} while (get_next_char (str_passin, &i, &nc));

	
	return 0;
}

static int matchtagset(lua_State *L) {
	luaL_checkstring(L, 1);
	lua_settop(L, 1);
	lua_pushinteger(L, 0); // searching start point
	lua_pushcclosure(L, l_matchtagset, 2);
  
	return 1;
}





static const struct luaL_Reg lgstringlib [] = {
	{"matchtagset", matchtagset},
	{"startsWith", startsWith},
	{"endsWith", endsWith},
//	{"bsub", bsub},	
	{"rfind", rfind},
	{"ltrim", ltrim},
	{"rtrim", rtrim},
	{"trim", trim},
	{"split", split},
	{"splitset", splitset},
	
	{NULL, NULL}
};

int luaopen_lgstring (lua_State *L) {
	luaL_register(L, "lgstring", lgstringlib);
	return 1;

}

/*int main() {*/
/*	lua_State *L;*/

/*  L = luaL_newstate();*/
/*  luaL_openlibs(L);*/
/*	l_gomatch(L);*/
/*	return 0;*/
/*}*/

