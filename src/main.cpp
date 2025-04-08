// hp2p.cpp : Defines the entry point for the application.
//

#include "peer.h"
#include <iostream>

using namespace std;

int main()
{
	Peer p;
	if (!p.Start()) {
		cout << "Peer start failed." << endl;
		return -1;
	}

	std::cout << "Peer started.\nPress enter to exit...\n";
	std::cin.get();

	p.Stop();
	return 0;
}
