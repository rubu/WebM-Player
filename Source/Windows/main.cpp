#include "PlayerWindow.h"
#include "../Exception.h"

#include <cstdlib>
#include <stdexcept>
#include <Windows.h>


int WINAPI WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in_opt LPSTR lpCmdLine, __in int nShowCmd)
{
	try
	{
		PlayerWindow player_window(hInstance);
		MSG Mesage{};
		while (GetMessage(&Mesage, NULL, 0, 0) > 0)
		{
			DispatchMessage(&Mesage);
		}
	}
	catch (const ExceptionBase& exception)
	{
		return EXIT_FAILURE;
	}
	catch (const std::exception& exception)
	{
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}