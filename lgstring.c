#include <float.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>


#define TRUE 1
#define FALSE 0

#define TAGNUM 6
#define PAIR 2

const char tagset[TAGNUM][PAIR][3] = {
	{ "{{", "}}" },
	{ "{%", "%}" },
	{ "{(", ")}" },
	{ "{-", "-}" },
	{ "{^", "^}" },
	{ "{<", ">}" }
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

