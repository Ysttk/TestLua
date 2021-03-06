// TestLua.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <ctype.h>

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
	double* dest = (double*)lua_touserdata(l, lua_upvalueindex(1));
	double delta = lua_tonumber(l, -1);
	(*dest) += delta;
	return 0;
}

int __CallSub(lua_State* l)
{

	return 1;
}

// 通用lua调用C函数机制声明//////////
/////////////////////////////////////
#include <stdlib.h>

typedef void (*FuncType) (); //所有调用的函数指针归一成为普通指针
typedef int (*StubFuncType) (lua_State*);	//lua调用C函数同一入口

struct CFuncDescription {	//所有C函数的描述结构体
	char* argDesc;
	FuncType cFunc;
	int argSizeInByte, retSizeInByte;
};


/*
参数和返回值类型描述定义：
b -- boolean
s -- int16
i -- int32
I -- int64
u -- unsigned int32
U -- unsigned int64
S -- string
c -- char
f -- float
d -- double
*/

int TypeSize[256];
bool InitTypeSize()
{
	memset(TypeSize, 0, sizeof(TypeSize));
	TypeSize['b'] = 1;
	TypeSize['s'] = 2;
	TypeSize['i'] = 4;
	TypeSize['I'] = 8;
	TypeSize['u'] = 4;
	TypeSize['U'] = 8;
	TypeSize['S'] = sizeof(void*);
	TypeSize['f'] = 4;
	TypeSize['d'] = 8;
	return true;
}
int initialized = InitTypeSize();


std::vector<CFuncDescription*> AllCFuncDescVec;


int CalculateTypeListSize(char* typeList)
{
	int size = 0;
	int len = strlen(typeList);
	for (int i=0; i<len; i++)
		if (TypeSize[typeList[i]]==0)
			assert(false);
		else
			size += TypeSize[typeList[i]];
	return size;
}


void _RegistLuaCFunction(lua_State* l, StubFuncType stubFunc, FuncType cFunc, const char* funcNameInLua, const char* argDesc)
{
	CFuncDescription* funcDesc = new CFuncDescription();
	funcDesc->argDesc = new char[strlen(argDesc)+1];
	strcpy(funcDesc->argDesc, argDesc);
	int len = strlen(argDesc);
	int i;
	for (i=0; i<len; i++)
		if (argDesc[i]==':') break;
	char argTypes[20], retTypes[20];
	if (i>0) {
		strncpy(retTypes, argDesc, i);
		retTypes[i]=0;
		funcDesc->retSizeInByte = CalculateTypeListSize(retTypes);
	}
	if (i<len-1) {
		strncpy(argTypes, &argDesc[i+1], len-i);
		argTypes[len-i]=0;
		funcDesc->argSizeInByte = CalculateTypeListSize(argTypes);
	}
	funcDesc->cFunc = cFunc;
	AllCFuncDescVec.push_back(funcDesc);
	
	lua_pushlightuserdata(l, (void*)funcDesc);
	lua_pushcclosure(l, stubFunc, 1);
	lua_setglobal(l, funcNameInLua);
}

#define RegistLuaCFunction(l, cFunc, argDesc) _RegistLuaCFunction(l, CFuncStubForAllCFunInLua, (FuncType)cFunc, #cFunc, argDesc)

 

