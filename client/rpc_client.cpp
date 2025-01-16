#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <conio.h>
#include <cctype>
#include <string>
#include "cli1.h"
#pragma comment(lib, "rpcrt4.lib")
#pragma comment(lib, "advapi32.lib")

const size_t blockSize = 1048576;

unsigned char* szStringBinding = NULL;

void connect(const std::string& IP, const std::string& port = "9000") {
	RPC_STATUS status = RpcStringBindingComposeA(
		NULL,
		(RPC_CSTR)("ncacn_ip_tcp"),
		(RPC_CSTR)(IP.c_str()),
		(RPC_CSTR)(port.c_str()),
		NULL,
		&szStringBinding);

	if (status)
		exit(status);

	status = RpcBindingFromStringBindingA(szStringBinding, &hSolutionBinding);

	if (status)
		exit(status);
}

int login(void) {
	std::string username;
	std::string password;

	while (true) {
		std::cout << "Enter username" << std::endl;
		std::getline(std::cin, username);
		std::cout << "Enter password" << std::endl;
		std::getline(std::cin, password);

		if (LoginUserRequest((unsigned char*)username.c_str(), (unsigned char*)password.c_str()))
			std::cout << "Connection failed! Please try again." << std::endl << std::endl;
		else
			break;
	}
	std::cout << "Connection succeed!" << std::endl;
	return 0;
}

void serverToHostCopy(const std::string& filename) {
	unsigned char* out = new unsigned char[blockSize];
	memset(out, 0, blockSize);
	unsigned size = ServerToHostRequest((const unsigned char*)filename.c_str(), out, blockSize);

	if (size != 0) {
		FILE* fout = fopen(filename.c_str(), "wb");
		fwrite(out, sizeof(char), size, fout);

		do {
			memset(out, 0, blockSize);
			size = ServerToHostRequest((const unsigned char*)filename.c_str(), out, blockSize);
			if (size == 0) {
				break;
			}
			fwrite(out, sizeof(char), size, fout);

		} while (size == blockSize);

		fclose(fout);
	}
	else std::cout << "File not found or not available. " << std::endl;
	delete[] out;
}

void hostToServerCopy(const std::string& filename) {
	FILE* file = fopen((const char*)filename.c_str(), "rb");
	fseek(file, 0, SEEK_END);
	unsigned size = ftell(file);
	fseek(file, 0, SEEK_SET);

	unsigned int sent = 0;
	unsigned counter = 0;

	while (true) {
		if (size - sent >= blockSize) {
			unsigned char* in = new unsigned char[blockSize + 4];
			fseek(file, counter, SEEK_SET);
			fread(in, sizeof(char), blockSize, file);
			if (HostToServerRequest((const unsigned char*)filename.c_str(), (unsigned char*)in, blockSize, counter) != 0) {
				std::cout << "This user does not have rights to write files to the server." << std::endl;
				return;
			}
			delete[] in;
			sent += blockSize;
			counter += blockSize;
		}
		else {
			unsigned char* in = new unsigned char[blockSize];
			fread(in, sizeof(char), size - sent, file);
			fclose(file);
			HostToServerRequest((const unsigned char*)filename.c_str(), (unsigned char*)in, size - sent, counter);
			delete[] in;
			break;
		}
	}

}

void AcceptableCommands(void) {
	std::cout << "Available commands: " << std::endl;
	std::cout << "    1. Delete file on server" << std::endl;
	std::cout << "    2. Copy file from server to the host" << std::endl;
	std::cout << "    3. Copy file from host to server" << std::endl;
	std::cout << "    4. Exit" << std::endl;
}

const std::string getFilename(void) {
	static std::string filename;
	std::cout << "Enter file name" << std::endl;
	std::getline(std::cin, filename);
	return filename;
}

void callCommand(void) {
	static std::string filename;
	static std::string str;
	std::getline(std::cin, str);

	try {
		unsigned number = std::stoi(str);
		switch (number % 4) {
		case 0:
			LogoutUserRequest();
			exit(1);
			break;
		case 1: // Remove file
			RemoveFileRequest((const unsigned char*)getFilename().c_str());
			break;
		case 2: // Copy server -> host
			serverToHostCopy(getFilename());
			break;
		case 3: // Copy host -> server
			hostToServerCopy(getFilename());
			break;
		default:
			break;
		}
	}
	catch (std::invalid_argument& except) {
		std::cout << "Please enter a propper value. ";
	}
}

int main() {
	RPC_STATUS status;

	std::cout << "Enter IP" << std::endl;
	std::string ip = "127.0.0.1";
	std::getline(std::cin, ip);
	connect(ip);
	login();

	while (true) {
		AcceptableCommands();
		callCommand();
	}

	status = RpcStringFreeA(&szStringBinding);

	if (status)
		exit(status);

	status = RpcBindingFree(&hSolutionBinding);

	if (status)
		exit(status);
}

// Memory allocation function for RPC.
void* __RPC_USER midl_user_allocate(size_t size)
{
	return malloc(size);
}

// Memory deallocation function for RPC.
void __RPC_USER midl_user_free(void* p)
{
	free(p);
}