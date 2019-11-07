
#include "network_handle.h"
#include <iostream>
#include <string>

using namespace std;

int main(void)
{
    string mess = "Hello World";
    SimplePacket<string> strPacket(mess);
    std::cout << *(string *)strPacket.get_val_ptr() << endl;

    // std::cout << testINT() << endl;

    return 0;
}