void* __InsertArgByArgType(void* stackPoint, lua_State* l, char argType)
{
	switch (argType) {
		case 'b': {
			bool v = lua_toboolean(l, -1);
			*((bool*)stackPoint) = v;
			stackPoint = ((char*)stackPoint)+1;
				  }
			break;	  
		case 's': {
			INT16 v = (INT16)lua_tointeger(l, -1);
			*(((INT16*)stackPoint)) = v;
			stackPoint = ((INT16*)stackPoint)+1;
				  }
			break;
		case 'i': {
			INT32 v = (INT32)lua_tointeger(l, -1);
			*(((INT32*)stackPoint)) = v;
			stackPoint = ((INT32*)stackPoint)+1;
				  }
			break;
		case 'I': {
			INT64 v = (INT64)lua_tointeger(l, -1);
			*(((INT64*)stackPoint)) = v;
			stackPoint = ((INT64*)stackPoint)+1;
				  }
			break;
		case 'u': {
			UINT32 v = (UINT32)lua_tointeger(l, -1);
			*(((UINT32*)stackPoint)) = v;
			stackPoint = ((UINT32*)stackPoint)+1;
				  }
			break;
		case 'U': {
			UINT64 v = (UINT64)lua_tointeger(l, -1);
			*(((UINT64*)stackPoint)) = v;
			stackPoint = ((UINT64*)stackPoint)+1;
				  }
			break;
		case 'S': {
			const char* v = lua_tostring(l, -1);
			char* vv = new char[strlen(v)+1];	//这里需要回收操作！！本例子中没有处理
			strcpy(vv, v);
			*(((char**)stackPoint)) = vv;
			stackPoint = ((char**)stackPoint)+1;
				  }
			break;
		case 'c': {
			const char* v = lua_tostring(l, -1);
			*(((char*)stackPoint)) = *v;
			stackPoint = ((char*)stackPoint)+1;
				  }
			break;
		case 'f': {
			float v = (float)lua_tonumber(l, -1);
			*((float*)stackPoint) = v;
			stackPoint = ((float*)stackPoint)+1;
				  }
			break;
		case 'd': {
			double v = (double)lua_tonumber(l, -1);
			*((double*)stackPoint) = v;
			stackPoint = ((double*)stackPoint)+1;
				  }
			break;
	}
	return stackPoint;
}

void __PushRetByArgType(lua_State* l, INT64 genRetVal, char retType)
{
	switch (retType) {
		case 'b': {
			int retVal = (int)genRetVal;
			lua_pushboolean(l, retVal);
				  }
			break;
		case 's':
		case 'i':
		case 'I':
		case 'u':
		case 'U': {
			lua_Number retVal = (lua_Number)genRetVal;
			lua_pushnumber(l, retVal);
				  }
			break;
		case 'c': {
			char retVal[2];
			retVal[0] = (char)genRetVal;
			retVal[1] = 0;
			lua_pushstring(l, retVal);
				  }
			break;
		case 'S': {
			char* retVal = (char*)genRetVal;
			lua_pushstring(l, retVal);
				  }
			break;
		case 'f': {
			float* tv = (float*)&genRetVal;
			float v = *(tv+1);
			lua_pushnumber(l, (lua_Number)v);
				  }
			break;
		case 'd': {
			double v = *((double*)&genRetVal);
			lua_pushnumber(l, (lua_Number)v);
				  }
			break;
	}
}

int CFuncStubForAllCFunInLua(lua_State* l)
{
	CFuncDescription* funcDesc = (CFuncDescription*)lua_touserdata(l, lua_upvalueindex(1));
	char tmp[20];
	int splitIdx = 0;
	for (splitIdx=0; splitIdx<strlen(funcDesc->argDesc); splitIdx++)
		if (funcDesc->argDesc[splitIdx] == ':') break;
	if (splitIdx<strlen(funcDesc->argDesc)) {
		//void* argSpace = alloca(funcDesc->argSizeInByte);
		//alloca函数不会老老实实按照传入的大小扩展堆栈，因此，这里只能手写来扩展堆栈
		void* argSpace;
#ifdef WIN
		void* tmpStore1, *tmpStore2;
		int argSize = funcDesc->argSizeInByte;
		__asm {
			mov tmpStore1, eax;
			mov eax, argSize;
			sub esp, eax;
			mov argSpace, esp;
			mov eax, tmpStore1;
		}
#else
#error "Not Implemented Platform"
#endif
		char tmpBuf[512];
		void* currPoint = argSpace;
		strncpy(tmp, &funcDesc->argDesc[splitIdx+1], strlen(funcDesc->argDesc)-splitIdx-1);
		tmp[strlen(funcDesc->argDesc)-splitIdx-1] = 0;
		int argLen = strlen(funcDesc->argDesc)-splitIdx-1;
		currPoint = (char*)currPoint + funcDesc->argSizeInByte;
		for (int i=0; i<argLen; i++) {
			currPoint = (char*)currPoint - TypeSize[tmp[argLen-i-1]];
			__InsertArgByArgType(currPoint, l, tmp[argLen-i-1]);
			lua_pop(l, 1);
		}
	}

	funcDesc->cFunc();	//调用那个函数

	void* retVal;
#ifdef WIN
	__asm {
		mov retVal, eax;
	}
#endif
	int retNum = 0;
	if (splitIdx > 0) {
		strncpy(tmp, funcDesc->argDesc, splitIdx);
		tmp[splitIdx]=0;
		retNum =1;
		int retLen = splitIdx;
		__PushRetByArgType(l, (INT64)retVal, tmp[0]); //这里只处理了一个返回值的情况，多个返回值可以用vector包装返回处理一下
	}

	if (funcDesc->argSizeInByte >0 ) {
		//平堆栈
#ifdef WIN
		int argSize = funcDesc->argSizeInByte;
		void* tmpStore;
		__asm {
			mov tmpStore, eax;
			mov eax, argSize;
			add esp, eax;
			mov eax, tmpStore;
		}
#endif
	}

	return retNum;
}


