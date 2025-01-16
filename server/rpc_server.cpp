∩╗┐#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "cli1.h"

#pragma  comment(lib,"rpcrt4.lib")
#pragma comment(lib, "advapi32.lib")

handle_t handler = 0;

int deleteFile(
	/* [in] */ handle_t IDL_handle,
	/* [in] */ const unsigned char* path)
{
	printf("deleting %s\n", path);
	if (remove((const char*)path) == -1)
	{
		switch (errno)
		{
		case EACCES:
			printf("remove error: file only for reading\n");
			break;
		case ENOENT:
			printf("remove error: incorrect path\n");
			break;
		}
		return errno;
	}
	printf("File successfully deleted!\n");
	return 0;
}

int copyFile(
	/* [in] */ handle_t IDL_handle,
	/* [string][in] */ const unsigned char* path,
	/* [size_is][out][in] */ unsigned char* data,
	/* [in] */ int size,
	/* [in] */ int offset,
	/* [in] */ int mode)
{
	FILE* inputFile = NULL;
	if (mode == 0)
		inputFile = fopen((const char*)path, "wb+");
	else
	{
		inputFile = fopen((const char*)path, "ab+");
		while (inputFile == NULL)
		{
			inputFile = fopen((const char*)path, "ab+");
			Sleep(100);
		}

	}
	if (inputFile == NULL)
	{
		if (errno == EACCES)
			printf("Access denied\n");
		else if (errno == ENOENT)
			printf("Incorrect path\n");
		else
			printf("Unknown error\n");
		return errno;
	}
	//fseek(inputFile, offset, SEEK_SET);
	int tmp = fwrite(data, 1, size, inputFile);
	Sleep(5);
	fclose(inputFile);

	printf("%d bytes of %s fully copied!\n", size + offset, path);
	return 0;
}

int downloadFile(
	/* [in] */ handle_t IDL_handle,
	/* [string][in] */ const unsigned char* path,
	/* [out] */ unsigned char data[65536],
	/* [out] */ int* size,
	/* [in] */ int offset)
{
	FILE* outputFile = NULL;
	if ((outputFile = fopen((const char*)path, "rb")) == NULL)
	{
		if (errno == EACCES)
			printf("Access denied\n");
		else if (errno == ENOENT)
			printf("Incorrect path\n");
		else
			printf("Unknown error\n");
		return errno;
	}
	int read = 0, tmp = 0;
	fseek(outputFile, offset, SEEK_SET);
	tmp = fread(data, 1, 65536, outputFile);
	Sleep(5);
	read += tmp;
	fclose(outputFile);
	*size = read;
	printf("%d bytes of %s fully downloaded!\n", read, path);
	return 0;
}

void Shutdown(
	/* [in] */ handle_t IDL_handle)
{
	RPC_STATUS ret;

	ret = RpcMgmtStopServerListening(NULL);

	if (ret)
		exit(ret);

	ret = RpcServerUnregisterIf(NULL, NULL, FALSE);

	if (ret)
		exit(ret);

	printf("Session successfully ended\n");
}

int authentication(
	/* [in] */ handle_t IDL_handle,
	/* [string][in] */ const unsigned char* login,
	/* [string][in] */ const unsigned char* password)
{
	//∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜ ∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜ ∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜ ∩┐╜∩┐╜ ∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜ ∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜
	if (LogonUserA((LPCSTR)login, NULL, (LPCSTR)password, LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &IDL_handle) == 0)
	{
		printf("Error LogonUserA: %d(user %s)\n", GetLastError(), login);
		return 1;
	}
	else
	{
		//∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜ ∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜ ∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜ ∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜ ∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜ ∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜ ∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜ ∩┐╜ ∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜ ∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜
		if (ImpersonateLoggedOnUser(IDL_handle) == 0)
			return 2;
		else
		{
			printf("%s connected\n", login);
			return 0;
		}
	}
}

void logout(
	/* [in] */ handle_t IDL_handle)
{
	int ret = RevertToSelf();
	if (!ret)
		printf("%d\n", GetLastError());
}
int printError(const char* func, int code)
{
	printf("%s - Error code: %d\n", func, code);
	exit(code);
}

int main()
{
	RPC_STATUS ret;

	ret = RpcServerUseProtseqEpA(
		(RPC_CSTR)("ncacn_ip_tcp"), //∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜ TCP/IP ∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜
		RPC_C_PROTSEQ_MAX_REQS_DEFAULT, //∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜ ∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜ TCP/IP
		(RPC_CSTR)("9000"), //∩┐╜∩┐╜∩┐╜∩┐╜
		NULL); //NULL ∩┐╜∩┐╜∩┐╜ TCP/IP
	if (ret)
		printError("RpcServerUseProtseqEpA", ret);

	ret = RpcServerRegisterIfEx(InterfaceRPC_v1_0_s_ifspec, NULL, NULL, RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH, RPC_C_LISTEN_MAX_CALLS_DEFAULT, NULL);
	if (ret)
		printError("RpcServerRegisterIfEx", ret);

	ret = RpcServerListen(
		1,//∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜ ∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜ ∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜ ∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜
		RPC_C_LISTEN_MAX_CALLS_DEFAULT,//∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜ ∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜ ∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜ ∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜
		0);//∩┐╜∩┐╜∩┐╜ ∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜∩┐╜
	if (ret)
		printError("RpcServerListen", ret);

	return 0;
}

void __RPC_FAR* __RPC_API midl_user_allocate(size_t len)
{
	return(malloc(len));
}

void __RPC_API midl_user_free(void __RPC_FAR* ptr)
{
	free(ptr);
}