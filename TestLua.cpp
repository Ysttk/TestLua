// TestLua.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int HelloWorld(double a, const char* str) 
{
	printf("hello world from c:%lf, %s\n", a, str);
	return (int)a+1;
}

int CallHelloWorld(lua_State* l)
{
	if (lua_type(l,-1)!=LUA_TSTRING) 
		printf("1st param type:%s\n", lua_typename(l, lua_type(l, -1)));
	if (lua_type(l,-2)!=LUA_TNUMBER)
		printf("2nd param type:%s\n", lua_typename(l, lua_type(l,-2)));
	const char* str = lua_tolstring(l, -1, NULL);
	LUA_NUMBER n = lua_tonumber(l, -2);
	int re = HelloWorld(n, str);
	LUA_NUMBER lre = re;
	lua_pushnumber(l, lre);
	return 1;
}

template<typename T> T __Add(T *src, T delta)
{
	*src += delta;
	return *src;
}

template<typename T> T __Sub(T* src, T delta)
{
	*src -= delta;
	return *src;
}

int __CallAdd(lua_State* l)
{
	int* dest = (int*)lua_touserdata(l, lua_upvalueindex(1));
	double delta = lua_tonumber(l, -1);
	*dest += delta;
	return 0;
}

int __CallSub(lua_State* l)
{

	return 1;
}


//C++类变量、函数

typedef void* FuncType;

class A {
	public:
		void SayHello() { print("Hello World from class member function A.SayHello()\n"); }
};


void _RegistLuaCClass(lua_State*l, char* className)
{
	lua_newtable(l);
	int tblIdx = lua_gettop();
	lua_setfield(l, LUA_GLOBALSINDEX, className);
}

#define RegistLuaCClass(l,class) _RegistLuaCClass(l, #class)

int CFuncSubForAllCFunInLua(lua_State* l)
{

}

void _RegistLuaCClassFunction(lua_State* l,char* class,FuncType cFunc, char* funcNameInLua, char* argDescrip) 
{

}

#define RegistLuaCClassFunction(l, class, func, argDesc)  _RegistLuaCClassFunction(l, #class, (FuncType)cFunc, #cFunc, argDesc)



int _tmain(int argc, _TCHAR* argv[])
{
	lua_State* l;
	int re;
	l = luaL_newstate();
	luaL_openlibs(l);
	//luaL_dostring(l, "print ([[hello world!]])");

	//C、C++调用Lua函数
	//re = luaL_loadstring(l, "function Pfunc() print([[hello world]]) end");
	//re = luaL_dostring(l, "function P() print([[hello world]]) end");
	//lua_getfield(l, LUA_GLOBALSINDEX, "P");
	//if (lua_type(l,-1)!=LUA_TFUNCTION)
	//	printf("%s\n", lua_typename(l, lua_type(l, -1)));
	//else
	//	re = lua_pcall(l, 0, 0, 0);

	//lua调用C函数、变量
	//lua_pushcfunction(l, CallHelloWorld);
	//lua_setfield(l, LUA_GLOBALSINDEX, "HelloWorld");
	//re = luaL_dostring(l, "print(HelloWorld(2.5,[[abc]]))");

	//以下设置C变量的回调失败，原因不明
	//double start = 1.0;
	//printf("start value before lua:%lf\n", start);
	//lua_pushnumber(l, start);
	//lua_setfield(l, LUA_GLOBALSINDEX, "start");
	//lua_newtable(l);
	//lua_setfield(l, LUA_GLOBALSINDEX, "DoubleMT");
	//lua_getfield(l, LUA_GLOBALSINDEX, "DoubleMT");
	//int doubleMtIdx = lua_gettop(l);
	//lua_pushlightuserdata(l, &start);
	//lua_pushcclosure(l, __CallAdd, 1);
	//int v1 = lua_gettop(l);
	//lua_setfield(l, doubleMtIdx, "__cAdd");
	//luaL_dostring(l, "function DoubleMT.__add(op1, op2) print([[add called]]) DoubleMT.__cAdd(op2) return op1+op2 end function DoubleMT.__index(tbl, k) print([[index called]]) end");
	////lua_getfield(l, LUA_GLOBALSINDEX, "start");
	////lua_getfield(l, LUA_GLOBALSINDEX, "DoubleMT");
	////lua_setmetatable(l, -2);
	//luaL_dostring(l, "print(debug.setmetatable(start, DoubleMT)) print (debug.getmetatable(start).__index) ");
	//re = luaL_dostring(l, "start=start+1 print([[start value]], start)");
	//printf("start value after lua:%lf\n", start);




	//lua调用C++类及其成员函数和变量

	lua_close(l);
	return 0;
}