//C++类变量、函数



class A {
	public:
		void SayHello() { printf("Hello World from class member function A(%x).SayHello()\n", this); }
};


void _RegistLuaCClass(lua_State*l, char* className)
{
	lua_newtable(l);
	int tblIdx = lua_gettop(l);
	lua_setglobal(l, className);
}

#define RegistLuaCClass(l,class) _RegistLuaCClass(l, #class)



void _RegistLuaCClassFunction(lua_State* l,char* className,FuncType cFunc, char* funcNameInLua, char* argDescrip) 
{

}

#define RegistLuaCClassFunction(l, className, func, argDesc)  _RegistLuaCClassFunction(l, #className, (FuncType)func, #func, argDesc)


#ifdef WIN
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char* argv[])
#endif
{
	lua_State* l;
	int re;
	l = luaL_newstate();
	luaL_openlibs(l);
	//luaL_dostring(l, "print ([[hello world!]])");

	//C、C++调用Lua函数
	//re = luaL_loadstring(l, "function Pfunc() print([[hello world]]) end");
	//re = luaL_dostring(l, "function P() print([[hello world]]) end");
	//lua_getglobal(l, "P");
	//if (lua_type(l,-1)!=LUA_TFUNCTION)
	//	printf("%s\n", lua_typename(l, lua_type(l, -1)));
	//else
	//	re = lua_pcall(l, 0, 0, 0);

	//lua调用C函数、变量
	//lua_pushcfunction(l, CallHelloWorld);
	//lua_setglobal(l, "HelloWorld");
	//luaL_dostring(l, "print(HelloWorld)");
	//re = luaL_dostring(l, "print(HelloWorld(2.5,[[abc]]))");
	
	//lua调用C函数、变量；C函数使用通用Stub函数注册
	RegistLuaCFunction(l, HelloWorld, "i:dS");
	luaL_dostring(l, "print(HelloWorld(2.5, [[abc]]))");

	//以下设置C变量的回调失败，原因不明
	//double start = 1.0;
	//printf("start value before lua:%lf\n", start);
	//lua_pushnumber(l, start);
	//lua_setglobal(l, "start");
	//lua_newtable(l);
	//lua_setglobal(l, "DoubleMT");
	//lua_getglobal(l, "DoubleMT");
	//int doubleMtIdx = lua_gettop(l);
	//lua_pushlightuserdata(l, &start);
	//lua_pushcclosure(l, __CallAdd, 1);
	//int v1 = lua_gettop(l);
	//lua_setfield(l, doubleMtIdx, "__cAdd");
	//luaL_dostring(l, "function DoubleMT.__add(op1, op2) print([[add called]]) DoubleMT.__cAdd(op2) return op1+op2 end function DoubleMT.__index(tbl, k) print([[index called]]) end");
	////lua_getglobal(l, "start");
	////lua_getglobal(l, "DoubleMT");
	////lua_setmetatable(l, -2);
	//luaL_dostring(l, "print(debug.setmetatable(start, DoubleMT)) print (debug.getmetatable(start).__index) ");
	//re = luaL_dostring(l, "start=start+1 print([[start value]], start)");
	//printf("start value after lua:%lf\n", start);




	//lua调用C++类及其成员函数和变量

	lua_close(l);
	return 0;
}

